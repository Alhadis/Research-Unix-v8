#include "ec.h"
#if NEC > 0


#include "../h/param.h"
#include "../h/systm.h"
#include "../h/pte.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../h/ubareg.h"
#include "../h/ubavar.h"
#include "../h/conf.h"
#include "../h/ioctl.h"
#include "../h/ttyld.h"
#include "../h/stream.h"
#include "../h/ethernet.h"
#include "sparam.h"

extern	char ECmem[];			/* .set in locore.s */
#define	ECADDR	((caddr_t)(ECmem))	/* Virt addr of EC buffer mem */

/* NOTE: The ECRHBF/ECRLBF/ECTBF definitions must match the address
	and size of EC memory as defined in locore.s in the kernel.
	If locore maps in full 32K, then use all buffers, otherwise,
	change accordingly.
*/

#define	ENETDOWN	ENODEV
#define	ECRHBF	11
#define	ECRLBF	1
#define	ECTBF	0

/* Following ethernet address only has meaning if UE prom is missing */

#define ECPRI   (PZERO+8)
#define	ECBFSZ	2048		/* Size of each EC buffer */

struct	ecdevice	{
	short	ecrcr;		/* Transmit Control Register */
	short	ecxcr;		/* Receive Control Register */
};

/* Bits in ecxcr: */
#define	EC_WBN		0x0010		/* Write Buffer Number */
#define	EC_CLRDN	0x0020		/* Clear Done */
#define	EC_IE		0x0040		/* Interrupt Enable */
#define	EC_DONE		0x0080		/* Xmt/Rcv complete */
#define	EC_CLR		0x0100		/* Clear EC */
#define	EC_CLRJAM	0x2000		/* Clear Jam */
#define	EC_JIE		0x4000		/* Jam Interrupt Enable */
#define	EC_JAM		0x8000		/* Jam */

/* Bits WBN, CLRDN, IE, and DONE are also used for ecrcr */
#define	EC_BITS		"\20\20JAM\17JIE\16CLRJAM\11CLR\10DONE\7IE\6CLRDN\5WBN"

/* Bits in ecrcr: */
#define	EC_RANDATA	0xA00	/* Alternating 1/0 in 4 data bits */
#define	EC_ADATAMSK	0xF00	/* Mask for 4 data bits */
#define	EC_OWN_BDCST	0x600	/* Recog addrs for me + broadcast*/
#define	EC_ARALL	0	/* Pass all addresses */
#define	EC_AWCLK	0x1000	/* Clock data into address recog ram */
#define	EC_AROM		0x2000	/* Select rom vs default ram */
#define	EC_ASTEP	0x4000	/* Step rom/ram address one bit */


/*
 * Useful combinations
 */
#define	EC_READ		(EC_OWN_BDCST|EC_IE|EC_WBN)
#define	EC_WRITE	(EC_JIE|EC_IE|EC_WBN)
#define	EC_CLEAR	(EC_JIE|EC_IE|EC_CLRJAM)

#define	ECXBUSY	010
#define	ECDEBUG	0200

/* corresponds to minor device numbers (# of servers) */
#define	CHANS_PER_UNIT	8

struct	ec {			/* per controller */
	char	myethadr[6];	/* My Ethernet address */
	char	attached;	/* Non zero when ec has been attached */
	char	active;
	short	collisions;		/* Jam counter */
	int	state;		/* Device state */
	int	bits;
	int	ierrors, oerrors;
	int	ipackets, opackets;
} ec[NEC] = {	{0x02, 0x60, 0x8c, 0x00, 0x4, 0x62},	};

#define	NECCHAN	(CHANS_PER_UNIT * NEC)
struct	ecchan {
	int	unit;
	int	packets;
	struct	queue *ecq;
	int	type;
} ecchan[NECCHAN];

int	nodev(), ecopen(), ecclose(), ecput(), ecsrv();
static struct qinit ecrinit = { nodev, NULL, ecopen, ecclose, 0, 0 };
static struct qinit ecwinit = { ecput, NULL, ecopen, ecclose, 1514, 0 };
	/* 1514 bytes is minimum highwater mark to send 1514 byte packets */
struct streamtab ecsinfo = { &ecrinit, &ecwinit };

/* JUNK FOR AUTOCONFIGURE */
struct uba_device *ecinfo[1];
int ecprobe(), ecattach();
u_short ecstd[] = { 0 };
struct uba_driver ecdriver =
	{ ecprobe, 0, ecattach, 0, ecstd, "ec", ecinfo };
extern bsize[];

ecprobe(reg)
caddr_t reg;
{
	register int br, cvec;		/* value-result */
	br = 0x16;			/* BR Level 16 = UNIBUS 6 */
	cvec = 0440;			/* Int vector */
	return 1;
}

/* ARGSUSED */
ecattach(ui)
struct uba_device *ui;
{
	register struct ecdevice *ecaddr = (struct ecdevice *)ui->ui_addr;
	register struct ec *ecu = &ec[ui->ui_unit];
	unsigned short s0, s1;
	register i, j;
	register char *cp;
	char bdethadr[6];		/* copy of bd rom; -1=>chip missing */


	if(ecu->attached)			/* Do this only once */
		return;
	ecaddr->ecxcr = EC_CLR;		/* Reset the EC */

/* Fetch out the ROM ethernet address into tmp ethadr */

	ecaddr->ecrcr = EC_AROM;		/* Select the ROM */
	cp = bdethadr;
	for(i = 0; i < 6; i++) {
		*cp = 0;
		for(j = 0; j <= 4; j += 4) {
			s1 = ecaddr->ecrcr;
			*cp |= ((s1 >> 8) & 017) << j;
			for(s1 = 0; s1 < 4; s1++) {
				ecaddr->ecrcr = EC_ASTEP+EC_AROM;
				ecaddr->ecrcr = EC_AROM;
			}
		}
		cp++;
	}
	for(i = 0, cp = bdethadr; i < 6; i++)
		if(*cp++ != -1) {	/* Rom is good if not -1 */
			for(i = 0; i < 6; i++)
				ecu->myethadr[i] = bdethadr[i];
		}
/* use the RAM for addr recog */
	laddrec(ui);

	for(i = ECRHBF; i >= ECRLBF; i--)
		ecaddr->ecrcr = EC_READ + i;
	ecsrand((long)ecu->myethadr[5]);
	ecu->attached = 1;
	ecu->bits = EC_READ;
}

laddrec(ui)
struct uba_device *ui;
{
	register struct ecdevice *ecaddr = (struct ecdevice *)ui->ui_addr;
	register struct ec *ecu = &ec[ui->ui_unit];
	register	i, j, s0, s1;
	register char	*cp;

/* Have to load myethadr into hardware ram! */
	ecaddr->ecxcr = EC_CLR;	/* Reset bit ptr */
	for(i = 0, cp = ecu->myethadr; i < 6; i++, cp++)
		for(j = 0; j <= 4; j += 4) {
			s1 = ((*cp >> j) & 017) << 8;
			ecaddr->ecrcr = s1;
			ecaddr->ecrcr = s1 | EC_AWCLK;
			ecaddr->ecrcr = s1;
			for(s0 = 0; s0 < 4; s0++) {
				ecaddr->ecrcr = EC_ASTEP;
				ecaddr->ecrcr = 0;
			}
		}
}


/* ARGSUSED */
ecopen(q, dev)
register struct queue *q;
register dev_t dev;
{
	register struct ecchan *ecd;
	register struct ec *ecu;
	register unit;

	dev = minor(dev);
	unit = dev / CHANS_PER_UNIT;

	if (dev >= NECCHAN)
		return(0);
	if (unit >= NEC)
		return(0);

	ecu = &ec[unit];
	if(!ecu->attached)
		return(0);
	ecd = &ecchan[dev];
	if (ecd->ecq)
		return(0);

	ecd->unit = unit;
	ecd->ecq = q;
	q->ptr = (caddr_t)ecd;
	q->flag |= QBIGB | QDELIM;
	WR(q)->ptr = (caddr_t)ecd;
	WR(q)->flag |= QBIGB|QDELIM;

	if (ecu->state & ECDEBUG)
		printf("ECo%d, ", minor(dev));
	return(1);
}
ecclose(q)
register struct queue *q;
{
	register struct ecchan *ecd;
	register struct ec *ecu;
	register unit;

	ecd = (struct ecchan *)q->ptr;
	ecu = &ec[ecd->unit];
	if (ecu->state & ECDEBUG)
		printf("ECc\n");
	flushq(WR(q), 1);
	ecd->ecq = NULL;
}

ecput(q, bp)
register struct queue *q;
register struct block *bp;
{
	register struct ecchan *ecd;
	register struct ec *ecu;
	register unit, type, count, ps;

	ecd = (struct ecchan *)q->ptr;
	unit = ecd->unit;
	ecu = &ec[unit];

	switch(bp->type) {
	
		case M_IOCTL:
			ecioctl(q, bp);
			return;

		case M_DATA:
			putq(q, bp);
			return;
		
		case M_DELIM:
			break;

		default:
			freeb(bp);
			return;
	}

	putq(q, bp);
	ps = spl6();
	ecd->packets++;
	if (ecu->active == 0)
		ecstart(unit);
	splx(ps);
}
ecstart(unit)
{
	register u_char	*tsp, *fsp;
	register struct ec *ecu;
	register struct ecchan *ecd;
	register struct ecdevice *ecaddr = (struct ecdevice *)ecinfo[unit]->ui_addr;
	register struct queue *q, *bq;
	register struct block *bp, *head, *nbp, **bnext;
	register len, count = 0, i;

	ecu = &ec[unit];
	if (ecu->active) {
		printf(" start but active %d\n", unit);
		ecu->active = 0;
	}
	ecd = &ecchan[unit * CHANS_PER_UNIT];
	for(i = 0; i < CHANS_PER_UNIT; i++, ecd++)
		if (ecd->ecq && ecd->packets > 0)
			break;
	if (i >= CHANS_PER_UNIT)
		return;

	ecd->packets--;
	q = WR(ecd->ecq);

	/* The packet must be justified to the end of the buffer.
	 * Therefore, we have to count the bytes before copying.
	 */
	bnext = &head;
	while (*bnext = bp = getq(q)) {
		if (bp->type == M_DELIM) {
			bp->next = 0;
			break;
		}
		count += bp->wptr - bp->rptr;
		bnext = &bp->next;
	}
	bp = head;

	/* if there is no ethernet header in packet, throw it out */
	if (count < sizeof(struct ether_out)) {
		while(bp) {
			nbp = bp->next;
			freeb(bp);
			bp = nbp;
		}
		return;
	}
	ecu->active = 1;

	/* test for small packets */
	if (count < 46 + sizeof(struct ether_out))
		count = 46 + sizeof(struct ether_out);

	/* test for too large packets */
	else if (count > 1500 + sizeof(struct ether_out))
		count = 1500 + sizeof(struct ether_out);

	/* Fill buffer - packet is justified to end of buffer. */
	tsp = (u_char *)(ECmem+(ECBFSZ*ECTBF));
	*(short *)tsp = (ECBFSZ) - count;	/* Stuff offset into buffer */
	tsp += ((ECBFSZ) - count);		/* Adjust ptr into buffer */

	while (count > 0 && bp != 0) {
		len = bp->wptr - bp->rptr;
		fsp = bp->rptr;
		count -= len;
		while (len-- > 0)		/* Could this use scopy()? */
			*tsp++ = *fsp++;
		nbp = bp->next;
		freeb(bp);
		bp = nbp;
	}

	/* if no buffers left but count > 0, (i.e. padded packet), zero fill */
	while (count-- > 0)
		*tsp++ = 0;

	/* flush remaining buffers, if any (too large packet: count > 1500) */
	while (bp) {
		nbp = bp->next;
		freeb(bp);
		bp = nbp;
	}

	if (ecu->state & ECDEBUG) {
		register i;
		printf("\ncount=%d, packet = ", count);
		tsp = (u_char *)(ECmem+(ECBFSZ*ECTBF)) + (ECBFSZ - count);
		for(i = 0; i < count; i++)
			printf("%02x ", (unsigned int)*tsp++);
		printf("\n");
	}
	ecu->state |= ECXBUSY;
	ecu->collisions = 0;
	ecaddr->ecxcr = EC_WRITE + ECTBF;
}

ecioctl(q, bp)
register struct queue *q;
register struct block *bp;
{
#ifndef EIOCSETP
#define	EIOCSETP	(('e'<<8)|3)	/* set ec_state (state of driver) */
#define	EIOCGETP	(('e'<<8)|4)	/* get ec_state (driver state) */
#define	EIOCAREC	(('e'<<8)|5)	/* set address recognition */
#endif
	register struct ecdevice *ecaddr;
	register struct ecchan *ecd;
	register struct ec *ecu;
	register u_char	*msg;
	register	unit, i, ps, cmd;

	ecd = (struct ecchan *)q->ptr;
	unit = ecd->unit;
	ecu = &ec[unit];
	ecaddr = (struct ecdevice *)ecinfo[unit]->ui_addr;
	cmd = ((union stmsg *)(bp->rptr))->ioc1.com;
	msg = (u_char *)&((union stmsg *)(bp->rptr))->ioc1.sb;

	bp->type = M_IOCACK;
	if (ecu->state & ECDEBUG)
		printf("ecioctl: cmd = %x\n");
	switch(cmd) {
		case ENIOADDR:          /* get my Ethernet address */
			bcopy(ecu->myethadr, (int *)msg, 6);
			break;

/*
 * SETP, SETD, SSIG, AREC should be super user ioctl calls.
 * Since it is impossible to know who the initiator of an ioctl call is,
 * it would be desirable to have a special minor device number for super user
 * privileges
 */
		case EIOCSETP:
		case EIOCGETP:
			if (cmd == EIOCGETP) {
				*(int *)msg = ecu->state;
			} else {
				ecu->state = *(int *)msg;
			}
			if (ecu->state & ECDEBUG)
				printf("state = %o\n", ecu->state);
			break;

		case ENIOTYPE:
			ecd->type = *(int *)msg;
			break;
			
		case EIOCAREC:
			ecu->bits = *(int *)msg;
			ecu->bits |= EC_IE|EC_WBN;
			if (ecu->state & ECDEBUG)
				printf("set addr rec, bits = %b\n", ecu->bits, EC_BITS);
			ps = spl6();
			ecaddr->ecxcr = EC_CLR;		/* Reset the UE */
			for(i = ECRHBF; i >= ECRLBF; i--)
				ecaddr->ecrcr = ecu->bits + i;
			splx(ps);
			break;

		default:
			bp->type = M_IOCNAK;
			break;
	}
	qreply(q, bp);
}


/* Come here only after hardware has decided packet is for us--
   either exactly, or multi-cast */
ecrint(unit)
{
	register struct ecchan *ecd;
	register struct ec *ecu = &ec[unit];
	register struct ecdevice *ecaddr = (struct ecdevice *)ecinfo[unit]->ui_addr;
	register struct queue *q;
	register struct block *bp;
	register short buf;			/* Extract done rcv buffer */
	register short type;
	register short *fsp;
	register len, min, i;
	register u_char *p;

	while (ecaddr->ecrcr & EC_DONE) {
		buf = ecaddr->ecrcr & 0xF;		/* Extract buffer # */
		if (buf < ECRLBF || buf > ECRHBF)
			panic("ec%d: rcvr interrupt (bad buf=%d)\n", unit, buf);

		/* fsp points to the buffer in VAX virt memory */

		fsp = (short *)(ECmem+(ECBFSZ*buf));
		if (*fsp < 0) {		/* negative offset => FCS err */
			ecu->ierrors++;
			goto out;
		}
		if (((*fsp) & 0x3fff) == 0) {  /* 0 offset => TOO LONG pkt */
			ecu->ierrors++;
			goto out;
		}
/* *fsp contains offset to end of packet - ie (length + beginning offset);
   packet starts at offset 528 into the buffer */

		if (*fsp < 592 || *fsp > 2046) {  /* illegal count */
			ecu->ierrors++;
			goto out;
		}
		len = *fsp - (528+4);	/* +4 is the FCS */
		fsp = fsp + (528/2);		/* Offset of beg of packet */
		type = *(fsp + 6);
		p = (u_char *)fsp;

		ecd = &ecchan[unit * CHANS_PER_UNIT];
		for (i = 0; i < CHANS_PER_UNIT; i++, ecd++)
			if (ecd->ecq && ecd->type == type)
				break;
		if (i >= CHANS_PER_UNIT)
			goto out;
		q = ecd->ecq;
		if (q->next->flag & QFULL) {
			if(ecu->state & ECDEBUG)
				printf(" q full\n");
			ecu->ierrors++;
			goto out;
		}
		while (len > 0) {
			bp = allocb(len);
			if (bp == 0) {
				printf("ecrint no buffers\n");
				goto out;
			}
			min = MIN((bp->lim - bp->base), len);
			len -= min;
			while (min-- > 0)
				*bp->wptr++ = *p++;
			(*q->next->qinfo->putp)(q->next, bp);
		}
		if (putctl(q->next, M_DELIM))
			ecu->ipackets++;
		else 
			printf("ecrint no DELIM bp\n");
	out:
		ecaddr->ecrcr = ecu->bits | EC_CLRDN | buf;
	}
}

ecjint(unit)
{
	register struct ecdevice *ecaddr = (struct ecdevice *)ecinfo[unit]->ui_addr;
	register struct ec *ecu = &ec[unit];
	register i;

	if(++ecu->collisions > 8) {
		printf("ec gak\n");
		ecu->oerrors++;
		/* Hmmm, can't xmit in 8 tries */
		ecaddr->ecxcr = EC_CLR;		/* Reset the UE */
						/* And re-read all rcv buffers */
		for(i = ECRHBF; i >= ECRLBF; i--)
			ecaddr->ecrcr = ecu->bits + i;
		ecu->state &= ~ECXBUSY;
	} else {
		ecu->collisions++;
		backoff(ecu->collisions);
		ecaddr->ecxcr = EC_CLEAR; /* RESET */
	}
}


/* backoff:  this routine implements an approximation of the binary
 *	exponential backoff algorithm.  When called with an argument n,
 *	the retry number, this routine will delay (in a busy loop)
 *	a random amount of time between 0 and 2**n slot times.  A slot
 *	time is defined as 51.2 microseconds.  This routine assumes it
 *	takes about 300 microseconds to be called after the jam actually
 *	occurs, and that the random number generator takes about 50
 *	microseconds.
 */
backoff(n)
register int n;
{
	long ecrand();
	register int time;

	if (n < 4)
	    return;
	if (n > 10)
	    n = 10;

 /* compute number of slot times to delay */
	time = ((int)ecrand() & ((1 << n) - 1)) - 7;
	if (time <= 0)
	    return;

	while(time--)
	    ecidle();
}

static	long	randx = 1;

ecsrand(x)
unsigned x;
{
	randx = x;
}

ecrand()
{
	return(((randx = randx*1103515245 + 12345)>>16) & 077777);
}


ecidle()
{
	register i = 40;

	while(--i) ;
}

/*
 * ecxint gets called when a transmit completes normally.
 */
ecxint(unit)
{
	register struct ecdevice *ecaddr = (struct ecdevice *)ecinfo[unit]->ui_addr;
	register struct ec *ecu;
	register struct ecchan *ecd;
	register struct block *bp;
	register buf = ecaddr->ecxcr & 0xF;

	if((ecaddr->ecxcr & EC_DONE) == 0 || buf != ECTBF)
		printf("ec%d: xmit interrupt-bad bufno, xcsr=%b\n", unit, ecaddr->ecxcr, EC_BITS);

	ecu = &ec[unit];
	if(ecu->active == 0) {
		printf("ec%d: stray xmit interrupt, xcsr=%b\n", unit, ecaddr->ecxcr, EC_BITS);
		return;
	}

	ecaddr->ecxcr = EC_CLRDN;
	ecu->state &= ~ECXBUSY;
	ecu->active = 0;
	ecu->opackets++;
	ecstart(unit);
}
scopy(f, t, c)
register u_char	*f, *t;
register	c;
{
	while(c-- > 0)
		*t++ = *f++;
}
#endif

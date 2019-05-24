
/*
 * Interlan Ethernet Communications Controller interface.
 * Provides fairly raw access to a number of Interlan controllers.
 * Minor device N talks to unit N / 8. Each written record should
 * be an interlan output packet - 6 bytes addr, 2 bytes type, data.
 * An ethernet packet type may be associated with a minor device with
 * the ENIOTYPE ioctl; input packets of that type will be sent to
 * the minor device in question. Input packets include 6 bytes src addr,
 * 6 bytes dest, 2 bytes type, and data.
 *
 * The physical address of a controller may be fetched with the ENIOADDR
 * ioctl on a minor device associated with that unit.
 */
#include "il.h"
#if NIL > 0

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/stream.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../h/ubavar.h"
#include "../h/conf.h"
#include "../h/ioctl.h"
#include "../h/ttyld.h"
#include "../h/ill_reg.h"
#include "../h/ethernet.h"

int	ilprobe(), ilattach(), ilrint(), ilcint();
struct	uba_device *ilinfo[NIL];
u_short	ilstd[] = { 0 };
struct	uba_driver ildriver =
	{ ilprobe, 0, ilattach, 0, ilstd, "il", ilinfo };

#define ILOUTSTANDING	1	/* max # of rcv bufs in controller q */
#define BOGUS		1	/* CDONE doesn't work */

struct il {	/* per controller */
	int attached;
	int active;
	int rcvpending;
	int ipackets, opackets;
	int ierrors, oerrors;
	int collisions;
	struct block *bp;	/* waiting to be filled */
	struct block *nbp;	/* next block to be filled in this packet */
	int len;		/* amount left in this packet */
	struct queue *tq;	/* current transmit q */
	struct block *freebp;
	unsigned char addr[6];
} il[NIL];

extern u_char blkdata[];	/* stream.c */
extern long blkubad;
int ilprintfs = 0;

struct il_stats ilstats;

#define CHANS_PER_UNIT	8
#define NILCHAN		(CHANS_PER_UNIT * NIL)
struct ilchan{	/* per stream */
	int unit;
	int packets;	/* # of packets on q to output */
	struct queue *rq;
	int type;	/* ethernet protocol # */
	int haveheader;	/* ethernet header has gone by */
} ilchan[NILCHAN];

ilprobe(reg)
caddr_t reg;
{
	register int br, cvec;	/* r11, r10 */
	register struct ildevice *addr = (struct ildevice *) reg;
	register i;

#ifdef lint
	br = 0; cvec = br; br = cvec; i = br; br = i;
#endif

	addr->il_csr = ILC_OFFLINE|IL_CIE;
	DELAY(100000);
	i = addr->il_csr;	/* clear CDONE */
	if(cvec > 0 && cvec != 0x200)
		cvec -= 4;
	return(1);
}

ilattach(ui)
struct uba_device *ui;
{
	register struct il *is = &il[ui->ui_unit];
	register struct ildevice *addr = (struct ildevice *)ui->ui_addr;
	int ubaddr, s;


	s = spl6();

	addr->il_csr = ILC_RESET;
	while((addr->il_csr&IL_CDONE) == 0)
		;
	if(addr->il_csr&IL_STATUS)
		printf("il%d: reset failed, csr=%b\n", ui->ui_unit,
			addr->il_csr, IL_BITS);
	
	ubaddr = uballoc(ui->ui_ubanum, (caddr_t)&ilstats,
			 sizeof(ilstats), 0);
	if(ubaddr == 0){
		printf(" no uballoc\n");
		goto nostats;
	}
	addr->il_bar = ubaddr&0xffff;
	addr->il_bcr = sizeof(ilstats);
	addr->il_csr = ((ubaddr >> 2) & IL_EUA) | ILC_STAT;
	while((addr->il_csr&IL_CDONE) == 0)
		;

	ubarelse(ui->ui_ubanum, &ubaddr);
	bcopy(ilstats.ils_addr, is->addr, sizeof(is->addr));

nostats:
	addr->il_csr = ILC_ONLINE;
	while((addr->il_csr&IL_CDONE) == 0)
		;
	/*
	 * ask ilcint to set up the first rcv buffer,
	 * since the block stuff doesn't seem to be
	 * initialized yet.
	 */
	is->rcvpending = ETHERMTU;

	is->active = 0;
	is->attached = 1;
	splx(s);
}

ilstart(unit)
{
	int ubaddr, count;
	struct uba_device *ui = ilinfo[unit];
	register struct il *is = &il[unit];
	register struct ildevice *addr;
	struct ilchan *icp;
	struct block *bp;
	register struct queue *q;

	if(is->active){
		printf(" start but active\n", unit);
		is->active = 0;
	}

	for(icp = ilchan; icp < &ilchan[NILCHAN]; icp++)
		if(icp->rq && icp->unit == unit && icp->packets > 0)
			break;
	if(icp >= &ilchan[NILCHAN])
		return;
	q = WR(icp->rq);

	is->active = 1;
	addr = (struct ildevice *)ui->ui_addr;
#ifdef BOGUS
	is->tq = q;
	bp = getq(q);
	if(bp == 0){
		printf("ilstart no bp\n");
		is->active = 0;
		return;
	}
	ildebug(bp, 1);
	ubaddr = iladdr((caddr_t)(bp->rptr));
	addr->il_bar = ubaddr&0xffff;
	addr->il_bcr = bp->wptr - bp->rptr;
	addr->il_csr = ((ubaddr>>2)&IL_EUA)|ILC_LDXMIT|IL_RIE|IL_CIE;
	is->freebp = bp;
	/*
	 * I would have given the entire packet to the controller,
	 * but it's brain-damaged. So I wait for the interrupt to
	 * put the next one on.
	 */
#else
	while(bp = getq(q)){
		if(bp->type != M_DATA){
			/* don't free, it's re-used */
			break;
		}
		ubaddr = iladdr((caddr_t)bp->rptr);
		addr->il_bar = ubaddr&0xffff;
		addr->il_bcr = bp->wptr - bp->rptr;
		addr->il_csr = ((ubaddr>>2)&IL_EUA)|ILC_LDXMIT|IL_RIE;
		ilcdone(addr, "LDX");
		freeb(bp);
	}
	if(bp == 0){
		printf("ilstart no delim?\n");
		bp = allocb(1);
	}
	bp->wptr = bp->rptr;
	--(icp->packets);
	ubaddr = iladdr((caddr_t)bp->base);
	addr->il_bar = ubaddr&0xffff;
	addr->il_bcr = 0;
	addr->il_csr = ((ubaddr>>2)&IL_EUA)|ILC_XMIT|IL_CIE|IL_RIE;
	/* go, baby */
	is->opackets++;
	freeb(bp);
#endif BOGUS
}

/*
 * transmit done
 */
ilcint(unit)
{
	int ubaddr;
	register struct il *is = &il[unit];
	struct uba_device *ui = ilinfo[unit];
	register struct ildevice *addr = (struct ildevice *)ui->ui_addr;
	short csr;
	register struct block *bp;

	if(is->active == 0){
		printf("il%d: stray xmit interrupt, csr=%b\n", unit,
			addr->il_csr, IL_BITS);
		return;
	}
#ifdef BOGUS
	if(is->tq){
		if(is->freebp)
			freeb(is->freebp);
		is->freebp = 0;
		if((bp = getq(is->tq)) == 0){
			printf("gak\n");
			is->active = 0;
			is->tq = 0;
			return;
		}
		ubaddr = iladdr((caddr_t)(bp->rptr));
		addr->il_bar = ubaddr&0xffff;
		addr->il_bcr = bp->wptr - bp->rptr;
		if(bp->type == M_DELIM){
			addr->il_csr = ((ubaddr>>2)&IL_EUA)|ILC_XMIT|IL_RIE|IL_CIE;
			((struct ilchan *)(is->tq->ptr))->packets -= 1;
			is->tq = 0;
			freeb(bp);
		} else {
			addr->il_csr = ((ubaddr>>2)&IL_EUA)|ILC_LDXMIT|IL_RIE|IL_CIE;
			is->freebp = bp;
		}
		return;
	}
#endif BOGUS
	csr = addr->il_csr;
	is->active = 0;
	if(is->rcvpending)
		ilsetup(is, addr, is->rcvpending);

	/* check status of last xmit */
	csr &= IL_STATUS;
	if(csr > ILERR_RETRIES){
		printf("il%d: tx error 0x%x\n", unit, csr);
		is->oerrors++;
	} else if(csr > ILERR_SUCCESS){
		is->collisions++;
	}
#ifdef BOGUS
	else is->opackets++;
#endif
	ilstart(unit);
}

ilrint(unit)
{
	register struct il *is = &il[unit];
	register struct ildevice *addr = (struct ildevice *)ilinfo[unit]->ui_addr;
	register struct il_rheader *hp;
	int len;
	struct block *bp, *bp1;
	register struct ilchan *icp;
	register struct queue *q;

	if(is->bp == 0)
		panic("ilrint no is->bp");
	if(is->nbp == 0)
		panic("ilrint no is->nbp");
	bp = is->nbp;
	if(bp == is->bp){
		/* first buffer of a packet */
		hp = (struct il_rheader *)bp->rptr;
		is->len = hp->ilr_length;
		is->len += 4;
	}
	is->len -= (bp->wptr - bp->rptr);
	is->nbp = bp->next;
	/* ilsetup will take care of huge packets by asking
	 * the controller to truncate.
	 */
	if(is->len <= 0)
		goto done;
	if((bp->wptr - bp->rptr) % 8)	/* not chaining */
		goto done;
	if(is->nbp == 0){
		/* more, more */
		if(is->active){
			is->rcvpending = is->len;
		} else {
			ilsetup(is, addr, is->len);
		}
	}
	return;

done:
	bp = is->bp;
	hp = (struct il_rheader *)bp->rptr;
	len = hp->ilr_length - sizeof(struct il_rheader);

	if(len < 46 || len > ETHERMTU || is->len > 0){
		if(ilprintfs)
			printf("il%d: ilr_length %d is-len %d\n",
				unit, hp->ilr_length, is->len);
		is->ierrors++;
		goto setup;
	}
	if(hp->ilr_status&ILFSTAT_L)
		is->ierrors++;
	len += sizeof(struct il_rheader) - 4;

	for(icp = ilchan; icp < &ilchan[NILCHAN]; icp++)
		if(icp->rq && icp->unit == unit && icp->type == hp->ilr_type)
			break;
	if(icp >= &ilchan[NILCHAN]){
		if(ilprintfs)
			printf("type %x?\n", hp->ilr_type);
		goto setup;
	}
	q = icp->rq;
	if(q->next->flag&QFULL){
		if(ilprintfs)
			printf(" q full\n");
		is->ierrors++;
		goto setup;
	}
	bp->rptr = &(hp->ilr_dhost[0]);
	ildebug(bp, 0);
	len = hp->ilr_length - 4;
	while(bp && bp != is->nbp){
		if (bp->wptr - bp->rptr > len)
			bp->wptr = bp->rptr + len;
		len -= bp->wptr - bp->rptr;
		bp1 = bp->next;
		if (bp->wptr > bp->rptr)
			(*q->next->qinfo->putp)(q->next, bp);
		else
			freeb(bp);
		bp = bp1;
	}
	is->bp = is->nbp;
	bp = allocb(1);
	if(bp){
		bp->type = M_DELIM;
		(*q->next->qinfo->putp)(q->next, bp);
		is->ipackets++;
	} else {
		printf("ilrint no DELIM bp\n");
	}
setup:
	/* free up blocks in a rejected packet */
	bp = is->bp;
	while(bp && bp != is->nbp){
		bp1 = bp->next;
		freeb(bp);
		bp = bp1;
	}
	is->bp = bp;
	if(is->active){		/* don't interfere w/ output */
		is->rcvpending = ETHERMTU;
		return;
	}
	ilsetup(is, addr, ETHERMTU);
}

iladdr(memaddr)
caddr_t memaddr;
{
	register long a;

	if((unsigned)memaddr < (unsigned)blkdata){
		printf("memaddr %x blkdata %x\n", memaddr, blkdata);
		panic("iladdr");
	}
	a = (long)memaddr - (long)(caddr_t)blkdata + ((long)blkubad&0x03ffff);
	/* sure */
	return(a);
}

int	nodev(), ilopen(), ilclose(), ilput();
struct	qinit ilrinit = { nodev, NULL, ilopen, ilclose, 0, 0 };
struct	qinit ilwinit = { ilput, NULL, ilopen, ilclose, ETHERMTU-6, 64 };
struct	streamtab ilsinfo = { &ilrinit, &ilwinit };

ilopen(q, dev)
register struct queue *q;
register dev_t dev;
{
	register struct ilchan *icp;
	register struct il *is;
	int unit, s;

	dev = minor(dev);
	unit = dev / CHANS_PER_UNIT;
	if(dev >= NILCHAN)
		return(0);
	if(unit >= NIL)
		return(0);
	is = &il[unit];
	if(is->attached == 0)
		return(0);
	icp = &ilchan[dev];
	if(icp->rq)
		return(0);

	icp->rq = q;
	q->ptr = (caddr_t)icp;
	WR(q)->ptr = (caddr_t)icp;
	WR(q)->flag |= QDELIM|QBIGB;
	q->flag |= QDELIM;

	icp->unit = unit;
	icp->type = 0;
	icp->packets = 0;

	s = spl6();
	if(is->rcvpending && is->active == 0){
		/* first open, supply rcv buffer */
		is->active = 1;
		ilcint(unit);
	}
	splx(s);
	return(1);
}

ilclose(q)
register struct queue *q;
{
	register struct ilchan *icp;

	icp = (struct ilchan *)q->ptr;
	icp->rq = 0;
	icp->packets = 0;
}

/*
 * Ilput expects the first block of each packet to contain a six byte
 * ethernet address followed by a two byte packet type number in
 * network byte order.
 */
ilput(q, bp)
register struct queue *q;
	struct block *bp;
{
	register struct il *is;
	int unit, s;
	register struct ilchan *icp;

	icp = (struct ilchan *)q->ptr;
	unit = icp->unit;

	if(bp->type == M_DATA){
		if (!icp->haveheader) {
			icp->haveheader = 1;
			ilfixheader(bp);
		}
		putq(q, bp);
		return;
	} else if(bp->type == M_IOCTL){
		ilioctl(q, bp);
		return;
	} else if(bp->type != M_DELIM){
		freeb(bp);
		return;
	}
	/* have end of packet */
	icp->haveheader = 0;
	putq(q, bp);
	is = &il[unit];
	s = spl6();
	icp->packets++;
	if(is->active == 0)
		ilstart(unit);
	splx(s);
}

/*
 *	Interlan drivers accept a strange type of ethernet header.
 *	It is the normal ethernet header with the source field removed.
 */
ilfixheader(bp)
register struct block *bp;
{
	register struct ether_in *ep;

	if (bp->wptr - bp->rptr < sizeof(struct ether_out)) {
		printf("ether_header too short\n");
		return;
	}
	ep = (struct ether_in *)(bp->rptr);
	bcopy(ep->dhost, ep->shost, 6);
	bp->rptr += 6;
}

#define ILLDEBSIZE 64
struct {
	time_t	time;
	unsigned short code;
	unsigned char addr[6];
} illdebarr[ILLDEBSIZE];

int illindex = 0;

ildebug(bp, code)
register struct block *bp;
{
	illdebarr[illindex].time = time;
	illdebarr[illindex].code = code;
	bcopy(bp->rptr, illdebarr[illindex].addr, 6);
	illindex = (illindex + 1) % ILLDEBSIZE;
}

ilioctl(q, bp)
register struct queue *q;
register struct block *bp;
{
	union stmsg *sp;
	register struct ilchan *icp;
	int unit;
	struct uba_device *ui;
	struct ildevice *addr;

	icp = (struct ilchan *)q->ptr;
	sp = (union stmsg *)bp->rptr;
	bp->type = M_IOCACK;
	switch(sp->ioc0.com){
	case ENIOTYPE:
		icp->type = *((int *)(sp->iocx.xxx));
		break;
	case ENIOADDR:
		bcopy(il[icp->unit].addr, sp->iocx.xxx, 6);
		break;
	case ENIOCMD:		/* perform a non-dma interlan command */
		unit = icp->unit;
		ui = ilinfo[unit];
		addr = (struct ildevice *)ui->ui_addr;
		addr->il_csr = *((int *)(sp->iocx.xxx));
		*((int *)(sp->iocx.xxx)) = ilcdone(addr, "CMD");
		break;
	default:
		bp->type = M_IOCNAK;
		break;
	}
	qreply(q, bp);
}

ilsetup(is, addr, n)
register struct il *is;
register struct ildevice *addr;
{
	register struct block *bp;
	int ubaddr, count;

	if(is->active)
		panic("ilsetup active");
	/*
	 * is->bp is start of buffer chain, is->nbp is next buffer
	 * to be DMA'd into. if is->bp is zero, there is no chain
	 * yet, else if is->nbp is zero, add stuff to the end of
	 * the chain, else only add stuff if we need to.
	 */
	if(is->nbp && is->bp == 0){
		printf("il: nbp but no bp in ilsetup\n");
		is->nbp = 0;
	}
	count = 0;
	/* count bytes and buffers already loaded into controller */
	for(bp = is->nbp; bp; bp = bp->next){
		n -= bp->wptr - bp->rptr;
		count++;
	}
	if(n <= 0)
		return;
	if(count > ILOUTSTANDING)
		printf("ilsetup: already %d in bar/bcr q!\n", count);
	if(is->bp == 0){
		bp = is->nbp = 0;
	} else {
		for(bp = is->bp; bp->next; bp = bp->next)
			;
	}
	/* now bp is 0 or else the last block in the chain */
	while(count < ILOUTSTANDING && n > 0){
		if(bp == 0){
			is->bp = is->nbp = bp = allocb(n);
		} else {
			bp->next = allocb(n);
			bp = bp->next;
		}
		if(bp == 0){
			is->rcvpending = n;
			return;
		}
		bp->next = 0;
		if(is->nbp == 0)
			is->nbp = bp;
		bp->rptr = bp->base;
		bp->wptr = bp->lim;
		if(n > ETHERMTU){	/* tell controller to truncate */
			bp->wptr -= 2;
			n = 0;
		}
		n -= bp->wptr - bp->rptr;
		count++;
		ubaddr = iladdr((caddr_t)(bp->rptr));
		addr->il_bar = ubaddr & 0xffff;
		addr->il_bcr = bp->wptr - bp->rptr;
		addr->il_csr = ((ubaddr>>2)&IL_EUA)|ILC_RCV|IL_RIE;
		ilcdone(addr, "RCV");
	}
	is->rcvpending = 0;
}

ilcdone(addr, s)
struct ildevice *addr;
char *s;
{
	int count;

	count = 0;
	while((addr->il_csr & IL_CDONE) == 0){
		count++;
		if(count > 200000)
			break;
	}
	if(count > 200000){
		printf("%s: addr 0%o csr 0x%x\n", s, addr, addr->il_csr);
		return(1);
	}
	return(addr->il_csr & 0xf);
}
#endif NIL

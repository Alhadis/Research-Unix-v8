#define BSD41
#define	VAXONE
#define TIMEOUT

#ifdef MARVIN
#define TXDRIVER
#endif
#ifdef CANTOR
#define TXDRIVER
#endif
#ifdef HILBERT
#define TXDRIVER
#endif
#ifdef DAISY
#define TXDRIVER
#endif

#ifdef USG5
#define NSN 1
#else
#include "sn.h"
#endif
#if NSN > 0

#ifdef BSD41
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/pte.h"
#include "../h/map.h"
#include "../h/vm.h"
#include "../h/dk.h"
#include "../h/cmap.h"
#include "../h/ubareg.h"
#include "../h/ubavar.h"
#include "../h/cpu.h"
#include "../h/proc.h"
#include "../h/snet.h"
#include "../h/packet.h"
#include "../h/channel.h"
#include "../h/status.h"
#endif

#ifdef SNDEBUG
int sndebug = 1;
#else
int sndebug = 0;
#endif

/* flags for status */
#define INPUT	1
#define OUTPUT	2

#define min(x,y) ((x) < (y) ? (x) : (y))
#define swab(x) (unsigned short)((((x)>>8)&0xff) | ((x) << 8))

#define SNPRI	28

#define CHKINIT 0xbf37

struct device {
	short drcs;
	short drout;
	short drin;
};

/* Command and Status word */
#define CSR0 0x1
#define CSR1 0x2
#define RIE 0x20
#define TIE 0x40
#define TEMPTY 0x80
#define RFULL 0x8000

/*
 *	CSR1 CSR0
 *	 0    0 	Write Data
 *	 0    1 	Write EOP
 *	 1    0 	Write Command
 *	 1    1 	Read Status
 */
#define DATA_MODE	(RIE)
#define EOP_MODE	(CSR0 | RIE)
#define CMD_MODE	(CSR1 | RIE)
#define STATUS_MODE	(CSR0 | CSR1 | RIE)

/* Command word format
 *
 *	+-----------------------------------------------+
 *	|  |B |R |A |           |              |        |
 *	|  |C |S |B |    DID    |              |  FCN   |
 *	|  |T |T |T |           |              |        |
 *	+-----------------------------------------------+
 *	 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
 */
/* Function codes */
#define NOP  	0x0
#define BOARD_RESET	0x5
#define MASTER_CLEAR	0x6

#define ABORT(did)	(0x1000 | (did))
#define RESET(did)	(0x2000 | (did))
#define BROAD_CAST(did)	(0x4000 | (did))


/* SNET status word bits
 *
 *	+-----------------------------------------------+
 *	|                             |E |S |I |O |  |  |
 *	|                             |O |N |B |B |  |  |
 *	|                             |P |K |E |E |  |  |
 *	+-----------------------------------------------+
 *	 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
 */	
#define OUTBUF_E	0x4
#define INBUF_E	0x8
#define SNACK	0x10	/* transmitter error */
#define EOP	0x20	/* we have read and EOP frame */

static int sn_multiplex;

struct openwait sn_openwait[NUMDEV];
struct openwait *sn_owhead, *sn_owtail;

#define SNFILE(dev)  (minor(dev) & 0x3f)
#define SNUNIT(dev) ((minor(dev) & 0xc0) >> 6)

struct chdat sn_chans[NUMLCH];
#define MAXMSGLEN (512+8)
struct sn_buffers {
	short inbuf[MAXMSGLEN];
	short outbuf[MAXMSGLEN];
} sn_buffers[NUMLCH];

#define NOPDID -1

#ifdef TIMEOUT
#define CKFUZZ 	4	/* number of timeouts we wait for a response */
#ifdef USG5
#define CKTICKS	(HZ/4)	/* number of ticks per timeout */
#else
#define CKTICKS	(hz/4)	/* number of ticks per timeout */
#endif

int sn_state;
#define	OPEN 1		/* set if at least one sn? is open */
#define	TIMER 2		/* set if at timer is running */

int cktimchk();
int sn_opncnt;
#endif TIMEOUT

struct snmach snmach[NUMDEV] = {
	{ 1, 255 },
	{ 2, 254 },
	{ 3, 253 },
	{ 4, 252 },
	{ 5, 251 },
	{ 6, 250 },
	{ 7, 249 },
	{ 8, 248 } };

int sn_once;

#ifdef BSD42
#define RETURN(x)	{return(x);}
#define ARGS(x)		args->x
#else
#define RETURN(x)	{u.u_error = x; return;}
#define ARGS(x)		args.x
#endif

#ifdef USG5
extern struct sninfo sn_sninfo[];
extern int sn_cnt;
extern struct device *sn_addr[];
#else
struct sninfo sn_sninfo[NSN];
int	snprobe(), snattach();
struct	uba_device *sndinfo[NSN];
u_short	snstd[] = {0};
struct uba_driver sndriver =
	{ snprobe, 0, snattach, 0, snstd, "sn", sndinfo};

snprobe(reg)
caddr_t reg;
{
	register int br, cvec;	/* value result */
	register struct device *snaddr = (struct device *)reg;

	br = 0x15;
#ifdef VAXONE
	cvec = 0510;
#else
#ifdef CANTOR
	if (((int)snaddr & 017777) == 007770)
		cvec = 0504;
	if (((int)snaddr & 017777) == 007750)
		cvec = 0464;
#else
	cvec = 0504;
#endif
#endif
#ifdef BSD41
	return(1);
#endif
#ifdef BSD42
 	return(sizeof(struct device));
#endif
}

/*ARGSUSED*/
snattach(ui)
register struct uba_device *ui;
{
}
#endif

snopen(dev, rw)
dev_t dev;
int rw;
{
	register struct device *draddr;
#ifndef USG5
	struct uba_device *ui;
#endif
	register struct sninfo *sninfop;
	int mdev = SNFILE(dev);
	int unit = SNUNIT(dev);

	if( sndebug )
		printf("snopen(%x,%x)\n", dev, rw);

#ifdef USG5
	if ( mdev > NUMDEV || unit >= sn_cnt) {
		RETURN ( ENXIO );
	}
	draddr = sn_addr[unit];
#else
	if (mdev > NUMDEV || unit >= NSN ||
	    (ui = sndinfo[unit]) == 0 || ui->ui_alive == 0) {
		RETURN ( ENXIO );
	}
	draddr = (struct device *)ui->ui_addr;
#endif

	if ( sn_once == 0 ) {
		struct chdat *cp = sn_chans;

		sn_once++;
		for( ; cp<&sn_chans[NUMLCH]; cp++) {
			cp->lchnum = -1;
			cp->dlchnum = -1;
		}
		sn_owhead = sn_owtail = sn_openwait;
	}

	sninfop = &sn_sninfo[unit];
	spl6();
	if (sninfop->sndevlock[mdev]++ != 0) {
		spl0();
		RETURN ( EBUSY );
	}
#ifdef TIMEOUT
	sn_opncnt++;
	sn_state |= OPEN;
	if ((sn_state & TIMER) == 0) {
		sn_state |= TIMER;
		timeout(cktimchk, (caddr_t)0, CKTICKS);
	}
#endif TIMEOUT
	if( sninfop->snlock++ == 0 ) {
#ifdef TXDRIVER
		{
		extern struct txinfo tx_sninfo[];

		if (tx_sninfo[unit].snlock != 0) {
			spl0();
			return;
		}
		}
#endif
		draddr->drcs = CMD_MODE;
		draddr->drout = 0;
		draddr->drout = BOARD_RESET;
		draddr->drout = 0;
		draddr->drcs = DATA_MODE;
	}

	spl0();
	return(0);
}

snclose(dev)
{
	register struct chdat *cp;
#ifndef USG5
	struct uba_device *ui;
#endif
	register struct sninfo *sninfop;
	register struct device *draddr;
	int mdev = SNFILE(dev);
	int unit = SNUNIT(dev);
	int resetmachno;
	struct openwait *curtail;
	
	if( sndebug )
		printf("snclose(%x)\n", dev);
#ifdef USG5
	if ( unit >= sn_cnt) {
		RETURN ( ENXIO );
	}
	draddr = sn_addr[unit];
#else
	if ( unit >= NSN) {
		RETURN ( ENXIO );
	}
	ui = sndinfo[unit];
	draddr = (struct device *)ui->ui_addr;
#endif

	sninfop = &sn_sninfo[unit];
	spl6();
	if( sninfop->snlock == 0 || mdev > NUMDEV 
			|| sninfop->sndevlock[mdev] == 0 ) {
		spl0();
		RETURN ( ENXIO );
	}

	resetmachno = -1;
	for(cp=sninfop->snlink[mdev]; cp; cp=cp->link) {
		if (cp->pdid == NOPDID)
			resetmachno = cp->machno;
		chclose(cp);
	}
	if (resetmachno != -1) {
		for(cp=sn_chans; cp < &sn_chans[NUMLCH]; cp++) {
			if (cp->pdid == NOPDID && cp->machno == resetmachno)
				goto noreset;
		}
		draddr->drcs = CMD_MODE;
		draddr->drout =  RESET(resetmachno << 8);
		draddr->drcs = EOP_MODE;
		draddr->drout = 0;
		draddr->drcs = DATA_MODE;
	}

noreset:
	for (curtail = sn_owtail; curtail != sn_owhead; 
			curtail = ownext(curtail)) {
		if (curtail->pid == u.u_procp->p_pid) {
			curtail->pid = 0;
			goto nowakeup;
		}
	}
	while (sn_owtail != sn_owhead) {
		curtail = sn_owtail;
		sn_owtail = ownext(sn_owtail);
		if (curtail->pid) {
			wakeup(curtail);
			break;
		}
	}

nowakeup:
	sninfop->snlink[mdev] = 0;
	sninfop->sndevlock[mdev] = 0;
#ifdef TIMEOUT
	if (--sn_opncnt <= 0) {
		sn_state &= ~OPEN;
		sn_opncnt = 0;
	}
#endif TIMEOUT
	if (--sninfop->snlock == 0 ) {
#ifdef TXDRIVER
		{
		if (tx_sninfo[unit].snlock != 0)
			return;
		}
#endif
		draddr->drcs &= ~RIE;
	}
	spl0();
	return(0);
}

static short ratshole[MAXMSGLEN];
static struct chdat ratschan;

#ifdef USG5
snintr(unit)
#else
snrint(unit)
#endif
int unit;
{
	register struct chdat *cp;
	register short *sp, *dp;
	register int i;
	register struct sninfo *sninfop;
	register struct device *draddr;
	unsigned short chksum, xchk;
	short len;
	unsigned short chan;
	short *osp;
#ifndef USG5
	struct uba_device *ui;
#endif

#ifdef USG5
	draddr = sn_addr[unit];
#else
	ui = sndinfo[unit];
	draddr = (struct device *)ui->ui_addr;
#endif
	dp = &draddr->drin;

	sninfop = &sn_sninfo[unit];
	sninfop->snrcnt++;

loop:
	chksum = CHKINIT;

	chan = *dp;
#ifdef TXDRIVER
	/* unusual process numbers go to tx driver */
	if ((chan & 0xff) <= 200 || ((unsigned int)chan) >= 0x2000 ) {
		txintrmt(unit, chan);
		goto getstatus;
	}
#endif
	chksum ^= chan;
	chan = swab(chan);
	len = *dp;
	chksum ^= len;
	len = swab(len);
	if( len > MAXMSGLEN ) {
		sninfop->snnbadlen++;
		goto getstatus;
	}
	sninfop->sninbytes += len+4;

	for(cp=sn_chans; cp < &sn_chans[NUMLCH]; cp++)
		if( cp->xchan == chan && cp->snunit == unit ) {
			if( cp->input.done == 0 && len <= cp->input.len )
				osp = sp = cp->input.buf;
			else
				osp = sp = ratshole;
			goto found;
		}
	osp = sp = ratshole;
	cp = &ratschan;
	sninfop->snnnolc++;
found:
			
	for(i=len; i>0; i-=8) {
		switch( i ) {
		default:
			*sp = *dp;
			chksum ^= *sp++;
		case 7:
			*sp = *dp;
			chksum ^= *sp++;
		case 6:
			*sp = *dp;
			chksum ^= *sp++;
		case 5:
			*sp = *dp;
			chksum ^= *sp++;
		case 4:
			*sp = *dp;
			chksum ^= *sp++;
		case 3:
			*sp = *dp;
			chksum ^= *sp++;
		case 2:
			*sp = *dp;
			chksum ^= *sp++;
		case 1:
			*sp = *dp;
			chksum ^= *sp++;
		}
	}
	if( (i = ((len < 0 ? 0 : len) + 3)&0xf) != 0 ) {
		for(i^=0xf; i-->=0; )
			chksum ^= *dp;
	}
	if( chksum != (xchk = *dp) ) {
		sninfop->snnchksum++;
		if( sndebug ) {
			short *usp = osp;
			int x = len;

			printf("snchksum (%x, %x):", chan, len);
			for(; --x>=0; )
				printf(" %x", *usp++);
			printf(" %x (%x)\n", xchk, chksum);
		}
	} else if( len > 0 ) {
		if( osp == ratshole ) {
			cp->flags |= SENTRNR;
			snxstart(unit, cp, RNR);
			sninfop->snsrnr++;
		} else {
			sninfop->snndata++;
			cp->input.done = 1;
			cp->input.len = len;
			snxstart(unit, cp, ACK);
			sninfop->snsack++;
			wakeup((caddr_t)cp->input.buf);
			if( cp->flags&MULTIPLEX )
				wakeup((caddr_t)&sn_multiplex);
		}
	} else {
		switch( len ) {
		case ACK:
			sninfop->snnack++;
			cp->flags &= ~SENTDATA;
			cp->output.done = 1;
			wakeup((caddr_t)cp->output.buf);
			break;

		case RDY:
			sninfop->snnrdy++;
			snxstart(unit, cp, RACK);
			if( cp->output.done == 0 && cp->output.len > 0 ) {
				snxstart(unit, cp, DATA);
				sninfop->snrout++;
			}
			break;

		case RNR:
			sninfop->snnrnr++;
			cp->flags &= ~SENTDATA;
			break;

		case RACK:
			sninfop->snnrack++;
			cp->flags &= ~SENTRDY;
			break;

		default:
			printf("snrint: unit %d, bad type %d\n", unit, len);
			break;
		}
	}

getstatus:
	draddr->drcs = STATUS_MODE;
	i = *dp;
	draddr->drcs = DATA_MODE;
	if ((i&INBUF_E) == 0) {
		sninfop->sninloop++;
		goto loop;
	}
}

snxstart(unit, cp, type)
int unit;
register struct chdat *cp;
int type;
{
	register short *sp, *dp;
	register struct sninfo *sninfop;
	register struct device *draddr;
	register int i;
	unsigned short chksum;
#ifndef USG5
	struct uba_device *ui;
#endif
	int len, numretry;

#ifdef USG5
	draddr = sn_addr[unit];
#else
	ui = sndinfo[unit];
	draddr = (struct device *)ui->ui_addr;
#endif
	dp = &draddr->drout;

	sninfop = &sn_sninfo[unit];

	if( type == DATA )  {
		len = cp->output.len;
		sninfop->snoutbytes += len+4;
	} else {
		len = type;
		sninfop->snoutbytes += 4;
	}
	draddr->drcs = STATUS_MODE;

	i = draddr->drin;
	if ( (i&OUTBUF_E) == 0 ) {
		sninfop->snoutfull++;
		do {
			i = draddr->drin;
			sninfop->sncntoutfull++;
		} while( (i&OUTBUF_E) == 0 );
	}
	
	numretry = 0;
	sninfop->snxcnt++;
retry:
	sp = cp->output.buf;
	chksum = CHKINIT;
	if (cp->pdid == NOPDID) {
		draddr->drcs = CMD_MODE;
		*dp = cp->machno<<8;
		draddr->drcs = DATA_MODE;
		*dp = i = swab(cp->dlchnum);
	} else {
		draddr->drcs = CMD_MODE;
		*dp = cp->pdid<<8;
		draddr->drcs = DATA_MODE;
		*dp = i = (cp->dlchnum<<8) | cp->machno;
	}
	chksum ^= (unsigned short)i;
	*dp = i = swab(len);
	chksum ^= (unsigned short)i;
	for( i=len; i>0; i-=8 ) {
		switch( i ) {
		default:
			chksum ^= *sp;
			*dp = *sp++;
		case 7:
			chksum ^= *sp;
			*dp = *sp++;
		case 6:
			chksum ^= *sp;
			*dp = *sp++;
		case 5:
			chksum ^= *sp;
			*dp = *sp++;
		case 4:
			chksum ^= *sp;
			*dp = *sp++;
		case 3:
			chksum ^= *sp;
			*dp = *sp++;
		case 2:
			chksum ^= *sp;
			*dp = *sp++;
		case 1:
			chksum ^= *sp;
			*dp = *sp++;
		}
	}
	if( (i = ((len < 0 ? 0 : len) + 3)&0xf) != 0 ) {
		for(i^=0xf; i-->=0; )
			*dp = 0;
	}
	draddr->drcs = EOP_MODE;
	*dp = chksum;
	draddr->drcs = STATUS_MODE;
	i = draddr->drin;
	draddr->drcs = DATA_MODE;
	if( i&SNACK ) {
		sninfop->snsnack++;
		if( ++numretry < 100 )
			goto retry;
		sninfop->snlost++;
	}
	cp->ckticks = CKFUZZ;
	if( type == DATA )  {
		cp->flags |= SENTDATA;
	}
}

snioctl(dev, cmd, addr, flag)
caddr_t addr;
dev_t dev;
{
	register struct chdat *cp;
	register struct reqinfo *rp;
	register struct sninfo *sninfop;
	register struct bufinfo *bp;
	register struct chdat *dp;
	struct status status;
	int i;
	struct status *ustatus;
	char *usaddr;
	int ulen;
	int error;
	int mdev = SNFILE(dev);
	int unit = SNUNIT(dev);

#ifdef USG5
	if ( unit >= sn_cnt) {
		RETURN ( ENXIO );
	}
#else
	if ( unit >= NSN) {
		RETURN ( ENXIO );
	}
#endif
	sninfop = &sn_sninfo[unit];

	switch( cmd ) {
	case NIOOPEN: {
#ifdef BSD42
 		struct oargs *args = (struct oargs *)addr;
#else
		struct oargs args;

		if (copyin(addr, (caddr_t)&args, sizeof(args))) {
			RETURN ( EFAULT );
		}
#endif
		spl6();
		for(cp=sn_chans; cp<&sn_chans[NUMLCH]; cp++)
			if( cp->lchnum == -1 && cp->dlchnum == -1 )
				break;
		if( cp >= &sn_chans[NUMLCH] ) {
			spl0();
			RETURN(  ENFILE );
		}
		for(dp=sn_chans; dp<&sn_chans[NUMLCH]; dp++)
			if( dp->lchnum != -1 && dp->lmachno == ARGS(lmachno)
			   && dp->lchnum == ARGS(lchno)
			   && dp->snunit == unit ) {
				spl0();
				RETURN(  EEXIST );
			}
		cp->lchnum = ARGS(lchno);
		cp->dlchnum = ARGS(dlchno);
		spl0();
		cp->lmachno = ARGS(lmachno);
		cp->machno = ARGS(dmachno);
		cp->xchan = (cp->lmachno<<8)|cp->lchnum;
		cp->snunit = unit;
		cp->pdid = NOPDID;
		bp = &cp->input;
		i = cp-sn_chans;
		bp->buf = sn_buffers[i].inbuf;
		bp->len = MAXMSGLEN;
		bp->done = 0;
		bp = &cp->output;
		bp->buf = sn_buffers[i].outbuf;
		bp->len = bp->done = 0;
		cp->flags = 0;
		cp->link = sninfop->snlink[mdev];
		sninfop->snlink[mdev] = cp;
		return(0);
	    }

	case NIOXOPEN: {
		struct snmach *snmachp;
		struct openwait *nexthead, *thishead;
#ifdef BSD42
 		struct oargs *args = (struct oargs *)addr;
#else
		struct oargs args;

		if (copyin(addr, (caddr_t)&args, sizeof(args))) {
			RETURN ( EFAULT );
		}
#endif
		for (;;) {
			for (snmachp = &snmach[0];
					snmachp < &snmach[NUMDEV] &&
						snmachp->machno;
					snmachp++) {
				spl6();
				for (cp=sn_chans; cp<&sn_chans[NUMLCH]; cp++)
					if( cp->lchnum == -1 && 
							cp->dlchnum == -1 )
						break;
				if( cp >= &sn_chans[NUMLCH] ) {
					goto xopensleep;
				}
				for(dp=sn_chans; dp<&sn_chans[NUMLCH]; dp++) {
					if( dp->lchnum != -1
					   && dp->lmachno == snmachp->lmachno
					   && dp->lchnum == ARGS(lchno)
					   && dp->snunit == unit ) {
						/* Channel already in use */
						break;
					}
				}
				if( dp >= &sn_chans[NUMLCH] ) 
					goto okmachp;
				spl0();
			}
xopensleep:
			nexthead = ownext(sn_owhead);
			if (nexthead == sn_owtail) {
				RETURN ( ENXIO );
			}
			thishead = sn_owhead;
			thishead->pid = u.u_procp->p_pid;
			sn_owhead = nexthead;
			sleep((caddr_t)thishead, SNPRI);
		}
okmachp:
		cp->lchnum = ARGS(lchno);
		cp->lmachno = snmachp->lmachno;
		spl0();
		cp->dlchnum = ARGS(dlchno);
		cp->machno = snmachp->machno;
		cp->xchan = (cp->lmachno<<8)|cp->lchnum;
		cp->snunit = unit;
		cp->pdid = NOPDID;
		bp = &cp->input;
		i = cp-sn_chans;
		bp->buf = sn_buffers[i].inbuf;
		bp->len = MAXMSGLEN;
		bp->done = 0;
		bp = &cp->output;
		bp->buf = sn_buffers[i].outbuf;
		bp->len = bp->done = 0;
		cp->flags = 0;
		cp->link = sninfop->snlink[mdev];
		sninfop->snlink[mdev] = cp;

		ARGS(lmachno) = snmachp->lmachno;
		ARGS(dmachno) = snmachp->machno;
#ifndef BSD42
		if (copyout((caddr_t)&args, addr, sizeof(args))) {
			RETURN ( EFAULT );
		}
#endif
		return(0);
	    }

	case NIOPOPEN: {
#ifdef BSD42
 		struct voargs *args = (struct voargs *)addr;
#else
		struct voargs args;

		if (copyin(addr, (caddr_t)&args, sizeof(args))) {
			RETURN ( EFAULT );
		}
#endif
		spl6();
		for(cp=sn_chans; cp<&sn_chans[NUMLCH]; cp++)
			if( cp->lchnum == -1 && cp->dlchnum == -1 )
				break;
		if( cp >= &sn_chans[NUMLCH] ) {
			spl0();
			RETURN(  ENFILE );
		}
		for(dp=sn_chans; dp<&sn_chans[NUMLCH]; dp++)
			if( dp->lchnum != -1 && dp->lmachno == ARGS(lmachno)
			   && dp->lchnum == ARGS(lchno)
			   && dp->snunit == unit ) {
				spl0();
				RETURN(  EEXIST );
			}
		cp->lchnum = ARGS(lchno);
		cp->lmachno = ARGS(lmachno);
		spl0();
		cp->dlchnum = ARGS(dlchno);
		cp->machno = ARGS(dmachno);
		cp->xchan = (cp->lmachno<<8)|cp->lchnum;
		cp->snunit = unit;
		cp->pdid = ARGS(pdid);
		bp = &cp->input;
		i = cp-sn_chans;
		bp->buf = sn_buffers[i].inbuf;
		bp->len = MAXMSGLEN;
		bp->done = 0;
		bp = &cp->output;
		bp->buf = sn_buffers[i].outbuf;
		bp->len = bp->done = 0;
		cp->flags = 0;
		cp->link = sninfop->snlink[mdev];
		sninfop->snlink[mdev] = cp;
		return(0);
	    }

	case NIOCLOSE: {
#ifdef BSD42
 		struct cargs *args = (struct cargs *)addr;
#else
		struct cargs args;

		if (copyin(addr, (caddr_t)&args, sizeof(args))) {
			RETURN ( EFAULT );
		}
#endif
		for(cp=sninfop->snlink[mdev], dp=0; cp; dp=cp, cp=cp->link)
			if( cp->lmachno == ARGS(lmachno)
			    && cp->lchnum == ARGS(lchno) ) {
				/* TIMING !!! */
				if( dp )
					dp->link = cp->link;
				else
					sninfop->snlink[mdev] = cp->link;
				chclose(cp);
				return(0);
			}
		RETURN( EBADF );
		}

	case NIOGET:
	case NIOPUT:
	{
#ifdef BSD42
 		struct gpargs *args = (struct gpargs *)addr;
#else
		struct gpargs args;

		if (copyin(addr, (caddr_t)&args, sizeof(args))) {
			RETURN ( EFAULT );
		}
#endif
		status.lchdir = (cmd == NIOGET ? INPUT : OUTPUT);
		status.lchnum = ARGS(lch);
		ustatus = ARGS(status);
		usaddr = ARGS(addr);
		ulen = ARGS(len);
		for(cp=sninfop->snlink[mdev]; cp; cp=cp->link) {
			if( cp->lchnum == ARGS(lch)
			    && cp->lmachno == ARGS(lmachno)) {
do_getput:
				bp = (cmd == NIOGET ? &cp->input : &cp->output);
				if( cmd == NIOPUT ) {
					copyin(usaddr, bp->buf, ulen);
					spl5();
					bp->len = (ulen+1)/2;
					snxstart(unit, cp, DATA);
					sninfop->snsdata++;
				} else {
					spl5();
					/*
					 * The following may be
					 * unnecessary in light of
					 * the later check.
					 */
					if( bp->done == 0 && cp->flags&SENTRNR ) {
						cp->flags &= ~SENTRNR;
						cp->flags |= SENTRDY;
						snxstart(unit, cp, RDY);
						sninfop->sns1rdy++;
					}
				}
				for (;;) {
					spl5();
					if (bp->done) break;
					sleep((caddr_t)bp->buf, SNPRI);
#ifdef TIMEOUT
					spl6();
					if (cp->flags & RETRY) {
						cp->flags &= ~RETRY;
						sninfop->sntimeout++;
						snxstart(unit, cp, (cp->flags&SENTDATA) ? DATA : RDY);
					}
#endif TIMEOUT
				}
				status.code = ST_OK;
				status.len = 2*bp->len;
				if( cmd == NIOGET ) {
					spl0();
					copyout(bp->buf, usaddr, 2*bp->len);
					spl5();
					bp->len = MAXMSGLEN;
					/*
					 * Believe it or not, we must check
					 * this here.  This is because
					 * we lowered the ipl to do the
					 * copyout().  During that time
					 * another message may have tried
					 * to sneak in.
					 */
					if( cp->flags&SENTRNR ) {
						cp->flags &= ~SENTRNR;
						snxstart(unit, cp, RDY);
						sninfop->snsrdy++;
					}
				} else
					bp->len = 0;
				bp->done = 0;
				spl0();
				error = 0;
				goto cpyout;
			}
		}
		status.code = ST_IARGS;
		error = EIO;
cpyout:
		copyout((caddr_t)&status, (caddr_t)ustatus, sizeof(status));
		RETURN ( error );
	}

	case NIOGETM:
	{
		struct pair pair;
		struct chdat *lchns[MAXMUX+1];
		register struct chdat **chp = lchns, **chpe = lchns;
#ifdef BSD42
 		struct gmargs *args = (struct gmargs *)addr;
#else
		struct gmargs args;

		if (copyin(addr, (caddr_t)&args, sizeof(args))) {
			RETURN ( EFAULT );
		}
#endif
		status.lchdir = INPUT;
		ustatus = ARGS(status);
		usaddr = ARGS(addr);
		ulen = ARGS(len);
		for(i=0; i<MAXMUX; i++) {
			if (copyin((caddr_t)ARGS(pairp++), (caddr_t)&pair,
					sizeof(struct pair))) {
				RETURN ( EFAULT );
			}
			if( pair.lchno == -1 )
				break;
			if (pair.lchno < 0 || pair.lchno >= 512
			    || pair.lmachno < 0 || pair.lmachno >= 512) {
				RETURN ( EBADF );
			}
			for(cp=sninfop->snlink[mdev]; cp; cp=cp->link) {
				if( cp->lchnum == pair.lchno
				    && cp->lmachno == pair.lmachno ) {
					*chpe++ = cp;
					break;
				}
			}
			if( cp >= &sn_chans[NUMLCH] ) {
				RETURN ( EBADF );
			}
		}
		spl5();
		for(chp=lchns; chp<chpe; chp++) {
			if( (*chp)->flags&MULTIPLEX ) {
				spl0();
				RETURN ( EBUSY );
			}
		}
		for(chp=lchns; chp<chpe; chp++)
			(*chp)->flags |= MULTIPLEX;

loop:
		spl5();
		for(chp=lchns; chp<chpe; chp++) {
			if( (*chp)->input.done ) {
				goto gotone;
			}
		}
		sleep((caddr_t)&sn_multiplex, SNPRI);
		goto loop;

gotone:
		cp = *chp;
		for(chp=lchns; chp<chpe; chp++)
			(*chp)->flags &= ~MULTIPLEX;
		spl0();
		status.lchnum = cp->lchnum;
		cmd = NIOGET;
		goto do_getput;
	}

	case NIOABORT:
	case NIORESET: {
		struct device *draddr;
#ifndef USG5
		struct uba_device *ui;
#endif
#ifdef BSD42
 		struct raargs *args = (struct raargs *)addr;
#else
		struct raargs args;

		if (copyin(addr, (caddr_t)&args, sizeof(args))) {
			RETURN ( EFAULT );
		}
#endif

#ifdef USG5
		draddr = sn_addr[unit];
#else
		ui = sndinfo[unit];
		draddr = (struct device *)ui->ui_addr;
#endif
		
		spl5();
		i = ARGS(machno) << 8;
		draddr->drcs = CMD_MODE;
		draddr->drout = (cmd == NIOABORT ? ABORT(i) : RESET(i));
		draddr->drcs = EOP_MODE;
		draddr->drout = 0;
		draddr->drcs = DATA_MODE;
		spl0();
		return(0);
	}

	case NIOREADSTATUS:
#ifdef BSD42
		copyout((caddr_t)sninfop, *(caddr_t *)addr, 
			sizeof sn_sninfo[0]);
#else
		copyout((caddr_t)sninfop, (caddr_t)addr, 
			sizeof sn_sninfo[0]);
#endif
		return(0);

	case NIOCHSTATUS: {

		struct chinfo chinfo;

		chinfo.numlch = NUMLCH;
		chinfo.chanaddr = &sn_chans[0];
#ifdef BSD42
		copyout((caddr_t)&chinfo, *(caddr_t *)addr, 
			sizeof chinfo);
		copyout((caddr_t)chinfo.chanaddr,
			(*(caddr_t *)addr)+sizeof chinfo,
			NUMLCH*sizeof sn_chans[0]);
#else
		copyout((caddr_t)&chinfo, (caddr_t)addr, 
			sizeof chinfo);
		copyout((caddr_t)chinfo.chanaddr, 
			(caddr_t)(addr+sizeof chinfo), 
			NUMLCH*sizeof sn_chans[0]);
#endif
		return(0);
	}

	case NIOSETMACH:
#ifdef BSD42
		copyin(*(caddr_t *)addr, (caddr_t)snmach, sizeof snmach);
#else
		copyin((caddr_t)addr, (caddr_t)snmach, sizeof snmach);
#endif
		return(0);


	case NIOGETMACH:
#ifdef BSD42
		copyout((caddr_t)snmach, *(caddr_t *)addr, sizeof snmach);
#else
		copyout((caddr_t)snmach, (caddr_t)addr, sizeof snmach);
#endif
		return(0);


	case NIOQSTATUS: {
		struct openinfo openinfo;

		openinfo.owaddr = sn_openwait;
		openinfo.head = sn_owhead;
		openinfo.tail = sn_owtail;

#ifdef BSD42
		copyout((caddr_t)&openinfo, *(caddr_t *)addr, sizeof openinfo);
		copyout((caddr_t)sn_openwait, 
			(*(caddr_t *)addr)+sizeof openinfo,
			NUMDEV*sizeof sn_openwait[0]);
#else
		copyout((caddr_t)&openinfo, (caddr_t)addr, sizeof openinfo);
		copyout((caddr_t)sn_openwait, (caddr_t)(addr+sizeof openinfo),
			NUMDEV*sizeof sn_openwait[0]);
#endif
		return(0);
	}

	case NIOZEROSTAT:
	{

		register char *p, *low, *high;

		low = (char *) &sn_sninfo[0];
#ifdef USG5
		high = low + sn_cnt*sizeof (struct sninfo);
#else
		high = low + NSN*sizeof (struct sninfo);
#endif

		for (p=low; p<high; p++)
			*p = '\0';
		return(0);
	}
	case NIOCHECK:
	case NIOWAIT:
	case NIOPURGE:
	case NIOSETVEC:
		;
	}
}

static
chclose(cp)
register struct chdat *cp;
{
	cp->lchnum = cp->dlchnum = cp->machno = -1;
	cp->flags = cp->xchan = cp->input.len = 0;
}

#ifdef TIMEOUT
cktimchk()
{
	register struct chdat *cp;
	register struct sninfo *sninfop;
	int unit;
	int s;

	for(cp=sn_chans; cp < &sn_chans[NUMLCH]; cp++) {
		if( cp->flags&(SENTDATA|SENTRDY) && --cp->ckticks == 0 ) {
			unit = cp->snunit;
			sninfop = &sn_sninfo[unit];
			sninfop->snsched++;
			sninfop->snenqlost[cp->machno]++;
			cp->flags |= RETRY;
			if (cp->flags & SENTDATA) {
				wakeup((caddr_t)cp->output.buf);
			} else {
				wakeup((caddr_t)cp->input.buf);
			}
		}
	}
	s = spl6();
	if (sn_state & OPEN) {
		timeout(cktimchk, (caddr_t)0, CKTICKS);
	} else {
		sn_state &= ~TIMER;
	}
	splx(s);
}
#endif TIMEOUT

#endif

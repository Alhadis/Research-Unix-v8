/*
 * stream driver for DK via KMC-11/KDI board
 */
#include "kdi.h"
#include "kmc.h"
#if NKDI && NKMC

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/stream.h"
#include "../h/ioctl.h"
#include "../h/pte.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../h/ubavar.h"
#include "../h/conf.h"
#include "../h/ttyld.h"
#include "../h/dkstat.h"
#include "../h/dkmod.h"

#define	NKMB	10		/* size of cmd/status buffers */
#define	KDIPRI	28

/* channel states */
#define	CLOSED	0
#define	RCLOSE	1		/* remote gone, local still around */
#define	LCLOSE	2		/* closed locally, not acked yet */
#define	OPEN	3		/* in use */

/*
 * format of UB addresses sent to KMC
 */
struct	kmaddr {
	u_short	hi;
	u_short	lo;
};

/*
 * KMC init packet
 */
struct	kinit {
	struct	kmaddr	cmdaddr;	/* UB addr of cmd buf */
	struct	kmaddr	stataddr;	/* UB addr of statbuf */
	struct	kmaddr	bufaddr;	/* UB addr of KMC workspace */
	struct	kmaddr	csraddr;	/* for DR11C - unused */
};

/*
 * command/status packets
 */
struct kin {				/* KMC command buffer */
	u_char	type;			/* command type */
	u_char	serno;			/* probably seq number */
	u_char	chan;			/* channel number */
	u_char	fill2;			/* probably seq number */
	u_short	len;			/* byte count */
	char	ctl;			/* possible control byte */
	char	mode;			/* command variant */
	struct	kmaddr addr;		/* UB location of buffer */
};

/*
 * Big structure with stuff that needs to be accessible on the unibus
 */
struct kmcdk {
	struct	kinit	kinit;			/* the init packet */
	struct	kin	cmd[NKMB];		/* KMC command buffer */
	struct	kin	stat[NKMB];		/* KMC status buffer */
	char	kmcbuf[16*1024];		/* temp space for KMC */
} k;

struct kdi {
	struct	queue *dkrq;
	struct	block *ibp;
	struct	block *obp;
	u_short	rsize;			/* # bytes in last read */
	char	chan;
	char	ostate;
} kdi[NKDI];


#define	KMBUSY	01
#define	KMSTOP	02
#define	KMBIG	04		/* rcvd lots last time, use big buffer */

/*
 * device structure
 */
struct device {
	char	sts;
	char	x1;
	union {
		struct {
			u_short	lo;
			u_char	hi;
		} a;
		struct {
			u_char	x2;
			u_char	x3;
			u_char	ch;
			u_char	ct;
			u_char	sh;
			u_char	st;
		} q;
	} u;
};

/*
 * KMC commands
 */
#define	KC_INIT		1
#define	KC_SEND		2
#define	KC_RCVB		3
#define	KC_CLOSE	4
#define	KC_XINIT	5
#define	KC_CMD		6
#define	KC_FLAG		7
#define	KC_SOI		8

/*
 * subcommands of KC_CMD or KC_SEND
 */
#define	OFLUSH		02	/* flush output */
#define	OSPND		04	/* suspend output */
#define	ORSME		010	/* resume output */
#define	OBOTM		0200	/* send BOTM trailer, not BOT */

/*
 * KMC reports
 */
#define	KS_SEND		024
#define	KS_RDB		025
#define	KS_EOI		026
#define	KS_CNTL		027
#define	KS_ERR		030

/*
 * KC_RCV modes
 */
#define	CBLOCK		0040	/* return on block boundary */
#define	CTIME		0100	/* return when time expires */

/*
 * KS_RDB mode
 */
#define	SFULL		0001	/* buffer full */
#define	SCNTL		0002	/* cntl char recv */
#define	SABORT		0010	/* rcv aborted */
#define	SBLOCK		0040	/* block boundary */
#define	STIME		0100	/* time limit expired */

/*
 * URP control characters
 */
#define	D_DELAY	0100
#define	D_BREAK	0110

/*
 *  KMC errors
 */
#define	E_NOQB	4		/* internal buffer runout */
#define	E_UMETA	7		/* unknown control character */

/*
 * tracing
 */
#define DEBUG
#ifdef	DEBUG
struct	kin ktrbuf[256];
struct	kin *ktrp = ktrbuf;
#define	TRACE(x) *ktrp++ = x; if (ktrp >= &ktrbuf[256]) ktrp = ktrbuf;
#else
#define	TRACE(x)	;
#endif

int	nodev(), kdiopen(), kdiclose(), kdiput(), kdiisrv();
struct	qinit kdirinit = { nodev, kdiisrv, kdiopen, kdiclose, 0, 0 };
struct	qinit kdiwinit = { kdiput, NULL, kdiopen, kdiclose, 512, 128 };
struct	streamtab kdiinfo = { &kdirinit, &kdiwinit };

extern	u_char	blkdata[];	/* stream IO blocks */
extern	long	blkubad;	/* unibus address of IO blocks */
long	kdiubad;		/* unibus address of device data */
struct	kmaddr kmcaddr();
int	kdiopened;
int	kmcbad;
char	kdistate[NKDI];
struct	dkmodule *kdmodp;
extern	struct dkmodule dkmod[];
extern	struct	uba_device *kmcdinfo[NKMC];
extern	struct	uba_driver kmcdriver;
extern	(*kdirint)();
extern	(*kdixint)();
struct	dkstat	dkstat;

/*
 * open channel
 */
kdiopen(q, dev)
register struct queue *q;
register dev_t dev;
{
	register struct kdi *kp;
	register struct block *bp;
	register mindev = minor(dev);

	if (mindev <= 0 || mindev >= NKDI || kmcbad)
		return(0);
	if (!kdiopened) {
		if (mindev > 1)
			return(0);
		for (kp = &kdi[0]; kp < &kdi[NKDI]; kp++)
			kp->chan = kp - &kdi[0];
		if (kdiinit() == 0)
			return(0);
		for (kdmodp = dkmod; ;kdmodp++) {
			if (kdmodp->dev==(dev_t)0 || kdmodp->dev==major(dev)) {
				kdmodp->dev = major(dev);
				kdmodp->dkstate = kdistate;
				kdmodp->nchan = NKDI;
				break;
			}
		}
		kdiopened++;
	}
	kp = &kdi[mindev];
	if (kdmodp->dkstate[mindev] != CLOSED) {
		if (mindev&01 && mindev>1) /* outgoing channels cannot reopen */
			return(0);
		if (kdmodp->dkstate[mindev] != OPEN)
			return(0);	/* closing channels cannot reopen */
		return(1);
	}
	kp->dkrq = q;
	q->ptr = (caddr_t)kp;
	WR(q)->flag |= QDELIM|QBIGB;
	q->flag |= QDELIM;
	WR(q)->ptr = (caddr_t)kp;
	kdmodp->dkstate[mindev] = OPEN;
	kp->ostate = 0;
	kcmd(KC_INIT, kp->chan, 0, 0, 0, (caddr_t)0);
	if ((bp = allocb(64)) == NULL) {
		kdmodp->dkstate[mindev] = 0;
		return(0);
	}
	kp->ibp = bp;
	kcmd(KC_RCVB, mindev, bp->lim-bp->wptr, 50,
	     CBLOCK|CTIME, (caddr_t)bp->wptr);
	return(1);
}

kdiclose(q)
register struct queue *q;
{
	register struct kdi *kp = (struct kdi *)q->ptr;
	register s, i;
	register struct block *bp;

	if (kdmodp->dkstate[kp->chan]==RCLOSE || kdmodp->listnrq==NULL)
		kdmodp->dkstate[kp->chan] = CLOSED;
	else if (kdmodp->dkstate[kp->chan] == OPEN) {
		kdmodp->dkstate[kp->chan] = LCLOSE;
		for (i=0; i<30 && (WR(q)->count || kp->obp); i++)
			tsleep((caddr_t)kp, KDIPRI, 1);
	}
	kp->dkrq = NULL;
	if (kdmodp->listnrq)
		putctl1(RD(kdmodp->listnrq), M_CLOSE, kp->chan);
	s = spl6();
	if (kp->obp) {
		freeb(kp->obp);
		kp->obp = NULL;
	}
	splx(s);
	kcmd(KC_CLOSE, kp->chan, 0, 0, 0, (caddr_t)NULL);
}

kdiput(q, bp)
register struct queue *q;
register struct block *bp;
{
	register struct kdi *kdp = (struct kdi *)q->ptr;
	register union stmsg *sp;
	register s;

	switch(bp->type) {

	case M_DATA:
	case M_DELIM:
	case M_DELAY:
	case M_BREAK:
		if (kdmodp->dkstate[kdp->chan] < LCLOSE) {
			freeb(bp);
			return;
		}
		putq(q, bp);
		if ((kdp->ostate&KMBUSY)==0)
			kmstart(kdp);
		return;

	case M_CLOSE:
		s = *bp->rptr;
		if (s < NKDI) {
			flushq(q, 1);
			if (kdmodp->dkstate[s] == OPEN) {
				kdmodp->dkstate[s] = RCLOSE;
				putctl(kdi[s].dkrq->next, M_HANGUP);
			} else if (kdmodp->dkstate[s] == LCLOSE)
				kdmodp->dkstate[s] = CLOSED;
			kcmd(KC_CLOSE, s, 0, 0, 0, (caddr_t)NULL);
		}
		freeb(bp);
		return;

	case M_IOCTL:
		sp = (union stmsg *)bp->rptr;
		bp->type = M_IOCACK;
		switch (sp->ioc0.com) {

		case TIOCSETP:
		case TIOCSETN:
			if (sp->ioc1.sb.sg_ispeed == 0)
				putctl1(q, M_CLOSE, kdp->chan);
		case KIOCISURP:
			bp->wptr = bp->rptr;
			break;

		case TIOCGETP:
			sp->ioc1.sb.sg_ispeed =
			  sp->ioc1.sb.sg_ospeed = B9600;
			break;

		case KIOCINIT:
			kcmd(KC_XINIT, kdp->chan, 0, 0, 0, (caddr_t)NULL);
			bp->rptr = bp->wptr;
			break;

		case KIOCSHUT:
			if (kdp->chan > 1) {
				bp->type = M_IOCNAK;
				break;
			}
			kdireset();
			bp->rptr = bp->wptr;
			break;

		case DIOCSTREAM:
			RD(q)->flag &= ~QDELIM;
			bp->rptr = bp->wptr;
			break;

		case DIOCRECORD:
			RD(q)->flag |= QDELIM;
			bp->rptr = bp->wptr;
			break;

		default:
			bp->wptr = bp->rptr;
			bp->type = M_IOCNAK;
			break;
		}
		qreply(q, bp);
		return;

	case M_STOP:
		s = OSPND;
		kdp->ostate |= KMSTOP;
		goto dontcmd;

	case M_FLUSH:
		flushq(q, 0);
		s = spl6();
		if (kdp->obp) {
			freeb(kdp->obp);
			kdp->obp = 0;
		}
		kdp->ostate &= ~(KMSTOP|KMBUSY);
		splx(s);
		s = OFLUSH|ORSME;
		goto docmd;

	case M_START:
	case M_HANGUP:
		s = ORSME;
		kdp->ostate &= ~KMSTOP;
		goto dontcmd;
	docmd:
		kcmd(KC_CMD, kdp->chan, 0, s, 0, (caddr_t)NULL);
	dontcmd:
		kmstart(kdp);
		break;

	default:		/* not handled; just toss */
		break;
	}
	freeb(bp);
}

kdiisrv(q)
register struct queue *q;
{
	register struct kdi *kp = (struct kdi *)q->ptr;

	if ((q->next->flag&QFULL)==0 && kp->ibp==NULL) {
		register struct block *bp = allocb(kp->ostate&KMBIG? 1024:64);
		if (bp == NULL)
			panic("kdi: can't alloc");
		kp->ibp = bp;
		kcmd(KC_RCVB, kp->chan, bp->lim - bp->wptr,
		     50, CBLOCK|CTIME, (caddr_t)bp->wptr);
	}
}

kmstart(kdp)
register struct kdi *kdp;
{
	register s = spl5();
	register struct block *bp;
	register struct queue *qp;
	register c;
	register struct block *xbp;

	if (kdp->ostate&(KMBUSY|KMSTOP)) {
		splx(s);
		return;
	}
	if ((qp = kdp->dkrq)==NULL || (qp = WR(qp))==NULL) {
		splx(s);
		return;
	}
	if (bp = getq(qp)) switch (bp->type) {

	case M_DELIM:
		kcmd(KC_SEND, kdp->chan, 0,
		     D_DELAY, 0, (caddr_t)bp->rptr);
		kdp->ostate |= KMBUSY;
		kdp->obp = bp;
		break;

	case M_DATA:
		c = OBOTM;
		if (xbp = qp->first) {
			if (xbp->type==M_DELIM) {
				if (xbp = getq(qp))
					freeb(xbp);
				c = 0;
			} 
		} else {	/* wait for delim or data */
			putbq(qp, bp);
			splx(s);
			return;
		}
		kcmd(KC_SEND, kdp->chan, bp->wptr-bp->rptr,
		     0, c, (caddr_t)bp->rptr);
		kdp->ostate |= KMBUSY;
		dkstat.output += bp->wptr - bp->rptr;
		kdp->obp = bp;
		break;

	case M_DELAY:
		c = D_DELAY;
		while (*bp->rptr>0) {
			c++;
			*bp->rptr >>= 1;
		}
		kcmd(KC_SEND, kdp->chan, 1, c, 0, (caddr_t)bp->rptr);
		kdp->ostate |= KMBUSY;
		kdp->obp = bp;
		break;

	case M_BREAK:
		kcmd(KC_SEND, kdp->chan, 0, D_BREAK, 0, (caddr_t)bp->rptr);
		kdp->ostate |= KMBUSY;
		kdp->obp = bp;
		break;

	default:
		printf("mesg %o in kdi\n", bp->type);
		freeb(bp);
		break;
	}
	splx(s);
}

kcmd(type, chan, len, ctl, mode, addr)
caddr_t addr;
{
	register i;
	register struct device *dp = (struct device *)kmcdinfo[0]->ui_addr;
	register struct kin *kp;
	register s = spl5();
	static struct kmaddr nulladr = {0, 0};
	static serno;

	i = dp->u.q.ch;
	if (i >= NKMB)
		panic("cmd hd %d in kcmd\n", i);
	kp = &k.cmd[i];
	kp->type = type;
	kp->serno = ++serno;
	kp->chan = chan;
	kp->len = len;
	kp->ctl = ctl;
	kp->mode = mode;
	if (addr)
		kp->addr = kmcaddr(addr, (caddr_t)blkdata, blkubad);
	else
		kp->addr = nulladr;
	i++;
	if (i >= NKMB)
		i = 0;
	dp->u.q.ch = i;
	TRACE(*kp);
	splx(s);
}

/*
 * Interrupt routine-- unload status buffer:
 * release write blocks, collect input
 */
kdiintr()
{
	register struct device *dp = (struct device *)(kmcdinfo[0]->ui_addr);
	register struct kin *sp;
	register struct kdi *kp;
	register struct block *bp;
	register c;

	for ( ; (c = dp->u.q.st) != dp->u.q.sh; dp->u.q.st = c) {
		if (c >= NKMB)
			panic("kdi stat buf is %d\n", c);
		sp = &k.stat[c];
		TRACE(*sp);
		c++;
		if (c >= NKMB)
			c = 0;
		if (sp->chan >= NKDI) {
			printf("kdi stat chan is 0%o\n", sp->chan);
			printf("type: %o len: 0%o mode: 0%o\n");
			continue;
		}
		kp = &kdi[sp->chan];
		switch (sp->type) {

		case KS_SEND:
			if (kp->obp) {
				freeb(kp->obp);
				kp->obp = NULL;
			}
			kp->ostate &= ~KMBUSY;
			if (kp->dkrq && WR(kp->dkrq)->first)
				kmstart(kp);
			break;

		case KS_RDB:
			bp = kp->ibp;
			kp->ibp = NULL;
			if (kp->dkrq==NULL || kdmodp->dkstate[kp->chan]!=OPEN) {
				if (bp)
					freeb(bp);
				break;
			}
			if (sp->mode & SABORT) {
				printf("kdi rcv abort chan %d mode %o bp %x\n",
				  kp->chan, sp->mode, bp);
				if (bp)
					kp->ibp = bp;
				kp->rsize = 0;
				kp->ostate &= ~KMBIG;
				break;
			}
			if (bp == NULL) {
				printf("kdi intr: no ibp\n");
				break;;
			}
			/* special hacks */
			if (sp->mode&020
			  || sp->mode==0100 && bp->wptr+sp->len == bp->lim) {
				printf("I");
				freeb(bp);
			} else {
				bp->wptr = bp->lim - sp->len;
				kp->rsize += bp->wptr - bp->rptr;
				dkstat.input += bp->wptr - bp->rptr;
				(*kp->dkrq->next->qinfo->putp)
				       (kp->dkrq->next, bp);
				kp->ibp = NULL;
				if(sp->mode&(SBLOCK|STIME)) {
					putctl(kp->dkrq->next, M_DELIM);
					kp->ostate &= ~KMBIG;
					if (kp->rsize >= 512)
						kp->ostate |= KMBIG;
					kp->rsize = 0;
				}
				if (sp->mode&SCNTL) {
					switch (sp->ctl) {

					case D_BREAK:
						putctl(kp->dkrq->next, M_BREAK);
						break;

					}
				}
			}
			if ((kp->dkrq->next->flag&QFULL) == 0) {
				bp = allocb(kp->ostate&KMBIG? 1024:64);
				if (bp == NULL)
					panic("kdi: can't alloc");
				kp->ibp = bp;
				kcmd(KC_RCVB, kp->chan, bp->lim - bp->wptr,
				     50, CBLOCK|CTIME, (caddr_t)bp->wptr);
			}
			break;
		
		case KS_EOI:
			printf("kdi chan %d rcv EOI %x\n", sp->chan, sp->len);
			break;

		case KS_CNTL:
			/*
			printf("kdi chan %d rcv ctl %x\n", sp->chan, sp->len);
			*/
			break;

		case KS_ERR:
			printf("kdi chan %d error %d\n", sp->chan, sp->len);
			if (sp->len==E_NOQB || sp->len==E_UMETA) {
				printf("ignored\n");
				break;
			}
			kdireset();
			break;

		}
	}
}

kdiinit()
{
	register struct device *kp = (struct device *)kmcdinfo[0]->ui_addr;
	register i;
	struct kmaddr ka;
	static time_t kditime;
	extern time_t time;

	/* close all open channels? */
	if (kditime+30 > time)
		return(0);
	kp->sts = 0;			/* initialize KMC */
	if (kdiubad == 0) {
		kdiubad = uballoc(kmcdinfo[0]->ui_ubanum,
				   (caddr_t)&k, sizeof(k), 0);
		if (kdiubad == 0)
			panic("UB AD 0 in kdiinit");
	}
	k.kinit.cmdaddr = kmcaddr((caddr_t)k.cmd, (caddr_t)&k, kdiubad);
	k.kinit.stataddr = kmcaddr((caddr_t)k.stat, (caddr_t)&k, kdiubad);
	k.kinit.bufaddr = kmcaddr((caddr_t)k.kmcbuf, (caddr_t)&k, kdiubad);
	ka = kmcaddr((caddr_t)&k.kinit, (caddr_t)&k, kdiubad);
	kp->u.a.lo = ka.lo;
	kp->u.a.hi = ka.hi;
	kditime = time;
	kp->sts = 1;			/* start handshake */
	for (i = 0; i<100000; i++) {
		if (kp->sts == 2) {
			kdirint = kdiintr;
			kdixint = kdiintr;
			return(1);			/* KMC is running OK */
		}
		if (kp->sts == 3) {
			printf("KMC entered state 3\n");
			return(0);
		}
	}
	printf("KMC did not come ready\n");
	return(0);
}

struct kmaddr
kmcaddr(memaddr, membase, ubbase)
caddr_t memaddr, membase, ubbase;
{
	register struct kmaddr r;
	register long a;

	a = (long)memaddr - (long)membase + ((long)ubbase & 0x03ffff);
	r.hi = a >> 16;
	r.lo = (u_short)a;
	return(r);
}

/*
 * kmc reload
 */
kdireload()
{
	kdiopened = 0;
	kdmodp = 0;
	kmcbad = 0;
}

/*
 * zap all open channels
 */
kdireset()
{
	register struct kdi *kp;

	if (kdmodp==0)
		return;
	for (kp = kdi; kp < &kdi[NKDI]; kp++) {
		register state = kdmodp->dkstate[kp->chan];
		if ((state==OPEN || state==LCLOSE) && kp->dkrq) {
			flushq(WR(kp->dkrq), 1);
			kdmodp->dkstate[kp->chan] = state==OPEN? RCLOSE: CLOSED;
			putctl(kp->dkrq->next, M_HANGUP);
		}
	}
	kmcbad = 1;
}

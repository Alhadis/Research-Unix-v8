/*
 *  Raw line discipline and listener stuff for talking to controller
 */
#include "cm.h"
#if NCM
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/stream.h"
#include "../h/ioctl.h"
#include "../h/pte.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../h/ubavar.h"
#include "../h/conf.h"
#include "../h/dkstat.h"
#include "../h/dkmod.h"
#include "sparam.h"
#include "dk.h"
#include "kdi.h"
#include "kmc.h"

/* channel states */
#define	CLOSED	0
#define	RCLOSE	1
#define	LCLOSE	2
#define	OPEN	3

/*
 * CMC/Listener message structure
 */
struct dialin {
	char	type;
	char	srv;
	short	param0;
	short	param1;
	short	param2;
	short	param3;
	short	param4;
	short	param5;
};

struct dialout {
	short	chan;
	struct	dialin d;
};

/*
 * Message codes
 */
#define	T_LSTNR	4
#define	T_CHG	3
#define	T_REPLY	10
#define	D_CLOSE	1
#define	D_ISCLOSED 2

/*
 * Raw-mode line discipline for DK (only)
 */
int	nulldev(), rdkopen(), rdkclose(), rdkiput(), rdkoput(), rdkosrv();
static	struct qinit rdkrinit = { rdkiput, NULL, rdkopen, rdkclose, 72, 36};
static	struct qinit rdkwinit = { rdkoput, rdkosrv, rdkopen, nulldev, 72, 36};
struct	streamtab rdkinfo = { &rdkrinit, &rdkwinit };

extern	struct	dkstat dkstat;
#define	NDKMOD	(NDK+NKMC)	/* number of different DK devices */
struct	dkmodule dkmod[NDKMOD];

#define	NDKRAW	32
struct	rdk {
	struct	queue *q;
	struct	dkmodule *module;
	short	chan;
} rdk[NDKRAW];

rdkopen(q, dev)
register struct queue *q;
{
	static timer_on = 0;
	register struct rdk *dkp, *edkp;
	register struct dkmodule *dkm;

	edkp = NULL;
	for (dkp=rdk; ; dkp++) {
		if (dkp >= &rdk[NDKRAW])
			break;
		if (dkp->q == q) {
			if (dkp->chan != minor(dev) || WR(q)->ptr==NULL) {
				printf("q %x dkp %x\n", q, dkp);
				panic("rdkopen botch");
			}
			return(1);
		}
		if (dkp->q == NULL && edkp==NULL)
			edkp = dkp;
	}
	if (edkp==NULL)
		return(0);
	for (dkm=dkmod; ; dkm++) {
		if (dkm >= &dkmod[NDKMOD])
			return(0);
		if (major(dev) == dkm->dev)
			break;
	}
	edkp->q = q;
	edkp->chan = minor(dev);
	edkp->module = dkm;
	q->flag |= QDELIM;
	q->ptr = 0;
	WR(q)->ptr = (caddr_t)edkp;
	if (timer_on==0) {
		dktimer();
		timer_on = 1;
	}
	return(1);
}

rdkclose(q)
register struct queue *q;
{
	register i;
	register struct block *bp;
	register struct rdk *dkp = (struct rdk *)WR(q)->ptr;

if (dkp==NULL) printf("null ptr; q %x wq %x\n", q, WR(q));
	if (WR(q)==dkp->module->listnrq)
		dkp->module->listnrq = NULL;
	while (bp = getq(q))
		(*q->next->qinfo->putp)(q->next, bp);
	dkp->q = NULL;
	dkp->module = NULL;
}

/*
 * output put procedure.
 */
rdkoput(q, bp)
register struct queue *q;
register struct block *bp;
{
	register struct block *bp1;
	register struct ioctl1 {
		int	com;
		int	stuff;
	} *iop;
	register struct rdk *dkp = (struct rdk *)q->ptr;
	register struct dkmodule *modp = dkp->module;
	register i;

	switch (bp->type) {

	case M_IOCTL:
		switch (((struct ioctl1 *)bp->rptr)->com) {

		/* hang everybody up */
		case DIOCHUP:
			/* must be on channel 1 or 0 */
			if (dkp->chan <= 1) {
				dkmesg(modp, T_LSTNR, 0, 0, 0, 1);
				bp->type = M_IOCACK;
				bp->rptr = bp->wptr;
				qreply(q, bp);
				return;
			}
			break;

		/* announce listener channel */
		case DIOCLHN:
			if (modp->listnrq == NULL) {
				modp->listnrq = q;
				dkmesg(modp, T_LSTNR, 0, 0, 0, 0);
				bp->type = M_IOCACK;
				bp->rptr = bp->wptr;
				qreply(q, bp);
				return;
			}
			break;

		/* delay input */
		case DIOCSTOP:
			RD(q)->ptr = (caddr_t)1;
			bp->type = M_IOCACK;
			bp->rptr = bp->wptr;
			qreply(q, bp);
			return;

		/* suggest a free outgoing channel */
		case DIOCCHAN:
			for (i=3; i<modp->nchan; i+=2) {
				if (modp->dkstate[i]==CLOSED) {
					((struct ioctl1 *)bp->rptr)->stuff = i;
					bp->wptr =
					     bp->rptr+sizeof(struct ioctl1);
					bp->type = M_IOCACK;
					break;
				}
			}
			qreply(q, bp);
			return;

		default:
			(*q->next->qinfo->putp)(q->next, bp);
			return;
		}
		bp->type = M_IOCNAK;
		bp->wptr = bp->rptr;
		qreply(q, bp);
		return;
	}
	putq(q, bp);
}

rdkosrv(q)
register struct queue *q;
{
	register struct block *bp;

	while ((q->next->flag&QFULL)==0 && (bp = getq(q)))
		(*q->next->qinfo->putp)(q->next, bp);
}

/*
 * input put procedure
 *   -- ignores all control bytes
 */
rdkiput(q, bp)
register struct queue *q;
register struct block *bp;
{
	register struct rdk *dkp = (struct rdk *)WR(q)->ptr;

	switch (bp->type) {

	case M_DATA:
		if (WR(q)==dkp->module->listnrq && dklstnr(dkp->module, bp))
			return;
		if (q->next->flag&QFULL) { /* sorry, you lose */
			freeb(bp);
			return;
		}
		if (q->ptr) {	/* input delayed */
			putq(q, bp);
			qpctl(q, M_DELIM);
			return;
		}
		(*q->next->qinfo->putp)(q->next, bp);
		putctl(q->next, M_DELIM);
		return;

	case M_IOCACK:
	case M_IOCNAK:
	case M_HANGUP:
		(*q->next->qinfo->putp)(q->next, bp);
		return;

	case M_CLOSE:
		if (dkp->module->dkstate[*bp->rptr] == RCLOSE)
			dkmesg(dkp->module, T_CHG, D_ISCLOSED, 0, *bp->rptr, 0);
		else
			dkmesg(dkp->module, T_CHG, D_CLOSE, 0, *bp->rptr, 0);
		freeb(bp);
		return;

	default:
		freeb(bp);
	}
}

/*
 * listener sends a message to CMC
 */
dkmesg(modp, type, srv, p0, p1, p2)
register struct dkmodule *modp;
{
	register struct dialout *dp;
	register struct block *bp;

	if (modp->listnrq==NULL || modp->listnrq->next->flag&QFULL)
		return;
	if ((bp = allocb(sizeof(struct dialout))) == NULL)
		return;		/* hope it will be retried later */
	dp = (struct dialout *)bp->wptr;
	dp->chan = 1;		/* channel 1 builtin */
	dp->d.type = type;
	dp->d.srv = srv;
	dp->d.param0 = p0;
	dp->d.param1 = p1;
	dp->d.param2 = p2;
	bp->wptr += sizeof(struct dialout);
	(*modp->listnrq->next->qinfo->putp)(modp->listnrq->next, bp);
	putctl(modp->listnrq->next, M_DELIM);
}

/*
 * Receive message for listener
 */
dklstnr(modp, bp)
register struct block *bp;
struct dkmodule *modp;
{
	register struct dialin *dialp;
	register i;
	register struct queue *listnrq = modp->listnrq;
	register struct rdk *dkp;

	dialp = (struct dialin *)bp->rptr;
	switch (dialp->type) {

	case T_CHG:
		i = dialp->param1;
		if (i <= 0 || i >= modp->nchan) {
			dkstat.chgstrange++;
			if (dialp->srv)
				dkmesg(modp, T_CHG, D_ISCLOSED, 0, i, 0);
			freeb(bp);
			return(1);
		}
		switch (dialp->srv) {

		case D_CLOSE:		/* remote shutdown */
			switch (modp->dkstate[i]) {

			case RCLOSE:
				dkstat.notclosed++;
			case OPEN:
				putctl1(listnrq->next, M_CLOSE, i);
				break;

			case LCLOSE:
			case CLOSED:
				dkmesg(modp, T_CHG, D_ISCLOSED, 0, i, 0);
				putctl1(listnrq->next, M_CLOSE, i);
				break;
			}
			break;
		
		case D_ISCLOSED:
			switch (modp->dkstate[i]) {

			case LCLOSE:
			case CLOSED:
				putctl1(listnrq->next, M_CLOSE, i);
				break;

			case OPEN:
			case RCLOSE:
				dkstat.isclosed++;
				break;
			}
			break;
		}
		freeb(bp);
		return(1);

	case T_REPLY:	/* CMC sends reply; find, and hand to process */
		i = dialp->param0;
		if (i < 0 || i >= modp->nchan)
			return(0);
		for (dkp=rdk; dkp<&rdk[NDKRAW]; dkp++) {
			if (dkp->module==modp && dkp->chan==i) {
				(*dkp->q->next->qinfo->putp)(dkp->q->next, bp);
				putctl(dkp->q->next, M_DELIM);
				return(1);
			}
		}
		return(0);

	default:
		return(0);
	}
}

/*
 * 15-second timer
 */
dktimer()
{
	register i;
	register struct dkmodule *dkp;

	for (dkp = dkmod; dkp < &dkmod[NDKMOD]; dkp++) {
		if (dkp->listnrq) {
			dkmesg(dkp, T_LSTNR, 0, 0, 0, 0);
			for (i=0; i< dkp->nchan; i++)
				if (dkp->dkstate[i] == LCLOSE)
					dkmesg(dkp, T_CHG, D_CLOSE, 0, i, 0);
		}
	}
	timeout(dktimer, (caddr_t)NULL, 15*hz);
}
#endif

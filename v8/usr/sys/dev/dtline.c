/*
 * Line discipline for Datakit TDK protocol
 */
#define	NDT	16
#define	TR(x)	/**/

#include "../h/param.h"
#include "../h/stream.h"
#include "../h/conf.h"
#include "../h/ioctl.h"
#include "../h/ttyld.h"

struct dt {
	char	nack;
	char	cack;
	char	nchar;
	char	state;
	short	lastecho;
	struct	queue *queue;
} dt[NDT];

/* states */
#define	CLOG1	01
#define	CLOG2	02
#define	STOP	04
#define USE	010

/* DK control packet codes */
#define	D_BREAK	07
#define	D_ECHO	010
#define	D_ACK	020
#define	D_DELAY	030
#define	D_SNDBRK 03
#define DKDATA	0400

#define	DTNC	32		/* chars sent between ECHO requests */

int dtliput(), dtlisrv(), dtlopen(), dtlclose(), putq(), dtlosrv();
static struct qinit dtlrinit = {dtliput, dtlisrv, dtlopen, dtlclose, 129, 129};
static struct qinit dtlwinit = {putq, dtlosrv, dtlopen, dtlclose, 129, 65};
struct streamtab  dtlinfo = {&dtlrinit, &dtlwinit};

/*
 * open
 */
dtlopen(q, dev)
register struct queue *q;
{
	register struct dt *dp;
	extern int dtltimer(), hz;
	static timer_on = 0;

	if (timer_on==0) {
		timeout(dtltimer, (caddr_t)NULL, 10*hz);
		timer_on = 1;
	}
	dp = (struct dt *)q->ptr;
	if (dp)
		return(1);
	for (dp = dt; dp < &dt[NDT]; dp++) {
		if ((dp->state&USE)==0) {
			dp->state = USE;
			dp->lastecho = 0;
			dp->cack = 0;
			dp->nack = 0;
			q->ptr = (caddr_t)dp;
			WR(q)->ptr = (caddr_t)dp;
			dp->queue = WR(q);
			q->flag &= ~QNOENB;
			return(1);
		}
	}
	return(0);
}

dtlclose(q)
register struct queue *q;
{
	register struct dt *dp = (struct dt *)q->ptr;

	dp->state = 0;
}


/*
 * output server
 */
dtlosrv(q)
register struct queue *q;
{
	register struct block *bp;
	register struct dt *dp;
	register c;

	dp = (struct dt *)q->ptr;
	for (;;) {
		if (q->next->flag & QFULL)	/* unlikely for DK */
			return;
		if ((bp = getq(q)) == NULL)
			return;
		if (dp->state & (STOP|CLOG1)) {
			switch (bp->type) {

			case M_DATA:
			case M_BREAK:
			case M_DELAY:
				putbq(q, bp);
				q->flag |= QWANTR;
				return;
			}
		}
		switch (bp->type) {

		case M_STOP:
			freeb(bp);
			dp->state |= STOP;
			q->flag |= QWANTR;
			continue;		/* in case there is a START */

		case M_START:
			freeb(bp);
			dp->state &= ~STOP;
			continue;

		case M_FLUSH:
			flushq(q, 0);
			freeb(bp);
			dp->state &= ~STOP;
			continue;

		case M_DATA:
			if (bp->wptr - bp->rptr <= DTNC - dp->nchar) {
				TR(bp->wptr-bp->rptr);
				dp->nchar += bp->wptr - bp->rptr;
				(*q->next->qinfo->putp)(q->next, bp);
			} else if (dp->nchar < DTNC) {
				register struct block *bp1;
				bp1 = allocb(DTNC);
				if (bp1==NULL) {
					putbq(q, bp);
					dp->state |= CLOG1;
					return;
				}
				bcopy(bp->rptr, bp1->wptr, DTNC-dp->nchar);
				bp->rptr += DTNC-dp->nchar;
				bp1->wptr += DTNC-dp->nchar;
				dp->nchar = DTNC;
				TR(bp1->wptr-bp1->rptr);
				(*q->next->qinfo->putp)(q->next, bp1);
				putbq(q, bp);
			}
			break;

		case M_BREAK:
			bp->type = M_CTL;
			*bp->wptr++ = D_SNDBRK;
			(*q->next->qinfo->putp)(q->next, bp);
			dp->nchar++;
			break;

		case M_DELAY:
			c = *bp->rptr;
			*bp->rptr = D_DELAY;
			bp->type = M_CTL;
			while (c) {
				(*bp->rptr)++;
				c >>= 1;
			}
			TR(0377);
			(*q->next->qinfo->putp)(q->next, bp);
			dp->nchar++;
			break;

		case M_DELIM:
			freeb(bp);
			continue;

		case M_IOCTL:
			switch (((union stmsg *)bp->rptr)->ioc0.com) {

			case TIOCSETP:
			case TIOCSETN:
				bp->wptr = bp->rptr;
			case TIOCGETP:
				bp->type = M_IOCACK;
				qreply(q, bp);
				continue;
			}
			/* flow through to */
		default:
			dp->nchar += bp->wptr - bp->rptr;
			(*q->next->qinfo->putp)(q->next, bp);
			break;
		}
		if (dp->nchar >= DTNC) {
			if ((bp = allocb(1)) == NULL) {
				dp->state |= CLOG1;
				return;
			}
			dp->nchar -= DTNC;
			dp->nack++;
			dp->nack &= 03;
			if (dp->cack == ((dp->nack+2) & 03))
				dp->state |= CLOG1;
			TR(0200+dp->nack+(((dp->state&CLOG1)!=0)<<5));
			bp->type = M_CTL;
			*bp->wptr++ = D_ECHO + dp->nack;
			(*q->next->qinfo->putp)(q->next, bp);
		}
	}
}

/*
 * put procedure for input.
 */
dtliput(q, bp)
register struct queue *q;
register struct block *bp;
{
	register struct dt *dp;
	register c;

	dp = (struct dt *)q->ptr;
	switch (bp->type) {

	case M_CTL:
		bp->type = M_DATA;
		c = *bp->rptr++;
		if ((c & 0370) == D_ACK) {	/* resp to my ECHO */
			dp->cack = c&03;
			if (dp->state & CLOG1) {
				dp->state &= ~(CLOG1|CLOG2);
				qenable(WR(q));
			}
			TR(0300+dp->cack);
		} else if ((c & 0370) == D_ECHO) {	/* request for an ACK */
			dp->lastecho = D_ACK + (c&07);
			qenable(q);
		} else if (c == D_BREAK) {
			flushq(q, 0);
			putctl(q->next, M_BREAK);
		} else
			printf("DT unkn ctrl %o\n", c);
		break;

	case M_DATA:
		break;

	case M_IOCACK:
	case M_IOCNAK:
	case M_HANGUP:
		(*q->next->qinfo->putp)(q->next, bp);
		return;

	default:
		printf("unknown msg type %d in dtliput\n", bp->type);
		freeb(bp);
		return;
	}
	if (bp->wptr > bp->rptr && (q->flag&QFULL)==0)
		putq(q, bp);
	else
		freeb(bp);
}

/*
 * DT input server
 *  -- copy data; send out latest ack if next queue is not full.
 */
dtlisrv(q)
register struct queue *q;
{
	register struct block *bp;
	register struct dt *dp;
	register s;

	while ((q->next->flag & QFULL) == 0) {
		bp = getq(q);
		if (bp == NULL) {
			dp = (struct dt *)q->ptr;
			s = spl5();
			if (dp->lastecho) {
				putctl1(WR(q)->next, M_CTL, dp->lastecho);
				dp->lastecho = 0;
			}
			splx(s);
			return;
		}
		(*q->next->qinfo->putp)(q->next, bp);
	}
}

dtltimer()
{
	register struct dt *dp;

	for (dp = dt; dp < &dt[NDT]; dp++) {
		if ((dp->state&(USE|CLOG1)) == (USE|CLOG1)) {
			if (dp->state&CLOG2) {
				register struct queue *q = dp->queue;
				putctl1(q->next, M_CTL, D_ECHO + dp->nack);
				TR(0340+dp->nack);
			}
			dp->state |= CLOG2;
		}
	}
	timeout(dtltimer, (caddr_t)NULL, 10*hz);
}

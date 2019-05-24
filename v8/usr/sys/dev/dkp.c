/*
 * New Datakit protocol
 */

#include "../h/param.h"
#include "../h/stream.h"
#include "../h/conf.h"
#include "../h/ioctl.h"
#include "../h/dkstat.h"
#include "../h/ttyld.h"
#include "dkp.h"

#if 	NDKP>0
#ifdef	CAREFUL
#define	TRC(c)	*dkptrp++ = c; if (dkptrp>=&dkptrb[1024]) dkptrp=dkptrb
char	dkptrb[1024]; char *dkptrp = dkptrb;
#else
#define	TRC(c)
#endif

struct dkp {
	struct	queue	*rdq;	/* associated read queue */
	struct	block	*inp;	/* msg being collected */
	struct	block	*inpe;	/*  end of msg */
	short	state;		/* flags */
	short	trx;		/* # bytes in trailer being collected */
	short	indata;		/* # bytes in message being collected */
	u_char	iseq;		/* last good input sequence number */
	u_char	lastecho;	/* last echo/rej sent */
	char	WS;		/* first non-consumed message */
	char	WACK;		/* first non-acknowledged message */
	char	WNX;		/* next message to be sent */
	u_char	XW;		/* size of xmit window */
	u_char	timer;		/* timeout for xmit */
	u_char	outcnt;		/* count output chars for char mode */
	u_char	trbuf[3];	/* trailer being collected */
	struct	block *xb[8];	/* the xmit window buffer */
};

extern	struct	dkstat dkstat;

/*
 *  Protocol control bytes
 */
#define	SEQ	0010		/* sequence number, ends trailers */
#undef	ECHO
#define	ECHO	0020		/* echos, data given to next queue */
#define	REJ	0030		/* rejections, transmission error */
#define	ACK	0040		/* acknowledgments */
#define	BOT	0050		/* beginning of trailer */
#define	BOTM	0051		/* beginning of trailer, more data follows */
#define	BOTS	0052		/* seq update algorithm on this trailer */
#define	SOU	0053		/* start of unsequenced trailer */
#define	EOU	0054		/* end of unsequenced trailer */
#define	ENQ	0055		/* xmitter requests flow/error status */
#define	CHECK	0056		/* xmitter requests error status */
#define	INITREQ 0057		/* request initialization */
#define	INIT0	0060		/* disable trailer processing */
#define	INIT1	0061		/* enable trailer procesing */
#define	AINIT	0062		/* response to INIT0/INIT1 */
#undef	DELAY
#define	DELAY	0100		/* real-time printing delay */
#define	BREAK	0110		/* Send/receive break (new style) */

#define	OPEN	01
#define	LCLOSE	02
#define	RCLOSE	04
#define	XCHARMODE 010
#define	OPENING	020
#define	RJING	040
#define	STOPPED	0100
#define	RCHARMODE 0200

#define	DKPPRI	28
#define	DKPTIME	2

struct	dkp	dkp[NDKP];

int	dkpiput(), dkpisrv(), dkpoput(), dkposrv(), dkpopen(), cdkpopen();
int	dkpclose();
static	struct qinit cdkprinit = { dkpiput,dkpisrv,cdkpopen,dkpclose,512,64 };
static	struct qinit dkprinit = { dkpiput,dkpisrv,dkpopen,dkpclose,512,64 };
static	struct qinit dkpwinit = { dkpoput,dkposrv,dkpopen,dkpclose,128,65 };
struct	streamtab dkpinfo = { &dkprinit, &dkpwinit };
struct	streamtab cdkpinfo = { &cdkprinit, &dkpwinit };

dkpopen(q)
{
	return(rdkpopen(q, !XCHARMODE));
}

cdkpopen(q)
{
	return(rdkpopen(q, XCHARMODE));
}

rdkpopen(q, mode)
register struct queue *q;
{
	register struct dkp *dkpp;
	static timer = 0;
	int dkptimer();

	if (timer == 0) {
		timer = 1;
		timeout(dkptimer, (caddr_t)NULL, 60);
	}
	if (q->ptr)
		dkpp = (struct dkp *)q->ptr;
	else {
		for (dkpp = dkp; dkpp->state!=0; dkpp++)
			if (dkpp >= &dkp[NDKP])
				return(0);
		dkpp->rdq = q;
		q->ptr = (caddr_t)dkpp;
		WR(q)->ptr = (caddr_t)dkpp;
		WR(q)->flag |= QNOENB;
		putctl(q->next, M_FLUSH);
		dkpp->timer = DKPTIME;
		dkpp->trx = 0;
		dkpp->iseq = 0;
		dkpp->lastecho = ECHO+0;
		dkpp->WS = 1;
		dkpp->WACK = 1;
		dkpp->WNX = 1;
		dkpp->XW = 3;
		if (mode!=XCHARMODE) {
			WR(q)->flag |= QDELIM;
			dkpp->state = OPENING | RCHARMODE;
			putctl1(WR(q)->next, M_CTL, INIT1);
		} else {
			dkpp->XW = 1;
			dkpp->state = RCHARMODE | XCHARMODE | OPEN;
			putctl1(WR(q)->next, M_CTL, INIT0);
		}
	}
	return(1);
}

/*
 * Shut it down.
 *  The problem is to dispose of unacked stuff in the window.
 *   -- no real solution; the receiver might hang on for hours.
 *   Give it 15 seconds.
 */
dkpclose(q)
register struct queue *q;
{
	register struct dkp *dkpp;
	register s = spl5();
	register i;
	register struct block *bp;

	dkpp = (struct dkp *)q->ptr;
	dkpp->state |= LCLOSE;
	flushq(q, 1);
	for (i=0; dkpp->WACK < dkpp->WNX && i<15; i++)
		tsleep((caddr_t)dkpp, DKPPRI, 1);
	if (dkpp->WACK < dkpp->WNX)
		dkprack(dkpp, ACK+((dkpp->WNX-1) & 07));
	dkpinflush(dkpp);
	splx(s);
	dkpp->state = 0;
	flushq(WR(q), 1);
}


/*
 * Process a bunch of input
 *   -- for now, ignore strange control bytes
 */
dkpisrv(q)
register struct queue *q;
{
	register struct dkp *dkpp = (struct dkp *)q->ptr;
	register struct block *bp;
	register c;

	while (bp = getq(q)) {
		if (bp->type == M_CTL) {
			c = *bp->rptr & 0370;
			if (c==REJ || c==ECHO) {
				dkpp->lastecho = *bp->rptr;
				(*WR(q)->next->qinfo->putp)(WR(q)->next, bp);
			} else
				freeb(bp);
			continue;
		}
		if ((q->next->flag&QFULL)==0 || bp->type>=QPCTL
		 || dkpp->state&RCLOSE) {
			TRC('G'); TRC(*bp->rptr);
			(*q->next->qinfo->putp)(q->next, bp);
		} else {
			putbq(q, bp);
			return;
		}
	}
}

/*
 * Packet arrives.
 */
dkpiput(q, bp)
struct queue *q;
register struct block *bp;
{
	register struct dkp *dkpp;
	register i;
	register struct block *nbp;

	if ((dkpp = (struct dkp *)q->ptr)==NULL) {
		freeb(bp);
		return;
	}
	switch (bp->type) {

	moredata:
		bp->rptr++;
		bp->type = M_DATA;
	case M_DATA:
		if (bp->rptr >= bp->wptr||q->flag&QFULL||dkpp->state&LCLOSE) {
			freeb(bp);
			return;
		}
		if (dkpp->state & RCHARMODE) {
			putq(q, bp);
			return;
		}
		switch (dkpp->trx) {

			case 1:
			case 2:
				dkpp->trbuf[dkpp->trx++] = *bp->rptr;
				goto moredata;
			
			default:
				dkpp->trx = 0;
			case 0:
				break;
		}
		bp->next = NULL;
		if (dkpp->indata > 256) { 	/* protect against garbage */
			freeb(bp);
			return;
		}
		if (dkpp->inp) {
			dkpp->inpe->next = bp;
			dkpp->inpe = bp;
		} else {
			dkpp->inp = bp;
			dkpp->inpe = bp;
		}
		dkpp->indata += bp->wptr - bp->rptr;
		return;

	case M_CTL:
		switch (*bp->rptr) {

		case ENQ:
			putctl1(WR(q)->next, M_CTL, dkpp->lastecho);
		case CHECK:
			putctl1(WR(q)->next, M_CTL, ACK+dkpp->iseq);
			dkpinflush(dkpp);
			goto moredata;

		case AINIT:
			dkpp->state &= ~OPENING;
			dkpp->state |= OPEN;
			qenable(WR(q));
			dkpinflush(dkpp);
			goto moredata;

		case INIT0:
		case INIT1:
			putctl1(WR(q)->next, M_CTL, AINIT);
			if (*bp->rptr==INIT0 && (dkpp->state&RCHARMODE)==0) {
				dkpp->state |= RCHARMODE;
				dkpp->XW = 1;
				q->flag &= ~QDELIM;
			} else if (*bp->rptr==INIT1 && (dkpp->state&RCHARMODE)){
				dkpp->state &= ~RCHARMODE;
				dkpp->XW = 3;
				q->flag |= QDELIM;
			}
			dkpinflush(dkpp);
			dkpp->iseq = 0;
			wakeup(dkpp);
			goto moredata;

		case INITREQ:
			if (dkpp->state&XCHARMODE)
				putctl1(WR(q)->next, M_CTL, INIT0);
			else {
				if (dkpp->WS < dkpp->WNX)
					dkprack(dkpp, ECHO+((dkpp->WNX-1)&07));
				dkpp->WS = 1;
				dkpp->WACK = 1;
				dkpp->WNX = 1;
				putctl1(WR(q)->next, M_CTL, INIT1);
			}
			dkpinflush(dkpp);
			goto moredata;

		case BREAK:
			qpctl(q, M_BREAK);
			dkpp->indata++;
			goto moredata;

		case BOT:
		case BOTS:
		case BOTM:
			dkpp->trx = 1;
			dkpp->trbuf[0] = *bp->rptr;
			goto moredata;

		case REJ+0: case REJ+1: case REJ+2: case REJ+3:
		case REJ+4: case REJ+5: case REJ+6: case REJ+7:
			if (dkpp->state&RCHARMODE)
				goto moredata;
			TRC('r');
			if (((*bp->rptr+1)&07) == (dkpp->WACK&07)
			 && (dkpp->state&RJING) == 0) {
				dkstat.dkprxmit++;
				for (i=dkpp->WACK; i<dkpp->WNX; i++) {
					TRC('Z');
					TRC('0' + (i&07));
					dkpp->state |= RJING;
					dkpxmit(WR(q), dkpp->xb[i&07], i);
				}
			}
			goto moredata;
		
		case ACK+0: case ACK+1: case ACK+2: case ACK+3:
		case ACK+4: case ACK+5: case ACK+6: case ACK+7:
		case ECHO+0: case ECHO+1: case ECHO+2: case ECHO+3:
		case ECHO+4: case ECHO+5: case ECHO+6: case ECHO+7:
			dkprack(dkpp, *bp->rptr);
			goto moredata;

		case SEQ+0: case SEQ+1: case SEQ+2: case SEQ+3:
		case SEQ+4: case SEQ+5: case SEQ+6: case SEQ+7:
			i = *bp->rptr & 07;
			if (dkpp->state & RCHARMODE) {
				TRC('e');
				qpctl1(q, M_CTL, ECHO+i);
				goto moredata;
			}
			if (dkpp->trx !=3
			 || dkpp->indata != dkpp->trbuf[1] + (dkpp->trbuf[2]<<8)
			 || i != ((dkpp->iseq+1)&07)) {	/* reject? */
				if (dkpp->trx != 3)
					dkstat.dkprjtrs++;
				else if (i != ((dkpp->iseq+1)&07))
					dkstat.dkprjseq++;
				else
					dkstat.dkprjpks++;
				dkpinflush(dkpp);
				if (dkpp->trbuf[0]==BOTS)
					dkpp->iseq = i;
				TRC('R'); TRC('0'+dkpp->iseq);
				TRC(dkpp->trx!=3?'t':(i!=(dkpp->iseq+1)&07?'s':'c'));
				qpctl1(q, M_CTL, REJ+dkpp->iseq);
				goto moredata;
			}
			/* accept */
			while (nbp = dkpp->inp) {
				dkpp->inp = nbp->next;
				putq(q, nbp);
			}
			TRC('A'); TRC('0'+i);
			dkpp->inpe = NULL;
			dkpp->trx = 0;
			dkpp->indata = 0;
			dkpp->iseq = i;
			qpctl1(q, M_CTL, ECHO+i);
			if (dkpp->trbuf[0] != BOTM)
				qpctl(q, M_DELIM);
			goto moredata;

		default:
			if (*bp->rptr < 0200)	/* non-supervisory */
				dkpp->indata++;
			qpctl1(q, M_CTL, *bp->rptr);
			goto moredata;
		}

	case M_HANGUP:
		dkpp->state |= RCLOSE;
		flushq(WR(q), 1);
		dkprack(dkpp, ECHO+((dkpp->WNX-1) & 07));
		putq(q, bp);
		return;

	case M_IOCACK:
	case M_IOCNAK:
	case M_CLOSE:
		(*q->next->qinfo->putp)(q->next, bp);
		return;

	default:
		freeb(bp);
		return;
	}
}

/*
 * --- Output processor
 */

/*
 * accept data from writer
 *  -- handle most non-data messages
 */
dkpoput(q, bp)
register struct queue *q;
register struct block *bp;
{
	register struct dkp *dkpp = (struct dkp *)q->ptr;
	register union stmsg *sp;
	register x;

	if (dkpp->state & RCLOSE) {
		freeb(bp);
		return;
	}
	switch (bp->type) {

	case M_STOP:
		dkpp->state |= STOPPED;
		freeb(bp);
		return;

	case M_START:
		dkpp->state &= ~STOPPED;
		freeb(bp);
		qenable(q);
		return;

	case M_FLUSH:
		flushq(q, 0);
		freeb(bp);
		return;

	case M_IOCTL:
		sp = (union stmsg *)bp->rptr;
		switch (sp->ioc0.com) {

		case TIOCSETP:
		case TIOCSETN:
			x = sp->ioc1.sb.sg_ispeed;
			bp->wptr = bp->rptr;
			bp->type = M_IOCACK;
			qreply(q, bp);
			if (x==0)
				putctl(OTHERQ(q), M_HANGUP);
			return;

		case TIOCGETP:
			sp->ioc1.sb.sg_ispeed =
			  sp->ioc1.sb.sg_ospeed = B9600;
			bp->type = M_IOCACK;
			qreply(q, bp);
			return;

		case DIOCSTREAM:
			RD(q)->flag &= ~QDELIM;
			bp->wptr = bp->rptr;
			bp->type = M_IOCACK;
			qreply(q, bp);
			return;
		
		case DIOCRECORD:
			if ((dkpp->state&RCHARMODE) == 0) {
				RD(q)->flag | = QDELIM;
				bp->wptr = bp->rptr;
				bp->type = M_IOCACK;
			} else
				bp->type = M_IOCNAK;
			qreply(q, bp);
			return;

		case KIOCINIT:
			if (dkpp->state&XCHARMODE)
				putctl1(q->next, M_CTL, INIT0);
			else {
				if (dkpp->WS < dkpp->WNX)
					dkprack(dkpp, ECHO+((dkpp->WNX-1)&07));
				dkpp->WS = 1;
				dkpp->WACK = 1;
				dkpp->WNX = 1;
				putctl1(q->next, M_CTL, INIT1);
			}
			bp->wptr = bp->rptr;
			bp->type = M_IOCACK;
			qreply(q, bp);
			return;

		case KIOCISURP:
			bp->wptr = bp->rptr;
			bp->type = M_IOCACK;
			qreply(q, bp);
			return;

		default:
			(*q->next->qinfo->putp)(q->next, bp);
			return;
		}

	case M_DELAY:
		x = *bp->rptr;
		*bp->rptr = DELAY;
		bp->type = M_CTL;
		while (x) {
			(*bp->rptr)++;
			x >>= 1;
		}
		goto putonq;

	case M_CLOSE:
		(*q->next->qinfo->putp)(q->next, bp);
		return;

	default:
		freeb(bp);
		return;

	case M_BREAK:
		bp->type = M_CTL;
		*bp->wptr++ = BREAK;
	case M_DELIM:
	case M_DATA:
	putonq:
		putq(q, bp);
		if (dkpp->WNX < dkpp->WS+dkpp->XW)
			qenable(q);
		return;
	}
}

/*
 * Out server:
 *  if space in window, process queue
 */
dkposrv(q)
register struct queue *q;
{
	register struct dkp *dkpp = (struct dkp *)q->ptr;
	register struct block *bp, *xbp;
	int c;

	if (dkpp->state & (STOPPED|OPENING))
		return;
	while (dkpp->WNX < dkpp->WS+dkpp->XW) {
		if ((bp = getq(q)) == NULL)
			break;
		if (dkpp->state & XCHARMODE) {
			switch (bp->type) {

			case M_DELIM:
				freeb(bp);
				continue;

			default:
				dkpp->outcnt += bp->wptr - bp->rptr;
				(*q->next->qinfo->putp)(q->next, bp);
				if (dkpp->outcnt >= 64) {
					putctl1(q->next, M_CTL,
					   SEQ+(dkpp->WNX&07));
					dkpp->WNX++;
					dkpp->WACK = dkpp->WNX;
					dkpp->outcnt = 0;
				}
				continue;
			}
		}
		/*
		 * look ahead for delimiters.
		 * Don't transmit a single data block,
		 * to avoid 0-len block next time
		 */
		if (bp->type==M_DATA && (xbp = q->first) && xbp->type==M_DELIM){
			xbp = getq(q);
			if (xbp)
				freeb(xbp);	/* toss delimiter */
			bp->type = M_DELIM;
		}
		if (bp->type==M_DATA && q->first==NULL) {
			putbq(q, bp);
			return;
		}
		TRC('x'); TRC('0'+dkpp->WS/10); TRC('0'+dkpp->WS%10);
		TRC('.'); TRC('0'+dkpp->WNX/10); TRC('0'+dkpp->WNX%10);
		if (dkpp->xb[dkpp->WNX&07]) {
			freeb(dkpp->xb[dkpp->WNX&07]);
			printf("dkp losing block");
		}
		dkpp->xb[dkpp->WNX & 07] = bp;
		dkpxmit(q, bp, dkpp->WNX);
		dkpp->WNX++;
	}
}

/*
 *  Send out a message, with trailer.
 */
dkpxmit(q, bp, seqno)
struct queue *q;
register struct block *bp;
{
	register struct dkp *dkpp = (struct dkp *)q->ptr;
	register type;
	register size;
	register struct block *xbp;

	if (bp==NULL) {
		printf("null bp in dkpxmit\n");
		return;
	}
	type = bp->type;
	size = bp->wptr - bp->rptr;
	seqno &= 07;
	/* send ptr to block, if non-empty */
	if (size) {
		if ((xbp = allocb(0)) == NULL)
			return;
		TRC('X'); TRC('0'+seqno);
		xbp->rptr = bp->rptr;
		xbp->wptr = bp->wptr;
		if (type!=M_DELIM)
			xbp->type = type;
		(*q->next->qinfo->putp)(q->next, xbp);
	}
	/* send trailer */
	if ((xbp = allocb(3)) == NULL)
		return;
	xbp->type = M_CTL;
	*xbp->wptr++ = type==M_DATA? BOTM: BOT;
	*xbp->wptr++ = size;
	*xbp->wptr++ = size >> 8;
	(*q->next->qinfo->putp)(q->next, xbp);
	putctl1(q->next, M_CTL, SEQ + seqno);
	dkpp->timer = DKPTIME;
}

/*
 * Receive an ack of some sort for a transmitted message.
 *  Advance various windows.
 */
dkprack(dkpp, msg)
register struct dkp *dkpp;
{
	register struct block **bpp;
	register seqno, i;

	seqno = msg & 07;
	msg &= 0370;
	/* invariants: 0 <= WS <= WACK <= WNX; seqno maximal < WNX; WS < 8 */
	if (seqno >= dkpp->WNX)
		seqno -= 8;
	else if (seqno+8 < dkpp->WNX)
		seqno += 8;
	dkpp->state &= ~RJING;
	for (i=dkpp->WS; i<=seqno; i++) {
		bpp = &dkpp->xb[i&07];
		if (*bpp) {
			freeb(*bpp);
			*bpp = NULL;
		}
	}
	if ((int)dkpp->WACK <= seqno)
		dkpp->WACK = seqno+1;
	if (msg==ECHO) {
		TRC('E'); TRC('0'+(seqno&07));
		if (dkpp->WS <= seqno) {
			dkpp->timer = DKPTIME;	/* push off timeout */
			dkpp->WS = seqno+1;
			if (dkpp->WNX<dkpp->WS+dkpp->XW && WR(dkpp->rdq)->count)
				qenable(WR(dkpp->rdq));
		}
	} else {
		for (i=dkpp->WACK; i<dkpp->WNX; i++) {
			if (dkpp->xb[i&07]==0)
			 printf("WS %d WACK %d WNX %d i %d seqno %d\n",
			   dkpp->WS, dkpp->WACK, dkpp->WNX, i, seqno);
			dkpxmit(WR(dkpp->rdq), dkpp->xb[i&07], i);
			dkstat.dkprxmit++;
		}
	}
	if (dkpp->WS >= 8) {
		dkpp->WS -= 8;
		dkpp->WACK -= 8;
		dkpp->WNX -= 8;
	}
}

dkptimer()
{
	register struct dkp *dkpp;
	register struct queue *q;

	for (dkpp = dkp; dkpp < &dkp[NDKP]; dkpp++) {
		if ((dkpp->state&(OPEN|OPENING)) == 0)
			continue;
		if (--dkpp->timer>0)
			continue;
		q = WR(dkpp->rdq)->next;
		if (q->flag&QFULL)
			continue;
		if (dkpp->state & XCHARMODE) {
			if (dkpp->WS < dkpp->WNX)
				putctl1(q, M_CTL, SEQ+((dkpp->WNX-1)&07));
			dkpp->timer = 10;
			continue;
		}
		if (dkpp->state&OPENING)
			putctl1(q, M_CTL, INIT1);
		if (dkpp->WS != dkpp->WNX)
			putctl1(q, M_CTL, ENQ);
		dkpp->timer = DKPTIME;
	}
	timeout(dkptimer, (caddr_t)NULL, 60);
}

/*
 * throw away data in front of the barrier, and clear the trailer buffer
 */
dkpinflush(dkpp)
register struct dkp *dkpp;
{
	register struct block *bp;

	while (bp = dkpp->inp) {
		dkpp->inp = bp->next;
		freeb(bp);
	}
	dkpp->inpe = NULL;
	dkpp->trx = 0;
	dkpp->indata = 0;
}
#endif

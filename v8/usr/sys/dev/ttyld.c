#include "../h/param.h"
#include "../h/stream.h"
#include "../h/ioctl.h"
#include "../h/ttyld.h"
#include "../h/conf.h"
#include "tty.h"

extern	char	partab[];

#define	CANBSIZ	256		/* size of largest input line */

struct	ttyld	tty[NTTY];

char	maptab[] = {
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,'|',000,000,000,000,000,'`',
	'{','}',000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,'\\',000,'~',000,
	000,'A','B','C','D','E','F','G',
	'H','I','J','K','L','M','N','O',
	'P','Q','R','S','T','U','V','W',
	'X','Y','Z',000,000,000,000,000,
};

int	ttyopen(), ttyclose(), ttyldin(), ttyinsrv(), ttyosrv();
static struct qinit ttrinit = { ttyldin, ttyinsrv, ttyopen, ttyclose, 600, 60};
static struct qinit ttwinit = { putq, ttyosrv, ttyopen, ttyclose, 300, 200};
struct streamtab ttyinfo = { &ttrinit, &ttwinit};

/*
 * TTY open
 */
ttyopen(qp, dev)
register struct queue *qp;
{
	register struct ttyld *tp;
	static struct tchars tchars = {CINTR,CQUIT,CSTART,CSTOP,CEOT,0377};

	if (qp->ptr)				/* already attached */
		return(1);
	for (tp = tty; tp->t_state&TTUSE; tp++)
		if (tp >= &tty[NTTY-1])
			return(0);
	tp->t_state = TTUSE;
	tp->t_flags = ECHO|CRMOD;
	tp->t_delct = 0;
	tp->t_col = 0;
	tp->t_erase = CERASE;
	tp->t_kill = CKILL;
	tp->t_chr = tchars;
	qp->ptr = (caddr_t)tp;
	qp->flag |= QDELIM|QNOENB;
	WR(qp)->ptr = (caddr_t)tp;
	return(1);
}

ttyclose(qp)
register struct queue *qp;
{
	register struct ttyld *tp = (struct ttyld *)qp->ptr;

	tp->t_state = 0;
}

/*
 * Queue put procedure for tty input
 */
ttyldin(q, bp)
struct queue *q;
register struct block *bp;
{
	register struct ttyld *tp;
	register c;
	register struct queue *wrq = WR(q);	/* writer side */
	int escape, flags;

	tp = (struct ttyld *)q->ptr;
	flags = tp->t_flags;
	if (bp->type!=M_DATA) {
		switch(bp->type) {

		case M_DELIM:
			freeb(bp);
			return;

		case M_BREAK:
			if (tp->t_flags&RAW) {		/* speed-change hack*/
				bp->type = M_DATA;
				if (bp->wptr<bp->lim)
					*bp->wptr++ = '\0';
				break;
			}
			ttysig(q, SIGINT);
			freeb(bp);
			return;

		case M_HANGUP:
		case M_IOCACK:
		case M_IOCNAK:
			(*q->next->qinfo->putp)(q->next, bp);
			return;

		case M_IOCTL:
			ttldioc(WR(q), bp, q, 1);
			return;
		}
		flags |= RAW;
	}
	if (tp->t_flags&TANDEM && tp->t_state&TTBLOCK
	 && q->count <= q->qinfo->lolimit) {
		tp->t_state &= ~TTBLOCK;
		putd(putq, WR(q), tp->t_chr.t_startc);
	}
	if (flags&RAW) {
		if ((q->next->flag&QFULL)==0 && q->count==0)
			(*q->next->qinfo->putp)(q->next, bp);
		else
			putq(q, bp);
		return;
	}
	while (bp->rptr<bp->wptr) {
		c = *bp->rptr++ & 0177;
		if (tp->t_state&TTSTOP) {
			if (c!=tp->t_chr.t_stopc
			 || tp->t_chr.t_stopc==tp->t_chr.t_startc) {
				tp->t_state &= ~TTSTOP;
				putctl(wrq->next, M_START);
			}
		} else {
			if (c==tp->t_chr.t_stopc) {
				tp->t_state |= TTSTOP;
				putctl(wrq->next, M_STOP);
			}
		}
		if (c==tp->t_chr.t_stopc || c==tp->t_chr.t_startc)
			continue;
		if (c==tp->t_chr.t_intrc) {
			ttysig(q, SIGINT);
			continue;
		}
		if (c==tp->t_chr.t_quitc) {
			ttysig(q, SIGQUIT);
			continue;
		}
		if (c=='\r' && tp->t_flags&CRMOD)
			c = '\n';
		if (tp->t_flags&LCASE && c>='A' && c<='Z')
			c += 'a'-'A';
		escape = 0;
		if (tp->t_flags & CBREAK) {
			if ((q->next->flag&QFULL)==0 && q->count==0)
				putd(q->next->qinfo->putp, q->next, c);
			else
				putd(putq, q, c);
		} else {
			if (tp->t_state&TTESC) {
				escape = 1;
				c |= 0200;
			}
			if (c == '\\')
				tp->t_state |= TTESC;
			else {
				tp->t_state &= ~TTESC;
				if (c == ('\\'|0200)) {
					c &= 0177;
					tp->t_state |= TTESC;
				}
				/* ttyhog? */
				if (q->count<512 || (c=='\n' && tp->t_delct==0))
					putd(putq, q, c);
				else
					c = '\007';
			}
			c &= 0177;
			if (c=='\n'||c==tp->t_chr.t_eofc||c==tp->t_chr.t_brkc) {
				register struct block *bp1;
				if (bp1 = allocb(1)) {
					bp1->type = M_DELIM;
					putq(q, bp1);
					tp->t_delct++;
				}
				qenable(q);
			}
		}
		if (tp->t_flags&TANDEM && (tp->t_state&TTBLOCK) == 0
		 && q->count >= (q->qinfo->limit+q->qinfo->lolimit)/2 ) {
			q->next->flag |= QWANTW;
			tp->t_state |= TTBLOCK;
			putd(putq, wrq, tp->t_chr.t_stopc);
		}
		if (tp->t_flags&ECHO && (wrq->flag&QFULL)==0) {
			register c1 = c&0177;
			if (c1!=CEOT) {
				putd(putq, wrq, c1);
				if(wrq->next->flag&QDELIM && bp->wptr==bp->rptr)
					putctl(wrq, M_DELIM);
			}
			if (c==tp->t_kill && (tp->t_flags&CBREAK)==0
			 && !escape) {
				putd(putq, wrq, '\n');
				if (wrq->next->flag&QDELIM)
					putctl(wrq, M_DELIM);
			}
		}
	}
	freeb(bp);
}

/*
 * tty input server processing.  Erase-kill and escape processing;
 * gathering into lines.
 */

ttyinsrv(q)
register struct queue *q;
{
	register struct ttyld *tp;
	register struct block *bp;
	register char *op;
	register c;
	static char canonb[CANBSIZ];

	tp = (struct ttyld *)q->ptr;
	if (q->next->flag&QFULL)
		return;
	if (tp->t_flags&(CBREAK|RAW)) {
		while ((q->next->flag&QFULL)==0 && (bp = getq(q)))
			(*q->next->qinfo->putp)(q->next, bp);
	} else {
		op = canonb;
		while (tp->t_delct) {
			bp = getq(q);
			if (bp==NULL || bp->type!=M_DATA) {
				if (op > canonb)
					putcpy(q->next, canonb, op-canonb);
				if (bp) {
					(*q->next->qinfo->putp)(q->next, bp);
					tp->t_delct--;
					op = canonb;
					continue;
				}
				return;
			}
			while (bp->rptr<bp->wptr) {
				c = *bp->rptr++;
				if ((c&0200) == 0) {	/* not escaped */
					if (c == tp->t_erase) {
						if (op > canonb)
							op--;
						continue;
					}
					if (c == tp->t_kill) {
						op = canonb;
						continue;
					}
					if (c == tp->t_chr.t_eofc)
						continue;
				} else {
					c &= 0177;
					if (tp->t_flags&LCASE && maptab[c])
						c = maptab[c];
					else if (c==tp->t_erase || c==tp->t_kill
					      || c==tp->t_chr.t_eofc)
						;
					else
						*op++ = '\\';
				}
				*op++ = c;
				if (op >= &canonb[CANBSIZ-1]) {
					putcpy(q->next, canonb, op-canonb);
					op = canonb;
				}
			}
			freeb(bp);
		}
	}
	if (tp->t_flags&TANDEM && tp->t_state&TTBLOCK
	 && q->count <= q->qinfo->lolimit) {
		tp->t_state &= ~TTBLOCK;
		putd(putq, WR(q), tp->t_chr.t_startc);
	}
}

/*
 * TTY write processing: delays, tabs, CR/NL and the like.
 */
ttyosrv(q)
register struct queue *q;
{
	register struct ttyld *tp;
	register struct block *bp;

	tp = (struct ttyld *)q->ptr;
	while (bp = getq(q)) {
		switch(bp->type) {

		default:
			freeb(bp);
			continue;

		case M_DELIM:
			if ((q->next->flag & QDELIM) == 0) {
				freeb(bp);
				continue;
			}
			if (q->next->flag & QFULL) {
				putbq(q, bp);
				return;
			}
			(*q->next->qinfo->putp)(q->next, bp);
			continue;

		case M_IOCTL:
			if (q->next->flag & QFULL) {
				putbq(q, bp);
				return;
			}
			ttldioc(q, bp, RD(q), 0);
			continue;

		case M_FLUSH:
			flushq(q, 0);
		case M_IOCNAK:		/* flow through */
		case M_IOCACK:
			(*q->next->qinfo->putp)(q->next, bp);
			continue;

		case M_DATA:
		case M_BREAK:
			if (q->next->flag & QFULL) {
				putbq(q, bp);
				return;
			}
			if (tp->t_flags&RAW || bp->type==M_BREAK) {
				(*q->next->qinfo->putp)(q->next, bp);
			} else
				outconv(q, bp);
			continue;
		}
	}
}

outconv(q, ibp)
struct queue *q;
register struct block *ibp;
{
	register struct ttyld *tp;
	register struct block *obp = NULL;
	register c;
	register count, ctype;

	tp = (struct ttyld *)q->ptr;
   more:
	while (ibp->rptr < ibp->wptr) {
		if (obp==NULL || obp->wptr >= obp->lim) {
			if (obp)
				(*q->next->qinfo->putp)(q->next, obp);
			if (q->next->flag&QFULL || (obp=allocb(QBSIZE))==NULL) {
				putbq(q, ibp);
				return;
			}
		}
		/*
		 * The following dance is an inner loop
		 */
		count = ibp->wptr - ibp->rptr;
		if ((c = obp->lim - obp->wptr) < count)
			count = c;
		while ((ctype = partab[c = *ibp->rptr++ & 0177] & 077) == 0) {
			tp->t_col++;
			*obp->wptr++ = c;
			if (--count <= 0)
				goto more;
		}
		if (c=='\t' && (tp->t_flags&TBDELAY)==XTABS) {
			for (;;) {
				*obp->wptr++ = ' ';
				tp->t_col++;
				if ((tp->t_col & 07) == 0)	/* every 8 */
					break;
				if (obp->wptr >= obp->lim) {
					ibp->rptr--;
					break;
				}
			}
			continue;
		}

		/*
		 * turn <nl> to <cr><lf> if desired.
		 */
		if (c=='\n' && tp->t_flags&CRMOD) {
			if ((tp->t_state&TTCR)==0) {
				tp->t_state |= TTCR;
				c = '\r';
				ctype = partab['\r'] & 077;
				--ibp->rptr;
			} else
				tp->t_state &= ~TTCR;
		}
		/*
		 * store character
		 */
		*obp->wptr++ = c;
		/*
		 * Calculate delays and column movement
		 */
		count = 0;
		switch (ctype) {

		/* ordinary */
		case 0:
			tp->t_col++;
			break;
	
		/* non-printing */
		case 1:
			break;
	
		/* backspace */
		case 2:
			if (tp->t_col)
				tp->t_col--;
			break;
	
		/* newline */
		case 3:
			ctype = (tp->t_flags >> 8) & 03;
			if(ctype == 1) { /* tty 37 */
				if (tp->t_col)
					count = max(((unsigned)tp->t_col>>4) + 3, (unsigned)6);
			} else if (ctype == 2)	/* vt05 */
				count = 6;
			if ((tp->t_flags&CRMOD)==0)
				tp->t_col = 0;
			break;
	
		/* tab */
		case 4:
			ctype = (tp->t_flags >> 10) & 03;
			if(ctype == 1) { /* tty 37 */
				count = 1 - (tp->t_col | ~07);
				if (count < 5)
					count = 0;
			}
			tp->t_col |= 07;
			tp->t_col++;
			break;
	
		/* vertical motion */
		case 5:
			if(tp->t_flags & VTDELAY)
				count = 127;
			break;
	
		/* carriage return */
		case 6:
			ctype = (tp->t_flags >> 12) & 03;
			if (ctype == 1) 	 /* tn 300 */
				count = 5;
			else if (ctype == 2)	/* ti 700 */
				count = 10;
			else if (ctype == 3)
				count = 20;
			tp->t_col = 0;
			break;
		}
		if (count) {
			(*q->next->qinfo->putp)(q->next, obp);
			if (obp = allocb(1)) {
				obp->type = M_DELAY;
				*obp->wptr++ = count;
				(*q->next->qinfo->putp)(q->next, obp);
			}
			obp = NULL;
		}
	}
	if (obp)
		(*q->next->qinfo->putp)(q->next, obp);
	freeb(ibp);
}

/*
 * Reader generates a signal and passes it up
 */
ttysig(q, sig)
register struct queue *q;
{
	register struct ttyld *tp = (struct ttyld *)q->ptr;

	flushq(q, 0);			/* flush reader */
	flushq(WR(q), 0);
	tp->t_state &= ~TTESC;
	tp->t_delct = 0;
	putctl1(q->next, M_FLUSH);
	putctl1(q->next, M_SIGNAL, sig);
	putctl(WR(q)->next, M_FLUSH);
}

ttldioc(q, bp, rdq, fromdev)
register struct block *bp;
struct queue *q, *rdq;
{
	register struct ttyld *tp;
	register union stmsg *sp;
	register rawcb;
	int s;

	sp = (union stmsg *)bp->rptr;
	tp = (struct ttyld *)q->ptr;
	switch (sp->ioc0.com) {

	/*
	 * Set new parameters
	 */
	case TIOCSETP:
	case TIOCSETN:
		s = spl6();
		if (sp->ioc1.sb.sg_flags & (RAW|CBREAK)
		  && (rdq->next->flag&QFULL)==0) {
			register struct block *bp1;
			ttyinsrv(rdq);
			while (bp1 = getq(rdq))
				(*rdq->next->qinfo->putp)(rdq->next, bp1);
		}
		tp->t_erase = sp->ioc1.sb.sg_erase;
		tp->t_kill = sp->ioc1.sb.sg_kill;
		tp->t_flags = sp->ioc1.sb.sg_flags;
		splx(s);
		if (fromdev) {
			bp->type = M_IOCACK;
			qreply(rdq, bp); /* reply to device side */
		} else
			(*q->next->qinfo->putp)(q->next, bp); /* pass to device */
		if (tp->t_flags & (RAW|CBREAK))
			rdq->flag &= ~(QDELIM|QNOENB);
		else
			rdq->flag |= QDELIM|QNOENB;
		break;

	/*
	 * Send current parameters to user
	 */
	case TIOCGETP:
		sp->ioc1.sb.sg_erase = tp->t_erase;
		sp->ioc1.sb.sg_kill = tp->t_kill;
		sp->ioc1.sb.sg_flags = tp->t_flags;
		bp->wptr = bp->rptr+sizeof(struct ioc1);
		if (fromdev) {
			bp->type = M_IOCACK;
			qreply(rdq, bp); /* reply to device side */
		} else
			(*q->next->qinfo->putp)(q->next, bp); /* pass to device */
		break;

	/*
	 * Set and fetch special characters
	 */
	case TIOCSETC:
		tp->t_chr = sp->ioc2.sb;
		bp->wptr = bp->rptr;
		bp->type = M_IOCACK;
		if (fromdev)
			qreply(rdq, bp); /* reply to device side */
		else
			qreply(q, bp); /* reply to process side */
		break;

	case TIOCGETC:
		sp->ioc2.sb = tp->t_chr;
		bp->wptr = bp->rptr+sizeof(struct ioc2);
		bp->type = M_IOCACK;
		if (fromdev)
			qreply(rdq, bp); /* reply to device side */
		else
			qreply(q, bp); /* reply to process side */
		break;

	default:
		if (fromdev) {
			bp->type = M_IOCACK;
			qreply(rdq, bp); /* reply to device side */
		} else
			(*q->next->qinfo->putp)(q->next, bp); /* pass to device */
		break;

	}
}

/*
 *  message processor-- written data turns into control, read control
 *   turns into data
 *  rmesg is just the opposite
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/stream.h"
#include "../h/conf.h"
#include "mesg.h"

#define	MS_OPEN	01000
#define	MS_DEL	02000
#define	MS_MASK	0377

struct block *msgcollect();
int	msctodput(), msctodsrv(), msdtocput(), msdtocsrv(), msgopen(), rmsgopen(),
	msgclose(), rmsgclose();
struct qinit msgrinit = { msctodput, msctodsrv, msgopen, msgclose, 128, 65 };
struct qinit msgwinit = { msdtocput, msdtocsrv, msgopen, msgclose, 128, 65 };
struct streamtab msginfo = { &msgrinit, &msgwinit };

struct qinit rmsgrinit = { msdtocput, msdtocsrv, rmsgopen, rmsgclose, 128, 65 };
struct qinit rmsgwinit = { msctodput, msctodsrv, rmsgopen, rmsgclose, 128, 65 };
struct streamtab rmsginfo = { &rmsgrinit, &rmsgwinit };

struct mesg {
	short	type;
	short	size;
} mesg[NMESG];

msgopen(q, dev)
register struct queue *q;
{
	register struct mesg *mp;

	if (WR(q)->ptr)
		return(1);
	for (mp = mesg; mp->type&MS_OPEN; mp++)
		if (mp >= &mesg[NMESG-1])
			return(0);
	mp->type = MS_OPEN;
	mp->size = 0;
	q->ptr = (caddr_t)0;		/* "stopped" flag */
	WR(q)->ptr = (caddr_t)mp;
	WR(q)->flag |= QNOENB;
	q->flag |= QDELIM;
	return(1);
}

rmsgopen(q, dev)
register struct queue *q;
{
	register struct mesg *mp;

	if (q->ptr)
		return(1);
	for (mp = mesg; mp->type&MS_OPEN; mp++)
		if (mp >= &mesg[NMESG-1])
			return(0);
	mp->type = MS_OPEN;
	mp->size = 0;
	WR(q)->ptr = (caddr_t)0;		/* "stopped" flag */
	q->ptr = (caddr_t)mp;
	q->flag |= QNOENB;
	q->flag |= QDELIM;
	return(1);
}

msgclose(q)
register struct queue *q;
{
	if (WR(q)->ptr)
		((struct mesg*)WR(q)->ptr)->type = 0;
}

rmsgclose(q)
register struct queue *q;
{
	if (q->ptr)
		((struct mesg*)q->ptr)->type = 0;
}

msctodput(q, bp)
register struct queue *q;
register struct block *bp;
{
	register struct mesg *mp = (struct mesg *)(OTHERQ(q)->ptr);
	register struct queue *bq = backq(q);

	/* propagate changes in delimiter status */
	if (mp->type&MS_DEL) {
		if ((bq->flag&QDELIM) == 0) {
			mp->type &= ~MS_DEL;
			putctl(q, M_NDEL);
		}
	} else {
		if (bq->flag&QDELIM) {
			mp->type |= MS_DEL;
			putctl(q, M_YDEL);
		}
	}
	if (bp->type==M_STOP) {
		freeb(bp);
		q->ptr = (caddr_t)1;	/* mark stopped */
		return;
	}
	if (bp->type>=QPCTL) {		/* including M_START */
		if (bp->type==M_START)
			freeb(bp);
		else {
			/* ioctl transparency */
			if (q->ptr == (caddr_t)2
			 && (bp->type==M_IOCACK || bp->type==M_IOCNAK)) {
				(*q->next->qinfo->putp)(q->next, bp);
				q->ptr = (caddr_t)0;
				return;
			}
			putq(q, bp);
		}
		q->ptr = (caddr_t)0;	/* mark unstopped */
		qenable(q);
		return;
	}
	putq(q, bp);
	return;
}

msctodsrv(q)
register struct queue *q;
{
	register struct block *bp, *hbp;
	register type;

	for (;;) {
		if (q->next->flag & QFULL || q->ptr==(caddr_t)1)
			return;
		if ((bp = getq(q)) == NULL)
			return;
		if ((hbp = allocb(sizeof(struct mesg))) == NULL) {
			putbq(q, bp);
			return;
		}
		((struct mesg *)(hbp->wptr))->type = type = bp->type;
		((struct mesg *)(hbp->wptr))->size = bp->wptr - bp->rptr;
		hbp->wptr += sizeof(struct mesg);
		(*q->next->qinfo->putp)(q->next, hbp);
		bp->type = M_DATA;
		if (bp->wptr > bp->rptr)
			(*q->next->qinfo->putp)(q->next, bp);
		else
			freeb(bp);
		putctl(q->next, M_DELIM);
		if (type == M_HANGUP)
			putctl(q->next, M_HANGUP);
	}
}

msdtocput(q, bp)
register struct queue *q;
register struct block *bp;
{
	switch (bp->type) {

	default:
		freeb(bp);
		return;

	case M_FLUSH:
		flushq(OTHERQ(q), 1);
	case M_IOCACK:
	case M_IOCNAK:
	case M_HANGUP:
		(*q->next->qinfo->putp)(q->next, bp);
		return;

	case M_DATA:
	case M_IOCTL:
		putq(q, bp);
		qenable(q);
		return;
	}
}

msdtocsrv(q)
register struct queue *q;
{
	register struct block *bp;
	register struct mesg *mp = (struct mesg *)q->ptr;

	for (;;) {
		if (q->next->flag & QFULL)
			return;
		if (mp->size == 0) {		/* Start of message */
			bp = msgcollect(q, sizeof(struct mesg), 0);
			if (bp == NULL)
				return;
			mp->size = ((struct mesg *)bp->rptr)->size;
			mp->type &= ~MS_MASK;
			mp->type |= ((struct mesg *)bp->rptr)->type &MS_MASK;
			if (mp->size < 0) {
				mp->size = 0;
				printf("size<0 in msdtocsrv\n");
			}
			if (mp->size==0) {
				bp->type = mp->type;
				bp->rptr = bp->wptr;
				if (bp->type==M_YDEL)
					q->flag |= QDELIM;
				else if (bp->type==M_NDEL)
					q->flag &= ~QDELIM;
				(*q->next->qinfo->putp)(q->next, bp);
				continue;
			}
			freeb(bp);
		}
		bp = msgcollect(q, mp->size, (mp->type&MS_MASK) == M_DATA);
		if (bp == NULL)
			return;
		bp->type = mp->type;
		mp->size -= bp->wptr - bp->rptr;
		(*q->next->qinfo->putp)(q->next, bp);
	}
}

struct block *
msgcollect(q, size, isdata)
register struct queue *q;
{
	register struct block *nbp, *bp;
	register ninb;

	if (q->first==NULL)
		return(NULL);
	nbp = allocb(size);
	if (nbp == NULL)
		return(NULL);
	if (size > nbp->lim - nbp->wptr)
		size = nbp->lim - nbp->wptr;
	while (size) {
		bp = getq(q);
		if (bp == NULL)
			break;
		if (bp->type != M_DATA) {
			/* prevent the ack from being turned to data */
			if (bp->type==M_IOCTL)
				OTHERQ(q)->ptr = (caddr_t)2;
			(*q->next->qinfo->putp)(q->next, bp);
			continue;
		}
		ninb = bp->wptr - bp->rptr;
		if (ninb <= size) {
			bcopy((caddr_t)bp->rptr, (caddr_t)nbp->wptr, ninb);
			size -= ninb;
			nbp->wptr += ninb;
			freeb(bp);
			continue;
		}
		bcopy((caddr_t)bp->rptr, (caddr_t)nbp->wptr, size);
		nbp->wptr += size;
		bp->rptr += size;
		size = 0;
		putbq(q, bp);
	}
	if (nbp->rptr >= nbp->wptr) {
		freeb(nbp);
		return(NULL);
	}
	if (size==0 || isdata)
		return(nbp);
	putbq(q, nbp);
	return(NULL);
}

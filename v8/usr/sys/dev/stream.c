#include "../h/param.h"
#include "../h/stream.h"
#include "../h/mtpr.h"
#include "../h/conf.h"
#include "sparam.h"

#define	M_HIPRI	127			/* for use of putbq */

#ifndef	NBLKBIG
#define	NBLKBIG	0
#endif
#define	NBLOCK (NBLKBIG+NBLK64+NBLK16+NBLK4)
struct	block cblock[NBLOCK]; 		/* allocation of blocks */
u_char	blkdata[1024*NBLKBIG+64*NBLK64+NBLK16*16+NBLK4*4];
long	blkubad;			/* unibus address of blocks */
struct	queue queue[NQUEUE];		/* allocation of queues */
struct	queue *qhead;			/* head of queues to run */
struct	queue *qtail;			/* last queue */
struct	block *qfreelist[4];		/* allocation of freelist heads */
int	cblockC[] = { NBLK4, NBLK16, NBLK64, NBLKBIG };
int	cblockM[] = { 1000, 1000, 1000, 1000 };

int	rbsize[] = { 4, 16, 64, 1024 };	/* real block sizes */
int	bsize[] = { 4, 16, 64, 250 };	/* size for q limits */

int	nballoc;

struct block *
allocb(size)
register size;
{
	register struct block *bp;
	register s = spl6();
	register class;

	if (size <= 4)
		class = 0;
	else if (size <= 16)
		class = 1;
	else if (size <= 64)
		class = 2;
	else {
		class = 3;
		nballoc++;
	}
	if (bp = qfreelist[class])
		qfreelist[class] = bp->next;
	else {
		if (qfreelist[2])
			return(allocb(64));
		panic("allocb out of blocks\n");
	}
	splx(s);
	if (--cblockC[class] < cblockM[class])
		cblockM[class] = cblockC[class];
	bp->rptr = bp->base;
	bp->wptr = bp->base;
	bp->lim = bp->base+rbsize[class];
	bp->class = class;
	bp->type = M_DATA;
	bp->next = NULL;
	return(bp);
}

freeb(bp)
register struct block *bp;
{
	register s = spl6();
	register struct block *bp1;
	register class = bp->class;

#ifdef CAREFUL
	if (bp < &cblock[0] || bp >= &cblock[NBLOCK])
		printf("freeing %x\n", bp);
	bp1 = qfreelist[class];
	while (bp1) {
		if (bp1 == bp)
			panic("Free of free block");
		bp1 = bp1->next;
	}
#endif
	bp->next = qfreelist[class];
	qfreelist[class] = bp;
	cblockC[class]++;
	splx(s);
}

struct block *
getq(q)
register struct queue *q;
{
	register struct block *bp;
	register s = spl6();

	if ((bp = q->first) == NULL) {
		if ((q->flag&QENAB) == 0)
			q->flag |= QWANTR;
	} else {
if (bp < &cblock[0] || bp >= &cblock[NBLOCK]) panic("getting bad block\n", bp);
		if ((q->first = bp->next) == NULL)
			q->last = NULL;
		q->count -= bsize[bp->class];
		if (q->count < q->qinfo->limit)
			q->flag &= ~QFULL;
		q->flag &= ~QWANTR;
	}
	if (q->count<=q->qinfo->lolimit && q->flag&QWANTW && OTHERQ(q)->next) {
		register struct queue *bq = backq(q);
		if (bq->qinfo->srvp) {
			qenable(bq);
		}
		q->flag &= ~QWANTW;
	}
	splx(s);
	return(bp);
}

putq(q, bp)
register struct queue *q;
register struct block *bp;
{
	int s;

	if (bp->type==M_FLUSH)
		flushq(q, 0);
	s = spl6();
	if (q->first==NULL) {			/* empty, just tack on */
		q->first = bp;
		q->last = bp;
		bp->next = NULL;
	} else if (bp->type<QPCTL || q->last->type>=QPCTL) {	/* put at end */
		register struct block *lastp = q->last;
		register n = bp->wptr - bp->rptr;
		if (bp->type==M_DATA && lastp->type==M_DATA
		 && n <= lastp->lim-lastp->wptr && lastp->wptr>=lastp->base){
			bcopy(bp->rptr, lastp->wptr, n);
			lastp->wptr += n;
			freeb(bp);
			bp = NULL;
		} else {
			lastp->next = bp;
			q->last = bp;
			bp->next = NULL;
		}
	} else {				/* pri, put after any others */
		register struct block *nbp = q->first;
		if (nbp->type < QPCTL) {
			bp->next = q->first;
			q->first = bp;
		} else {
			while (nbp->next->type>=QPCTL)
				nbp = nbp->next;
			bp->next = nbp->next;
			nbp->next = bp;
		}
	}
	if (bp) {
		q->count += bsize[bp->class];
		if (bp->type >= QPCTL && bp->type!= M_HIPRI)
			q->flag |= QWANTR;
	}
	if (q->count >= q->qinfo->limit)
		q->flag |= QFULL|QWANTW;
	if ((q->flag & (QWANTR|QENAB|QNOENB)) == QWANTR && q->qinfo->srvp)
		qenable(q);
	splx(s);
}

/*
 * Put stuff back at beginning of Q
 * (but after any priority msgs)
 */
putbq(q, bp)
register struct queue *q;
register struct block *bp;
{
	register savetype = bp->type;
	register s = spl6();

	bp->type = M_HIPRI;		/* fake priority, to force to start */
	putq(q, bp);
	bp->type = savetype;
	splx(s);
}

/*
 * empty a queue.  Leave any non-data messages, unless flag is 1.
 */
flushq(q, flag)
register struct queue *q;
{
	register struct block *bp, *nbp;
	register s = spl6();

	bp = q->first;
	q->first = NULL;
	if (q->last)
		q->last->next = NULL;
	q->last = NULL;
	q->count = 0;
	q->flag &= ~QFULL;
	while (bp) {
		nbp = bp->next;
		if (bp->type != M_DATA && bp->type != M_DELIM
		 && bp->type != M_CTL && bp->type != M_DELAY
		 && bp->type != M_FLUSH && !flag)
			putq(q, bp);
		else {
			if (bp->type == M_PASS)
				printf("flushing PASS %x\n",*(int *)(bp->rptr));
			freeb(bp);
		}
		bp = nbp;
	}
	if (q->flag&QWANTW && OTHERQ(q)->next) {
		q->flag &= ~QWANTW;
		qenable(backq(q));
	}
	splx(s);
}

qinit()
{
	register struct block *bp;
	register i, j;
	register u_char *base;

	base = blkdata;
	/* blocks are allocated on unibus for DMA.  Assumes unibus 0 */
	blkubad = uballoc(0, (caddr_t)blkdata, sizeof(blkdata), 0);
	if (blkubad == 0)
		panic("Cannot map blocks on unibus in qinit");
	i = 0;
	for (j=0; j<NBLK4; i++, j++) {
		bp = &cblock[i];
		bp->class = 0;
		bp->base = base;
		base += 4;
		bp->next = qfreelist[0];
		qfreelist[0] = bp;
	}
	for (j=0; j<NBLK16; i++, j++) {
		bp = &cblock[i];
		bp->class = 1;
		bp->base = base;
		base += 16;
		bp->next = qfreelist[1];
		qfreelist[1] = bp;
	}
	for (j=0; j<NBLK64; i++, j++) {
		bp = &cblock[i];
		bp->class = 2;
		bp->base = base;
		base += 64;
		bp->next = qfreelist[2];
		qfreelist[2] = bp;
	}
	for (j=0; j<NBLKBIG; i++, j++) {
		bp = &cblock[i];
		bp->class = 3;
		bp->base = base;
		base += 1024;
		bp->next = qfreelist[3];
		qfreelist[3] = bp;
	}
}

/*
 * allocate a pair of queues
 */
struct queue *
allocq()
{
	register struct queue *qp;
	static struct queue zeroR =
	  { NULL,NULL,NULL,NULL,NULL,NULL,0,QUSE|QREADR};
	static struct queue zeroW =
	  { NULL,NULL,NULL,NULL,NULL,NULL,0,QUSE};

	for (qp = queue; qp < &queue[NQUEUE]; qp += 2) {
		if ((qp->flag & QUSE) == 0) {
			*qp = zeroR;
			*WR(qp) = zeroW;
			return(qp);
		}
	}
	return(NULL);
}

/*
 * Put one data char on a queue, using f
 */
putd(f, q, c)
int (*f)();
register struct queue *q;
{
	register struct block *bp;
	register s = spl6();

	if (f==putq && (bp = q->last) && bp->type==M_DATA && bp->wptr<bp->lim) {
		*bp->wptr++ = c;
		splx(s);
	 } else {
		splx(s);
		if ((bp = allocb(16)) == NULL)
			return(0);
		bp->type = M_DATA;
		*bp->wptr++ = c;
		(*f)(q, bp);
	}
	return(1);
}

/*
 * Put a single-byte control record on queue (>=0100 implies QPCTL)
 */
putctl(q, c)
struct queue *q;
{
	register struct block *bp;

	if ((bp = allocb(1)) == NULL)
		return(0);
	bp->type = c;
	(*q->qinfo->putp)(q, bp);
	return(1);
}

/*
 * Control record with a single-byte parameter
 */
putctl1(q, c, p)
struct queue *q;
{
	register struct block *bp;

	if ((bp = allocb(1)) == NULL)
		return(0);
	bp->type = c;
	*bp->wptr++ = p;
	(*q->qinfo->putp)(q, bp);
	return(1);
}

/*
 * put control record, using putq instead of queue's putp
 */
qpctl(q, d)
register struct queue *q;
{
	register struct block *bp = allocb(1);

	if (bp) {
		bp->type = d;
		putq(q, bp);
	}
}

qpctl1(q, c, d)
register struct queue *q;
{
	register struct block *bp = allocb(1);

	if (bp) {
		bp->type = c;
		*bp->wptr++ = d;
		putq(q, bp);
	}
}

/*
 * Copy a literal record onto queue
 */
putcpy(q, cp, n)
register struct queue *q;
register char *cp;
{
	register struct block *bp;
	register nm;

	while (n) {
		if ((bp = allocb(n)) == NULL)	/* sorry */
			return;
		bp->type = M_DATA;
		nm = bp->lim - bp->wptr;
		if (nm > n)
			nm = n;
		bcopy(cp, bp->wptr, nm);
		cp += nm;
		bp->wptr += nm;
		n -= nm;
		(*q->qinfo->putp)(q, bp);
	}
}

/*
 * return the queue upstream from this one
 */
struct queue *
backq(q)
register struct queue *q;
{
	q = OTHERQ(q);
	if (q->next) {
		q = q->next;
		return(OTHERQ(q));
	}
	q = OTHERQ(q);
	printf("backq called with no back (Q %x)\n", q);
	panic("backq");
	return(NULL);
}

/*
 * Send a block back up the queue in reverse from this
 * one (e.g. to respond to ioctls)
 */
qreply(q, bp)
register struct queue *q;
struct block *bp;
{
	q = OTHERQ(q);
	(*q->next->qinfo->putp)(q->next, bp);
}

/*
 * Enable a queue: put it on list of those whose srvp's are
 * ready to run.
 */
qenable(q)
register struct queue *q;
{
	register s;

	s = spl6();
	if (q->flag & QENAB) {
		splx(s);
		return;
	}
	if (q->qinfo->srvp==NULL) {
		splx(s);
		return;
	}
	q->flag |= QENAB;
	q->link = NULL;
	if (qhead==NULL)
		qhead = q;
	else
		qtail->link = q;
	qtail = q;
	setqsched();
	splx(s);
}

/*
 * Run the srvp's of each enabled queue
 *	-- Should not be reentered
 */
queuerun()
{
	register struct queue *q;
	register s;
	extern int queueflag;
	extern char *panicstr;

	if (panicstr)
		return;		/* to minimize destruction */
	s = spl6();
	queueflag++;
	while (q = qhead) {
		if ((qhead = q->link) == NULL)
			qtail = NULL;
		q->flag &= ~QENAB;
		splx(s);
		if (q->qinfo->srvp != NULL)
			(*q->qinfo->srvp)(q);
		else
			printf("Q %x run with no srvp\n", q);
		spl6();
	}
	queueflag--;
	splx(s);
}

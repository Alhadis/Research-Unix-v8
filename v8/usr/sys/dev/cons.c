/*
 * Vax console driver
 */
#include "../h/param.h"
#include "../h/inode.h"
#include "../h/stream.h"
#include "../h/ioctl.h"
#include "../h/ttyld.h"
#include "../h/cons.h"
#include "../h/mtpr.h"
#include "../h/conf.h"
#include "../h/cpu.h"

#define	BUSY	02
#define	TIMEOUT	04

short	cnstate;
struct	queue	*cnrq;
struct	queue	*cnwq;

int	cnopen(), cnclose(), cnoput(), nodev();

static struct qinit cnrinit = { nodev, NULL, cnopen, cnclose, 0, 0 };
static struct qinit cnwinit = { cnoput, NULL, cnopen, cnclose, 200, 100 };
struct streamtab cninfo = { &cnrinit, &cnwinit };

cnopen(qp, dev)
register struct queue *qp;
{
	cnrq = qp;
	cnwq = WR(qp);
	mtpr(RXCS, mfpr(RXCS)|RXCS_IE);
	mtpr(TXCS, mfpr(TXCS)|TXCS_IE);
	return(1);
}

cnclose(qp)
{
	cnrq = NULL;
	cnwq = NULL;
}

/*
 * Console write put routine
 */
cnoput(q, bp)
register struct queue *q;
register struct block *bp;
{

	switch(bp->type) {

	case M_IOCTL:		/* just acknowledge */
		switch (((union stmsg *)bp->rptr)->ioc0.com) {

		case TIOCSETP:
		case TIOCSETN:
			bp->wptr = bp->rptr;
		case TIOCGETP:
			bp->type = M_IOCACK;
			qreply(q, bp);
			return;

		default:
			bp->type = M_IOCNAK;
			bp->wptr = bp->rptr;
			qreply(q, bp);
			return;
		}

	case M_STOP:
		cnstate |= TTSTOP;
		break;

	case M_START:
		cnstate &= ~TTSTOP;
		cnstart();
		break;

	case M_FLUSH:
		flushq(q, 0);
		break;

	case M_DELAY:
	case M_DATA:
		putq(q, bp);
		cnstart();
		return;
	
	default:		/* not handled; just toss */
		break;
	}
	freeb(bp);
}

/*
 * Console receive interrupt
 */
/*ARGSUSED*/
cnrint(dev)
{
	register int c;

	c = mfpr(RXDB);
#if VAX780
	if (c & RXDB_ID && cpu == VAX_780)
		cnrfl (c);
#endif
	if (c&RXDB_ID || cnrq==NULL)
		return;
	if ((cnrq->next->flag & QFULL) == 0)
		putd(cnrq->next->qinfo->putp, cnrq->next, c);
}


/*
 * Transmitter interrupt
 */
/*ARGSUSED*/
cnxint(dev)
{
	cnstate &= ~BUSY;
	cnstart();
#if VAX780
	/* if the console wasn't started, try the floppy */
	if (cpu==VAX_780 && (cnstate & BUSY) == 0)
		conxfl();
#endif
}

cntime()
{
	cnstate &= ~TIMEOUT;
	cnstart();
}

cnstart()
{
	register s;
	register struct block *bp;

	if (cnwq==NULL)
		return;
	s = spl6();
	if ((cnstate & (TIMEOUT|BUSY|TTSTOP))==0 && cnwq->count) {
		bp = getq(cnwq);
		switch (bp->type) {

		case M_DATA:
			mtpr(TXDB, *bp->rptr++);
			cnstate |= BUSY;
			if (bp->rptr >= bp->wptr)
				freeb(bp);
			else
				putbq(cnwq, bp);
			break;

		case M_DELAY:
			timeout(cntime, (caddr_t)NULL, (int)*bp->rptr);
			cnstate |= TIMEOUT;
		default:	/* flow through */
			freeb(bp);
			break;
		}
	}
	splx(s);
}

/*
 * Print a character on console.
 * Attempts to save and restore device
 * status.
 */
cnputc(c)
register c;
{
	register s, timo;

	timo = 30000;
	/*
	 * Try waiting for the console tty to come ready,
	 * otherwise give up after a reasonable time.
	 */
	while((mfpr(TXCS)&TXCS_RDY) == 0)
		if(--timo == 0)
			break;
	if(c == 0)
		return;
	s = mfpr(TXCS);
	mtpr(TXCS, 0);
	mtpr(TXDB, c&0xff);
	if(c == '\n')
		cnputc('\r');
	cnputc(0);
	mtpr(TXCS, s);
}

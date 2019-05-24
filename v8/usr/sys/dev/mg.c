/*
 * DR11C for Mergenthaler 202
 */
#include "mg.h"
#if NMG
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/stream.h"
#include "../h/ioctl.h"
#include "../h/pte.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../h/ubavar.h"
#include "../h/conf.h"

struct device {
	unsigned short csr;
	unsigned short wbuf;
	unsigned short rbuf;
};

struct	device	*MGADDR;

struct {
	char busy;
	struct queue *rq;
	struct queue *wq;
} mg;


/*
 * Hardware control bits
 */
#define CSR0 1
#define CSR1 2
#define REQA 0200
#define REQB 0100000
#define INTA 0100
#define INTB 040


int	nodev(), mgopen(), mgclose(), mgoput();

static	struct qinit mgrinit = { nodev, NULL, mgopen, mgclose, 0, 0 };
	struct qinit mgwinit = { mgoput, NULL, mgopen, mgclose, 200, 100 };
struct	streamtab mginfo = { &mgrinit, &mgwinit };

int	mgattach(), mgprobe();
struct	uba_device *mgstuff[NMG];
u_short	mgstd[] = { 0 };
struct	uba_driver mgdriver =
	{ mgprobe, 0, mgattach, 0, mgstd, "mg", mgstuff };

mgprobe(reg)
caddr_t reg;
{
	register int br, cvec;
	register struct device *mgaddr = (struct device *)reg;

	mg.busy = 0;
	mg.rq = mg.wq = NULL;
	MGADDR = mgaddr;
	mgaddr->csr = INTA;
	DELAY(10000);
	mgaddr->csr = 0;
	return(1);
}

mgattach()
{
}

/*
 * comment
 */
mgopen(q, dev)
register struct queue *q;
register dev_t dev;
{
	register d, c;

	d = minor(dev);
	MGADDR->csr |= CSR0 | CSR1;
	if(d == 0){
		if(mg.rq != NULL) goto bad;
		c = MGADDR->rbuf;
		MGADDR->csr |= INTB;
		mg.rq = q;
	} else if(d == 1){
		if(mg.wq != NULL) goto bad;
		MGADDR->csr |= INTA;
		mg.wq = WR(q);
	} else {
		goto bad;
	}
	q->ptr = (caddr_t) minor(dev);
	WR(q)->ptr = (caddr_t) minor(dev);
	return(1);
bad:
	return(0);
}

/*
 * close MG
 */
mgclose(q)
register struct queue *q;
{
	int dev;

	dev = (int) q->ptr;
	if(dev){
		mg.wq = NULL;
	} else {
		MGADDR->csr &= (~INTB);
		/* flush output */;
		flushq(WR(q), 1);
		mg.rq = NULL;
	}
}

/*
 * DK receiver interrupt.
 */
mgrint(dev)
{
	register struct queue *q;
	register struct block *bp;
	register c;

	q = mg.rq;

	c = MGADDR->rbuf;
	if(q == NULL) return;
	if((q->flag & QFULL) == 0){
		putd(q->next->qinfo->putp, q->next, c);
	}
}

/*
 * MG put procedure
 */
mgoput(q, bp)
register struct queue *q;
register struct block *bp;
{
	int s, r, n;

	switch(bp->type){
	case M_IOCTL:
		bp->type = M_IOCNAK;
		bp->wptr = bp->rptr;
		qreply(q, bp);
		return;
	case M_FLUSH:
		flushq(q, 0);
		break;
	case M_DATA:
		putq(q, bp);
		mgstart();
		return;
	default:
		break;
	}
	freeb(bp);
}

mgtint(dev)
{
	mg.busy = 0;
	mgstart();
}

mgstart()
{
	register c, s;
	register struct block *bp;

	if(mg.wq == NULL) return;

	s = spl6();
	if((mg.busy == 0) && mg.wq->count){
		bp = getq(mg.wq);
		switch(bp->type){
		case M_DATA:
			MGADDR->wbuf = *bp->rptr++;
			mg.busy = 1;
			if(bp->rptr >= bp->wptr)
				freeb(bp);
			else
				putbq(mg.wq, bp);
			break;
		default:
			freeb(bp);
			break;
		}
	}
	splx(s);
}
#endif

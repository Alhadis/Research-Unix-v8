/*
 * connector l.d.:  install on a stream-mounted file.
 * Opens on the file send new, unique pipe to the
 * server and return the other end of the pipe.
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/stream.h"
#include "../h/ioctl.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../h/ubavar.h"
#include "../h/conf.h"
#include "../h/file.h"
#include "../h/inode.h"
#include "../h/ttyld.h"
#include "connld.h"

#if	NCONNLD

#define	STIPRI	28
int	connopen(), connput(), nulldev(), conngput();

static	struct qinit connrinit = { connput, 0, connopen, nulldev, 0, 0 };
static	struct qinit connwinit = { connput, 0, connopen, nulldev, 0, 0 };
struct streamtab connldinfo = { &connrinit, &connwinit };

static	struct qinit connrgrab = { conngput, 0, nulldev, nulldev, 0, 0 };
static	struct qinit connwgrab = { connput, 0, nulldev, nulldev, 0, 0 };
struct streamtab connginfo = { &connrgrab, &connwgrab };

connopen(q, dev)
register struct queue *q;
{
	struct inode *ip1, *ip2;
	register struct file *fp, *nfp;
	register s;
	register struct block *bp;
	register struct queue *nq;
	register ioc;

	if ((int)q->ptr == 0) {		/* the open on push does nothing */
		q->ptr = (caddr_t)1;
		return(1);
	}
	/* make pipe, send one end to other side */
	if ((fp = allocfile()) == NULL)
		return(0);
	if (makepipe(&ip1, &ip2)==0) {
		fp->f_count = 0;
		return(0);
	}
	fp->f_inode = ip2;
	fp->f_flag = FREAD|FWRITE;
	fp->f_count--;
	nq = RD(ip1->i_sptr->wrq);
	if (sndfile(WR(q), fp)==0 || qattach(&connginfo, nq, (dev_t)0)==0) {
		stclose(ip1);
		iput(ip1);
		return(0);
	}
	nq = backq(nq);
	/* wait for reply */
	s = spl5();
	while ((bp = getq(nq))==NULL) {
		if (tsleep((caddr_t)nq, STIPRI, 0) != TS_OK) {
			stclose(ip1);
			iput(ip1);
			return(0);
		}
	}
	splx(s);

	switch(bp->type) {

	case M_HANGUP:
		freeb(bp);
		stclose(ip1, 1);
		iput(ip1);
		return(0);

	case M_PASS:	/* accept, and provide a newer file */
		stclose(ip1, 1);
		iput(ip1);
		nfp = ((struct kpassfd *)bp->rptr)->f.fp;
		ip1 = nfp->f_inode;
		ip1->i_count++;
		closef(nfp);
		return((long)ip1);

	case M_IOCTL:
		ioc = ((union stmsg *)bp->rptr)->ioc0.com;
		if (ioc==FIOACCEPT || ioc==FIOREJECT) {
			bp->type = M_IOCACK;
			qreply(nq, bp);
			if (ioc==FIOREJECT) {
				stclose(ip1, 1);
				iput(ip1);
				return(0);
			}
			while (bp = getq(nq))
				(*nq->next->qinfo->putp)(nq->next, bp);
			qdetach(nq, 1);
			return((long)ip1);
		}
	default:		/* flow through */
		(*nq->next->qinfo->putp)(nq->next, bp);
		while (bp = getq(nq))
			(*nq->next->qinfo->putp)(nq->next, bp);
		qdetach(nq, 1);
		return((long)ip1);
	}
}

connput(q, bp)
register struct queue *q;
register struct block *bp;
{
	switch (bp->type) {

	case M_HANGUP:
	case M_IOCTL:
	case M_IOCACK:
	case M_IOCNAK:
	case M_PASS:
		(*q->next->qinfo->putp)(q->next, bp);
		return;
	}
	freeb(bp);
}

conngput(q, bp)
register struct queue *q;
register struct block *bp;
{
	putq(q, bp);
	wakeup((caddr_t)q);
}
#endif

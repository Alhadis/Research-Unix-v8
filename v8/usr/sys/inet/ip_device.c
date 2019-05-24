/*
 * ip device driver; each minor device is one protocol.
 * so tcp would be placed on top of ip minor device #6.
 */

#include "inet.h"
#if NINET
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/stream.h"
#include "../h/ioctl.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/inet/in.h"
#include "../h/inet/ip_var.h"
#include "../h/ttyld.h"

int	nodev(), ipdopen(), ipdclose(), ipdput(), ipdosrv();
static	struct qinit ipdrinit = { nodev, NULL, ipdopen, ipdclose, 0, 0 };
		/* was 0, 0 (unused) originally */
	struct qinit ipdwinit = { ipdput, ipdosrv, ipdopen, ipdclose, IP_BODY_LIMIT,
				  129 };
		/* was IP_BODY_LIMIT (4096), 129 originally */
struct	streamtab ipdinfo = { &ipdrinit, &ipdwinit };

struct queue *ipdstate[256];
int ipprintfs;

ipdopen(q, dev)
register struct queue *q;
dev_t dev;
{
	dev = minor(dev);

	if(ipdstate[dev]){
		return(0);
	}
	ipdstate[dev] = q;
	q->ptr = (caddr_t)dev;
	q->flag |= QDELIM;
	WR(q)->ptr = (caddr_t)dev;
	WR(q)->flag |= QNOENB;
	return(1);
}

ipdclose(q)
register struct queue *q;
{
	int dev;

	dev = (int)q->ptr;
	ipdstate[dev] = 0;
}

ipdput(q, bp)
register struct queue *q;
register struct block *bp;
{
	union stmsg *sp;
	struct foo{
		u_long dst;
		u_long gate;
	} foo;
	int i;
	u_long *lp;

	switch(bp->type){
	case M_IOCTL:
		sp = (union stmsg *)(bp->rptr);
		bp->type = M_IOCACK;
		switch(sp->ioc0.com){
		case IPIOROUTE:
			bcopy(sp->iocx.xxx, &foo, sizeof(foo));
			if(ip_doroute(foo.dst, foo.gate))
				bp->type = M_IOCNAK;
			break;
		case IPIOGETIFS:
			freeb(bp);
			bp = allocb(64);
			bp->type = M_IOCACK;
			sp = (struct stmsg *)(bp->rptr);
			lp = (u_long *)(sp->iocx.xxx);
			for(i = 0; i < NINET; i++){
				if((ipif[i].flags&IFF_UP)==0)
					continue;
				*lp++ = ipif[i].that;
				*lp++ = ipif[i].thishost;
			}
			*lp++ = 0;
			bp->wptr = (u_char *)lp;
			break;
		default:
			bp->type = M_IOCNAK;
			break;
		}
		qreply(q, bp);
		return;
	case M_DATA:
		putq(q, bp);
		break;
	case M_DELIM:
		putq(q, bp);
		qenable(q);
		break;
	default:
		freeb(bp);
		break;
	}
}

ipdrint(bp, dev)
register struct block *bp;
unsigned dev;
{
	register struct block *bp1;
	register struct queue *q;

	q = ipdstate[dev];
	if(q){
		if(q->next->flag&QFULL){
			bp_free(bp);
			if(ipprintfs)
				printf("ipdrint: QFULL\n");
			ipstat.ips_qfull++;
			return;
		}
		while(bp){
			bp1 = bp->next;
			(*q->next->qinfo->putp)(q->next, bp);
			bp = bp1;
		}
		bp = allocb(0);
		if(bp){
			bp->type = M_DELIM;
			(*q->next->qinfo->putp)(q->next, bp);
		} else {
			printf("ipdrint: no allocb for DELIM\n");
		}
	} else {
		bp_free(bp);
	}
}

ipdosrv(q)
register struct queue *q;
{
	register struct block *bp, *tail, *head;

	head = tail = 0;
	while(bp = getq(q)){
		bp->next = 0;
		if(bp->type != M_DATA){
			freeb(bp);
			if(head)
				ip_output(head, 0, 0);
			else
				printf("osrv, DELIM & no DATA\n");
			head = tail = 0;
		} else if(head == 0){
			head = tail = bp;
		} else {
			tail->next = bp;
			tail = bp;
		}
	}
}
#endif

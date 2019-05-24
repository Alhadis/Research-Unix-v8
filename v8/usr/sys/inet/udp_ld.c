/*
 * udp line discipline; only one, to be pushed on /dev/ip17.
 */

#include "udp.h"
#if NUDP > 0
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/stream.h"
#include "../h/ioctl.h"
#include "../h/ttyld.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../h/ubavar.h"
#include "../h/conf.h"

#include "../h/inet/in.h"
#include "../h/inet/ip.h"
#include "../h/inet/ip_var.h"
#include "../h/inet/udp.h"
#include "../h/inet/udp_var.h"

struct queue *udpqueue;

int	udpopen(), udpiput(), udpisrv(), udpclose();
int	udposrv(), udpoput();
static	struct qinit udprinit = { udpiput, udpisrv, udpopen, udpclose, 
				  UDP_MSG_LIMIT, 512 };
static	struct qinit udpwinit = { putq, udposrv, udpopen, udpclose, 
				  UDP_MSG_LIMIT, 64 };
struct streamtab udpinfo = { &udprinit, &udpwinit };

udpopen(q, dev)
register struct queue *q;
{
	if (q->ptr)
		return(0);
	udpqueue = q;	/* RD queue */
	q->flag |= QDELIM;
	WR(q)->flag |= QDELIM;
	q->ptr = (caddr_t)1;
	WR(q)->ptr = (caddr_t)1;
	q->flag |= QNOENB;	/* ipiput calls qenable() */
	return(1);
}

udpclose(q)
register struct queue *q;
{
	if(udpqueue == q)
		udpqueue = 0;
}

udpisrv(q)
register struct queue *q;
{
	register struct block *bp, *head, *tail;

	/* there is now a whole packet waiting
	 * on this queue; strip it off and pass to udp_input().
	 * things other than data or delims are forwarded directly
	 * by udpiput().
	 */
	head = tail = (struct block *) 0;
	while(bp = getq(q)){
		if(bp->type == M_DELIM){
			freeb(bp);
			if(head){
				udp_input(head);
			} else {
				printf("udpisrv: delim, no data\n");
			}
			head = tail = (struct block *) 0;
		} else if(bp->type == M_DATA){
			bp->next = (struct block *) 0;
			if(head == (struct block *) 0){
				head = bp;
			} else {
				tail->next = bp;
			}
			tail = bp;
		} else {
			printf("weird type 0%o in udpisrv\n", bp->type);
			(*q->next->qinfo->putp)(q->next, bp);
		}	
	}
	if(head || tail){
		printf("udpisrv: data & no dELIM\n");
	}
}


udpiput(q, bp)
register struct queue *q;
register struct block *bp;
{
	switch(bp->type){
	case M_DATA:
		putq(q, bp);	/* putq does compression into blocks */
		break;
	case M_DELIM:
		putq(q, bp);
		qenable(q);
		break;
	default:
		(*q->next->qinfo->putp)(q->next, bp);
		break;
	}
		
}

udposrv(q)
register struct queue *q;
{
	register union stmsg *sp;
	register struct block *bp;

	while(bp = getq(q)){
		if(bp->type == M_IOCTL){
			sp = (union stmsg *)bp->rptr;
			switch(sp->ioc0.com){
			default:
				(*q->next->qinfo->putp)(q->next, bp);
				break;
			}
		} else {
			(*q->next->qinfo->putp)(q->next, bp);
		}
	}
}

udp_ldout(bp)
register struct block *bp;
{
	register struct block *bp1;
	register struct queue *q;

	if(udpqueue == 0){
		bp_free(bp);
		return(1);
	}
	q = WR(udpqueue);
	if(q->next->flag&QFULL){
		printf("udp_ldout: QFULL\n");
		bp_free(bp);
		return(1);
	}
	while(bp){
		bp1 = bp->next;
		(*q->next->qinfo->putp)(q->next, bp);
		bp = bp1;
	}
/*
 * send delim
 */

	putctl(q->next, M_DELIM);
}
#endif

/*
 * ip line discipline, to be pushed on an ethernet controller.
 * collects data till a delim, passes it to ip_input().
 */

#include "inet.h"
#include "arp.h"
#include "uarp.h"
#if NINET > 0
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
#include "../h/inet/ip_var.h"
#include "../h/ethernet.h"

struct ipif ipif[NINET];
int Ninet = NINET;		/* let netstat find the number of interfaces */

int	ipopen(), ipiput(), ipisrv(), ipclose();
int	iposrv(), ipoput();
static	struct qinit iprinit = { ipiput, ipisrv, ipopen, ipclose, IP_MSG_LIMIT, 64};
		/* was IP_MSG_LIMIT (4096), 64 originally */
static	struct qinit ipwinit = { putq, iposrv, ipopen, ipclose, 1024, 64 };
		/* was 512, 64 originally */
struct streamtab ipinfo = { &iprinit, &ipwinit };

ipopen(q, dev)
register struct queue *q;
{
	static int timing;
	register i;
	register struct ipif *fp;

	if (q->ptr)
		return(1);
	if(!timing){
		timing = 1;
		ip_slowtimo();
	}
	for (i=0; ipif[i].queue!=0 && i<NINET; i++)
		;
	if (i >= NINET)
		return(0);
	fp = &ipif[i];
	fp->queue = q;	/* that's the RD q */
	fp->flags = IFF_UP;
	fp->that = fp->thishost = 0;
	fp->ipackets = fp->opackets = fp->ierrors = fp->oerrors = 0;
	fp->mtu = 1500;
	fp->arp = -1;
	fp->dev = dev;
	q->flag |= QDELIM;
	WR(q)->flag |= QDELIM;
	q->ptr = (caddr_t)fp;
	WR(q)->ptr = (caddr_t)fp;
	q->flag |= QNOENB;	/* ipiput calls qenable() */
	return(1);
}

ipclose(q)
register struct queue *q;
{
	register struct ipif *ifp;

	ifp = (struct ipif *)q->ptr;
#if NARP > 0
	if (ifp->arp >= 0)
		arp_disable(ifp->arp);
#endif
	ifp->queue = 0;
	ifp->flags = 0;
}

ipisrv(q)
register struct queue *q;
{
	register struct block *bp, *head, *tail;
	register struct ipif *ifp;

	/* there is now a whole packet waiting
	 * on this queue; strip it off and pass to ip_input().
	 * things other than data or delims are forwarded directly
	 * by ipiput().
	 */
	head = tail = (struct block *) 0;
	ifp = (struct ipif *)q->ptr;
	while(bp = getq(q)){
		if(bp->type == M_DELIM){
			freeb(bp);
			if(head){
				if((ifp->flags & IFF_ARP)
				   && (head->wptr - head->rptr) >= 14){
					/* blow away ether header */
					head->rptr += 6 + 6 + 2;
				}
				ip_input(head);
				ifp->ipackets++;
			} else {
				printf("ipisrv: delim, no data\n");
				ifp->ierrors++;
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
			printf("weird type %d in ipisrv\n", bp->type);
			(*q->next->qinfo->putp)(q->next, bp);
		}	
	}
	if(head || tail){
		printf("ipisrv: data & no dELIM\n");
	}
}


ipiput(q, bp)
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

iposrv(q)
register struct queue *q;
{
	struct x{
		unsigned int in;
		unsigned char en[6];
	} *xp;
	register union stmsg *sp;
	register struct block *bp;
	register struct ipif *ifp;
	register int *intp;

	ifp = (struct ipif *)q->ptr;
	while(bp = getq(q)){
		if(bp->type == M_IOCTL){
			sp = (union stmsg *)bp->rptr;
			switch(sp->ioc0.com){
			case IPIOARP:
#if NARP > 0
				if (ifp->arp >= 0)
					printf("IP: already arping\n");
				else {
					ifp->arp = arp_enable(ifp->dev,
							ETHERPUP_IPTYPE,
							sizeof(u_long), 1,
							&ifp->thishost);
					if (ifp->arp == -1) {
						bp->type = M_IOCNAK;
						qreply(q, bp);
						break;
					}
				}
#endif
				ifp->flags |= IFF_ARP;
				bp->type = M_IOCACK;
				bp->wptr = bp->rptr;
				qreply(q, bp);
				break;
			case IPIORESOLVE:
				xp = (struct x *)(sp->iocx.xxx);
#if NUARP > 0
				arp_install(xp->in, xp->en);
#endif
				bp->wptr = bp->rptr;
				bp->type = M_IOCACK;
				qreply(q, bp);
				break;
			case IPIOHOST:
				intp = (int *)(sp->iocx.xxx);
				ifp->that = *intp;
				ifp->flags |= IFF_HOST;
				bp->type = M_IOCACK;
				qreply(q, bp);
				ip_doroute(ifp->that, 0);
				break;
			case IPIOMTU:
				intp = (int *)(sp->iocx.xxx);
				ifp->that = *intp;
				ifp->mtu = *intp;
				bp->type = M_IOCACK;
				qreply(q, bp);
				break;
			case IPIONET:
				intp = (int *)(sp->iocx.xxx);
				ifp->that = *intp;
				ifp->flags &= (~IFF_HOST);
				bp->type = M_IOCACK;
				qreply(q, bp);
				ip_doroute(ifp->that, 0);
				break;
			case IPIOLOCAL:
				intp = (int *)(sp->iocx.xxx);
				ifp->thishost = *intp;
				bp->type = M_IOCACK;
				qreply(q, bp);
				break;
			default:
				(*q->next->qinfo->putp)(q->next, bp);
				break;
			}
		} else {
			(*q->next->qinfo->putp)(q->next, bp);
			if(bp->type == M_DELIM)
				ifp->opackets++;
		}
	}
}

struct ipif *
ip_ifonnetof(dst)
unsigned long dst;
{
	extern ipprintfs;
	struct ipif *ifp;

	/* point-to-point links first */
	for(ifp = &ipif[0]; ifp < &ipif[NINET]; ifp++){
		if((ifp->flags & IFF_UP) && (ifp->flags & IFF_HOST)){
			if(dst == ifp->that)
				return(ifp);
		}
	}
	/* now normal nets */
	for(ifp = &ipif[0]; ifp < &ipif[NINET]; ifp++){
		if((ifp->flags & (IFF_UP|IFF_HOST)) == IFF_UP){
			if(in_netof(dst) == ifp->that)
				return(ifp);
		}
	}
	if(ipprintfs)
		printf("ifonnetof %x?\n", dst);
	return(0);
}

struct ipif *
ip_ifwithaddr(addr)
unsigned long addr;
{
	struct ipif *ifp;
	unsigned long net;

	net = in_netof(addr);
	for(ifp = &ipif[0]; ifp < &ipif[NINET]; ifp++){
		if(ifp->flags & IFF_UP){
			/* address of this host */
			if(addr == ifp->thishost)
				return(ifp);
			/* address of this host's network */
			if(addr == in_netof(ifp->thishost))
				return(ifp);
			/* address on a network simulated by this node */
			if(net == ifp->thishost)
				return(ifp);
		}
	}
	return(0);
}

ip_ldout(bp, dst, ifp)
register struct block *bp;
unsigned long dst;		/* host byte order */
register struct ipif *ifp;
{
#if NARP > 0 || NUARP > 0
	extern struct block *arp_resolve();
#endif
	register struct block *bp1;
	struct goo{
		unsigned int addr;
		char unused[4];
	} *goop;
	register struct queue *q;

	if(ifp->queue == 0){
		printf("ifp but no queue in ip_ldout\n");
		bp_free(bp);
		return(0);
	}
	q = WR(ifp->queue);
	if(q->next->flag & QFULL){
		bp_free(bp);
		ifp->oerrors++;
		return(1);
	}
#if NARP > 0
	if(ifp->flags & IFF_ARP){
		bp = arp_resolve(ifp->arp, bp, &dst);
		if(bp == 0)
			return(1);
	}
#endif
#if NUARP > 0
	if(ifp->flags & IFF_ARP){
		bp = arp_resolve(ifp->queue, bp, dst);
		if(bp == 0)
			return(1);
	}
#endif
	while(bp){
		if(bp->rptr < bp->base || bp->rptr >= bp->lim
		   || bp->wptr < bp->base || bp->wptr > bp->lim){
			printf("ip_ldout: bad block 0x%x\n", bp);
			panic("ip_ldout");
		}
		bp1 = bp->next;
		(*q->next->qinfo->putp)(q->next, bp);
		bp = bp1;
	}
	bp1 = allocb(0);
	if(bp1){
		bp1->type = M_DELIM;
		(*q->next->qinfo->putp)(q->next, bp1);
		ifp->opackets++;
	} else {
		printf("ip_ldout: no allocb for delim\n");
		ifp->oerrors++;
	}
	return(0);
}
#endif

#include "inet.h"
#include "uarp.h"
#if NUARP > 0 && NINET > 0

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/stream.h"
#include "../h/conf.h"
#include "../h/ioctl.h"
#include "../h/ethernet.h"
#include "../h/inet/in.h"
#include "../h/inet/ip_var.h"

/*
 * Address resolution code. ip_ldout() calls arp_resolve if IFF_ARP
 * is set to map an internet address into a 48 bit ethernet address.
 * If arp_resolve finds the address in its tables, it prepends an
 * ethernet header to the packet and returns it. Otherwise, it sends
 * a message to the ipconfig waiting on the specified queue asking
 * it to do all the hard work. The user process at some point will use
 * the IPIORESOLVE ioctl to update the tabes.
 */

/* temporary. soon to be hashed. */
#define NARP 50
struct ip_arp ip_arps[NARP];	/* this table should never have holes in it */

int Nip_arp = NARP;	/* number of address translations for netstat */

arp_install(in, en)
unsigned long in;
unsigned char *en;
{
	register struct ip_arp *ap;

	for(ap = &ip_arps[0]; ap < &ip_arps[NARP]; ap++){
		if(ap->inaddr == in)
			break;
		if(ap->inaddr == 0)
			break;
	}
	if(ap >= &ip_arps[NARP])
		return(1);
	ap->inaddr = in;
	bcopy((caddr_t)en, (caddr_t)(ap->enaddr), 6);
	return(0);
}

struct block *
arp_resolve(q, bp, dst)
struct queue *q;
register struct block *bp;
unsigned long dst;
{
	register struct ip_arp *ap;
	register struct block *bp1;
	struct ether_in *hp;

	for(ap = &ip_arps[0]; ap < &ip_arps[NARP]; ap++)
		if(ap->inaddr == dst || ap->inaddr == 0)
			break;
	if(ap->inaddr == 0 || ap >= &ip_arps[NARP]){
		arp_request(q, dst);
		bp_free(bp);
		return(0);
	}
	bp1 = allocb(sizeof(struct ether_in));
	if(bp1 == 0){
		printf("no bp for arp_resolve\n");
		bp_free(bp);
		return(0);
	}
	bp1->type = M_DATA;
	bp1->wptr = bp1->rptr + sizeof(struct ether_in);
	bp1->next = bp;
	hp = (struct ether_in *)(bp1->rptr);
	hp->type = htons(ETHERPUP_IPTYPE);
	bcopy((caddr_t)(ap->enaddr), (caddr_t)(hp->dhost), 6);
	return(bp1);
}

arp_request(q, dst)
register struct queue *q;
unsigned long dst;
{
	struct block *bp;

	if(q->next->flag & QFULL){
		printf("arp q full\n");
		return;
	}
	bp = allocb(4);
	if(bp == 0)
		return;
	bp->type = M_DATA;
	bp->wptr = bp->rptr + 4;
	*((u_long *)(bp->rptr)) = dst;
	(*q->next->qinfo->putp)(q->next, bp);
	bp = allocb(1);
	if(bp == 0)
		return;
	bp->type = M_DELIM;
	(*q->next->qinfo->putp)(q->next, bp);
}
#endif NUARP

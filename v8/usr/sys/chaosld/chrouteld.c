/*
 *             C H R O U T E  L D
 *
 * Chaosnet packet routing line discipline.
 *
 *
 * (c) Copyright 1985  Nirvonics, Inc.
 *
 * Written by Kurt Gollhardt
 * Last update Tue Mar 12 04:21:35 1985
 *
 * This software is the property of Nirvonics, Inc.
 * All rights reserved.
 *
 */

#include "chroute.h"
#if NCHROUTE > 0
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/stream.h"
#include "../h/ioctl.h"
#include "../h/ttyld.h"
#include "../h/conf.h"
#include "../h/order.h"
#include "../h/ethernet.h"
#include "../chaosld/types.h"
#include "../chaosld/constants.h"
#include "../chaosld/chrouteld.h"


struct chif    Chif[NCHROUTE];
struct chroute Chroute[CHNSUBNET];

static struct chif  *primary;

int  NChropen;
int  NChroute = NCHROUTE;
int  NChsubnet = CHNSUBNET;

int  Chrtimer;	/* Routing timer running */

int chroute_open(), chroute_close(), chroute_put(), chroute_srv();
static struct qinit chroute_init = {
          chroute_put, chroute_srv, chroute_open, chroute_close, 512, 64
};
struct streamtab chrouteinfo = { &chroute_init, &chroute_init };

#define DEBUG

#ifdef DEBUG
     int  Chrdebug = 0;
#    define debug(x)     if (Chrdebug) x
#else
#    define debug(x)
#endif

static struct statxcvr	 zstat;


chroute_open(q, dev)
     register struct queue    *q;
     dev_t     dev;
{
     register int   i;
     register struct chif     *ifp;

     if (q->ptr)    /* If this stream is already open, don't do anything */
          return 1;

     /* Look for a free interface structure */

     if (NChropen++ == NCHROUTE)
	  return 0;  /* If there are none left, the open fails */
     for (i = 0, ifp = Chif; ifp->rdq != 0; ++ifp)
          ;

     ifp->rdq = q;
     ifp->my_addr.ch_addr = 0;
     ifp->arp = -1;
     ifp->if_dev = dev;
     ifp->if_cost = HIGH_COST / 4;  /* We expect this to be set by an ioctl,
                                        but just in case ... */
     ifp->if_stat = zstat;

     q->flag |= QDELIM|QNOENB;
     WR(q)->flag |= QDELIM|QNOENB;
     q->ptr = WR(q)->ptr = (caddr_t)ifp;

     if (!Chrtimer)
	  ch_rtimer();

     return 1;
}


chroute_close(q)
     register struct queue    *q;
{
     register struct chif     *ifp;

     --NChropen;
     ifp = (struct chif *)q->ptr;
     Chroute[ifp->my_addr.ch_subnet].rt_type = CHNOPATH;
     arp_disable(ifp->arp);
     ifp->rdq = (struct queue *)0;
     ifp->my_addr.ch_addr = 0;
     if (ifp == primary)
          primary = (struct chif *)0;
     q->ptr = 0;
}


chroute_put(q, bp)
     register struct queue    *q;
     register struct block    *bp;
{
     debug(printf("chroute_put(%s, %d)\n",
                    ((q->flag & QREADR)? "RD" : "WR"), bp->type));

     switch (bp->type) {
          case M_DATA:
	       putq(q, bp);
	       break;
          case M_DELIM:
	       putq(q, bp);
	       qenable(q);
	       break;
          case M_IOCTL:
	       /* Process an ioctl only if it's going towards the device */
	       if (!(q->flag & QREADR) && chroute_ioctl(q, bp))
	            break;
          default:  /* Anything else, just pass on to the next guy */
	       (*q->next->qinfo->putp)(q->next, bp);
     }
}


chroute_srv(q)
     register struct queue    *q;
{
     register struct block    *bp;
     register struct chif     *ifp;
     register struct chroute  *r;
     int       minsize;

     /* We have a complete packet in the queue, composed of some number
          of M_DATA's followed by an M_DELIM.
        We know there are no other block types, because chroute_put()
	  won't give us anything else.  */

     debug(printf("chroute_srv(%s)\n", ((q->flag & QREADR)? "RD" : "WR")));

     if (bp = q->first) {
          if (bp->type == M_DELIM) {
	       printf("chroute: delimiter without data\n");
	       flush_packet(q);
	       return;
          }
	  minsize = sizeof(struct pkt_header);
	  if (q->flag & QREADR) {
	       /* The packet is coming from the device */
	       minsize += sizeof(struct ether_in);
          }
          if (block_pullup(q, minsize) == 0) {
	       flush_packet(q);
	       return;
          }
	  if (q->flag & QREADR) {
	       /* Packet is from device; strip off ethernet header */
	       q->first->rptr += sizeof(struct ether_in);
	       ifp = (struct chif *)q->ptr;
	       ifp->ist_rcvd++;
	       if (ifp->my_addr.ch_addr != 0) {
		    r = &Chroute[ifp->my_addr.ch_subnet];
		    r->rt_type = CHDIRECT;
		    r->rt_cost = ifp->if_cost;
		    r->rt_path.ifp = ifp;
	       }
          }
	  chroute_dispatch(q);

          /* See if there's another packet on the queue */
	  for (bp = q->first; bp != NOBLOCK; bp = bp->next)
	       if (bp->type == M_DELIM) {
	            qenable(q);    /* re-enable queue for the next packet */
		    break;
               }
     }
}


     /* Handle an ioctl */

chroute_ioctl(q, bp)
     register struct queue    *q;
     register struct block    *bp;
{
     register union stmsg     *sp;
     register struct chif     *ifp;
     register struct chroute  *rt;
     register struct chaddr_pair   *chp;

     ifp = (struct chif *)q->ptr;
     sp = (union stmsg *)bp->rptr;
     switch (sp->iocx.com) {
          case CHIOCADDR:
	       bp->wptr = bp->rptr;
	       if (ifp->my_addr.ch_addr != 0) {
	            Chroute[ifp->my_addr.ch_subnet].rt_type = CHNOPATH;
		    arp_disable(ifp->arp);
               }
	       ifp->my_addr.ch_addr = *(int *)(sp->iocx.xxx);
	       if (ifp->my_addr.ch_subnet >= CHNSUBNET
	                 || ifp->my_addr.ch_subnet == 0) {
	            ifp->my_addr.ch_addr = 0;
		    bp->type = M_IOCNAK;
		    break;
               }
	       rt = &Chroute[ifp->my_addr.ch_subnet];
	       rt->rt_type = CHDIRECT;
	       rt->rt_cost = ifp->if_cost;
	       rt->rt_path.ifp = ifp;
               /* Should we check if this fails? */
	       ifp->arp = arp_enable(ifp->if_dev, ETHERPUP_CHAOSTYPE,
	                      sizeof(chaddr), 0, &ifp->my_addr);
               bp->type = M_IOCACK;
	       break;
          case CRIOCOST:
	       ifp->if_cost = *(u_short *)(sp->iocx.xxx);
	       bp->wptr = bp->rptr;
	       bp->type = M_IOCACK;
	       break;
          case CRIOPRIMARY:
	       primary = ifp;
	       bp->wptr = bp->rptr;
	       bp->type = M_IOCACK;
	       break;
          default:
	       return 0;   /* Let someone else deal with it */
     }

     qreply(q, bp);
     return 1;
}


     /* We've got a packet to send (or receive); send it to the right place */

chroute_dispatch(q)
     register struct queue    *q;
{
     register struct block    *bp;
     register struct pkt_header *ph;
     register struct chroute  *rt;
     register struct chif     *ifp;
     register int        len, isdelim;
     struct ether_out    *ep;
     chaddr              dest;

     ph = (struct pkt_header *)q->first->rptr;
     ifp = (struct chif *)q->ptr;
     debug(printf("chroute_dispatch: op 0%o len %d\n", ph->ph_op, ph->ph_len));

     if (q->flag & QREADR && ph->ph_op == RUTOP) {
          if (block_pullup(q, ph->ph_len + sizeof(*ph)) == 0) {
	       ifp->ist_rej++;
	       flush_packet(q);
	       return;
          }
          chroute_rcvrut(q->first->rptr);
	  flush_packet(q);
	  return;
     }

     if (primary && (ph->ph_daddr.ch_addr == primary->my_addr.ch_addr
                      || (q->flag & QREADR && ph->ph_daddr.ch_addr == 0))) {
          ph->ph_daddr = primary->my_addr;
          if (ph->ph_len > CHMAXDATA) {
	       ifp->ist_len++;
	       flush_packet(q);
	       return;
          }
	  len = sizeof(struct pkt_header) + ph->ph_len;
	  while (bp = getq(q)) {
	       if (bp->type == M_DATA) {
	            if (bp->wptr - bp->rptr > len)
		         bp->wptr = bp->rptr + len;
                    len -= bp->wptr - bp->rptr;
		    if (bp->wptr <= bp->rptr) {
		         freeb(bp);
			 continue;
		    }
	       }
               debug(print_buf(bp, "rcvd:"));
               isdelim = (bp->type == M_DELIM);
	       (*primary->rdq->next->qinfo->putp)(primary->rdq->next, bp);
	       if (isdelim)
	            break;
          }
	  if (len != 0)
	       ifp->ist_len++;
          return;
     }

     if (q->flag & QREADR) {
          if (ph->ph_daddr.ch_host == 0) {
	       ifp->ist_rej++;
	       flush_packet(q);
	       return;
          } else if (++ph->ph_fc == 0) {
               /* Overforwarded packet */
	       flush_packet(q);
	       return;
          }
     }

     dest = ph->ph_daddr;
     if (ph->ph_daddr.ch_host == 0)
          ph->ph_daddr.ch_subnet = 0;
     if (dest.ch_subnet >= CHNSUBNET
            || (rt = &Chroute[dest.ch_subnet])->rt_type == CHNOPATH) {
	  flush_packet(q);
	  return;
     }

     if (rt->rt_type == CHFIXED || rt->rt_type == CHBRIDGE)
          dest = rt->rt_path.bridge;
     ifp = Chroute[dest.ch_subnet].rt_path.ifp;

     if ((bp = allocb(sizeof(struct ether_out))) == 0) {
          ifp->ist_abrt++;
	  flush_packet(q);
	  return;
     }
     ep = (struct ether_out *)bp->wptr;
     bp->wptr += sizeof(struct ether_out);
     ep->type = hfirst_short(ETHERPUP_CHAOSTYPE);

     if (dest.ch_host == 0) /* If address is 0, broadcast to everyone */
          arp_broadcast(ep->dhost);
     else if (arp_getaddr(ifp->arp, &dest, ep->dhost) == -1) {
          ifp->ist_abrt++;
          flush_packet(q);
	  return;
     }

     do {
          debug(print_buf(bp, "xmit:"));
          isdelim = (bp->type == M_DELIM);
          (*(WR(ifp->rdq)->next->qinfo->putp)) (WR(ifp->rdq)->next, bp);
          if (isdelim)
               break;
     } while (bp = getq(q));

     ifp->ist_xmit++;
}


     /* Handle RUT (Routing) Packets */
chroute_rcvrut(ph)
     register struct pkt_header    *ph;
{
     register struct rut_data      *rd;
     register struct chroute       *rt;
     register int   i;

     if ((i = ph->ph_saddr.ch_subnet) >= CHNSUBNET) {
          printf("CHAOS: Bad subnet (%d) in RUT packet\n", i);
	  return;
     }
     if (Chroute[i].rt_type != CHDIRECT) {
          printf("CHAOS: RUT packet from unconnected subnet %d\n", i);
	  return;
     }

     debug(printf("chroute_rcvrut(%d)\n", i));

     rd = (struct rut_data *)((char *)ph + sizeof(struct pkt_header));
     for (i = ph->ph_len; i > 0; i -= sizeof(struct rut_data), ++rd) {
          if (rd->rd_subnet >= CHNSUBNET)
	       continue;
	  rt = &Chroute[rd->rd_subnet];
	  switch (rt->rt_type) {
	       case CHBRIDGE:
	            if (rd->rd_cost > rt->rt_cost)
		         break;
	       case CHNOPATH:
                    rt->rt_cost = rd->rd_cost;
		    rt->rt_type = CHBRIDGE;
		    rt->rt_path.bridge = ph->ph_saddr;
		    break;
               case CHDIRECT:
	       case CHFIXED:
	            break;
               default:
	            printf("CHAOS: Bad entry in routing table, type = %d\n",
		              rt->rt_type);
          }
     }
}


#ifdef DEBUG

print_buf(bp, s)
     register struct block *bp;
     register char *s;
{
     register u_char *p;
     register int   i;

     printf("%s msg type %d len %d", s, bp->type, bp->wptr - bp->rptr);
     for (i = 0, p = bp->rptr; p < bp->wptr; ++i, ++p) {
          if ((i & 31) == 0)
	       printf("\n%s", s);
          printf(" %x", *p);
     }
     printf("\n");
}

#endif DEBUG

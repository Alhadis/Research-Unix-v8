/*
 *             C H  P A C K E T
 *
 * Packet handling utilities for the Chaosnet line discipline.
 *
 *
 * (c) Copyright 1985  Nirvonics, Inc.
 *
 * Written by Kurt Gollhardt
 * Last update Thu Feb 28 14:44:14 1985
 *
 * This software is the property of Nirvonics, Inc.
 * All rights reserved.
 *
 */

#include "ch.h"
#if NCH > 0
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/stream.h"
#include "../h/conf.h"
#include "../chaosld/types.h"
#include "../chaosld/constants.h"
#include "../chaosld/globals.h"

#define CAREFUL


     /*
      * Get blocks from a queue, up to the next delimiter,
      * and place them into a packet structure.
      */

struct packet *get_packet(q, wflag)
     register struct queue    *q;
{
     register struct packet   *pk;
     register struct block    *bp, **nextbp;
     register int   len;

     len = wflag? 1 : sizeof(struct pkt_header);
     if ((bp = getq(q)) == NOBLOCK)
          return NOPKT;

#ifdef CAREFUL
     if (bp->type != M_DATA ||
               bp->wptr - bp->rptr < len) {
	  freeb(bp);
          printf("bad block in get_packet()\n");
	  return NOPKT;
     }
#endif CAREFUL

     if ((pk = pkalloc()) == NOPKT) {
          freeb(bp);
	  flush_packet(q);
	  return NOPKT;
     }

     if (wflag)
          pk->pk_op = *bp->rptr++;
     else {
          pk->ph = *(struct pkt_header *)(bp->rptr);
          bp->rptr += sizeof(struct pkt_header);
     }
     if (bp->rptr >= bp->wptr) {
          freeb(bp);
	  bp = getq(q);
     }

     pk->pk_lenword = 0;
     nextbp = &pk->data;
     while (bp != NOBLOCK && bp->type != M_DELIM) {
          pk->pk_lenword += bp->wptr - bp->rptr;
          *nextbp = bp;
	  nextbp = &bp->next;
	  bp = getq(q);
     }
     *nextbp = NOBLOCK;
     if (bp != NOBLOCK)
          freeb(bp);     /* Throw away the delimiter */

     pk->time = Chclock;
     pk->next = NOPKT;
     return pk;
}


     /*
      * Take the header and data blocks from a packet
      * and put them onto the queue.
      * Any storage associated with the packet is freed.
      */

put_packet(q, pk)
     struct queue   *q;
     struct packet  *pk;
{
     putcpy(q->next, &pk->ph, sizeof(struct pkt_header));
     put_pkdata(q, pk);
}


     /*
      * Take the data blocks from a packet
      * and put them onto the queue.
      * Any storage associated with the packet is freed.
      */

put_pkdata(q, pk)
     register struct queue    *q;
     register struct packet   *pk;
{
     register struct block    *bp, *nextb;

     for (bp = pk->data; bp != NOBLOCK; bp = nextb) {
          nextb = bp->next;
	  (*q->next->qinfo->putp) (q->next, bp);
     }

     if (q->flag & QDELIM)
          putctl(q->next, M_DELIM);

     pk->data = NOBLOCK;
     free_packet(pk);
}


     /*
      * Copy the header and data blocks from a packet onto a queue.
      */

copy_packet(q, pk)
     struct queue   *q;
     struct packet  *pk;
{
     putcpy(q->next, &pk->ph, sizeof(struct pkt_header));
     copy_pkdata(q, pk);
}


     /*
      * Copy the data blocks from a packet onto a queue.
      */

copy_pkdata(q, pk)
     register struct queue    *q;
     register struct packet   *pk;
{
     register struct block    *bp;

     for (bp = pk->data; bp != NOBLOCK; bp = bp->next)
          putcpy(q->next, bp->rptr, bp->wptr - bp->rptr);
     putctl(q->next, M_DELIM);
}


     /*
      * Obtain storage for a packet and perfrom minimal initialization.
      */

struct packet *
new_packet()
{
     register struct packet   *pk;

     if ((pk = pkalloc()) == NOPKT)
          return NOPKT;
     bfill(pk, sizeof(struct packet), 0);
     pk->time = Chclock;
     return pk;
}


     /*
      * Release all storage associated with a packet structure.
      */

free_packet(pk)
     register struct packet   *pk;
{
     free_pkdata(pk);
     pkfree(pk);
}


     /*
      * Release all data blocks in packet.
      */

free_pkdata(pk)
     register struct packet   *pk;
{
     register struct block    *bp, *nextb;

     for (bp = pk->data; bp != NOBLOCK; bp = nextb) {
          nextb = bp->next;
	  freeb(bp);
     }
     pk->data = NOBLOCK;
     pk->pk_lenword = 0;
}


     /*
      * Collapse all of the data blocks for a packet
      * into a single block if possible
      */

flatten(pk)
     register struct packet   *pk;
{
     register struct block    *bp, *bigb, *nextb;
     register int        n;
     int       ps;

     debug(DPKT,printf("flatten packet of length %d\n", pk->pk_len));
     if (pk->data->wptr - pk->data->rptr >= pk->pk_len)
          return 0;
     if ((bigb = allocb(pk->pk_len)) == NOBLOCK) {
          debug(DPKT|DABNOR,printf("flatten: can't alloc block\n"));
          return -1;
     }
     bigb->next = NOBLOCK;
     bp = pk->data;
     ps = spl6();
     pk->data = bigb;

     while (bp != NOBLOCK) {
          n = bp->wptr - bp->rptr;
	  if (bigb->lim - bigb->wptr < n)
	       n = bigb->lim - bigb->wptr;
          bcopy(bp->rptr, bigb->wptr, n);
	  bigb->wptr += n;
	  bp->rptr += n;
	  if (bp->rptr < bp->wptr) {
	       bigb->next = bp;
	       splx(ps);
	       debug(DPKT|DABNOR,printf("flatten: not enough room in block\n"));
	       return -1;
          }
          nextb = bp->next;
	  freeb(bp);
	  bp = nextb;
     }

     splx(ps);
     return 0;
}


     /*
      * Add some bytes of data to the end of a packet.
      * Allocate blocks as necessary.
      */

append_packet(pkt, buf, n)
     register struct packet   *pkt;
     char      *buf;
     int       n;
{
     register struct block    *bp;
     register int        nmin;

     if (pkt->data == NOBLOCK) {
          if ((bp = allocb(n)) == NOBLOCK)
	       return;
          pkt->data = bp;
	  bp->next = NOBLOCK;
     } else {
          for (bp = pkt->data; bp->next != NOBLOCK; bp = bp->next)
	       ;
     }

     while (n > 0) {
          if (bp->wptr == bp->lim) {
	       if ((bp->next = allocb(n)) == NOBLOCK)
	            return;
	       bp = bp->next;
	       bp->next = NOBLOCK;
          }
	  nmin = bp->lim - bp->wptr;
	  if (nmin > n)
	       nmin = n;
          bcopy(buf, bp->wptr, nmin);
          buf += nmin;
	  bp->wptr += nmin;
	  pkt->pk_lenword += nmin;
	  n -= nmin;
     }
}


/*
 * Free a list of packets
 */
freelist(pkt)
register struct packet *pkt;
{
     register struct packet *opkt;

     while ((opkt = pkt) != NOPKT) {
          pkt = pkt->next;
          free_packet(opkt);
     }
}

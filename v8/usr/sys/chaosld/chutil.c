/*
 *             C H  U T I L
 *
 * Utility routines for Chaosnet line discipline and Chroute line discipline.
 *
 *
 * (c) Copyright 1984  Nirvonics, Inc.
 *
 * Written by Kurt Gollhardt
 * Last update Fri Nov  9 18:21:33 1984
 *
 * This software is the property of Nirvonics, Inc.
 * All rights reserved.
 *
 */

#include "../h/param.h"
#include "../h/stream.h"


#ifdef pdp11
/*
 * Swap the word of n longs.
 */
swaplong(lp, n)
register short *lp;
register int n;
{
	register short temp;

	if (n)
		do {
			temp = *lp++;
			lp[-1] = *lp;
			*lp++ = temp;
		} while (--n);
}
#endif


chb_put(q, bp)
     register struct queue    *q;
     register struct block    *bp;
{
     switch (bp->type) {
          case M_DATA:
               putq(q, bp);   /* putq does compression into blocks */
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


bfill(addr, len, val)
     register u_char     *addr;
     register unsigned   len;
     register u_char     val;
{
     while (len--)
          *addr++ = val;
}

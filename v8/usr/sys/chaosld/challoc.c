/*
 *             C H  A L L O C
 *
 * Memory allocation/deallocation routines for Chaosnet line discipline.
 *
 *
 * (c) Copyright 1984  Nirvonics, Inc.
 *
 * Written by Kurt Gollhardt
 * Last update Mon Nov 12 16:27:57 1984
 *
 * This software is the property of Nirvonics, Inc.
 * All rights reserved.
 *
 */

#include "ch.h"
#if NCH > 0
#include "../h/param.h"
#include "../h/stream.h"
#include "../h/conf.h"
#include "../h/buf.h"
#include "../chaosld/constants.h"
#include "../chaosld/types.h"
#include "../chaosld/globals.h"

#define CHBLKSIZE   BSIZE(0)

static caddr_t ch_alloc();
static ch_free();


static struct buf   *pkhead;

struct packet  *pkalloc()
{
     return (struct packet *)ch_alloc(sizeof(struct packet), &pkhead);
}

pkfree(pkt)
     struct packet  *pkt;
{
     ch_free((caddr_t)pkt, sizeof(struct packet), &pkhead);
}


static struct buf   *cnhead;

struct connection   *cnalloc()
{
     return (struct connection *)ch_alloc(sizeof(struct connection), &cnhead);
}

cnfree(conn)
     struct connection   *conn;
{
     ch_free((caddr_t)conn, sizeof(struct connection), &cnhead);
}


#define NOBUF   (struct buf *)0
#define ALIGN(n)    (((n) + sizeof(int) - 1) & ~(sizeof(int) - 1))

static caddr_t ch_alloc(size, head)
     struct buf     **head;
{
     register struct buf *bufp;
     register int        i;
     int       ps = spl6();

     debug(DALLOC,printf("ch_alloc: size=0x%lx ", size));

     for (bufp = *head; bufp != NOBUF;) {
          if (bufp->b_resid > 0)
	       break;
          if ((bufp = bufp->av_forw) == *head)
	       bufp = NOBUF;
     }
     debug(DALLOC,printf("bufp=0x%lx ", bufp));

     if (bufp == NOBUF) {
          if ((bufp = geteblk()) == NOBUF) {
	       debug(DALLOC|DABNOR,printf("No more buffers available\n"));
	       return NOBUF;
          }
          if (*head == NOBUF)
	       *head = bufp->av_forw = bufp->av_back = bufp;
          else {
	       bufp->av_forw = (*head)->av_forw;
	       bufp->av_back = (*head)->av_back;
	       bufp->av_forw->av_back = bufp->av_back->av_forw = bufp;
          }
	  bufp->b_bcount = CHBLKSIZE / (size + 1);
	  if (ALIGN(bufp->b_bcount) + size * bufp->b_bcount > CHBLKSIZE)
	       bufp->b_bcount--;
	  bfill(bufp->b_un.b_addr, bufp->b_resid = bufp->b_bcount, 0);
	  debug(DALLOC,printf("new bufp=0x%lx bcount=%d ", bufp, bufp->b_bcount));
     }

     for (i = 0; i < bufp->b_bcount; ++i)
          if (bufp->b_un.b_addr[i] == 0)
	       break;
     debug(DALLOC,printf("i = %d\n", i));

     bufp->b_un.b_addr[i] = 1;
     bufp->b_resid--;

     splx(ps);
     return bufp->b_un.b_addr + ALIGN(bufp->b_bcount) + i * size;
}


static ch_free(addr, size, head)
     caddr_t        addr;
     struct buf     **head;
{
     register struct buf *bufp;
     register int        i;
     int       ps = spl6();

     debug(DALLOC,printf("ch_free: addr=0x%lx size=0x%lx ", addr, size));

     for (bufp = *head; bufp != NOBUF;) {
          i = (addr - bufp->b_un.b_addr - ALIGN(bufp->b_bcount)) / size;
	  if (i >= 0 && i < bufp->b_bcount)
	       break;
          if ((bufp = bufp->av_forw) == *head)
	       bufp = NOBUF;
     }
     debug(DALLOC,printf("bufp=0x%lx i=%d\n", bufp, i));

     if (bufp == NOBUF || bufp->b_un.b_addr[i] == 0) {
          splx(ps);
          printf("CHAOS: freeing un-allocated object %x (ch_free)\n", addr);
	  return;
     }

     bufp->b_un.b_addr[i] = 0;
     if (++bufp->b_resid == bufp->b_bcount) {
          if (bufp->av_forw == bufp)
	       *head = NOBUF;
          else {
               bufp->av_forw->av_back = bufp->av_back;
	       bufp->av_back->av_forw = bufp->av_forw;
               if (*head == bufp)
	            *head = bufp->av_forw;
          }
	  brelse(bufp);
          debug(DALLOC,printf("ch_free: released buf 0x%lx\n", bufp));
     }

     splx(ps);
}

#endif

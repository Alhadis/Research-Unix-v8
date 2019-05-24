/*
 *             D U M P  L D
 *
 * This line discipline is used for debugging.
 * It passes on everything given to it, in the same order,
 * but at the same time it creates a log of the message type
 * and contents for each message.
 *
 *
 * Written by Kurt Gollhardt  (Nirvonics, Inc.)
 * Last update Sat Mar 30 15:17:01 1985
 *
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/stream.h"
#include "../h/ioctl.h"
#include "../h/ttyld.h"
#include "../h/conf.h"
#include "../h/dumpl.h"

#include "dumpld.h"
#if NDUMPLD > 0

int  dumpput(), dumpopen(), dumpclose();

static struct qinit rinit = { dumpput, NULL, dumpopen, dumpclose, 300, 0 },
                    winit = { dumpput, NULL, dumpopen, dumpclose, 300, 200 };
struct streamtab dumpinfo = { &rinit, &winit };

#define DUMPSIZE    4096

static char    dumpbuf[NDUMPLD*2][DUMPSIZE];
struct dumpld  dumpld[NDUMPLD*2];
struct dumpinf dumpinf[NDUMPLD*2];

int ndumpbuf = NDUMPLD * 2;
int dumpver = 1;


dumpopen(q, dev)
     struct queue   *q;
     dev_t          dev;
{
     register struct dumpld *di;
     register struct dumpinf *dinf;

     if (q->ptr)
          return 1;
     for (di = dumpld; di < &dumpld[NDUMPLD*2]; di += 2)
          if (di->base == (char *)0)
	       break;
     if (di == &dumpld[NDUMPLD*2])
          return 0;

     dinf = &dumpinf[di - dumpld];

     di->fillp = di->base = dumpbuf[di - dumpld];
     di->size = DUMPSIZE;
     q->ptr = (caddr_t)di;
     ++di;
     di->fillp = di->base = dumpbuf[di - dumpld];
     di->size = DUMPSIZE;

     dinf->dev = dev;
     (++dinf)->dev = dev;

     WR(q)->ptr = (caddr_t)di;
     dumpupdate(q);
     dumpupdate(WR(q));

     return 1;
}


dumpclose(q)
     struct queue   *q;
{
     register struct dumpld *di = (struct dumpld *)q->ptr;

     dumpupdate(q);
     dumpupdate(WR(q));

     di->base = (char *)0;
     (di+1)->base = (char *)0;
     q->ptr = (caddr_t)0;
}


dumpput(q, bp)
     struct queue   *q;
     register struct block    *bp;
{
     register struct dumpld *di = (struct dumpld *)q->ptr;
     register u_char     *p;
     struct dumpheader	 hd;
     int  ps = spl6();

     hd.type = bp->type;
     hd.class = bp->class;
     hd.count = bp->wptr - bp->rptr;
     for (p = (u_char *)&hd; p < (u_char *)(&hd + 1);)
	  dumpchr(di, *p++);
     for (p = bp->rptr; p < bp->wptr;)
	  dumpchr(di, *p++);

     (*q->next->qinfo->putp) (q->next, bp);
     dumpupdate(q);

     splx(ps);
}


dumpchr(di, c)
     register struct dumpld *di;
     char c;
{
     *(di->fillp)++ = c;
     if (di->fillp >= di->base + di->size)
          di->fillp = di->base;
}


dumpupdate(q)
     register struct queue    *q;
{
     register struct queue    *qnext = q->next;

     q->flag |= qnext->flag & (QFULL|QWANTR|QBIGB) | backq(q)->flag & QDELIM;
     qnext->flag |= q->flag & QWANTW;
     q->count = qnext->count;
}

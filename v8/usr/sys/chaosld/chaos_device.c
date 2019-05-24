/*
 *             C H A O S _ D E V I C E
 *
 * Chaosnet pseudo-device.  This is the multiplexer/demultiplexer
 * used in conjunction with the Chaosnet line discipline.
 *
 *
 * (c) Copyright 1985  Nirvonics, Inc.
 *
 * Written by Kurt Gollhardt
 * Last update Thu Mar 14 16:03:34 1985
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
#include "../h/ioctl.h"
#include "../h/ttyld.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../chaosld/types.h"
#include "../chaosld/constants.h"
#include "../chaosld/globals.h"

#define CHIOPRIO    (PZERO+1)      /* Just interruptible */

int NCh = NCH; /* For peeking */
int NChopen;

int  nodev(), chdopen(), chdclose(), chdput(), chdosrv();
static struct qinit chdrinit = { nodev, NULL, chdopen, chdclose, 0, 0 },
              chdwinit = { chdput, chdosrv, chdopen, chdclose, 1024, 129 };
struct streamtab chdinfo = { &chdrinit, &chdwinit };


chdopen(q, dev)
     register struct queue    *q;
     dev_t                    dev;
{
     dev = minor(dev);

     if (ChaosQ == (struct queue *)0)   /* Make sure line-discipline is on */
          return 0;
     if (dev > NCH || Chconn[dev] != NOCONN)
          return 0;
     if ((Chconn[dev] = new_conn(dev)) == NOCONN)
          return 0;

     NChopen++;
     Chconn[dev]->cn_rdq = q;
     Chconn[dev]->cn_state |= CSSTARTING;
     q->ptr = WR(q)->ptr = (caddr_t)Chconn[dev];
     q->flag |= QDELIM;
     WR(q)->flag |= QNOENB|QBIGB;

     if (!Chtimer)
	  ch_timer();

     return 1;
}

chdclose(q)
     struct queue   *q;
{
     register struct connection    *conn;
     register struct packet   *pkt;
     int       ps;

     if ((conn = (struct connection *)q->ptr) == NOCONN)
          return;
     conn->cn_time = Chclock;

     switch (conn->cn_mode) {
          case CHSTREAM:
	       ps = spl6();
               if (setjmp(u.u_qsav)) {
                    if ((pkt = new_packet()) != NOPKT) {
                         pkt->pk_op = CLSOP;
	                 append_packet(pkt, "User interrupted", 16);
	            }
                    chld_close(conn, pkt);
                    goto shut;
               }
               if (conn->cn_flags & CHWRITER) {
                    /*
                     * We set this flag telling the interrupt time
                     * receiver to abort the connection if any new packets
                     * arrive.
                     */
                    conn->cn_sflags |= CHCLOSING;
                    /*
                     * Closing a stream transmitter involves flushing
                     * the last packet, sending an EOF and waiting for
                     * it to be acknowledged.  If the connection was
                     * bidirectional, the reader should have already
                     * read until EOF if everything is to be closed
                     * cleanly.
                     */
                    if (conn->cn_state == CSOPEN ||
                              conn->cn_state == CSRFCRCVD) {
                         if (conn->cn_toutput) {
                              chld_write(conn, conn->cn_toutput);
			      conn->cn_toutput = NULL;
                         }
                         if (conn->cn_state == CSOPEN)
                              chld_eof(conn);
                    }
                    while (!chtempty(conn)) {
                         conn->cn_sflags |= CHOWAIT;
                         sleep((caddr_t)&conn->cn_thead, CHIOPRIO);
                    }
		    if (conn->cn_flags & CHREADER) {
		         if (conn->cn_flags & CHSERVER)
			      chld_eof(conn);
                         else
			      goto close_wait;
                    }
               } else if (conn->cn_state == CSOPEN) {
                    /*
                     * If we are only reading then we should read the EOF
                     * before closing and wait for the other end to close.
                     */
close_wait:
                    if (conn->cn_flags & CHEOFSEEN)
                         while (conn->cn_state == CSOPEN)
                              sleep((caddr_t)conn, CHIOPRIO);
               }
               splx(ps);
               /* Fall into... */
          case CHRECORD: /* Record oriented close is just sending a CLOSE */
               if (conn->cn_state == CSOPEN) {
                    if ((pkt = new_packet()) != NOPKT)
                         pkt->pk_op = CLSOP;
                    chld_close(conn, pkt);
               }
     }
shut:
     chld_close(conn, NOPKT);

     free_conn(conn);
     q->ptr = (caddr_t)0;
     --NChopen;
}

chdstart(q, conn)
     register struct queue         *q;
     register struct connection    *conn;
{
     register struct packet   *pkt;
     register struct block    *bp;
     register struct chopen   *c;
     int       len, rwsize, err, ps;

     err = 0;
     conn->cn_sflags |= CHOPNWAIT;
     if (block_pullup(q, sizeof(struct chopen)) == 0) {
          err = EINVAL;
	  goto flush;
     }
     c = (struct chopen *)q->first->rptr;

     len = c->co_clength + c->co_length + (c->co_length? 1 : 0);
     if (len > CHMAXPKT || c->co_clength <= 0) {
	  err = E2BIG;
flush:
          flush_packet(q);
	  goto out;
     }
     if (block_pullup(q, len + sizeof(struct chopen)) == 0) {
          err = EINVAL;
	  goto flush;
     }
     bp = getq(q);
     flush_packet(q);
     c = (struct chopen *)bp->rptr;

     if ((pkt = new_packet()) == NOPKT) {
          err = ENOMEM;
	  goto out;
     }
     append_packet(pkt, bp->rptr += sizeof(struct chopen), c->co_clength);
     if (c->co_length) {
          append_packet(pkt, " ", 1);
          append_packet(pkt, bp->rptr += c->co_clength, c->co_length);
     }

     conn->cn_mode = c->co_mode;
     if (c->co_mode == CHSTREAM)
	  RD(q)->flag &= ~QDELIM;
     rwsize = (c->co_rwsize ? c->co_rwsize : CHDRWSIZE);
     if (c->co_host)
          chld_open(conn, c->co_host, rwsize, pkt);
     else
          chld_listen(conn, rwsize, pkt);

out:
     ps = spl6();
     if (conn->cn_sflags & CHOPNWAIT) {
	  if (c->co_async || err)
	       chdoreply(conn, err, NOPKT, c->co_async);
     }
     splx(ps);
}

chdoreply(conn, err, pkt, async)
     struct connection   *conn;
     struct packet       *pkt;
{
     struct choreply     c;
     struct queue        *q;

     conn->cn_sflags &= ~CHOPNWAIT;
     q = conn->cn_rdq;

     if (err == 0 && !async) {
	  if (conn->cn_flags & CHSERVER) {
	       if (conn->cn_state != CSRFCRCVD)
		    err = EIO;
	  } else {
	       if (conn->cn_state != CSOPEN &&
			 (conn->cn_state != CSCLOSED ||
			      pkt == NOPKT || pkt->pk_op != ANSOP))
		    err = EIO;
	  }
     }
     c.errno = err;
     putcpy(q->next, &c, sizeof(c));
     putctl(q->next, M_DELIM);
}

chdput(q, bp)
     register struct queue    *q;
     register struct block    *bp;
{
     switch(bp->type){
          case M_IOCTL:
               chdioctl(q, bp);
               break;
          case M_DATA:
               putq(q, bp);
               break;
          case M_DELIM:
               putq(q, bp);
	       if (!(((struct connection *)q->ptr)->cn_sflags & CHOQWAIT))
		    qenable(q);
               break;
          default:
               freeb(bp);
               break;
     }
}

chdioctl(q, bp)
     register struct queue    *q;
     register struct block    *bp;
{
     register struct connection    *conn;
     register struct packet   *pkt;
     union stmsg    *sp;
     int       ps, len;

     sp = (union stmsg *)(bp->rptr);
     if ((conn = (struct connection *)q->ptr) == NOCONN)
          goto bad;
     bp->type = M_IOCACK;

     switch(sp->ioc0.com){
          case CHIOCRPKT:
	       /* Read a packet that would normally be discarded.
	        * This is typically used to read the error message
	        * given with a CLS packet, or to look at the contact
	        * string for a listener.
		* The ioctl returns 2 bytes for the opcode, 2 bytes for
		* the (data) length, and the bytes of data.
	        */
               ps = spl6();
	       if ((pkt = conn->cn_expkt) == NOPKT || flatten(pkt))
	            break;
	       conn->cn_expkt = NULL;
               splx(ps);
               len = pkt->pk_len + 4 + sizeof(sp->ioc0.com);
	       if (bp->lim - bp->base < len) {
	            freeb(bp);
		    if ((bp = allocb(len)) == NOBLOCK
		              || bp->lim - bp->base < len)
                         break;
                    bp->type = M_IOCACK;
               }
	       sp = (union stmsg *)(bp->rptr = bp->base);
	       bp->wptr = bp->rptr + len;
	       sp->ioc0.com = CHIOCRPKT;
	       *(short *)(&sp->iocx.xxx[0]) = pkt->pk_op;
	       *(short *)(&sp->iocx.xxx[2]) = pkt->pk_len;
	       bcopy(pkt->data->rptr, &sp->iocx.xxx[4], pkt->pk_len);
	       free_packet(pkt);
	       goto reply;
          case CHIOCFLUSH:
	       /* Flush the current output packet if there is one.
	        * Applicable in stream mode only.
		*/
	       if (conn->cn_toutput != NOPKT) {
	            chld_write(conn, conn->cn_toutput);
		    conn->cn_toutput = NULL;
               }
	       goto noreply;
          case CHIOCOWAIT:
               /* Wait for all output to be acknowledged.
	        * If in stream mode, any pending output is flushed first.
	        * If *addr is non-zero, an EOF is also sent before waiting.
		*/
	       if (conn->cn_toutput != NOPKT) {
	            chld_write(conn, conn->cn_toutput);
		    conn->cn_toutput = NULL;
               }
               if (sp->iocx.xxx[0])
	            chld_eof(conn);
               ps = spl6();
	       if (chtempty(conn)) {
		    /* already empty */
		    splx(ps);
		    goto noreply;
	       }
	       conn->cn_sflags |= CHEMPWAIT;
	       conn->cn_wait = bp;
	       splx(ps);
	       return;
          case CHIOCGSTAT:
	       /* Return the status of the connection */
	       ps = spl6();
	       {
	            register struct chstatus *chst;

                    chst = (struct chstatus *)sp->iocx.xxx;
		    chst->st_fhost = conn->cn_faddr.ch_addr;
		    chst->st_cnum = conn->cn_lidx.ci_tidx;
		    chst->st_rwsize = conn->cn_rwsize;
		    chst->st_twsize = conn->cn_twsize;
		    chst->st_state = conn->cn_state;
		    chst->st_cmode = conn->cn_mode;
		    chst->st_oroom = conn->cn_twsize -
		                        (conn->cn_tlast - conn->cn_tacked);
                    chst->st_ptype = chst->st_plength = 0;
		    bp->wptr = (u_char *)(chst + 1);
               }
	       splx(ps);
	       goto reply;
          case CHIOCSWAIT:
	       /* Wait for the state of the connection to be different
	        * from the given state.
		*/
	       ps = spl6();
	       if (conn->cn_state != *(int *)sp->iocx.xxx) {
		    /* already out of the given state */
		    splx(ps);
		    goto noreply;
	       }
	       conn->cn_sflags |= CHSWAIT;
	       conn->cn_wait = bp;
	       splx(ps);
	       return;
          case CHIOCANSWER:
	       /* Answer an RFC.  This sets a bit saying the next write
	        * call should send an ANS packet and close the connection.
		*/
               if (conn->cn_state == CSRFCRCVD) {
	            conn->cn_flags |= CHANSWER;
		    goto noreply;
               }
	       break;
          case CHIOCREJECT:
	       /* Reject an RFC.  This closes the connection with a CLS
	        * packet containing an error message string.  The user
		* passes in a structure containing a pointer to the error
		* message (reason) and its length.
		*/
	       {
	            register struct chreject *cr;
                    int       flag, c;

                    cr = (struct chreject *)sp->iocx.xxx;
		    pkt = NOPKT;
		    flag = 0;
		    if (cr->cr_length > 0 && cr->cr_length <= CHMAXPKT) {
		         if ((pkt = new_packet()) == NOPKT)
			      ++flag;
			 else {
			      pkt->pk_op = CLSOP;
                              while (cr->cr_length-- > 0) {
			           if ((c = fubyte(cr->cr_reason++)) < 0)
					break;
			           append_packet(pkt, &c, 1);
                              }
                         }
                    }
		    chld_close(conn, pkt);
		    if (flag == 0)
		         goto noreply;
               }
	       break;
          case CHIOCACCEPT:
	       /* Accept an RFC causing the OPN packet to be sent */
               ps = spl6();
	       if (conn->cn_state == CSRFCRCVD) {
	            chld_accept(conn);
		    splx(ps);
		    goto noreply;
               }
	       splx(ps);
	       break;
     }

bad:
     bp->type = M_IOCNAK;
noreply:
     bp->wptr = bp->rptr;
reply:
     qreply(q, bp);
}

chdosrv(q)
     struct queue   *q;
{
     register struct block    *bp;

     if (chdwrite(q))
	  return;

     /* See if there's another packet on the queue */
     for (bp = q->first; bp != NOBLOCK; bp = bp->next)
	  if (bp->type == M_DELIM) {
	       qenable(q);    /* re-enable queue for the next packet */
	       break;
          }
}

chdwrite(q)
     register struct queue    *q;
{
     register struct packet   *pkt;
     register struct block    *bp, *tail;
     register struct connection        *conn;
     int       n, ps;

     if ((conn = (struct connection *)q->ptr) == NOCONN) {
          flush_packet(q);
	  return 1;
     }
     if (conn->cn_state == CSSTARTING) {
          chdstart(q, conn);
          return 0;
     }
     conn->cn_flags |= CHWRITER;

     ps = spl6();
     if (chtfull(conn)) {
	  conn->cn_sflags |= CHOQWAIT;
	  splx(ps);
	  return 1;
     }
     splx(ps);

     switch (conn->cn_mode) {
          case CHSTREAM:
               while ((bp = getq(q)) != NOBLOCK) {
                    if (bp->rptr >= bp->wptr) {
                         freeb(bp);
                         continue;
                    }
                    if (conn->cn_state != CSOPEN &&
                              (conn->cn_state != CSRFCRCVD ||
                               (conn->cn_flags & CHANSWER) == 0)) {
                         freeb(bp);
                         flush_packet(q);
                         return 1;
                    }
                    if ((pkt = conn->cn_toutput) == NOPKT) {
                         if ((pkt = new_packet()) == NOPKT) {
                              printf("chodsrv: can't allocate packet\n");
                              return 0;
                         }
                         setconn(conn, pkt);
                         pkt->pk_op = (conn->cn_flags & CHANSWER ?
                                             ANSOP : DATOP);
			 conn->cn_toutput = pkt;
                    }
                    n = CHMAXDATA - pkt->pk_len;
                    if (bp->wptr - bp->rptr >= n) {
                         append_packet(pkt, bp->rptr, n);
                         bp->rptr += n;
                         conn->cn_toutput = NOPKT;
                         if (bp->rptr < bp->wptr)
                              putbq(q, bp);
			 chld_write(conn, pkt);
                    } else {
                         bp->next = NOBLOCK;
                         if (pkt->data == NOBLOCK)
                              pkt->data = bp;
                         else {
                              for (tail = pkt->data; tail->next != NOBLOCK;)
                                   tail = tail->next;
                              tail->next = bp;
                         }
                         pkt->pk_lenword += bp->wptr - bp->rptr;
                    }
               }
               break;
          case CHRECORD:
               if ((pkt = get_packet(q, 1)) == NOPKT) {
                    printf("chdosrv: can't allocate packet\n");
                    return 0;
               }
               setconn(conn, pkt);
	       chld_write(conn, pkt);
               break;
     }

     return 0;
}

chdrint(conn, pkt)
     register struct connection    *conn;
     register struct packet   *pkt;
{
     register struct queue    *q;

     if (conn->cn_sflags & CHOPNWAIT)
          chdoreply(conn, 0, pkt, 0);
     if (ISDATA(pkt) || pkt->pk_op == EOFOP)
          conn->cn_flags |= CHREADER;

     q = conn->cn_rdq;
     if (q) {
          if (conn->cn_mode == CHSTREAM) {
	       if (pkt->pk_op == EOFOP)
	            conn->cn_flags |= CHEOFSEEN;
	       else
	            conn->cn_flags &= ~CHEOFSEEN;
               if (!READABLE(pkt)) {
	            conn->cn_expkt = pkt;
		    return;
               }
          }
          if (q->next->flag & QFULL) {
               free_packet(pkt);
               debug(DABNOR,printf("chdrint: QFULL\n"));
               return;
          }
	  ch_busy++;
          if (conn->cn_mode == CHRECORD)     /* Check for RECORD mode */
               putcpy(q->next, &pkt->pk_op, 1);
          put_pkdata(q, pkt);
	  --ch_busy;
     } else
          free_packet(pkt);
}


chd_newstate(conn)
     register struct connection    *conn;
{
     register struct queue    *qnext = conn->cn_rdq->next;
     int       ps = spl6();

     if (conn->cn_sflags & CHOPNWAIT)
	  chdoreply(conn, 0, NOPKT);

     if (conn->cn_sflags & CHSWAIT) {
          register union stmsg *sp = (union stmsg *)conn->cn_wait->rptr;

          if (conn->cn_state != *(int *)sp->iocx.xxx) {
	       conn->cn_wait->wptr = (u_char *)sp;
	       (*qnext->qinfo->putp) (qnext, conn->cn_wait);
	       conn->cn_sflags &= ~CHSWAIT;
	       conn->cn_wait = NOBLOCK;
          }
     }

     if (chdead(conn)) {      /* send an end-of-file */
          putctl(qnext, M_DELIM);
          putctl(qnext, M_DELIM);
     }

     splx(ps);
}

chd_newout(conn)
     register struct connection    *conn;
{
     int       ps = spl6();

     if (conn->cn_sflags & CHOWAIT)
          wakeup((caddr_t)&conn->cn_thead);

     if (conn->cn_sflags & CHEMPWAIT) {
          register union stmsg *sp = (union stmsg *)conn->cn_wait->rptr;

          if (chtempty(conn)) {
	       conn->cn_wait->wptr = (u_char *)sp;
	       qreply(conn->cn_rdq, conn->cn_wait);
	       conn->cn_sflags &= ~CHEMPWAIT;
	       conn->cn_wait = NOBLOCK;
          }
     }

     if (conn->cn_sflags & CHOQWAIT) {
	  conn->cn_sflags &= ~CHOQWAIT;
	  qenable(WR(conn->cn_rdq));
     }

     splx(ps);
}

#endif

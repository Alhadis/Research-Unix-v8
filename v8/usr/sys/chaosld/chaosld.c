/*
 *             C H A O S  L D
 *
 * Chaosnet line discipline, to be pushed on an ethernet controller.
 * Handles xmit and rcv windows, packet numbering, and connection states.
 *
 *
 * (c) Copyright 1985  Nirvonics, Inc.
 *
 * Written by Kurt Gollhardt
 * Last update Tue Mar 12 04:18:58 1985
 *
 * This software is the property of Nirvonics, Inc.
 * All rights reserved.
 *
 */

#include "ch.h"
#if NCH
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/stream.h"
#include "../h/ioctl.h"
#include "../h/ttyld.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../chaosld/types.h"
#define CHDEFINE
#include "../chaosld/globals.h"


extern int     chrcv(), chb_put();
int            chopen(), chclose(), chosrv(), chisrv();
static struct qinit chrinit = { chb_put, chisrv, chopen, chclose, 512, 64 };
static struct qinit chwinit = { putq, chosrv, chopen, chclose, 512, 64 };
struct streamtab chinfo = { &chrinit, &chwinit };

chopen(q, dev)
     register struct queue *q;
{
     if (q->ptr)
          return(1);
     if (ChaosQ != (struct queue *)0)   /* Already open */
          return(0);
     ChaosQ = q;     /* that's the RD q */
     q->flag |= QDELIM|QNOENB;     /* chiput calls qenable() */
     WR(q)->flag |= QDELIM;
     return(1);
}

chclose(q)
     struct queue   *q;
{
     struct connection   **connp;
     int       ps = spl6();

     for (connp = Chconn; connp < &Chconn[NCH]; ++connp)
          if (*connp != NOCONN)
	       chld_close(*connp, NOPKT);

     freelist(Chlsnlist);
     freelist(Chrfclist);
     Chlsnlist = Chrfclist = Chrfctail = NOPKT;
     Chmyaddr.ch_addr = 0;

     ChaosQ = (struct queue *)0;
     splx(ps);
}


chisrv(q)
     struct queue   *q;
{
     register struct block    *bp;

     chrcv(q);   /* Receive the packet */

     /* See if there's another packet on the queue */
     for (bp = q->first; bp != NOBLOCK; bp = bp->next)
	  if (bp->type == M_DELIM) {
	       qenable(q);    /* re-enable queue for the next packet */
	       break;
          }
}


chosrv(q)
     register struct queue *q;
{
     register union stmsg *sp;
     register struct block *bp;

     while(bp = getq(q)){
          if(bp->type == M_IOCTL){
               sp = (union stmsg *)bp->rptr;
               switch(sp->ioc0.com){
	            case CHIOCNAME:
		         copyin(*(caddr_t *)sp->iocx.xxx, Chmyname, CHSTATNAME);
                         bp->rptr = bp->wptr;
			 bp->type = M_IOCACK;
			 qreply(q, bp);
			 break;
	            case CHIOCADDR:
		         Chmyaddr.ch_addr = *(int *)sp->iocx.xxx;
			 /* fall through */
                    default:
                         (*q->next->qinfo->putp)(q->next, bp);
                         break;
               }
          } else
               (*q->next->qinfo->putp)(q->next, bp);
     }
}

chld_write(conn, pkt)
     register struct connection    *conn;
     register struct packet        *pkt;
{
     setconn(conn, pkt);

     switch (pkt->pk_op) {
          case ANSOP:
	  case FWDOP:
	       if (conn->cn_state != CSRFCRCVD ||
	                 (conn->cn_flags & CHANSWER) == 0)
                    goto err;
               chld_close(conn, pkt);
	       return 0;
          case RFCOP:
          case BRDOP:
	       if (conn->cn_state != CSRFCSENT)
	            goto err;
               break;
          case UNCOP:
	       pkt->pk_pkn = 0;
	       pkt->next = NOPKT;
	       senddata(pkt);
	       return 0;
          default:
	       if (!ISDATA(pkt))
	            goto err;
          case OPNOP:
	  case EOFOP:
	       if (conn->cn_state != CSOPEN)
	            goto err;
               setack(conn, pkt);
               break;
     }

     pkt->pk_pkn = ++conn->cn_tlast;
     senddata(pkt);
     return 0;

err:
     free_packet(pkt);
     return -1;
}


chld_eof(conn)
     struct connection   *conn;
{
     register struct packet   *pkt;

     if ((pkt = new_packet()) != NOPKT) {
	  setconn(conn, pkt);
          pkt->pk_op = EOFOP;
	  return chld_write(conn, pkt);
     }
     return 0;
}


chld_accept(conn)
     struct connection   *conn;
{
     conn->cn_state = CSOPEN;
     sendopn(conn);
}


chld_open(conn, daddr, rwsize, pkt)
     register struct connection    *conn;
     register struct packet   *pkt;
{
     conn->cn_faddr.ch_addr = daddr;
     conn->cn_state = CSRFCSENT;
     conn->cn_rwsize = rwsize;
     conn->cn_rsts = rwsize / 2;
     conn->cn_time = Chclock;
     pkt->pk_op = RFCOP;
     pkt->pk_fc = pkt->pk_ackn = 0;
     debug(DCONN,printf("Conn #%x: RFCS state\n", conn->cn_lidx.ci_idx));
     /*
      * By writing the RFC packet like a data packet, it will be kept
      * around for automatic retransmission until freed by the normal
      * receipting mechanism; xmitdone() and chrcv() facilitate this.
      * Since new_conn() clears the connection (including cn_tlast),
      * the packet number of the RFC will be 1 (pkn = ++cn_tlast).
      */
     chld_write(conn, pkt);
}


chld_listen(conn, rwsize, pkt)
     register struct connection    *conn;
     register struct packet   *pkt;
{
     register struct packet   *opkt, *pktl;
     int       ps;

     conn->cn_state = CSLISTEN;
     conn->cn_flags |= CHSERVER;
     conn->cn_rwsize = rwsize;
     pkt->pk_op = LSNOP;
     setconn(conn, pkt);

     if (flatten(pkt)) {
          free_packet(pkt);
	  return;
     }

     ps = spl6();
     opkt = NOPKT;
     for (pktl = Chrfclist; pktl != NOPKT; pktl = (opkt = pktl)->next)
          if (concmp(pktl, pkt->data->rptr, (int)pkt->pk_len)) {
	       if (opkt == NOPKT)
	            Chrfclist = pktl->next;
               else
	            opkt->next = pktl->next;
               if (pktl == Chrfctail)
	            Chrfctail = opkt;
               free_packet(pkt);
	       lsnmatch(pktl, conn);
	       chd_newstate(conn);
	       splx(ps);
	       return;
          }

     pkt->next = Chlsnlist;
     Chlsnlist = pkt;
     splx(ps);

     debug(DCONN,printf("Conn #%x: LISTEN state\n", conn->cn_lidx.ci_idx));
}


chld_close(conn, pkt)
     register struct connection    *conn;
     register struct packet        *pkt;
{
     int       ps = spl6();

     ch_busy++;
     switch (conn->cn_state) {
          case CSOPEN:
	  case CSRFCRCVD:
	       if (pkt != NOPKT) {
	            setconn(conn, pkt);
		    pkt->pk_ackn = pkt->pk_pkn = 0;
		    sendctl(pkt);
		    pkt = NOPKT;
               }
	       /* Fall into ... */
          case CSRFCSENT:
	       close_conn(conn, CSCLOSED, NOPKT);
	       break;
          case CSLISTEN:
	       rmlisten(conn);
	       break;
     }
     --ch_busy;
     splx(ps);

     if (pkt != NOPKT)
          free_packet(pkt);
}

#endif

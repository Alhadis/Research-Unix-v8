/*
 *             C H  R C V
 *
 * Chaosnet line discipline - handle a received packet.
 * Chrcv() is called by the service routine for the RD queue.
 *
 *
 * (c) Copyright 1984  Nirvonics, Inc.
 *
 * Written by Kurt Gollhardt
 * Last update Fri Nov 16 11:16:20 1984
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

#define send_los(pkt,str)     sendlos(pkt,str,sizeof(str)-1)

#define STS_AGE	    hz/4      /* Minimum pkt age for re-xmission on STS */


chrcv(q)
     struct queue   *q;
{
     register struct packet        *pkt;
     register struct connection    *conn;
     register struct sts_data      *sts;
     register unsigned   index;

     debug(DPKT,printf("Chrcv: "));
     if ((pkt = get_packet(q, 0)) == NOPKT)
          return;
     pkt->next = NOPKT;

     ch_busy++;
     if (pkt->pk_op == MNTOP)
          free_packet(pkt);
     else if (pkt->pk_op == RFCOP)
          rcvrfc(pkt);
     else if (pkt->pk_op == BRDOP)
          rcvbrd(pkt);
     /*
      * Check for various flavors of bad indices
      */
     else if ((index = pkt->pk_didx.ci_tidx) >= NCH ||
               (conn = Chconn[index]) == NOCONN ||
	       pkt->pk_didx.ci_idx != conn->cn_lidx.ci_idx) {
          debug(DPKT|DABNOR,printf("Packet with bad index: %x, op:0%o\n",
               pkt->pk_didx, pkt->pk_op));
          send_los(pkt, "Connection doesn't exist");
     /*
      * Handle responses to our RFC
      */
     } else if (conn->cn_state == CSRFCSENT)
          switch(pkt->pk_op) {
          case OPNOP:
               debug(DCONN,printf("Conn #%x: OPEN received\n", conn->cn_lidx));
               /*
                * Mark the connection as open, saving his index
                */
               conn->cn_state = CSOPEN;
               conn->cn_fidx = pkt->pk_sidx;
               conn->cn_time = Chclock;
               /*
                * Indicate we have received AND "read" the OPEN
                * Read his window size, indicate that he has
                * received and acked the RFC, and acknowledge the OPN
                */
               conn->cn_rread = conn->cn_rlast = pkt->pk_pkn;
	       flatten(pkt);
	       sts = (struct sts_data *)pkt->data->rptr;
               conn->cn_twsize = sts->sts_rwsize;
               receipt(conn, pkt->pk_ackn, pkt->pk_ackn);
               /*
                * Send him back an STS to get things going.
                */
               setsts(conn, pkt);
               reflect(pkt);
	       chd_newstate(conn);
               break;
          case CLSOP:
          case ANSOP:
          case FWDOP:
               debug(DCONN,printf("Conn #%x: CLOSE/ANS received for RFC\n", conn->cn_lidx));
               close_conn(conn, CSCLOSED, pkt);
               break;
          default:
               debug(DPKT|DABNOR,
                    printf("bad packet type for conn #%x in CSRFCSENT: %d\n",
                         conn->cn_lidx, pkt->pk_op));
               send_los(pkt, "Bad packet type reponse to RFC");
          }
     /*
      * Process a packet for an open connection
      */
     else if (conn->cn_state == CSOPEN) {
          conn->cn_time = Chclock;
          if (ISDATA(pkt))
               rcvdata(conn, pkt);
          else switch (pkt->pk_op) {
              case OPNOP:
               /*
                * Ignore duplicate opens.
                */
               debug(DPKT|DABNOR,printf("Duplicate open received\n"));
               free_packet(pkt);
               break;
          case SNSOP:
               debug(DPKT,prpkt(pkt, "SNS"));
               receipt(conn, pkt->pk_ackn, pkt->pk_ackn);
               setsts(conn, pkt);
               reflect(pkt);
               break;
          case EOFOP:
               rcvdata(conn, pkt);
               break;
          case LOSOP:
          case CLSOP:
               debug(DCONN, printf("Close rcvd on %x\n", conn->cn_lidx));
               close_conn(conn, pkt->pk_op == CLSOP ? CSCLOSED : CSLOST,
                    pkt);
               break;
               /*
                * Uncontrolled data - queue it at the head of the rlist
                */
          case UNCOP:
               receipt(conn, pkt->pk_ackn, pkt->pk_ackn);
	       ch_input(conn, pkt);  /* Send packet to pseudo-device */
               break;
          case STSOP:
               flatten(pkt);
	       sts = (struct sts_data *)pkt->data->rptr;
               if (sts->sts_rwsize > conn->cn_twsize)
                    chd_newout(conn);
               conn->cn_twsize = sts->sts_rwsize;
               receipt(conn, pkt->pk_ackn, sts->sts_receipt);
               free_packet(pkt);
	       chretran(conn, STS_AGE);  /* Re-xmit all but very recent pkts */
               break;
          default:
               debug(DPKT|DABNOR, printf("bad opcode:%d\n", pkt->pk_op));
               send_los(pkt, "Bad opcode");
               /* should we do close_conn here? */
          }
     /*
      * Connection is neither waiting for an OPEN nor OPEN.
      */
     } else {
          debug(DPKT|DABNOR,printf("Packet for conn #%x (not open) state=%d, op:%d\n", conn->cn_lidx, conn->cn_state, pkt->pk_op));
          send_los(pkt, "Connection is closed");
     }

     --ch_busy;
}

/*
 * Process a received data packet - or an EOF packet which is mostly treated
 * the same.
 */
rcvdata(conn, pkt)
     register struct connection    *conn;
     register struct packet        *pkt;
{
     register struct packet   *npkt;

     debug(DPKT,(prpkt(pkt,"DATA"),printf("\n")) );
     if (cmp_gt(pkt->pk_pkn, conn->cn_rread + conn->cn_rwsize)) {
          debug(DPKT|DABNOR,printf("Discarding data out of window\n"));
          free_packet(pkt);
          return;
     }
     receipt(conn, pkt->pk_ackn, pkt->pk_ackn);
     if (cmp_le(pkt->pk_pkn, conn->cn_rlast)) {
          debug(DPKT,printf("Duplicate data packet\n"));
          setsts(conn, pkt);
          reflect(pkt);
          return;
     }

     /*
      * Link the out of order list onto the new packet in case
      * it fills the gap between in-order and out-of-order lists
      * and to make it easy to grab all now-in-order packets from the
      * out-of-order list.
      */
     pkt->next = conn->cn_routorder;
     /*
      * Now transfer all in-order packets to the pseudo-device
      */
     for (npkt = pkt; npkt != NOPKT &&
                    npkt->pk_pkn == (unsigned short)(conn->cn_rlast + 1);
                    npkt = npkt->next) {
          conn->cn_rlast++;
          ch_input(conn, npkt);
     }
     /*
      * If we received any in-order pkts, check if spontaneous STS is needed
      */
     if (pkt != npkt) {
          debug(DPKT,printf("new ordered data packet\n"));
          conn->cn_routorder = npkt;
     } else {
     /*
      * Here we have received an out of order packet which must be
      * inserted into the out-of-order queue, in packet number order. 
      */
          for (npkt = pkt; (npkt = npkt->next) != NOPKT &&
                     cmp_gt(pkt->pk_pkn, npkt->pk_pkn); )
               pkt->next = npkt;     /* save the last pkt here */

          if (npkt != NOPKT && pkt->pk_pkn == npkt->pk_pkn) {
               debug(DPKT|DABNOR,
                    printf("Duplicate out of order packet\n"));
               pkt->next = NOPKT;
               setsts(conn, pkt);
               reflect(pkt);
          } else {
               if (npkt == conn->cn_routorder)
                    conn->cn_routorder = pkt;
               else
                    pkt->next->next = pkt;
               pkt->next = npkt;
               debug(DPKT|DABNOR,
                     printf("New out of order packet\n"));
          }
     }
}

/*
 * Process receipts and acknowledgements using recnum as the receipt.
 */
receipt(conn, acknum, recnum)
register struct connection *conn;
unsigned short acknum, recnum;
{
     register struct packet *pkt, *pktl;

     ch_busy++;
     /*
      * Process a receipt, freeing packets that we now know have been
      * received.
      */
     if (cmp_gt(recnum, conn->cn_trecvd)) {
          for (pktl = conn->cn_thead;
               pktl != NOPKT && cmp_le(pktl->pk_pkn, recnum);
               pktl = pkt) {
               pkt = pktl->next;
               free_packet(pktl);
          }
          if ((conn->cn_thead = pktl) == NOPKT)
               conn->cn_ttail = NOPKT;
          conn->cn_trecvd = recnum;
     }
     /*
      * If the acknowledgement is new, update our idea of the
      * latest acknowledged packet, and wakeup output that might be blocked
      * on a full transmit window.
      */
     if (cmp_gt(acknum, conn->cn_tacked))
          if (cmp_gt(acknum, conn->cn_tlast)) {
               debug(DABNOR, (printf("Invalid acknowledgment(%d,%d)\n",
                    acknum, conn->cn_tlast)));
          } else {
               conn->cn_tacked = acknum;
               chd_newout(conn);
          }
     --ch_busy;
}


#ifdef BRDBRIDGE

int Chbrdbridge = 1;
#define MAXBRD    8

#endif BRDBRIDGE

/*
 * Process a received BRD
 * The trickiness is that we must modify the bit map completely before
 * sending it to anyone - thus we must remember all those to send it to.
 */

rcvbrd(pkt)
     register struct packet   *pkt;
{
     int       bitlen = pkt->pk_ackn;

          /* Here pk_ackn is # bytes in subnet bitmap */
     if ((pkt->pk_len <= bitlen) || (bitlen >= CHNSUBNET/8)) {
          free_packet(pkt);
          return;
     }
     if (flatten(pkt))
          return;

#ifdef BRDBRIDGE
     if (Chbrdbridge) {
          register struct chroute  *r;
          register int   mask, i;
          struct packet  *npkt;
	  chaddr    addrs[MAXBRD], *adp;

          bitp = pkt->data->rptr;
          for (i = 0, r = Chroutetab; r < bitlen; i++)
               for (mask = 1; mask & 0377; mask <<= 1, r++)
                    if (r->rt_type == CHDIRECT && r->rt_cost < HIGH_COST &&
                              *bitp & mask && adp < &addrs[MAXBRD]) {
                         *bitp &= ~mask;
			 *adp++ = r->rt_path.ifp->my_addr;
                    }
          while (adp > addrs) {			 
               if ((npkt = new_packet()) == NOPKT)
                    return;
               npkt->ph = pkt->ph;
               append_packet(npkt, pkt->data->rptr, pkt->pk_len);
               npkt->pk_daddr = *--adp;
               npkt->pk_daddr.ch_host = 0;
	       npkt->pk_pkn = 1;
               sendctl(npkt);
          }
     }
#endif BRDBRIDGE

     pkt->pk_lenword = pkt->pk_len - bitlen;
     pkt->data->rptr += bitlen;
     pkt->pk_ackn = 0;
     rcvrfc(pkt);
}

/*
 * Process a received RFC
 */
rcvrfc(pkt)
     register struct packet   *pkt;
{
     register struct connection    *conn, **conptr;
     register struct service  *svp;
     extern struct service    Chservice[];
     struct packet  **opkt, *pktl;

     debug(DPKT,prpkt(pkt,"RFC/BRD"));
     /*
      * Check if this is a duplicate RFC, and if so throw it away,
      * and retransmit the OPEN.
      */
     for (conptr = &Chconn[0]; conptr < &Chconn[NCH];)
          if ((conn = *conptr++) != NOCONN &&
                    conn->cn_fidx.ci_idx == pkt->pk_sidx.ci_idx &&
                     conn->cn_faddr.ch_addr == pkt->pk_saddr.ch_addr) {
               if (conn->cn_state == CSOPEN) {
                    debug(DPKT|DABNOR,
                         printf("Rcvrfc: Retransmitting open chan #%x\n",
			           (*conptr)->cn_lidx.ci_idx));
                    chretran(conn, 0);
               } else {
                    debug(DPKT|DABNOR,
                         printf("Rcvrfc: Duplicate RFC: %x\n",
                              conn->cn_lidx.ci_idx));
               }
               free_packet(pkt);
               return;
          }

     if (flatten(pkt)) {
          free_packet(pkt);
	  return;
     }

     /*
      * Scan the listen list for a listener and if one is found
      * open the connection and remove the listen packet from the
      * listen list.
      */
     for (opkt = &Chlsnlist; (pktl = *opkt) != NOPKT; opkt = &pktl->next)
          if(concmp(pkt, pktl->data->rptr, (int)pktl->pk_len)) {
               conn = Chconn[pktl->pk_sidx.ci_tidx];
               *opkt = pktl->next;
               free_packet(pktl);
               lsnmatch(pkt, conn);
               chd_newstate(conn);
               return;
          }
     /*
      * Check for built-in services.
      */
     for (svp = Chservice; svp->name != 0; ++svp)
          if (concmp(pkt, svp->name, svp->len)) {
	       (*svp->func)(pkt);
	       return;
          }
     /*
      * There was no listener, so queue the RFC on the unmatched RFC list
      * again checking for duplicates.
      */
     if ((pktl = Chrfclist) == NOPKT) 
          Chrfclist = Chrfctail = pkt;
     else {
          do {
               if(pktl->pk_sidx.ci_idx == pkt->pk_sidx.ci_idx &&
                  pktl->pk_saddr.ch_addr == pkt->pk_saddr.ch_addr) {
                    debug(DPKT/*|DABNOR*/,printf("Rcvrfc: Discarding duplicate Rfc on Chrfclist\n"));
                    free_packet(pkt);
                    return;
               }
          } while ((pktl = pktl->next) != NOPKT);
          Chrfctail->next = pkt;
          Chrfctail = pkt;
     }
     debug(DCONN,printf("Rcvrfc: Queued Rfc on Chrfclist\n"));
     /*
      * A new unmatched RFC has been received.  Send a copy of it up to the
      * unmatched RFC server.
      */
     copy_pkdata(ChaosQ, pkt);
}

/*
 * An RFC has matched a listener, either by an RFC coming and finding a match
 * on the listener list, or by a listen being done and matching an RFC on the
 * unmatched RFC list. So we change the state of the connection to CSRFCRCVD
 */
lsnmatch(rfcpkt, conn)
     register struct packet        *rfcpkt;
     register struct connection    *conn;
{
     debug(DCONN,printf("Conn #%x: LISTEN matched \n", conn->cn_lidx));
     /*
      * Initialize the conection
      */
     conn->cn_time = Chclock;
     conn->cn_state = CSRFCRCVD;
     if (conn->cn_rwsize == 0)
          conn->cn_rwsize = CHDRWSIZE;
     conn->cn_faddr = rfcpkt->pk_saddr;
     conn->cn_fidx = rfcpkt->pk_sidx;

     /*
      * Indicate to the other end that we have received and "read"
      * the RFC so that the open will truly acknowledge it.
      */
     conn->cn_rlast = conn->cn_rread = rfcpkt->pk_pkn;
     /*
      * Queue up the RFC for the user to read if he wants it.
      */
     chdrint(conn, rfcpkt);
}

/*
 * Remove a listener from the listener list, due to the listener bailing out.
 * Called from top level at high priority
 */
rmlisten(conn)
     register struct connection    *conn;
{
     register struct packet   *opkt, *pkt;

     opkt = NOPKT;
     for (pkt = Chlsnlist; pkt != NOPKT; opkt = pkt, pkt = pkt->next)
          if (pkt->pk_sidx.ci_tidx == conn->cn_lidx.ci_tidx) {
               if(opkt == NOPKT)
                    Chlsnlist = pkt->next;
               else
                    opkt->next = pkt->next;
               free_packet(pkt);
               break;
          }
}

/*
 * Compare the RFC contact name with the listener name.
 */
concmp(rfcpkt, lsnstr, lsnlen)
     struct packet  *rfcpkt;
     register char  *lsnstr;
     register int   lsnlen;
{
     register char *rfcstr = (char *)rfcpkt->data->rptr;
     register int rfclen;

     debug(DPKT,{printf("Rcvrfc: Comparing ");
                 print_str(rfcpkt->pk_len, rfcstr);
		 printf(" and ");
		 print_str(lsnlen, lsnstr);
		 printf("\n");});
     for (rfclen = rfcpkt->pk_len; rfclen; rfclen--, lsnlen--)
          if (lsnlen <= 0)
               return ((*rfcstr == ' ') ? 1 : 0);
          else if (*rfcstr++ != *lsnstr++)
               return(0);
     return (lsnlen == 0);
}


ch_input(conn, pkt)
     struct connection   *conn;
     struct packet       *pkt;
{
     int       pkn, op, controlled;

     pkn = pkt->pk_pkn;
     op = pkt->pk_op;
     controlled = CONTROLLED(pkt);

     chdrint(conn, pkt);

     if (controlled) {
          conn->cn_rread = pkn;
	  if (op == EOFOP)
	       sendsts(conn);
     }
}


#ifdef DEBUG
prpkt(pkt, str)
register struct packet *pkt;
char *str;
{
     printf("op=%s(%o) len=%d fc=%d\ndhost=%o didx=%x\nshost=%o sidx=%x\npkn=%d ackn=%d\n",
          str, pkt->pk_op, pkt->pk_len, pkt->pk_fc, pkt->pk_daddr.ch_addr,
          pkt->pk_didx.ci_idx, pkt->pk_saddr.ch_addr, pkt->pk_sidx.ci_idx,
          (unsigned)pkt->pk_pkn, (unsigned)pkt->pk_ackn);
}

print_str(len, s)
     register int   len;
     register char  *s;
{
     while (len-- > 0 && *s != '\0')
          printf("%c", *s++);
}
#endif DEBUG

#endif

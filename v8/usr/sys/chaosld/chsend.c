/*
 *             C H  S E N D
 *
 * Chaosnet line discipline - routines to deal with sending out packets.
 *
 *
 * (c) Copyright 1985  Nirvonics, Inc.
 *
 * Written by Kurt Gollhardt
 * Last update Thu Feb 28 14:42:57 1985
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
#include "../chaosld/chrouteld.h"

/*
 * Send a STS packet on this connection
 *     if allocation fails it is not sent
 */
sendsts(conn)
register struct connection *conn;
{
     register struct packet *pkt;

     if ((pkt = new_packet()) != NOPKT) {
          setconn(conn, pkt);
          setsts(conn, pkt);
          sendctl(pkt);
     }
}

/*
 * Send a SNS packet on this connection
 *     if allocation failed nothing is sent
 */
sendsns(conn)
     register struct connection    *conn;
{
     register struct packet   *pkt;

     if ((pkt = new_packet()) != NOPKT) {
          setconn(conn, pkt);
          pkt->pk_op = SNSOP;
          setack(conn, pkt);
          sendctl(pkt);
     }
}

/*
 * Send an open in response to an RFC
 */

sendopn(conn)
     register struct connection    *conn;
{
     register struct packet   *pkt;

     if ((pkt = new_packet()) != NOPKT) {
          conn->cn_rsts = conn->cn_rwsize >> 1;
	  conn->cn_tlast = 0;
	  setconn(conn, pkt);
	  setsts(conn, pkt);
	  pkt->pk_op = OPNOP;
          chld_write(conn, pkt);
     }
}

/*
 * Sendctl - send a control packet that will not be acknowledged and
 *     will not be retransmitted (this actual packet).
 *     If anything is wrong (no path, bad subnet) we just throw it
 *     away. Remember, next is assumed to be == NOPKT.
 */
sendctl(pkt)
register struct packet *pkt;
{
     register struct chroute *r;

     debug(DSEND, (printf("Sending: %d ", pkt->pk_op), prpkt(pkt, "ctl"), printf("\n")));
     ch_busy++;
     put_packet(WR(ChaosQ), pkt);
     --ch_busy;
}

/*
 * Senddata - send a list of controlled packets.
 *     Similar to sendctl, but stuffs time, handles a list, and marks
 *     the packets as transmitted.
 */
senddata(pkt)
register struct packet *pkt;
{
     register struct packet   *nextpk;

     debug(DSEND, (printf("Sending: %d ", pkt->pk_op), prpkt(pkt, "data"), printf("\n")));
     ch_busy++;
     do {
          pkt->time = Chclock;
          nextpk = pkt->next;
          copy_packet(WR(ChaosQ), pkt);
          xmitdone(pkt);
     } while ((pkt = nextpk) != NOPKT);
     --ch_busy;
}

/*
 * Send the given RUT packet out on the given tranceiver, which has the given
 * cost.  If the "copy" flag is true, make a copy of the packet.
 * Note that if copy is not set, the packet data gets modified.
 */
sendrut(pkt, ifp, cost, copy)
register struct packet *pkt;
register struct chif *ifp;
unsigned short cost;
int copy;
{
     register struct rut_data *rd;
     struct rut_data *rdend;

     pkt->pk_saddr = ifp->my_addr;
     pkt->pk_daddr.ch_subnet = ifp->my_addr.ch_subnet;

     rdend = (struct rut_data *)(pkt->data->rptr + pkt->pk_len);
     for (rd = (struct rut_data *)pkt->data->rptr; rd < rdend; rd++)
          rd->rd_cost += cost;

     ch_busy++;
     if (copy) {
	  copy_packet(WR(ChaosQ), pkt);
	  for (rd = (struct rut_data *)pkt->data->rptr; rd < rdend; rd++)
	       rd->rd_cost -= cost;
     } else
	  put_packet(WR(ChaosQ), pkt);
     --ch_busy;
}

/*
 * Send a LOS in response to the given packet.
 * Don't bother if the packet is itself a LOS or a CLS since the other
 * end doesn't care anyway and would only return it again.
 * Append the host name to the error message.
 */
sendlos(pkt, str, msglen)
     register struct packet   *pkt;
     char      *str;
     int       msglen;
{
     if (pkt->pk_op == LOSOP || pkt->pk_op == CLSOP)
          free_packet(pkt);
     else {
          register struct block    *bp;
          register char  *cp;
          register int   namelen;

	  for (namelen = 0, cp = Chmyname; *cp++;)
	       ++namelen;

          free_pkdata(pkt);
          append_packet(pkt, str, msglen);
          append_packet(pkt, " (from ", 7);
          append_packet(pkt, Chmyname, namelen);
          append_packet(pkt, ")", 1);

          pkt->pk_op = LOSOP;
          reflect(pkt);
     }
}

/*
 * Send a control packet back to its source
 */
reflect(pkt)
     register struct packet   *pkt;
{
     register short temp;

     temp = pkt->pk_sidx.ci_idx;
     pkt->pk_sidx = pkt->pk_didx;
     pkt->pk_didx.ci_idx = temp;
     temp = pkt->pk_saddr.ch_addr;
     pkt->pk_saddr = pkt->pk_daddr;
     pkt->pk_daddr.ch_addr = temp;
     sendctl(pkt);
}

/*
 * Indicate that actual transmission of the current packet has been completed.
 * Note that this merely means that we've sent it on to the routing line
 * discipline, and there's no guarantee that it's really gone anywhere.
 *
 * If this is a controlled packet and the connection is open, put the packet
 * on the queue of transmitted packets awaiting acknowledgements, otherwise
 * throw away the packet.
 */
xmitdone(pkt)
     register struct packet   *pkt;
{
     register struct connection *conn;
     register struct packet   *npkt;

     ch_busy++;
     if (CONTROLLED(pkt) && pkt->pk_sidx.ci_tidx < NCH &&
               (conn = Chconn[pkt->pk_sidx.ci_tidx]) != NOCONN &&
               pkt->pk_sidx.ci_idx == conn->cn_lidx.ci_idx &&
               (conn->cn_state == CSOPEN || conn->cn_state == CSRFCSENT) &&
               cmp_gt(pkt->pk_pkn, conn->cn_trecvd)) {
          pkt->time = Chclock;
          if ((npkt = conn->cn_thead) == NOPKT || 
                    cmp_lt(pkt->pk_pkn, npkt->pk_pkn)) {
               pkt->next = npkt;
               conn->cn_thead = pkt;
          } else {
               for(; npkt->next != NOPKT; npkt = npkt->next)
                    if(cmp_lt(pkt->pk_pkn, npkt->next->pk_pkn))
                         break;
               pkt->next = npkt->next;
               npkt->next = pkt;
          }
          if(pkt->next == NOPKT)
               conn->cn_ttail = pkt;
     } else
          free_packet(pkt);
     --ch_busy;
}

#endif

#include "ch.h"
#if NCH > 0
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/chaos.h"
#include "../chaosld/types.h"
#include "../chaosld/constants.h"
#include "../chaosld/globals.h"

/*
 * Clock level processing.
 *	ch_timer should be called each clock tick (HZ per second)
 *	at a priority equal to or higher than LOCK.
 *
 * Terminology:
 *    Packet aging:	Retransmitting packets that are not acked within
 *			AGERATE ticks
 *
 *    Probe:		The sending of a SNS packet if not all of the packets
 *			we have sent have been acknowledged
 *
 *    Responding:	Send a SNS every so often to see if the guy is still
 *			alive (after NRESPONDS we declare him dead)
 *
 *    RFC aging:	The retransmission of RFC packets
 *
 * These rates might want to vary with the cost of getting to the host.
 *
 * Since these rates are dependent on a run-time variable
 * (This is a good idea if you think about it long enough),
 * We might want to initialize specific variables at run-time to
 * avoid recalculation if the profile of chclock is disturbing.
 */
#define MINRATE     ORATE          /* Minimum of following rates */

#define AGERATE     (hz)           /* Re-xmit pkt if not rcptd in time */
#define PROBERATE   (hz<<3)        /* Send SNS to get STS for receipts or
                                        to make sure the conn. is alive */
#define ORATE       (hz>>1)        /* Xmit current (stream) output packet
                                        if not touched in this time */
#define TESTRATE    (hz*45)        /* Respond testing rate */
#define NRESPONDS   3              /* Test this many times before timing
                                        out the connection */
#define UPTIME      (NRESPONDS*TESTRATE)     /* Nothing in this time and
                                        the connection is flushed */
#define RFCTIME     (hz*15)        /* Time before giving up on  RFC */

extern int NChopen;


ch_timer()
{
     register struct connection *conn;
     register struct connection **connp;
     register struct packet *pkt;
     chtime inactive;
     int probing;                  /* are we probing this time ? */
     static chtime nextclk = 1;    /* next time to do anything */
     static chtime nextprobe = 1;  /* next time to probe */

     Chtimer = 1;
     ++Chclock;
     if (ch_busy)
          goto leave;
     if (cmp_le(Chclock, nextclk))
          goto leave;
     nextclk += MINRATE;
     if (cmp_gt(Chclock, nextprobe)) {
          probing = 1;
          nextprobe += PROBERATE;
     } else
          probing = 0;
     debug(DNOCLK,goto leave);

     for (connp = &Chconn[0]; connp < &Chconn[NCH]; connp++) {
          if ((conn = *connp) == NOCONN ||
		    (conn->cn_state != CSOPEN && conn->cn_state != CSRFCSENT))
               continue;
          if ((pkt = conn->cn_toutput) != NOPKT &&
		    (inactive = Chclock - pkt->time) >= ORATE &&
		    !chtfull(conn)) {
               conn->cn_toutput = NOPKT;
               (void)chld_write(conn, pkt);
          }
          if (conn->cn_thead != NOPKT)
               chretran(conn, AGERATE);
          if (probing) {
               inactive = Chclock - conn->cn_time;
               if (inactive >= (conn->cn_state == CSOPEN ? UPTIME : RFCTIME)) {
                    debug(DCONN|DABNOR,
                         printf("Conn #%x: Timeout\n", conn->cn_lidx.ci_tidx));
                    close_conn(conn, CSINCT, NOPKT);
               } else if (conn->cn_state == CSOPEN &&
			 (conn->cn_tacked != conn->cn_tlast ||
			  chtfull(conn) || inactive >= TESTRATE)) {
                    debug(DCONN, printf("Conn #%x: Probe: %D\n",
                                  conn->cn_lidx.ci_tidx, inactive));
                    sendsns(conn);
               }
          }
     }

leave:
     if (NChopen == 0)
	  Chtimer = 0;
     else
	  timeout(ch_timer, (caddr_t)0, 1);
}

chretran(conn, age)
struct connection *conn;
{
     register struct packet *pkt, **opkt;
     register struct packet *lastpkt;
     register chtime inactive;
     struct packet *firstpkt = NOPKT;

     ch_busy++;

     for (opkt = &conn->cn_thead; pkt = *opkt;) {
	  inactive = Chclock - pkt->time;
          if (inactive >= age) {
               if (firstpkt == NOPKT) 
                    firstpkt = pkt;
               else
                    lastpkt->next = pkt;
               lastpkt = pkt;
               *opkt = pkt->next;
               pkt->next = NOPKT;
               setack(conn, pkt);
          } else
               opkt = &pkt->next;
     }

     --ch_busy;

     if (firstpkt != NOPKT) {
          debug(DCONN|DABNOR,
               printf("Conn #%x: Rexmit (op:%d, pkn:%d)\n",
                    conn->cn_lidx.ci_tidx, firstpkt->pk_op,
                    firstpkt->pk_pkn));
          senddata(firstpkt);
     }
}

#endif

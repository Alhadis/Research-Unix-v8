#include "../chunix/chsys.h"
#include "../chunix/chconf.h"
#include "../chaos/chaos.h"

/*
 * Clock level processing.
 *	ch_clock should be called each clock tick (HZ per second)
 *	at a priority equal to or higher that LOCK.
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
 *    Route aging:	Increase the cost of transmitting over gateways so we
 *			will use another gateway if the current gateway goes
 *			down.
 *    Route broadcast:	If we are connected to more than one subnet, broad
 *			cast our bridge status every BRIDGERATE seconds.
 *
 *    Interface hung:	Checking periodically for dead interfaces (or dead
 *			"other end"'s of point-to-point links).
 *
 * These rates might want to vary with the cost of getting to the host.
 * They also might want to reside in the chconf.h file if they are not a real
 * network standard.
 *
 * Since these rates are dependent on a run-time variable
 * (This is a good idea if you think about it long enough),
 * We might want to initialize specific variables at run-time to
 * avoid recalculation if the profile of chclock is disturbing.
 */
#define MINRATE		ORATE		/* Minimum of following rates */
#define HANGRATE	(Chhz>>1)	/* How often to check for hung
					   interfaces */
#define AGERATE		(Chhz)		/* Re-xmit pkt if not rcptd in time */
#define PROBERATE	(Chhz<<3)	/* Send SNS to get STS for receipts or
					   to make sure the conn. is alive */
#define ORATE		(Chhz>>1)	/* Xmit current (stream) output packet
					   if not touched in this time */
#define TESTRATE	(Chhz*45)	/* Respond testing rate */
#define ROUTERATE	(Chhz<<2)	/* Route aging rate */
#define BRIDGERATE	(Chhz*15)	/* Routing broadcast rate */
#define NRESPONDS	3		/* Test this many times before timing
					   out the connection */
#define UPTIME  	(NRESPONDS*TESTRATE)	/* Nothing in this time and
					   the connection is flushed */
#define RFCRATE		(Chhz*5)		/* Retransmit RFC's this often */
#define RFCTIME		(RFCRATE*CHRFCTRYS)	/* Try CHRFCTRYS times to RFC */

chtime Chclock;

ch_clock()
{
	register struct connection *conn;
	register struct connection **connptr;
	register struct packet *pkt;
	chtime inactive;
	int probing;			/* are we probing this time ? */
	static chtime nextclk = 1;	/* next time to do anything */
	static chtime nextprobe = 1;	/* next time to probe */
	static chtime nexthang = 1;	/* next time to chxtime() */
	static chtime nextroute = 1;	/* next time to age routing */
	static chtime nextbridge = 1;	/* next time to send routing */

	if (nextclk != ++Chclock)
		return;
	nextclk += MINRATE;
	if (cmp_gt(Chclock, nextprobe)) {
		probing = 1;
		nextprobe += PROBERATE;
	} else
		probing = 0;
	if (cmp_gt(Chclock, nexthang)) {
		chxtime();
		nexthang += HANGRATE;
	}
	if (cmp_gt(Chclock, nextroute)) {
		chroutage();
		nextroute += ROUTERATE;
	}
	if (cmp_gt(Chclock, nextbridge)) {
		chbridge();
		nextbridge += BRIDGERATE;
	}
	debug(DNOCLK,return);
	for (connptr = &Chconntab[0]; connptr < &Chconntab[CHNCONNS]; connptr++)
		if ((conn = *connptr) == NOCONN)
			continue;
		else if (conn->cn_state == CSOPEN) {
#ifdef CHSTRCODE
			/*
			 * Timeout the current output stream packet.
			 * The timeout value should vary per connection.
			 * (shades of x.29!!)
			 */
			if ((pkt = conn->cn_toutput) != NOPKT &&
			    cmp_gt(Chclock, pkt->pk_time + ORATE) &&
			    !chtfull(conn)) {
				conn->cn_toutput = NOPKT;
				/*
				 * We don't care if the packet will
				 * be lost since either the connection
				 * is no longer open anyway, or an
				 * ANSOP was sent in the wrong state.
				 */
				(void)ch_write(conn, pkt);
			}
#endif
			if (conn->cn_thead != NOPKT)
				clkretran(conn);
			if (probing) {
				inactive = Chclock - conn->cn_active;
				if (inactive >= UPTIME)
					chdead(conn);
				else if (conn->cn_tacked != conn->cn_tlast &&
					 inactive >= PROBERATE ||
					 inactive >= TESTRATE) {
					debug(DCONN,
						printf("Conn #%x: Probe: %D\n",
							conn->cn_lidx,
							inactive));
					sendsns(conn);
				}
			}
		} else if (conn->cn_state == CSRFCSENT) {
			/*
			 * The RFC packet, if it has finished being sent out
			 * will be at cn_thead.  If it is still in the process
			 * of being sent it will not be there yet.
			 */
			pkt = conn->cn_thead;
			inactive = Chclock - conn->cn_active;
			if (inactive >= RFCTIME) {
				debug(DCONN|DABNOR, printf("Conn #%x: RFC Timeout\n",conn->cn_lidx));
				clsconn(conn, CSCLOSED, NOPKT);
			} else if (pkt != NOPKT &&
				   cmp_gt(Chclock, pkt->pk_time + RFCRATE)) {
				debug(DCONN|DABNOR,printf("Conn #%x: RFC Retransmit\n",conn->cn_lidx));
				conn->cn_ttail = conn->cn_thead = NOPKT;
				senddata(pkt);
			}
		}
}

clkretran(conn)
struct connection *conn;
{
	register struct packet *pkt, **opkt;
	register struct packet *lastpkt;
	struct packet *firstpkt = NOPKT;

	for (opkt = &conn->cn_thead; pkt = *opkt;)
		if (cmp_gt(Chclock, pkt->pk_time + AGERATE)) {
			if (firstpkt == NOPKT) 
				firstpkt = pkt;
			else
				lastpkt->pk_next = pkt;
			lastpkt = pkt;
			*opkt = pkt->pk_next;
			pkt->pk_next = NOPKT;
		} else
			opkt = &pkt->pk_next;
	if (firstpkt != NOPKT) {
		debug(DCONN|DABNOR,
			printf("Conn #%x: Rexmit (op:%d, pkn:%d)\n",
				conn->cn_lidx, firstpkt->pk_op,
				firstpkt->pk_pkn));
		senddata(firstpkt);
	}
}
/*
 * The connection has been inactive too long, close it.
 */
chdead(conn)
register struct connection *conn;
{
	static char tomsg[] = "Foreign host not responding";
	register struct packet *pkt;

	debug(DCONN|DABNOR,printf("Conn #%x: Timeout\n", conn->cn_lidx));
	if ((pkt = pkalloc(sizeof(tomsg), 1)) != NOPKT) {
		pkt = pktstr(pkt, tomsg, sizeof(tomsg));
		pkt->pk_op = LOSOP;
	}
	clsconn(conn, CSINCT, pkt);
}
/*
 * Increase the cost of accessing a subnet via a gateway
 */
chroutage()
{
	register struct chroute *r;

	for (r = Chroutetab; r < &Chroutetab[CHNSUBNET]; r++)
		if ((r->rt_type == CHBRIDGE || r->rt_type == CHDIRECT) &&
		    r->rt_cost < CHHCOST)
			r->rt_cost++;
}
/*
 * Send routing packets on all directly connected subnets, unless we are on
 * only one.
 */
chbridge()
{
	register struct chroute *r;
	register struct packet *pkt;
	register struct rut_data *rd;
	register int ndirect;
	register int n;

	/*
	 * Count the number of subnets to which we are directly connected.
	 * If not more than one, then we are not a bridge and shouldn't
	 * send out routing packets at all.
	 * While we're at it, count the number of subnets we know we
	 * have any access to.  This number determines the size of the
	 * routine packet we need to send, if any.
	 */
	n = ndirect = 0;
	for (r = Chroutetab; r <= &Chroutetab[CHNSUBNET]; r++)
		switch(r->rt_type) {
		case CHDIRECT:
			ndirect++;
		default:
			n++;
			break;
		case CHNOPATH:
			;	
		}
	if (ndirect <= 1 ||
	    (pkt = pkalloc(n * sizeof(struct rut_data), 1)) == NOPKT)
		return;
	/*
	 * Build the routing packet to send out on each directly connected
	 * subnet.  It is complete except for the cost of the directly
	 * connected subnet we are sending it out on.  This cost must be
	 * added to each entry in the packet each time it is sent.
	 */
	pkt->pk_len = n * sizeof(struct rut_data);
	pkt->pk_op = RUTOP;
	pkt->pk_type = pkt->pk_daddr = pkt->pk_sidx = pkt->pk_didx = 0;
	pkt->pk_next = NOPKT;
	rd = pkt->pk_rutdata;
	for (n = 0, r = Chroutetab; r < &Chroutetab[CHNSUBNET]; r++, n++)
		if (r->rt_type != CHNOPATH) {
			rd->pk_subnet = n;
			rd->pk_cost = r->rt_cost;
			rd++;
		}
	/*
	 * Now send out this packet on each directly connected subnet.
	 * ndirect becomes zero on last such subnet.
	 */
	for (r = Chroutetab; r < &Chroutetab[CHNSUBNET]; r++)
		if (r->rt_type == CHDIRECT)
			sendrut(pkt, r->rt_xcvr, r->rt_cost, --ndirect);
}

#include "../chunix/chsys.h"
#include "../chunix/chconf.h"
#include "../chaos/chaos.h"

/*
 * Send a STS packet on this connection
 *	if allocation fails it is not sent
 */
sendsts(conn)
register struct connection *conn;
{
	register struct packet *pkt;

	if ((pkt = pkalloc(sizeof(struct sts_data), 1)) != NOPKT) {
		setpkt(conn, pkt);
		makests(conn, pkt);
		sendctl(pkt);
	}
}

/*
 * Send a SNS packet on this connection
 *	if allocation failed nothing is sent
 */
sendsns(conn)
register struct connection *conn;
{
	register struct packet *pkt;

	if ((pkt = pkalloc(0, 1)) != NOPKT) {
		setpkt(conn, pkt);
		pkt->pk_op = SNSOP;
		pkt->pk_lenword = 0;
		pkt->pk_ackn = conn->cn_racked = conn->cn_rread;
		sendctl(pkt);
	}
}

/*
 * Send an open in response to a RFC
 *	return NOPKT is allocation fails else, open packet
 */
sendopn(conn)
register struct connection *conn;
{
	register struct packet *pkt;

	if ((pkt = pkalloc(sizeof(struct sts_data), 1)) != NOPKT) {
		conn->cn_rsts = conn->cn_rwsize >> 1;
		conn->cn_tlast = 0;
		pkt->pk_op = OPNOP;
		pkt->pk_len = sizeof(struct sts_data);
		pkt->pk_receipt = conn->cn_rlast;
		pkt->pk_rwsize = conn->cn_rwsize;
		debug(DCONN,printf("Conn #%x: open sent\n",conn->cn_lidx));
		(void)ch_write(conn, pkt);
	}
}


/*
 * Following are functions and macros that deal with tranmitters directly
 *
 * Macro XMITHEAD (pkt, xcvr) queues pkt on xcvr for transmission, putting
 * it at the head of the queue for "quick" action.  The transmitter is
 * started if no packet is currently being transmitted.
 * It is assumed that the pkt is a single one and not a list.
 * Macro XMITTAIL is similar except that is queues the packet at the tail
 * of the transmit list, treats the pkt argument as a list, and puts a time
 * stamp in each packet in the list.
 * Note that XMITTAIL modifies the variable pkt.
 */
#define XMITHEAD(pkt, axcvr) \
		{ 	register struct chxcvr *xcvr = axcvr; \
			pkt->pk_next = xcvr->xc_list; \
			xcvr->xc_list = pkt; \
			if (xcvr->xc_tail == NOPKT) { \
				xcvr->xc_tail = pkt; \
				if (xcvr->xc_tpkt == NOPKT) \
					(*xcvr->xc_start)(xcvr); \
			} \
		}
/*
 * Sendctl - send a control packet that will not be acknowledged and
 *	will not be retransmitted (this actual packet).  Put the
 *	given packet at the head of the transmit queue so it is transmitted
 *	"quickly", i.e. before data packets queued at the tail of the queue.
 *	If anything is wrong (no path, bad subnet) we just throw it
 *	away. Remember, pk_next is assumed to be == NOPKT.
 */
sendctl(pkt)
register struct packet *pkt;
{
	register struct chroute *r;

	debug(DSEND, (printf("Sending: %d ", pkt->pk_op), prpkt(pkt, "ctl"), printf("\n")));
	if (pkt->pk_dsubnet >= CHNSUBNET ||
	    (r = &Chroutetab[pkt->pk_dsubnet])->rt_type == CHNOPATH)
		ch_free((char *)pkt);
	else if (pkt->pk_daddr == Chmyaddr)
		sendtome(pkt);
	else {
		if (r->rt_type == CHFIXED || r->rt_type == CHBRIDGE) {
			pkt->pk_xdest = r->rt_addr;
			r = &Chroutetab[r->rt_subnet];
		} else
			pkt->pk_xdest = pkt->pk_daddr;
		XMITHEAD(pkt, r->rt_xcvr);
	}
}
/*
 * Senddata - send a list of controlled packets, all assumed to be to the
 *	same destination.  Queue them on the end of the appropriate transmit
 *	queue.
 *	Similar to sendctl, but stuffs time, handles a list, and puts pkts
 *	at the end of the queue instead of at the beginning, and fakes
 *	transmission if it can't really send the packets (as opposed to
 *	throwing the packets away)
 */
senddata(pkt)
register struct packet *pkt;
{
	register struct chroute *r;

	debug(DSEND, (printf("Sending: %d ", pkt->pk_op), prpkt(pkt, "data"), printf("\n")));
	if (pkt->pk_dsubnet >= CHNSUBNET ||
	    (r = &Chroutetab[pkt->pk_dsubnet])->rt_type == CHNOPATH) {
		struct packet *npkt;

		do {
			npkt = pkt->pk_next;
			pkt->pk_next = NOPKT;
			xmitdone(pkt);
		} while ((pkt = npkt) != NOPKT);
	} else if (pkt->pk_daddr == Chmyaddr)
		sendtome(pkt);
	else {
		register struct chxcvr *xcvr;
		register unsigned short dest;

		if (r->rt_type == CHFIXED || r->rt_type == CHBRIDGE) {
			dest = r->rt_addr;
			r = &Chroutetab[r->rt_subnet];
		} else
			dest = pkt->pk_daddr;
		xcvr = r->rt_xcvr;
		if (xcvr->xc_list == NOPKT)
			xcvr->xc_list = pkt;
		else
			xcvr->xc_tail->pk_next = pkt;
		for (;;) {
			pkt->pk_time = Chclock;
			pkt->pk_xdest = dest;
			if (pkt->pk_next == NOPKT)
				break;
			pkt = pkt->pk_next;
		}
		xcvr->xc_tail = pkt; 
		if (xcvr->xc_tpkt == NOPKT)
			(*xcvr->xc_start)(xcvr);
	}
}
/*
 * Send the given RUT packet out on the given tranceiver, which has the given
 * cost.  If the "copy" flag is true, make a copy of the packet.
 * Note that if copy is not set, the packet data gets modified.
 */
sendrut(pkt, axcvr, cost, copy)
register struct packet *pkt;
register struct chxcvr *axcvr;
unsigned short cost;
int copy;
{
	register struct rut_data *rd;
	struct rut_data *rdend;

	if (copy) {
		struct packet *npkt;

		if ((npkt = pkalloc((int)pkt->pk_len, 1)) == NOPKT)
			return;
		movepkt(pkt, npkt);
		pkt = npkt;
	}
	rdend = (struct rut_data *)(pkt->pk_cdata + pkt->pk_len);
	for (rd = pkt->pk_rutdata; rd < rdend; rd++)
		rd->pk_cost += cost;
	pkt->pk_saddr = axcvr->xc_addr;
	pkt->pk_xdest = 0;
	XMITHEAD(pkt, axcvr);
}
/*
 * Send the (list of) packet(s) to myself - NOTE THIS CAN BE RECURSIVE!
 */
sendtome(pkt)
register struct packet *pkt;
{
	register struct packet *rpkt, *npkt;
	static struct chxcvr fakexcvr;

	/*
	 * Static structure is used to economize on stack space.
	 * We are careful to use it very locally so that recursion still
	 * works. Cleaner solutions don't recurse very well.
	 * Basically for each packet in the list, prepare it for
	 * transmission by executing xmitnext,
	 * complete the transmission by calling xmitdone,
	 * then receive the packet on the other side, possibly
	 * causing this routine to recurse (after we're done with the
	 * static structure until next time around the loop)
	 * When this routine is called with a list of data packets, things
	 * can get pretty weird.
	 */
	while (pkt != NOPKT) {
		/*
		 * Make the transmit list consist of one packet to send.
		 * Save the rest of the list in npkt.
		 */
		npkt = pkt->pk_next;
		pkt->pk_next = NOPKT;
		fakexcvr.xc_tpkt = NOPKT;
		fakexcvr.xc_list = fakexcvr.xc_tail = pkt;
		/*
		 * The xmitnext just dequeues pkt and sets ackn.
		 * It will free the packet if its not worth sending.
		 */
		(void)xmitnext(&fakexcvr);
		if (fakexcvr.xc_tpkt) {
			/*
			 * So it really should be sent.
			 * First make a copy for the receiving side in rpkt.
			 */
			if ((rpkt = pkalloc((int)pkt->pk_len, 1)) != NOPKT)
				movepkt(pkt, rpkt);
			/*
			 * This xmitdone just completes transmission.
			 * Now pkt is out of our hands.
			 */
			xmitdone(pkt);
			/*
			 * So transmission is complete. Now receive it.
			 */
			if (rpkt != NOPKT)
				rcvpkt(rpkt);
		}
		pkt = npkt;
	}
}
/*
 * Indicate that actual transmission of the current packet has been completed.
 * Called by the device dependent interrupt routine when transmission
 *  of a packet has finished.
 */
xmitdone(pkt)
register struct packet *pkt;
{
	register struct connection *conn;
	register struct packet *npkt;

	if (pkt->pk_saddr == Chmyaddr && CONTPKT(pkt) &&
	    pkt->pk_stindex < CHNCONNS &&
	    (conn = Chconntab[pkt->pk_stindex]) != NOCONN &&
	    pkt->pk_sidx == conn->cn_lidx &&
	    (conn->cn_state == CSOPEN || conn->cn_state == CSRFCSENT) &&
	    cmp_gt(pkt->pk_pkn, conn->cn_trecvd)) {
		pkt->pk_time = Chclock;
		if ((npkt = conn->cn_thead) == NOPKT || 
		    cmp_lt(pkt->pk_pkn, npkt->pk_pkn)) {
			pkt->pk_next = npkt;
			conn->cn_thead = pkt;
		} else {
			for( ; npkt->pk_next != NOPKT; npkt = npkt->pk_next)
				if(cmp_lt(pkt->pk_pkn, npkt->pk_next->pk_pkn))
					break;
			pkt->pk_next = npkt->pk_next;
			npkt->pk_next = pkt;
		}
		if(pkt->pk_next == NOPKT)
			conn->cn_ttail = pkt;
	} else
		ch_free((char *)pkt);
}
/*
 * Return the next packet on which to begin transmission (if none,  NOPKT).
 */
struct packet *
xmitnext(xcvr)
register struct chxcvr *xcvr;
{
	register struct packet *pkt;
	register struct connection *conn;

	while ((pkt = xcvr->xc_tpkt = xcvr->xc_list) != NOPKT) {
		if ((xcvr->xc_list = pkt->pk_next) == NOPKT)
			xcvr->xc_tail = NOPKT;
		if (pkt->pk_saddr == Chmyaddr && CONTPKT(pkt))
			if (pkt->pk_stindex < CHNCONNS &&
			    (conn = Chconntab[pkt->pk_stindex]) != NOCONN &&
			    pkt->pk_sidx == conn->cn_lidx &&
		    	    (conn->cn_state == CSOPEN || conn->cn_state == CSRFCSENT) &&
			    cmp_gt(pkt->pk_pkn, conn->cn_trecvd))
				pkt->pk_ackn = conn->cn_racked = conn->cn_rread;
			else {
				debug(DPKT,
					printf("invalid pkt to send: #%x\n",
						conn->cn_lidx));
				ch_free((char *)pkt);
				continue;
			}
		xcvr->xc_ttime = Chclock;
		break;
	}
	return (pkt);
}

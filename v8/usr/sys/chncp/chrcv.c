#include "../chunix/chsys.h"
#include "../chunix/chconf.h"
#include "../chaos/chaos.h"

/*
 * Receive side - basically driven by an incoming packet.
 */

#define send_los(x,y) sendlos(x,y,sizeof(y) - 1)

/*
 * Receive a packet - called from receiver interrupt level.
 */
rcvpkt(pkt)
register struct packet *pkt;
{
	register struct connection *conn;
	register unsigned index;

	debug(DPKT,printf("Rcvpkt: "));
	pkt->pk_next = NOPKT;
	if (pkt->pk_op == RUTOP)
		rcvrut(pkt);
	else if (pkt->pk_op >= MAXOP && !ISDATOP(pkt))
		ch_free((char *)pkt);
	else if (pkt->pk_daddr != Chmyaddr)
		if ((++pkt->pk_fc) == 0) {
			debug(DPKT|DABNOR,printf("Overforwarded packet\n"));
ignore:
			ch_free((char *)pkt);
		} else if (pkt->pk_saddr == Chmyaddr) {
			debug(DPKT|DABNOR,printf("Got my own packet back\n"));
			ch_free((char *)pkt);
		} else if (Chmyaddr == -1)
			ch_free((char *)pkt);
		else {
			debug(DPKT,printf("Forwarding pkt daddr=%x\n", pkt->pk_daddr));
			sendctl(pkt);
		}
	else if (pkt->pk_op == MNTOP)
		goto ignore;
	else if (pkt->pk_op == RFCOP)
		rcvrfc(pkt);
	/*
	 * Check for various flavors of bad indexes
	 */
	else if ((index = pkt->pk_dtindex) >= CHNCONNS ||
		 ((conn = Chconntab[index]) == NOCONN) ||
		 conn->cn_lidx != pkt->pk_didx) {
		debug(DPKT|DABNOR,printf("Packet with bad index: %x, op:%d\n",
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
			 * Make the connection open, taking his index
			 */
			conn->cn_state = CSOPEN;
			conn->cn_fidx = pkt->pk_sidx;
			conn->cn_active = Chclock;
			/*
			 * Indicate we have received AND "read" the OPEN
			 * Read his window size, indicate that he has
			 * received and acked the RFC, and acknowledge the OPN
			 */
			conn->cn_rread = conn->cn_rlast = pkt->pk_pkn;
			conn->cn_twsize = pkt->pk_rwsize;
			receipt(conn, pkt->pk_ackn, pkt->pk_ackn);
			/*
			 * Wakeup the user waiting for the connection to open.
			 */
			makests(conn, pkt);
			reflect(pkt);
			NEWSTATE(conn);
			break;
		case CLSOP:
		case ANSOP:
		case FWDOP:
			debug(DCONN,printf("Conn #%x: CLOSE/ANS received for RFC\n", conn->cn_lidx));
			clsconn(conn, CSCLOSED, pkt);
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
		conn->cn_active = Chclock;
		if (ISDATOP(pkt))
			rcvdata(conn, pkt);
		else switch (pkt->pk_op) {
	    	case OPNOP:
			/*
			 * Ignore duplicate opens.
			 */
			debug(DPKT|DABNOR,printf("Duplicate open received\n"));
			ch_free((char *)pkt);
			break;
		case SNSOP:
			debug(DPKT,prpkt(pkt, "SNS"));
			receipt(conn, pkt->pk_ackn, pkt->pk_ackn);
			makests(conn, pkt);
			reflect(pkt);
			break;
	    	case EOFOP:
			rcvdata(conn, pkt);
			break;
	    	case LOSOP:
		case CLSOP:
			debug(DCONN, printf("Close rcvd on %x\n", conn->cn_lidx));
			clsconn(conn, pkt->pk_op == CLSOP ? CSCLOSED : CSLOST,
				pkt);
			break;
			/*
			 * Uncontrolled data - que it at the head of the rlist
			 */
	    	case UNCOP:
			if ((pkt->pk_next = conn->cn_rhead) == NOPKT)
				conn->cn_rtail = pkt;
			conn->cn_rhead = pkt;
			if (pkt == conn->cn_rtail)
				INPUT(conn);
			receipt(conn, pkt->pk_ackn, pkt->pk_ackn);
			break;
	    	case STSOP:
			debug(DABNOR,prpkt(pkt, "STS"));
			debug(DABNOR,printf("Receipt=%d, Trans Window=%d\n",(unsigned)pkt->pk_idata[0], pkt->pk_idata[1]));
			if (pkt->pk_rwsize > conn->cn_twsize)
				OUTPUT(conn);
			conn->cn_twsize = pkt->pk_rwsize;
			receipt(conn, pkt->pk_ackn, pkt->pk_receipt);
			ch_free((char *)pkt);
			/* could check for very recent packets here */
			if ((pkt = conn->cn_thead) != NOPKT) {
				conn->cn_thead = conn->cn_ttail = NOPKT;
				senddata(pkt);
			}
			break;
		default:
			debug(DPKT|DABNOR, printf("bad opcode:%d\n", pkt->pk_op));
			send_los(pkt, "Bad opcode");
			/* should we do clsconn here? */
		}
	/*
	 * Connection is neither waiting for an OPEN nor OPEN.
	 */
	} else {
		debug(DPKT|DABNOR,printf("Packet for conn #%x (not open) state=%d, op:%d\n", conn->cn_lidx, conn->cn_state, pkt->pk_op));
		send_los(pkt, "Connection is closed");
	}
}

/*
 * Send a control packet back to its source
 */
reflect(pkt)
register struct packet *pkt;
{
	register short temp;

	temp = pkt->pk_sidx;
	pkt->pk_sidx = pkt->pk_didx;
	pkt->pk_didx = temp;
	temp = pkt->pk_saddr;
	pkt->pk_saddr = pkt->pk_daddr;
	pkt->pk_daddr = temp;
	sendctl(pkt);
}

/*
 * Process a received data packet - or an EOF packet which is mostly treated
 * the same.
 */
rcvdata(conn, pkt)
register struct connection *conn;
register struct packet *pkt;
{
	register struct packet *npkt;

	debug(DPKT,(prpkt(pkt,"DATA"),printf("\n")) );
	if (cmp_gt(pkt->pk_pkn, conn->cn_rread + conn->cn_rwsize)) {
		debug(DPKT|DABNOR,printf("Discarding data out of window\n"));
		ch_free((char *)pkt);
		return;
	}
	receipt(conn, pkt->pk_ackn, pkt->pk_ackn);
	if (cmp_le(pkt->pk_pkn, conn->cn_rlast)) {
		debug(DPKT,printf("Duplicate data packet\n"));
		makests(conn, pkt);
		reflect(pkt);
		return;
	}

	/*
	 * Link the out of order list onto the new packet in case
	 * it fills the gap between in-order and out-of-order lists
	 * and to make it easy to grab all now-in-order packets from the
	 * out-of-order list.
	 */
	pkt->pk_next = conn->cn_routorder;
	/*
	 * Now transfer all in-order packets to the in-order list
	 */
	for (npkt = pkt;
	     npkt != NOPKT &&
	     	npkt->pk_pkn == (unsigned short)(conn->cn_rlast + 1);
	     npkt = npkt->pk_next) {
		if (conn->cn_rhead == NOPKT)
			conn->cn_rhead = npkt;
		else
			conn->cn_rtail->pk_next = npkt;
		conn->cn_rtail = npkt;
		conn->cn_rlast++;
	
	}
	/*
	 * If we queued up any in-order pkts, check if spontaneous STS is needed
	 */
	if (pkt != npkt) {
		debug(DPKT,printf("new ordered data packet\n"));
		conn->cn_rtail->pk_next = NOPKT;
		conn->cn_routorder = npkt;
		if (conn->cn_rhead == pkt)
			INPUT(conn);
		/*
		 * If we are buffering "many" packets, keep him up to date
		 * about the window. The second test can be flushed if it is
		 * more important to receipt than to save sending an "extra"
		 * STS with no new acknowledgement.
		 * Lets do without it for a while.
		if ((short)(conn->cn_rlast - conn->cn_racked) > conn->cn_rsts &&
		    conn->cn_racked != conn->cn_rread)
			sendsts(conn);
		 */
	/*
	 * Here we have received an out of order packet which must be
	 * inserted into the out-of-order queue, in packet number order. 
	 */
	} else {
		for (npkt = pkt; (npkt = npkt->pk_next) != NOPKT &&
				 cmp_gt(pkt->pk_pkn, npkt->pk_pkn); )
			pkt->pk_next = npkt;	/* save the last pkt here */
		if (npkt != NOPKT && pkt->pk_pkn == npkt->pk_pkn) {
			debug(DPKT|DABNOR,
				printf("Duplicate out of order packet\n"));
			pkt->pk_next = NOPKT;
			makests(conn, pkt);
			reflect(pkt);
		} else {
			if (npkt == conn->cn_routorder)
				conn->cn_routorder = pkt;
			else
				pkt->pk_next->pk_next = pkt;
			pkt->pk_next = npkt;
			debug(DPKT|DABNOR,
				printf("New out of order packet\n"));
		}
	}
}

/*
 * Make a STS packet for a connection, reflecting its current state
 */
makests(conn, pkt) 
register struct connection *conn;
register struct packet *pkt;
{
	pkt->pk_op = STSOP;
	pkt->pk_lenword = sizeof(struct sts_data);
	pkt->pk_ackn = conn->cn_racked = conn->cn_rread;
	pkt->pk_receipt = conn->cn_rlast;
	pkt->pk_rwsize = conn->cn_rwsize;
}

/*
 * Process receipts and acknowledgements using recnum as the receipt.
 */
receipt(conn, acknum, recnum)
register struct connection *conn;
unsigned short acknum, recnum;
{
	register struct packet *pkt, *pktl;

	/*
	 * Process a receipt, freeing packets that we now know have been
	 * received.
	 */
	if (cmp_gt(recnum, conn->cn_trecvd)) {
		for (pktl = conn->cn_thead;
		     pktl != NOPKT && cmp_le(pktl->pk_pkn, recnum);
		     pktl = pkt) {
			pkt = pktl->pk_next;
			ch_free((char *)pktl);
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
		if (cmp_gt(acknum, conn->cn_tlast))
			debug(DABNOR, (printf("Invalid acknowledgment(%d,%d)\n",
				acknum, conn->cn_tlast)));
		else {
			conn->cn_tacked = acknum;
			OUTPUT(conn);
		}
}

/*
 * Send a LOS in response to the given packet.
 * Don't bother if the packet is itself a LOS or a CLS since the other
 * end doesn't care anyway and would only return it again.
 * Append the host name to the error message.
 */
sendlos(pkt, str, len)
register struct packet *pkt;
char *str;
{
	if (pkt->pk_op == LOSOP || pkt->pk_op == CLSOP)
		ch_free((char *)pkt);
	else {
		register char *cp;
		register int length;
		
		for (cp = Chmyname, length = 0; *cp && length < CHSTATNAME;) {
			length++;
			cp++;
		}
		if (pkt = pktstr(pkt, str, len + length + sizeof("(from )") - 1)) {
			chmove("(from ", &pkt->pk_cdata[len], 6);
			chmove(Chmyname, &pkt->pk_cdata[len + 6], length);
			chmove(")", &pkt->pk_cdata[len + 6 + length], 1);
			pkt->pk_op = LOSOP;
			reflect(pkt);
		}
	}
}

/*
 * Process a received RFC
 */
rcvrfc(pkt)
register struct packet *pkt;
{
	register struct connection *conn, **conptr;
	struct packet **opkt, *pktl;

	debug(DPKT,prpkt(pkt,"RFC"));
	debug(DPKT,printf("contact = %s\n", pkt->pk_cdata));
	/*
	 * Check if this is a duplicate RFC, and if so throw it away,
	 * and retransmit the OPEN.
	 */
	for (conptr = &Chconntab[0]; conptr < &Chconntab[CHNCONNS];)
		if ((conn = *conptr++) != NOCONN &&
		    conn->cn_fidx == pkt->pk_sidx &&
		    conn->cn_faddr == pkt->pk_saddr) {
			if (conn->cn_state == CSOPEN) {
				debug(DPKT|DABNOR,
					printf("Rcvrfc: Retransmitting open chan #%x\n", (*conptr)->cn_lidx));
				sendopn(conn);
			} else {
				debug(DPKT|DABNOR,
					printf("Rcvrfc: Duplicate RFC: %x\n",
						conn->cn_lidx));
			}
			ch_free((char *)pkt);
			return;
		}
	/*
	 * Scan the listen list for a listener and if one is found
	 * open the connection and remove the listen packet from the
	 * listen list.
	 */
	for (opkt = &Chlsnlist; (pktl = *opkt) != NOPKT; opkt = &pktl->pk_next)
		if(concmp(pkt, pktl->pk_cdata, (int)pktl->pk_len)) {
			conn = Chconntab[pktl->pk_stindex];
			*opkt = pktl->pk_next;
			ch_free((char *)pktl);
			lsnmatch(pkt, conn);
			NEWSTATE(conn);
			return;
		}
	if (concmp(pkt, "STATUS", 6)) {
		statusrfc(pkt);
		return;
	}
	if (concmp(pkt, "TIME", 4)) {
		timerfc(pkt);
		return;
	}
	if (Chrfcrcv == 0) {
		send_los(pkt, "All servers disabled");
		return;
	}
	/*
	 * There was no listener, so queue the RFC on the unmatched RFC list
	 * again checking for duplicates.
	 */
	pkt->pk_ackn = 0;	/* this is for ch_rskip, in chuser.c */
	pkt->pk_next = NOPKT;
	if ((pktl = Chrfclist) == NOPKT) 
		Chrfclist = Chrfctail = pkt;
	else {
		do {
			if(pktl->pk_sidx == pkt->pk_sidx &&
			   pktl->pk_saddr == pkt->pk_saddr) {
				debug(DPKT/*|DABNOR*/,printf("Rcvrfc: Discarding duplicate Rfc on Chrfclist\n"));
				ch_free((char *)pkt);
				return;
			}
		} while ((pktl = pktl->pk_next) != NOPKT);
		Chrfctail->pk_next = pkt;
		Chrfctail = pkt;
	}
	debug(DCONN,printf("Rcvrfc: Queued Rfc on Chrfclist: '%c%c%c%c'\n",
		pkt->pk_cdata[0], pkt->pk_cdata[1], pkt->pk_cdata[2],
		pkt->pk_cdata[3]));
	RFCINPUT;
}
/*
 * An RFC has matched a listener, either by an RFC coming and finding a match
 * on the listener list, or by a listen being done and matching an RFC on the
 * unmatched RFC list. So we change the state of the connection to CSRFCRCVD
 */
lsnmatch(rfcpkt, conn)
register struct packet *rfcpkt;
register struct connection *conn;
{
	debug(DCONN,printf("Conn #%x: LISTEN matched \n", conn->cn_lidx));
	/*
	 * Initialize the conection
	 */
	conn->cn_active = Chclock;
	conn->cn_state = CSRFCRCVD;
	conn->cn_rwsize = CHDRWSIZE;
	conn->cn_faddr = rfcpkt->pk_saddr;
	conn->cn_fidx = rfcpkt->pk_sidx;
	/*
	 * Queue up the RFC for the user to read if he wants it.
	 */
	rfcpkt->pk_next = NOPKT;
	conn->cn_rhead = conn->cn_rtail = rfcpkt;
	/*
	 * Indicate to the other end that we have received and "read"
	 * the RFC so that the open will truly acknowledge it.
	 */
	conn->cn_rlast = conn->cn_rread = rfcpkt->pk_pkn;
}
/*
 * Remove a listener from the listener list, due to the listener bailing out.
 * Called from top level at high priority
 */
rmlisten(conn)
register struct connection *conn;
{
	register struct packet *opkt, *pkt;

	opkt = NOPKT;
	for (pkt = Chlsnlist; pkt != NOPKT; opkt = pkt, pkt = pkt->pk_next)
		if (pkt->pk_stindex == conn->cn_ltidx) {
			if(opkt == NOPKT)
				Chlsnlist = pkt->pk_next;
			else
				opkt->pk_next = pkt->pk_next;
			ch_free((char *)pkt);
			break;
		}
}
/*
 * Compare the RFC contact name with the listener name.
 */
concmp(rfcpkt, lsnstr, lsnlen)
struct packet *rfcpkt;
register char *lsnstr;
register int lsnlen;
{
	register char *rfcstr = rfcpkt->pk_cdata;
	register int rfclen;

	debug(DPKT,printf("Rcvrfc: Comparing %s and %s\n", rfcstr, lsnstr));
	for (rfclen = rfcpkt->pk_len; rfclen; rfclen--, lsnlen--)
		if (lsnlen <= 0)
			return ((*rfcstr == ' ') ? 1 : 0);
		else if (*rfcstr++ != *lsnstr++)
			return(0);
	return (lsnlen == 0);
}
/*
 * Process a routing packet.
 */
rcvrut(pkt)
struct packet *pkt;
{
	register int i;
	register struct rut_data *rd;
	register struct chroute *r;

	debug(DPKT, ( prpkt(pkt,"RUT"),printf("\n") ) );
	rd = pkt->pk_rutdata;
	if (pkt->pk_ssubnet >= CHNSUBNET)
		printf("CHAOS: bad subnet %d in RUT pkt\n", pkt->pk_ssubnet);
	else if (Chroutetab[pkt->pk_ssubnet].rt_type != CHDIRECT)
		printf("CHAOS: RUT pkt from unconnected subnet %d\n",
			pkt->pk_ssubnet);
	else for (i = pkt->pk_len; i; i -= sizeof(struct rut_data), rd++) {
		if (rd->pk_subnet >= CHNSUBNET) {
			debug(DABNOR, printf("CHAOS: bad subnet %d in RUT pkt\n",
				rd->pk_subnet));
			continue;
		}
		r = &Chroutetab[rd->pk_subnet];
		switch (r->rt_type) {
		case 0:
		case CHBRIDGE:
			if (rd->pk_cost <= r->rt_cost) {
				r->rt_cost = rd->pk_cost;
				r->rt_type = CHBRIDGE;
				r->rt_addr = pkt->pk_saddr;
			}
			break;
		case CHDIRECT:
		case CHFIXED:
			break;
		default:
			printf("CHAOS: Illegal chaos routing table entry %d",
				r->rt_type);
		}
	}
	ch_free((char *)pkt);
}

/*
 * process a RFC for contact name STATUS
 */
statusrfc(rpkt)
register struct packet *rpkt;
{
	register struct packet *pkt;
	register struct chroute *r;
	register struct chxcvr *xp;
	register struct statdata *sp;
	register int i;

	for (i = 0, r = Chroutetab; r < &Chroutetab[CHNSUBNET]; r++)
		if (r->rt_type == CHDIRECT)
			i++;
	i *= sizeof(struct stathead) + sizeof(struct statxcvr);
	i += CHSTNAME;
	if ((pkt = pkalloc(i, 1)) == NOPKT) {
		ch_free((char *)rpkt);
		return;
	}
	pkt->pk_daddr = rpkt->pk_saddr;
	pkt->pk_didx = rpkt->pk_sidx;
	ch_free((char *)rpkt);
	pkt->pk_type = 0;
	pkt->pk_op = ANSOP;
	pkt->pk_next = NOPKT;
	pkt->pk_saddr = Chmyaddr;
	pkt->pk_sidx = pkt->pk_pkn = pkt->pk_ackn = 0;
	pkt->pk_lenword = i;
	chmove(Chmyname, pkt->pk_status.sb_name, CHSTATNAME);
	sp = &pkt->pk_status.sb_data[0];
	for (r = Chroutetab; r < &Chroutetab[CHNSUBNET]; r++)
		if (r->rt_type == CHDIRECT) {
			xp = r->rt_xcvr;
			sp->sb_ident = 0400 + xp->xc_subnet;
			sp->sb_nshorts = sizeof(struct statxcvr) / sizeof(short);
			sp->sb_xstat = xp->xc_xstat;
#ifdef pdp11
			swaplong(&sp->sb_xstat,
				sizeof(struct statxcvr) / sizeof(long));
#endif
			sp = (struct statdata *)((char *)sp +
				sizeof(struct stathead) +
				sizeof(struct statxcvr));
		}
	sendctl(pkt);
}

/*
 * process a RFC for contact name TIME 
 */
timerfc(pkt)
register struct packet *pkt;
{
	long t;

	pkt->pk_op = ANSOP;
	pkt->pk_next = NOPKT;
	pkt->pk_pkn = pkt->pk_ackn = 0;
	pkt->pk_lenword = sizeof(long);
	ch_time(&t);
	pkt->pk_ldata[0] = t;
#ifdef pdp11
		swaplong(&pkt->pk_ldata, 1);
#endif pdp11
	reflect(pkt);
}
#ifdef DEBUG
prpkt(pkt, str)
register struct packet *pkt;
char *str;
{
	printf("op=%s(%o) len=%d fc=%d\ndhost=%o didx=%x\nshost=%o sidx=%x\npkn=%d ackn=%d ",
		str, pkt->pk_op, pkt->pk_len, pkt->pk_fc, pkt->pk_dhost,
		pkt->pk_didx, pkt->pk_shost, pkt->pk_sidx,
		(unsigned)pkt->pk_pkn, (unsigned)pkt->pk_ackn);
}
#endif

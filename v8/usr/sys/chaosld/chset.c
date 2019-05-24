/*
 *             C H  S E T
 *
 * Chaosnet line discipline - routines to set various fields in a packet.
 *
 *
 * (c) Copyright 1984  Nirvonics, Inc.
 *
 * Written by Kurt Gollhardt
 * Last update Wed Oct 24 13:39:42 1984
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


/*
 * Set (STS) status info in a packet, reflecting the current state
 * of the connection.
 */
setsts(conn, pkt)
     register struct connection    *conn;
     register struct packet        *pkt;
{
     register struct block         *bp;
     register struct sts_data      sts;

     free_pkdata(pkt);
     pkt->pk_op = STSOP;
     pkt->pk_ackn = conn->cn_racked = conn->cn_rread;
     sts.sts_receipt = conn->cn_rlast;
     sts.sts_rwsize = conn->cn_rwsize;
     append_packet(pkt, &sts, sizeof(sts));
}


/*
 * Set connection info fields in a packet.
 * Many routines count on the fact that this clears pk_type and next.
 */
setconn(conn, pkt)
register struct connection *conn;
register struct packet *pkt;
{
	pkt->pk_daddr = conn->cn_faddr;
	pkt->pk_didx = conn->cn_fidx;
	pkt->pk_saddr = Chmyaddr;
	pkt->pk_sidx = conn->cn_lidx;
	pkt->pk_type = 0;
	pkt->next = NOPKT;
	pkt->pk_fc = 0;
}


/*
 * Set acknowledge field of packet from connection.
 */
setack(conn, pkt)
     struct connection   *conn;
     struct packet       *pkt;
{
     pkt->pk_ackn = conn->cn_racked = conn->cn_rread;
}

#endif

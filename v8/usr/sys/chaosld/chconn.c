/*
 *             C H  C O N N
 *
 * Connection handling utilities for the Chaosnet line discipline.
 *
 *
 * (c) Copyright 1984  Nirvonics, Inc.
 *
 * Written by Kurt Gollhardt
 * Last update Thu Dec  6 13:47:21 1984
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
 * Allocate a connection and return it, also allocating a slot in Chconntab
 */
struct connection *
new_conn(tidx)
{
     register struct connection *conn;
     static int uniq;

     if ((conn = cnalloc()) == NOCONN) {
          debug(DCONN|DABNOR,printf("Conn: alloc failed (packet)\n"));
          Chaos_error = CHNOCONN;
          return(NOCONN);
     }

     bfill((char *)conn, sizeof(struct connection), 0);
     if (++uniq == 0)
          uniq = 1;
     conn->cn_lidx.ci_uniq = uniq;
     conn->cn_lidx.ci_tidx = tidx;
     conn->cn_state = 0;
     conn->cn_time = Chclock;

     debug(DCONN,printf("Conn: alloc #%x\n", conn->cn_lidx));
     return(conn);
}
     
/*
 * Release a connection - freeing all associated storage.
 * This removes all trace of the connection.
 */
free_conn(conn)
     struct connection   *conn;
{
     Chconn[conn->cn_lidx.ci_tidx] = NOCONN;
     freelist(conn->cn_routorder);
     freelist(conn->cn_thead);
     if (conn->cn_toutput != NOPKT)
          free_packet(conn->cn_toutput);
     if (conn->cn_expkt != NOPKT)
          free_packet(conn->cn_expkt);
     if (conn->cn_wait != NOBLOCK) /* ??? */
          freeb(conn->cn_wait);
     debug(DCONN,printf("Conn: release #%x\n", conn->cn_lidx));
     cnfree(conn);
}


/*
 * Make a connection closed with given state, at interrupt time.
 * Queue the given packet on the input queue for the user.
 */
close_conn(conn, state, pkt)
     register struct connection    *conn;
     register struct packet        *pkt;
{
     freelist(conn->cn_thead);
     conn->cn_thead = conn->cn_ttail = NOPKT;
     conn->cn_state = state;
     debug(DCONN|DABNOR, printf("Conn #%x: CLOSED: state: %d\n",
          conn->cn_lidx, state));
     if (pkt != NOPKT)
          chdrint(conn, pkt);
     chd_newstate(conn);
}

#endif

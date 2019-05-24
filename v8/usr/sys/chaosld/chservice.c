/*
 *             C H  S E R V I C E
 *
 * Built-in services for the Chaosnet line discipline.
 *
 *
 * (c) Copyright 1984  Nirvonics, Inc.
 *
 * Written by Kurt Gollhardt
 * Last update Tue Nov 13 14:27:51 1984
 *
 * This software is the property of Nirvonics, Inc.
 * All rights reserved.
 *
 */

#include "ch.h"
#if NCH > 0
#include "../h/param.h"
#include "../h/systm.h"
#include "../chaosld/types.h"
#include "../chaosld/constants.h"
#include "../chaosld/globals.h"
#include "../chaosld/chrouteld.h"

extern struct chroute    Chroute[];

static    status_rfc(), time_rfc(), uptime_rfc(), dumprt_rfc();

struct service Chservice[] = {
     "STATUS",                6,   status_rfc,
     "TIME",                  4,   time_rfc,
     "UPTIME",                6,   uptime_rfc,
     "DUMP-ROUTING-TABLE",    18,  dumprt_rfc,
     0,                       0,   0
};

/*
 * process a RFC for contact name STATUS
 */
static status_rfc(pkt)
     register struct packet   *pkt;
{
     register struct chroute  *r;
     register struct chif     *ifp;
     struct statdata     sd;

     free_pkdata(pkt);
     append_packet(pkt, Chmyname, CHSTATNAME);

     for (r = Chroute; r < &Chroute[CHNSUBNET]; r++)
          if (r->rt_type == CHDIRECT) {
               ifp = r->rt_path.ifp;
               sd.sb_ident = 0400 + ifp->my_addr.ch_subnet;
               sd.sb_nshorts = sizeof(struct statxcvr) / sizeof(short);
               sd.sb_xstat = ifp->if_stat;
#ifdef pdp11
               swaplong(&sd.sb_xstat,
                    sizeof(struct statxcvr) / sizeof(long));
#endif
               append_packet(pkt, &sd, sizeof(sd));
          }

     pkt->pk_op = ANSOP;
     pkt->pk_didx.ci_idx = pkt->pk_pkn = pkt->pk_ackn = 0;
     reflect(pkt);
}

/*
 * process an RFC for contact name TIME 
 */
static time_rfc(pkt)
     register struct packet   *pkt;
{
     long t;

     pkt->pk_op = ANSOP;
     pkt->pk_pkn = pkt->pk_ackn = 0;
     ch_time(&t);
#ifdef pdp11
          swaplong(&t, 1);
#endif pdp11
     free_pkdata(pkt);
     append_packet(pkt, &t, sizeof(long));
     reflect(pkt);
}

/*
 * process an RFC for contact name UPTIME
 */
static uptime_rfc(pkt)
     register struct packet   *pkt;
{
     long t;

     pkt->pk_op = ANSOP;
     pkt->pk_pkn = pkt->pk_ackn = 0;
     ch_uptime(&t);
#ifdef pdp11
     swaplong(&t, 1);
#endif pdp11
     free_pkdata(pkt);
     append_packet(pkt, &t, sizeof(long));
     reflect(pkt);
}

/*
 * process an RFC for contact name DUMP-ROUTING-TABLE
 */
static dumprt_rfc(pkt)
     register struct packet   *pkt;
{
     register struct chroute  *r;
     register int        ndirect, i;
     struct {
          short     word1, word2;
     } sd;

     free_pkdata(pkt);
     ndirect = i = 0;
     for (r = Chroute; r < &Chroute[CHNSUBNET]; r++, i++) {
          sd.word2 = r->rt_cost;
          if (r->rt_type == CHDIRECT)
               sd.word1 = (ndirect++ << 1) + 1;
          else if (r->rt_type != CHNOPATH)
               sd.word1 = r->rt_path.bridge.ch_addr;
          else
	       sd.word1 = 0;
          append_packet(pkt, &sd, sizeof(sd));
     }

     pkt->pk_op = ANSOP;
     pkt->pk_didx.ci_idx = pkt->pk_pkn = pkt->pk_ackn = 0;
     reflect(pkt);
}

#endif

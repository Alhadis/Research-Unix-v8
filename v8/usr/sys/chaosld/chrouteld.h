/*
 *             C H R O U T E L D . H
 *
 * Definitions for use with the chroute line discipline for
 * chaosnet routing.
 *
 *
 * (c) Copyright 1984  Nirvonics, Inc.
 *
 * Written by Kurt Gollhardt
 * Last update Mon Sep 24 16:33:37 1984
 *
 * This software is the property of Nirvonics, Inc.
 * All rights reserved.
 *
 */

#ifndef _CHROUTE_
#define _CHROUTE_

     /* Subnet Routing Types */

#define CHNOPATH    0    /* No connection */
#define CHDIRECT    1    /* Direct connection to this subnet */
#define CHBRIDGE    2    /* Can get to this subnet via a bridge */
#define CHFIXED     3    /* Same as CHBRIDGE, but statically configured */

     /* Subnet Costs */

#define DIRECT_COST 10   /* Direct connection, e.g. DR11 parallel line */
#define CABLE_COST  16   /* High speed cable, e.g. ethernet */
#define ASYNC_COST  20   /* Low speed connection, e.g. asynchronous RS-232 */
#define HIGH_COST   512  /* infinity */

     /* Chroute_ld Per Interface Structure */

struct chif {
     struct queue   *rdq;     /* This interface's line discipline read queue */
     chaddr         my_addr;  /* Chaosnet address of this interface */
     dev_t          if_dev;   /* UNIX device number of this interface */
     u_short        if_cost;  /* Cost of sending via this interface */
     u_short        arp;      /* Address resolution channel number */
     struct statxcvr if_stat; /* Status information */
};

#define ist_xmit    if_stat.sx_Xmtd  /* # successfully transmitted packets */
#define ist_abrt    if_stat.sx_Abrt  /* # transmission failures */
#define ist_rcvd    if_stat.sx_Rcvd  /* # successfully received packets */
#define ist_rej     if_stat.sx_Rej   /* # reception failures */
#define ist_len     if_stat.sx_Leng  /* # length errors */

     /* Chroute_ld Per Subnet Routing Information */

struct chroute {
     u_char         rt_type;  /* Type of connection to this subnet */
     u_short        rt_cost;  /* Current cost of sending via this subnet */
     union {
          chaddr         bridge;   /* Chaosnet address of bridge */
	  struct chif    *ifp;     /* Interface for a direct connection */
     } rt_path;               /* Info on how to get to this subnet */
};

     /* Chaosnet-Ethernet Address Resolution Pair */

struct chaddr_pair {
     chaddr         ch_addr;       /* Chaosnet address */
     u_char         en_addr[6];    /* Ethernet address */
};

     /* Ioctl Codes */

#define CRIOADDR    (('C'<<8)|1)   /* Set Chaosnet and Ethernet addresses */
#define CRIOCOST    (('C'<<8)|2)   /* Set cost for this interface */
#define CRIOPRIMARY (('C'<<8)|3)   /* Establish this as the primary interface;
                                        i.e. packets addressed to us will be
					passed to this stream */

#endif

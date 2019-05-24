#ifndef _CHAOS_
#define _CHAOS_
/*
 * Accomodate Ritchie C compiler...
 */
#ifdef pdp11
#ifndef lint
#ifndef void
#define void int
#endif
#endif
#endif
#include "../chaos/constants.h"
/*
 * System and device independent include file for the Chaosnet NCP
 */

/*
 * A chaos index - a hosts connection identifier
 */
typedef	union	{
	unsigned short	ci_idx;		/* Index as a whole */
	struct	{
		unsigned char	ci_Tidx;	/* Connection table index */
		unsigned char	ci_Uniq;	/* Uniquizer for table slot */
	}		ci_bytes;
} chindex;
#define ci_uniq	ci_bytes.ci_Uniq
#define ci_tidx	ci_bytes.ci_Tidx

/*
 * A chaos network address.
 */
typedef	union	{
	unsigned short 		ch_addr;	/* Address as a whole */
	struct	{
		unsigned char	ch_Host;	/* Host number on subnet */
		unsigned char	ch_Subnet;	/* Subnet number */
	}		ch_bytes;
} chaddr;
#define ch_subnet	ch_bytes.ch_Subnet
#define ch_host		ch_bytes.ch_Host
/*
 * A chaosnet clock time - wrapsaround
 */
typedef unsigned short chtime;
/*
 * This is the part of the packet which is only for use by the ncp.
 * It is not transmitted over the network
 * NOTE that this structure is assumed to be at least 8 bytes by
 * the Interlan Ethernet driver, and assumed to be at most 8 bytes
 * by the storage allocator!!!!!  GGAAAAAAAAAAAGGGHHH
 */
struct	ncp_header	{
	struct packet	*nh_next;	/* Link to next packet on this list */
	chtime		nh_time;	/* Last time packet was processed */
	chaddr		nh_xdest;	/* Routed destination address */
};
/*
 * This is the part of the packet header that is transmitted over the
 * network, thus must have fixed, portable format from ncp to ncp
 */
struct	pkt_header	{
	unsigned char		ph_type;	/* Protocol type */
	unsigned char		ph_op;		/* Opcode of the packet */
	union {
		unsigned short	ph_lfcwhole;
		struct	{
			unsigned short ph_Len:12;	/* Length of packet */
			unsigned short ph_fcount:4;	/* Forwarding count */
		}	ph_lfcparts;
	}		ph_lenfc;
	chaddr		ph_daddr;		/* Destination address */
	chindex		ph_didx;		/* Destination index */
	chaddr		ph_saddr;		/* Source address */
	chindex		ph_sidx;		/* Source index */
	unsigned short	ph_pkn;			/* Packet number */
	unsigned short	ph_ackn;		/* Acknowledged packet number */
};
#define ph_len ph_lenfc.ph_lfcparts.ph_Len
/* This is the actual structure of a packet in core */
struct	packet	{
	struct ncp_header	pk_nhead;	/* NCP specific information */
	struct pkt_header	pk_phead;	/* Network header */
	union	{
		char 		pk_Cdata[sizeof(short)];/* Character data */
		short		pk_Idata[1];	/* word data */
		long		pk_Ldata[1];	/* long data */
		struct sts_data {		/* data of STS packets */
			unsigned short	pk_Receipt;
			unsigned short	pk_Rwsize;
		}		pk_stsdata;
		struct rut_data {		/* data of RUT packets */
			unsigned short	pk_subnet;
			unsigned short	pk_cost;
		}		pk_Rutdata[1];
		struct status	{
#define CHSTNAME 32
			char	sb_name[CHSTNAME];
			struct	statdata {
				struct stathead {
					unsigned short	sb_Ident;
					unsigned short	sb_Nshorts;
				}		sb_head;
				union {
					struct statxcvr {
						long	sx_Rcvd;
						long	sx_Xmtd;
						long	sx_Abrt;
						long	sx_Lost;
						long	sx_Crcr;
						long	sx_Crci;
						long	sx_Leng;
						long	sx_Rej;
					}		sb_Xstat;
				}			sb_union;
			}				sb_data[1];
		}		pk_Status;
	}			pk_data;
};

#define NOPKT		((struct packet *)0)
#define CHHEADSIZE	(sizeof(struct ncp_header)+sizeof(struct pkt_header))
#define pkalloc(x,y)	((struct packet *)ch_alloc((x)+CHHEADSIZE,(y)))
/*
 * Allocate an NCP packet given the number of shorts in the received packet.
 */
#define hpkalloc(nshorts)	((struct packet *)ch_alloc((int) \
				((nshorts) /* * sizeof(short) */ << 1) + \
				sizeof(struct ncp_header), 1))

/* macros for accessing packets fields */
#define pk_next		pk_nhead.nh_next
#define pk_time		pk_nhead.nh_time
#define pk_xdest	pk_nhead.nh_xdest.ch_addr
#define pk_type		pk_phead.ph_type
#define pk_op		pk_phead.ph_op
#define pk_len		pk_phead.ph_len
#define pk_fc		pk_phead.ph_lenfc.ph_lfcparts.ph_fcount
#define pk_lenword	pk_phead.ph_lenfc.ph_lfcwhole
#define pk_daddr	pk_phead.ph_daddr.ch_addr
#define pk_dhost	pk_phead.ph_daddr.ch_host
#define pk_dsubnet	pk_phead.ph_daddr.ch_subnet
#define pk_didx		pk_phead.ph_didx.ci_idx
#define pk_dtindex	pk_phead.ph_didx.ci_tidx
#define pk_saddr	pk_phead.ph_saddr.ch_addr
#define pk_shost	pk_phead.ph_saddr.ch_host
#define pk_ssubnet	pk_phead.ph_saddr.ch_subnet
#define pk_sidx		pk_phead.ph_sidx.ci_idx
#define pk_stindex	pk_phead.ph_sidx.ci_tidx
#define pk_suniq	pk_phead.ph_sidx.ci_uniq
#define pk_pkn		pk_phead.ph_pkn
#define pk_ackn		pk_phead.ph_ackn
#define pk_cdata	pk_data.pk_Cdata
#define pk_idata	pk_data.pk_Idata
#define pk_ldata	pk_data.pk_Ldata
#define pk_receipt	pk_data.pk_stsdata.pk_Receipt
#define pk_rwsize	pk_data.pk_stsdata.pk_Rwsize
#define pk_rutdata	pk_data.pk_Rutdata
#define pk_status	pk_data.pk_Status
#define sb_ident	sb_head.sb_Ident
#define sb_nshorts	sb_head.sb_Nshorts
#define sb_xstat	sb_union.sb_Xstat

#define ISDATOP(pkt)	(((pkt)->pk_op & DATOP) != 0)
#define CONTPKT(pkt)	(ISDATOP(pkt) || (pkt)->pk_op == RFCOP || \
			(pkt)->pk_op == OPNOP || (pkt)->pk_op == EOFOP)
/* Here are the packet types */
#define PKNML	00		/* Normal */
#define PKLSN	01		/* Listen list packet */
/*
 * This is the connection structure. These are allocated in a packet of the
 * appropriate size
 */
struct connection {
	struct	csys_header cn_syshead;	/* System dependent info  */
	unsigned char	cn_flags;	/* Random flags */
	unsigned char	cn_state;	/* State of the connection */
	chtime	cn_active;	/* Last time connection was active */
	chaddr	cn_Faddr;		/* Foreign address */
	chindex	cn_Fidx;		/* Foreign index */
	chindex cn_Lidx;		/* Local index */

	/* transmit side state */
	unsigned short	cn_twsize;	/* Transmit window size */
	unsigned short	cn_tlast;	/* Last packet we sent */
	unsigned short	cn_trecvd;	/* Last pkt receipted by him */
	unsigned short	cn_tacked;	/* Last pkt acked by him */
	struct packet	*cn_thead;	/* Head of list of pkts transmitted */
	struct packet	*cn_ttail;	/* Tail of list of pkts transmitted */
#ifdef CHSTRCODE
	struct packet	*cn_toutput;	/* Pkt being filled for output */
					/* NOT a list, just one packet */
	unsigned short	cn_troom;	/* Room left in cn_toutput packet */
					/* pk_len shows current fill level */
#endif
	/* receive side state */
	unsigned short	cn_rwsize;	/* Receive window size */
	unsigned short	cn_rlast;	/* Last pkt rcvd (in order) */
	unsigned short	cn_racked;	/* Last pkt acked by us */
	unsigned short	cn_rread;	/* Last pkt read by our user */
	unsigned short	cn_rsts;	/* Max rread-racked before auto STS */
#ifdef CHSTRCODE
	unsigned short	cn_roffset;	/* read offset in current packet */
					/* which is conn->cn_rhead */
#endif
	struct packet	*cn_rhead;	/* Head of ordered rcvd pkts */
	struct packet	*cn_rtail;	/* Tail of ordered received packets */
	struct packet	*cn_routorder;	/* list of out of order packets */
};

#define cn_fidx			cn_Fidx.ci_idx
#define cn_faddr		cn_Faddr.ch_addr
#define cn_fhost		cn_Faddr.ch_host
#define cn_fsubnet		cn_Faddr.ch_subnet
#define cn_lidx			cn_Lidx.ci_idx
#define cn_ltidx		cn_Lidx.ci_tidx
#define cn_luniq		cn_Lidx.ci_uniq

/* bit values for cn_flags */
#ifdef	CHSTRCODE
#define CHEOFSEEN	1	/* EOF packet received and acknowledged */
#endif
#define CHANSWER	2	/* This connection should send an ANS pkt */

/* macros for certain connection states */

#define chtfull(conn)	((conn)->cn_state == CSOPEN && \
			 (short)((conn)->cn_tlast - (conn)->cn_tacked) >= \
				(conn)->cn_twsize)
#define chtempty(conn)	((conn)->cn_state != CSOPEN || \
			 (conn)->cn_tlast == (conn)->cn_tacked)
#define chrempty(conn)	((conn)->cn_rhead == NOPKT && \
			 (conn)->cn_state == CSOPEN)


#define NOCONN		((struct connection *)0)
#define connalloc()	((struct connection *)ch_alloc(sizeof(struct connection),0))
/*
 * These are unsigned comparisons
 * all the casting is necessary due to compiler flakiness with shorts etc.
 * cmp_lt(a,b) is true if a < b, cmp_le(a,b) if a <= b, cmp_gt(a,b) if a > b
 * and cmp_ge(a,b) if a >= b
 */
#define cmp_gt(a,b) (0100000 & (b - a))
#define cmp_ge(a,b) !(cmp_lt(a,b)
#define cmp_lt(a,b) cmp_gt(b, a)
#define cmp_le(a,b) !cmp_gt(a,b)
/* codes for error returns in various places - needs cleaning up somewhat */

#define CHERROR		-1
#define CHEOF		-2
#define CHNOPKT		-3	/* No packets */
#define CHNOCONN	-4	/* No connections */
#define CHCTIMEOUT	-5	/* Time out */
#define CHTEMP		-6

/*
 * Network interface structure.
 *
 * There is one chxcvr structure for each interface connected to this host,
 * all of which are defined in the file "chconf.c". The xcinfo is a union
 * of structures needed for each device type's device dependent state.
 * This union should be defined in "chconf.h"
 * The bottom level device driver routines define arrays of these structures
 * one per interface of a given type and find the structure appropriate
 * to a given interface by indexing on the device number given in the
 * interrupt vector.  The top level gets at an chxcvr structure through
 * the routing table.
 * The ttime and rtime values are for timing-out hung transmitters or
 * receivers.  The drivers must ensure that tpkt and rpkt are only nonzero
 * when there is really a packet being received or transmitted.  This may
 * be partly useless for dma (or other packet-atomic) interfaces.
 */
struct	chxcvr	{
	struct packet	*xc_list;	/* Packets to be transmitted */
	struct packet	*xc_tail;	/* Tail of xc_list */
	struct packet	*xc_tpkt;	/* Packet being transmitted */
	struct packet	*xc_rpkt;	/* Packet being received */
	chtime	 	xc_ttime;	/* Time tpkt begun transmission */
	chtime		xc_rtime;	/* Time rpkt began reception */
	chaddr		xc_Addr;	/* Address of this interface */
	unsigned	*xc_devaddr;	/* Device address (UNIBUS) */
	int		(*xc_start)();	/* Start routine for idle xmtr */
	int		(*xc_reset)();	/* Reset routine for interface */
	struct statxcvr	xc_xstat;	/* Xcvr metering */
	union xcinfo	xc_info;	/* Device dependent info */
};
#define xc_addr		xc_Addr.ch_addr
#define xc_subnet	xc_Addr.ch_subnet
#define xc_host		xc_Addr.ch_host
#define xc_rcvd		xc_xstat.sx_Rcvd
#define xc_xmtd		xc_xstat.sx_Xmtd
#define xc_crcr		xc_xstat.sx_Crcr
#define xc_crci		xc_xstat.sx_Crci
#define xc_lost		xc_xstat.sx_Lost
#define xc_leng		xc_xstat.sx_Leng
#define xc_rej		xc_xstat.sx_Rej
#define xc_abrt		xc_xstat.sx_Abrt

#define NOXCVR		((struct chxcvr *)0)
/*
 * Routing table entry structure.
 * One per subnet possibly accessible from this host.
 * Entries for directly connected subnets point to hardware
 * transceiver structure (rt_type == CHDIRECT).
 * Bridges (rt_type == CHFIXED or CHBRIDGE) have address of
 * directly connected bridges.
 */
struct chroute	{
	union {
		struct chxcvr	*rt_Xcvr;	/* interface to use */
		chaddr		rt_Addr;	/* bridge address */
	}			rt_u;
	unsigned char		rt_type;	/* type of access */
	unsigned short		rt_cost;	/* cost of access path */
};
#define rt_xcvr		rt_u.rt_Xcvr
#define rt_addr		rt_u.rt_Addr.ch_addr
#define rt_host		rt_u.rt_Addr.ch_host
#define rt_subnet	rt_u.rt_Addr.ch_subnet

/* values for rt_type */
#define CHNOPATH	0	/* No path to this subnet yet (now) */
#define	CHDIRECT	1	/* Either chaos cable or other hardware */
#define CHFIXED		2	/* Unvarying bridge */
#define CHBRIDGE	3	/* Bridge - known via RUT packet */
/* initial values for rt_cost depending on rt_type */
#define CHDCOST		10	/* Directly connected hardware (ala dr11) */
#define CHCCOST		16	/* Chaos cable connection (ether) */
#define CHACOST		20	/* Async or other slow link */
#define CHHCOST		512	/* "high" cost */

/* definition of globals */

#ifdef CHDEFINE
#define EXTERN
#	ifdef DEBUG
	int			Chdebug = 0;
#	endif
#else
#define EXTERN extern
#	ifdef DEBUG
	EXTERN int		Chdebug;
#	endif
#endif CHDEFINE
EXTERN int			Chaos_error;
EXTERN struct connection	*Chconntab[CHNCONNS];	/* connection table */
EXTERN chtime			Chclock;  	/* clock (mod ??) */
EXTERN struct packet 		*Chlsnlist,	/* listening connections */
				*Chrfclist,	/* list of unmatched rfc's */
				*Chrfctail;	/* tail of same list */
EXTERN struct chroute		Chroutetab[CHNSUBNET];	/* subnet routing table */
EXTERN int			Chhz;		/* Hertz of clock */
EXTERN int			Chrfcrcv;	/* Flag for rfc reader */
extern short			Chmyaddr;	/* This ncp'c host number */
extern char			Chmyname[];	/* This ncp's host name */

extern char		*ch_alloc();
extern struct packet	*pktstr(), *ch_rnext(), *xmitnext();
extern struct connection *allconn(), *ch_open(), *ch_listen();

/* debugging instrumentation */

#ifdef DEBUG

#define debug(a,b)	if(Chdebug&(a)) b; else /* expect a ; after! */

#define DALLOC	1	/* Allocation tracing */
#define DTRANS	2	/* Transmitter tracing */
#define DCONN	4	/* Connection activity */
#define DPKT	8	/* Print packets */
#define DNOCLK	16	/* No clock timeouts */
#define DABNOR	32	/* Abnormal events */
#define DSEND	64	/* Trace each packet sent */

#else
#define debug(a,b)
#endif

#endif

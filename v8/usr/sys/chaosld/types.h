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
#include "../chaosld/constants.h"

/*
 * System and device independent include file for the Chaosnet NCP
 */

/*
 * User interface structures
 */

	/* Structure passed on open */
struct chopen {
	short	co_host;	/* Host address to contact or zero to listen */
	short	co_async;	/* If non-zero, don't wait for open done */
	short	co_clength;	/* Length of contact string (follows struct) */
	short	co_length;	/* Length of RFC data (follows contact str) */
	short	co_rwsize;	/* Receive window size (0 for default) */
	short	co_mode;	/* Connection mode (CHSTREAM or CHRECORD) */
};

	/* Structure returned on open */
struct choreply {
	int	errno;		/* Error code */
};

	/* Structure returned by CHIOCRPKT ioctl */
struct chrpkt {
	short	cp_pktop;	/* Packet opcode */
	short	cp_pklen;	/* Length of packet data */
	char	cp_buf[CHMAXDATA];	/* Packet data goes here */
};

	/* Record mode packet structure */
struct chpacket {
	unsigned char	cp_op;	/* Packet opcode */
	char		cp_data[CHMAXDATA];
};

	/* Structure returned by CHIOCGSTAT ioctl */
struct chstatus {
	short	st_fhost;	/* Remote host */
	short	st_cnum;	/* Local channel number */
	short	st_rwsize;	/* Receive window size */
	short	st_twsize;	/* Transmit window size */
	short	st_state;	/* Connection state */
	short	st_ptype;	/* OBSOLETE */
	short	st_plength;	/* OBSOLETE */
	short	st_cmode;	/* Mode of connection */
	short	st_oroom;	/* Output window space left */
};

	/* Structure passed to CHIOCREJECT ioctl */
struct chreject {
	char	*cr_reason;	/* Error message string */
	int	cr_length;	/* Length of error message string */
};

	/* Structure passed to CHIOCNAME ioctl */
struct chstatname {
	char	*cn_name;	/* Pointer to CHSTATNAME long string */
};

	/* FILE server login record structure */
struct chlogin {
	int	cl_pid;		/* Process id of server */
	short	cl_cnum;	/* Chaos channel number of server */
	short	cl_haddr;	/* Host address of other end */
	long	cl_ltime;	/* Login time */
	long	cl_atime;	/* Last time used. */
	char	cl_user[8];	/* User name */
};


/*
 * Chaosnet typedefs
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

/* A chaosnet clock time - wraps around */
typedef unsigned short   chtime;

/* A chaosnet packet number - wraps around */
typedef unsigned short   chpknum;

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
	chpknum		ph_pkn;			/* Packet number */
	chpknum		ph_ackn;		/* Acknowledged packet number */
};
#define ph_len ph_lenfc.ph_lfcparts.ph_Len
#define ph_fc  ph_lenfc.ph_lfcparts.ph_fcount
#define ph_lenword  ph_lenfc.ph_lfcwhole

/*   Internal packet structure for the Chaosnet line discipline
 */
struct packet {
     struct pkt_header   ph;       /* A copy of the packet header */
     struct block        *data;    /* Linked list of data blocks (no header) */
     struct packet       *next;    /* Link pointer for queue of packets */
     chtime              time;     /* Last time packet was processed */
};
#define pk_type     ph.ph_type
#define pk_op       ph.ph_op
#define pk_len      ph.ph_len
#define pk_fc       ph.ph_fc
#define pk_lenword  ph.ph_lenword
#define pk_daddr    ph.ph_daddr
#define pk_didx     ph.ph_didx
#define pk_saddr    ph.ph_saddr
#define pk_sidx     ph.ph_sidx
#define pk_pkn      ph.ph_pkn
#define pk_ackn     ph.ph_ackn

#define ISDATA(pkt)      (((pkt)->pk_op & DATOP) != 0)
#define CONTROLLED(pkt)  (ISDATA(pkt) || (pkt)->pk_op == RFCOP || \
                           (pkt)->pk_op == OPNOP || (pkt)->pk_op == EOFOP)
#define READABLE(pkt)    (ISDATA(pkt) || (pkt)->pk_op == ANSOP || \
                           (pkt)->pk_op == UNCOP || (pkt)->pk_op == EOFOP)

struct sts_data {		/* data of STS packets */
     chpknum	sts_receipt;
     chpknum	sts_rwsize;
};

struct rut_data {		/* data of RUT packets */
     unsigned short	rd_subnet;
     unsigned short	rd_cost;
};

struct status	{
     char      sb_name[CHSTATNAME];
     struct statdata {
          struct stathead {
               unsigned short sb_Ident;
               unsigned short sb_Nshorts;
          } sb_head;
          union {
               struct statxcvr {
                    long      sx_Rcvd;
                    long      sx_Xmtd;
                    long      sx_Abrt;
                    long      sx_Lost;
                    long      sx_Crcr;
                    long      sx_Crci;
                    long      sx_Leng;
                    long      sx_Rej;
               } sb_Xstat;
          } sb_union;
     } sb_data[1];
};
#define sb_ident	sb_head.sb_Ident
#define sb_nshorts	sb_head.sb_Nshorts
#define sb_xstat	sb_union.sb_Xstat

/*
 * This structure describes the state of an individual connection.
 */
struct connection {
     unsigned char  cn_sflags;     /* State (wait) flags */
     unsigned char  cn_flags;      /* Random flags */
     unsigned char  cn_state;      /* State of the connection */
     unsigned char  cn_mode;       /* Mode of this connection */
     chtime         cn_time;       /* Last time connection was active */
     chaddr         cn_faddr;      /* Foreign address */
     chindex        cn_fidx;       /* Foreign index */
     chindex        cn_lidx;       /* Local index */
     struct queue   *cn_rdq;       /* Read queue for this connection */
     struct block   *cn_wait;      /* Message block associated with a wait */

     /* transmit side state */
     chpknum        cn_twsize;     /* Transmit window size */
     chpknum        cn_tlast;      /* Last packet we sent */
     chpknum        cn_trecvd;     /* Last packet receipted by him */
     chpknum        cn_tacked;     /* Last pkt acked by him */
     struct packet  *cn_thead;     /* Head of list of pkts xmitted not acked */
     struct packet  *cn_ttail;     /* Tail of list of pkts xmitted not acked */
     struct packet  *cn_toutput;   /* Packet being filled for output */

     /* recieve side state */
     chpknum        cn_rwsize;     /* Receive window size */
     chpknum        cn_rlast;      /* Last pkt rcvd (in order) */
     chpknum        cn_racked;     /* Last pkt acked by us */
     chpknum        cn_rread;      /* Last pkt read by our user */
     chpknum        cn_rsts;       /* Max rread-racked before auto STS */
     struct packet  *cn_routorder; /* List of out-of-order packets */
     struct packet  *cn_expkt;     /* Extra packet which may be read */
};

/* Bit values for cn_sflags */
#define CHOPNWAIT   0x01      /* Waiting for an open to complete */
#define CHSWAIT     0x02      /* Waiting for the state to change */
#define CHOWAIT     0x04      /* Waiting for a packet to be output */
#define CHEMPWAIT   0x08      /* Waiting for the xmit window to empty */
#define CHCLOSING   0x10      /* We are in the process of closing the conn */
#define CHOQWAIT    0x20      /* Like CHOWAIT, but we're not blocked */

/* Bit values for cn_flags */
#define CHEOFSEEN   0x01      /* An EOF has been received (recently) */
#define CHANSWER    0x02      /* This connection should send an ANS pkt */
#define CHSERVER    0x04      /* Connection was established by listening */
#define CHWRITER    0x08      /* Connection has been used for writing */
#define CHREADER    0x10      /* Connection has been used for reading */

/* Macros for certain connection states */
#define chtfull(conn)    ((conn)->cn_state == CSOPEN && \
                    (conn)->cn_tlast - (conn)->cn_tacked >= (conn)->cn_twsize)
#define chtempty(conn)   ((conn)->cn_state != CSOPEN || \
                    (conn)->cn_tlast == (conn)->cn_tacked)
#define chdead(conn)     ((conn)->cn_state == CSCLOSED || \
                    (conn)->cn_state == CSLOST || (conn)->cn_state == CSINCT)


/* Structure for built-in service table */
struct service {
     char      *name;
     int       len;
     int       (*func)();
};

/*
 * These are unsigned comparisons
 * all the casting is necessary due to compiler flakiness with shorts etc.
 * cmp_lt(a,b) is true if a < b, cmp_le(a,b) if a <= b, cmp_gt(a,b) if a > b
 * and cmp_ge(a,b) if a >= b
 */
#define cmp_gt(a,b) (0100000 & ((b) - (a)))
#define cmp_ge(a,b) !cmp_lt(a,b)
#define cmp_lt(a,b) cmp_gt(b,a)
#define cmp_le(a,b) !cmp_gt(a,b)


#define NOBLOCK     (struct block *)0
#define NOPKT       (struct packet *)0
#define NOCONN      (struct connection *)0

#endif

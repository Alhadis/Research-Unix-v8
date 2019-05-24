#define DEBUG

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

EXTERN int		Chaos_error;
EXTERN struct connection *Chconn[NCH];	/* connection table */
EXTERN chtime		Chclock;  	/* clock (mod ??) */
EXTERN struct packet 	*Chlsnlist,	/* listening connections */
			*Chrfclist,	/* list of unmatched rfc's */
			*Chrfctail;	/* tail of same list */
EXTERN char		Chmyname[CHSTATNAME];	/* This ncp's host name */
EXTERN chaddr		Chmyaddr;	/* This ncp's chaos address */
EXTERN struct queue	*ChaosQ;	/* Read queue pointer */
EXTERN int		ch_busy;	/* Flag to lock out timers */
EXTERN int		Chtimer;	/* Main timer running */

struct packet	*pkalloc(), *get_packet(), *new_packet();
struct connection *cnalloc(), *new_conn();

/* debugging instrumentation */

#ifdef DEBUG

#define debug(a,b)	if(Chdebug&(a)) b    /* expect a ; after! */

#define DALLOC	1	/* Allocation tracing */
#define DTRANS	2	/* Transmitter tracing */
#define DCONN	4	/* Connection activity */
#define DPKT	8	/* Print packets */
#define DNOCLK	16	/* No clock timeouts */
#define DABNOR	32	/* Abnormal events */
#define DSEND	64	/* Trace each packet sent */
#define DUSER	128	/* User interface activity */

#else
#define debug(a,b)
#endif

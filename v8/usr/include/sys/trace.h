/*
 * Memory allocator trace points; all trace the amount of memory involved
 */
#define	TR_MALL		10	/* memory allocated */

/*
 * Paging trace points: all are <vaddr, pid>
 */
#define	TR_INTRANS	20	/* page intransit block */
#define	TR_EINTRANS	21	/* page intransit wait done */
#define	TR_FRECLAIM	22	/* reclaim from free list */
#define	TR_RECLAIM	23	/* reclaim from loop */
#define	TR_XSFREC	24	/* reclaim from free list instead of drum */
#define	TR_XIFREC	25	/* reclaim from free list instead of fsys */
#define	TR_WAITMEM	26	/* wait for memory in pagein */
#define	TR_EWAITMEM	27	/* end memory wait in pagein */
#define	TR_ZFOD		28	/* zfod page fault */
#define	TR_EXFOD	29	/* exec fod page fault */
#define	TR_VRFOD	30	/* vread fod page fault */
#define	TR_CACHEFOD	31	/* fod in file system cache */
#define	TR_SWAPIN	32	/* drum page fault */
#define	TR_PGINDONE	33	/* page in done */

/*
 * System call trace points.
 */
#define	TR_VADVISE	40	/* vadvise occurred with <arg, pid> */

/* iget tracing */
#define TR_IGET		1
#define TR_IGOT		2
#define TR_SEND		3
#define TR_RECV		4

/*
 * up/uda interrupt tracking
 */
#define TR_UBGO		10	/* disk command */
#define TR_UBINT	11	/* got interrupt */
#define TR_BFIN		12	/* in bflush from update */
#define TR_BFOUT	13	/* out of bflush from update */
#define TR_BDPON	14	/* alloc bdp */
#define TR_BDPOFF	15	/* de-alloc bdp */
#define TR_UDCMND	16	/* sent a command to the uda-50 */
#define TR_UDRESP	17	/* uda 50 interrupted */

/*
 * Miscellaneous
 */
#define	TR_STAMP	50	/* user said vtrace(VTR_STAMP, value); */

/*
 * This defines the size of the trace flags array.
 */
#define	TR_NFLAGS	100	/* generous */

#define	TRCSIZ		4096

/*
 * Specifications of the vtrace() system call, which takes one argument.
 */
#define	VTRACE		64+51

#define	VTR_DISABLE	0		/* set a trace flag to 0 */
#define	VTR_ENABLE	1		/* set a trace flag to 1 */
#define	VTR_VALUE	2		/* return value of a trace flag */
#define	VTR_UALARM	3		/* set alarm to go off (sig 16) */
					/* in specified number of hz */
#define	VTR_STAMP	4		/* user specified stamp */
#ifdef TRACE
#ifdef KERNEL
char	traceflags[TR_NFLAGS];
struct	proc *traceproc;
int	tracebuf[TRCSIZ];
unsigned tracex;
int	tracewhich;
#define	trace(a,b,c)	if (traceflags[a]) trace1(a,b,c)
#endif
#else
#define trace(a, b, c)
#endif

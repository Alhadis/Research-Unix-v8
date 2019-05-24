#define	QPCTL	0100		/* priority control message */
#define	QBSIZE	64		/* "standard" size of queue block*/

/*
 * data queue
 */
struct	queue {
	struct	qinit	*qinfo;		/* procs and limits for queue */
	struct	block	*first;		/* first data block */
	struct	block	*last;		/* last data block */
	struct	queue	*next;		/* Q of next stream */
	struct	queue	*link;		/* to next Q for scheduling */
	caddr_t	ptr;			/* to private data structure */
	short	count;			/* number of blocks on Q */
	u_short	flag;
};

/* Queue flags */
#define	QENAB	01			/* Queue is already enabled to run */
#define	QWANTR	02			/* Someone wants to read Q */
#define	QWANTW	04			/* Someone wants to write Q */
#define	QFULL	010			/* Q is considered full */
#define	QREADR	020			/* This is the reader (first) Q */
#define	QUSE	040			/* This queue in use (allocation) */
#define	QNOENB	0100			/* Don't enable Q via putq */
#define	QDELIM	0200			/* This queue generates delimiters */
#define	QBIGB	0400			/* This queue would like big blocks */

/*
 * queue information structure
 */
struct	qinit {
	int	(*putp)();		/* put procedure */
	int	(*srvp)();		/* service procedure */
	long	(*qopen)();		/* called on startup */
	int	(*qclose)();		/* called on finish */
	short	limit;			/* high water mark */
	short	lolimit;		/* low water mark */
};

#define	OTHERQ(q)	((q)->flag&QREADR? (q)+1: (q)-1)
#define	WR(q)		(q+1)
#define	RD(q)		(q-1)

/*
 * Queue data block
 */
struct	block {
	struct	block	*next;
	u_char	*rptr;
	u_char	*wptr;
	u_char	*lim;
	u_char	*base;
	char	type;
	char	class;
};

/*
 * Header for a stream: interface to rest of system
 */
struct stdata {
	struct	queue *wrq;		/* write queue */
	struct	block *iocblk;		/* return block for ioctl */
	struct	inode *inode;		/* backptr, for hangups */
	struct	proc	*wsel;		/* process write-selecting */
	struct	proc	*rsel;		/* process read-selecting */
	short	pgrp;			/* process group, for signals */
	char	flag;
	char	count;			/* # processes in stream routines */
};
#define	IOCWAIT	01			/* Someone wants to do ioctl */
#define RSLEEP	02			/* Someone wants to read */
#define	WSLEEP	04			/* Someone wants to write */
#define	HUNGUP	010			/* Device has vanished */
#define	RSEL	020			/* read-select collision*/
#define	WSEL	040			/* write-select collision */
#define	EXCL	0100			/* exclusive-use (no opens) */
#define	STWOPEN	0200			/* waiting for 1st open */

struct	block	*getq();
int	putq();
struct	block	*allocb();
struct	queue	*backq();
struct	queue	*allocq();

/*
 * Control messages (regular priority)
 */
#define	M_DATA	0		/* regular data (not ctl) */
#define	M_BREAK	01		/* line break */
#define	M_HANGUP 02		/* line disconnect */
#define	M_DELIM	03		/* data delimiter */
#define	M_ECHO	04		/* request ACK (1 param) */
#define	M_ACK	05		/* response to ECHO (1 param) */
#define	M_IOCTL	06		/* ioctl; set/get params */
#define	M_DELAY 07		/* real-time xmit delay (1 param) */
#define	M_CTL	010		/* device-specific control message */
#define	M_PASS	011		/* pass file */
#define	M_YDEL	012		/* stream has started generating delims */
#define	M_NDEL	013		/* stream has stopped generating delims */

/*
 * Control messages (high priority; go to head of queue)
 */
#define	M_SIGNAL 0101		/* generate process signal */
#define	M_FLUSH	0102		/* flush your queues */
#define	M_STOP	0103		/* stop transmission immediately */
#define	M_START	0104		/* restart transmission after stop */
#define	M_IOCACK 0105		/* acknowledge ioctl */
#define	M_IOCNAK 0106		/* negative ioctl acknowledge */
#define	M_CLOSE	0107		/* channel closes (dk only) */
#define	M_IOCWAIT 0110		/* stop ioctl timeout, ack/nak follows later */

#define	setqsched()	mtpr(SIRR, 0x1);

/*
 * for passing files across streams
 */
struct	kpassfd {
	union  {
		struct	file *fp;
		int	fd;
	} f;
	short	uid;
	short	gid;
	short	nice;
};

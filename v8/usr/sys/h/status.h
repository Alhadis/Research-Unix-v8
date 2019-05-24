struct status {
	short	code;		/* status code */
	short	len;		/* actual number of bytes transferred */
	short	lchnum;		/* logical channel number */
	char	lchdir;		/* sub-channel:recv,xmit,both */
};

/* possible values for code */
#define ST_OK   	0	/* successful return */
#define ST_IARGS	1	/* illegal arguments */
#define	ST_HARD 	2	/* hardware failure */
#define ST_PREV 	3	/* previous request already queued */
#define ST_NOPREV	4	/* no request currently queued */




/* PBLKSIZ is the maximum length of a key/record pair */
/* DBLKSIZ * BYTESIZE yeilds # Pblocks filled per Dblock */

/*	size of Pblock in bytes */
#define	PBLKSIZ	(64L*1024L)
/*	size of Dblock in bytes */
#define	DBLKSIZ	4096L
/*	size of char in bits */
#define	BYTESIZ	8

#ifndef	NULL
#define	NULL	((char *) 0)
#endif	NULL

/* data structures private to DB: */
/* structure of a Pblock: */
struct	Pblock
{
	union
	{
		unsigned short	P_off[PBLKSIZ / sizeof (unsigned short)];
		char		P_dat[PBLKSIZ];
	}
	p_u;
};
#define	p_off	p_u.P_off
#define	p_dat	p_u.P_dat

/* structure of a Dblock: */
struct	Dblock
{
	char	d_bits[DBLKSIZ];
};

/* public data types: */
/*	key and record data structure */
struct	datum
{
	char *	dptr;
	int	dsize;
};

/*	the state of a DB file */
struct	DBFILE
{
	/* public data: */
	int		d_flags;	/* flags; see below */
	char *		d_name;		/* name of the database file */
	/* private data: */
	/*	page file info: */
	int		d_pfd;		/* page file descriptor */
	long		d_pblk;		/* block number of page file */
	Pblock *	d_pbuf;		/* buffer for page file */
	/*	direct file info: */
	int		d_dfd;		/* direct file descriptor */
	long		d_dblk;		/* block number of direct file */
	Dblock *	d_dbuf;		/* buffer for direct file */
	/*	hashing info: */
	long		d_hmask;	/* hash mask */
	long		d_hnbit;	/* maximum bit # */
};

/* flag bits in d_flags: */
#define	DB_RONLY	0x0001	/* this database is read only */
#define	DB_SYNC		0x0002	/* sync() on every write (ouch!) */
#define	DB_PMOD		0x0004	/* disk is out of date with respect to d_pbuf */
#define	DB_DMOD		0x0008	/* disk is out of date with respect to d_dbuf */
#define	DB_CREATE	0x0010	/* create database files if they don't exist */
#define	DB_LOCK		0x0020	/* database is locked */
/*	user settable flags: */
#define	DB_USER		(DB_RONLY|DB_SYNC|DB_CREATE)

/* public functions defined in DB: */
extern DBFILE *	DBopen(char *, int);		/* open a DB file*/
extern void	DBclose(DBFILE *);		/* close a DB file*/
extern void	DBsync(DBFILE *);		/* flush any modified buffers*/
extern datum	DBget(DBFILE *, datum);		/* get rec at key*/
extern int	DBdel(DBFILE *, datum);		/* del rec at key*/
extern int	DBput(DBFILE *, datum, datum);	/* put rec at key*/
extern datum	DBkey0(DBFILE *);		/* get lowest key*/
extern datum	DBkeyn(DBFILE *, datum);	/* get next key*/
extern int	DBlock(DBFILE *);		/* lock a DB file */
extern int	DBunlock(DBFILE *);		/* unlock a DB file */
extern int	DBapp(DBFILE *, datum, datum);	/* append to rec at key */
extern int	DBins(DBFILE *, datum, datum);	/* insert before rec at key */

/* public data defined in DB */
extern int	DBdebug;	/* DB debugging level */
#define	DBDBCORE	0	/* errors that produce core files */
#define	DBDBERR		1	/* errors less catastrophic */
#define	DBDBWARN	2	/* various warnings */
#define	DBDBINFO	3	/* entry/exit info for DB*() above */

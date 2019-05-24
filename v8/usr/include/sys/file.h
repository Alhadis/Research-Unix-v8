/*
 * One file structure is allocated
 * for each open/creat/pipe call.
 * Main use is to hold the read/write
 * pointer associated with each open
 * file.
 */
struct	file
{
	short	f_flag;
	short	f_count;		/* reference count */
	struct inode *f_inode;		/* pointer to inode structure */
	off_t	f_offset;		/* read/write character pointer */
#ifdef	CHAOS
	caddr_t	f_conn;			/* Chaosnet connection pointer */
#endif	CHAOS
};

#ifdef	KERNEL
struct	file *file, *fileNFILE;	/* the file table itself */
int	nfile;

struct	file *getf();
struct	file *falloc();
struct	file *allocfile();
#endif

/* flags */
#define	FREAD	01
#define	FWRITE	02
#define	FPIPE	04
#define	FHUNGUP 010
#define	FALOCK	020	/* this file is advisory-locked */
#define	FRNBLK	040	/* no block on read */
#define	FWNBLK	0100	/* no block on write */

/* file.h.  NFILE is 20==19 files + the command line.
 * when i rewrite the compacting allocator, Block.block
 * will be dynamically allocated and growable.
 */
#define	NFILE		20
#define	NBLOCK		512
#define	BLOCKSIZE	(4096)

/*
 * Descriptor pointing to a disk block
 */
typedef struct{
	short	nbytes;	/* number of bytes used in this block */
	short	bnum;	/* which disk block it is */
}Block;
/*
 * Strings all live in a common buffer and are nicely garbage compacted.
 */
typedef struct{
	char	*s;	/* pointer to string */
	short	n;	/* number used; s[n]==0 */
	short	size;	/* size of allocated area */
}String;
String *bldstring();	/* (temporary String)-building function */
#define	DUBIOUS	1L	/* An unlikely date; means we don't know origin of buffer */
/*
 * A File is a file descriptor, a local buffer for the in-core block,
 * and an array of block pointers.  The order of the block pointers
 * in the file structure determines the order of the true data,
 * as opposed to the order of the bits in the file (c.f. ed temp files *).
 */
typedef struct{
	String	*name;	/* name of associated real file, "" if none */
	String	*str;	/* storage for in-core data */
	long	date;	/* date file was read in */
	short	namelen;/* length of name in menu */
	short	changed;/* changed since last write */
	long	origin;	/* file location of first char on screen */
	long	selloc;	/* start location of selected text */
	long	nsel;	/* number of chars selected */
	Block	*curblock;	/* block associated with File.str */
	short	nblocks;	/* number of blocks in File.block */
	/* The block array should be pointed to and allocated,
	 * not built into the structure (easy), so it can grow */
	Block	block[NBLOCK];	/* array of block pointers */
}File;
File file[NFILE];
String *buffer;		/* Place to save squirreled away text */
String *transmit;	/* String to send to Jerq */

File	*Fcreat();
String	*newstring();
long	length(), Fforwnl(), Fbacknl(), Fcountnl();
#define	YMAX	1024	/* as in jerq.h */
#define	DIAG	(&file[0])
#define	CURFILE	(&file[1])
#define	TRUE	1
#define	FALSE	0
long	loc1, loc2;	/* location of searched-for string */
char	*strcpy();
int	fileschanged;

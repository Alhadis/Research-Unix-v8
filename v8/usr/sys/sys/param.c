/*	param.c	4.2	81/04/02	*/

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/text.h"
#include "../h/inode.h"
#include "../h/file.h"
#include "../h/callout.h"
#include "../h/cmap.h"
/*
 * System parameter formulae.
 *
 * This file is copied into each directory where we compile
 * the kernel; it should be modified there to suit local taste
 * if necessary.
 *
 * Compiled with -DHZ=xx -DTIMEZONE=x -DDST=x -DMAXUSERS=xx
 */

int	hz = HZ;
int	timezone = TIMEZONE;
int	dstflag = DST;
#define	NPROC (20 + 8 * MAXUSERS)
int	nproc = NPROC;
int	ntext = 24 + MAXUSERS;
int	ninode = 3 * (NPROC + 16 + MAXUSERS) + 32;
int	nfile = 2 * (NPROC + 16 + MAXUSERS) + 32;
int	ncallout = 16 + MAXUSERS;

/*
 * These are initialized at bootstrap time
 * to values dependent on memory size
 */
int	nbuf, nswbuf;

/*
 * These have to be allocated somewhere; allocating
 * them here forces loader errors if this file is omitted.
 */
struct	proc *proc, *procNPROC;
struct	text *text, *textNTEXT;
struct	inode *inode, *inodeNINODE;
struct	file *file, *fileNFILE;
struct 	callout *callout;

struct	buf *buf, *swbuf;
short	*swsize;
int	*swpf;
char	*buffers;
struct	cmap *cmap, *ecmap;

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/acct.h"
#include "../h/inode.h"
#include "sparam.h"

struct	buf	bfreelist[BQUEUES];	/* buffer chain headers */
struct	buf	bswlist;	/* free list of swap headers */
struct	buf	*bclnlist;	/* header for list of cleaned pages */
struct	acct	acctbuf;
struct	inode	*acctp;

/*
 * Fetchable stream parameters
 */
int	Nqueue	= NQUEUE;
int	Nstream	= NSTREAM;
int	Nblk64	= NBLK64;
int	Nblk16	= NBLK16;
int	Nblk4	= NBLK4;
#ifndef	NBLKBIG
#define	NBLKBIG	0
#endif
int	Nblkbig	= NBLKBIG;
int	Nblock	= (NBLKBIG+NBLK64+NBLK16+NBLK4);

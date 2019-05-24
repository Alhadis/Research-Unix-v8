#include "../h/param.h"
#include "../h/conf.h"
/*
 * four ra configuration
 */
dev_t	rootdev	= makedev(7, 64);	/* 4K filesystem */
dev_t	pipedev	= makedev(7, 64);	/* 4K filesystem */
dev_t	argdev	= makedev(7, 1);	/* 1st swap area */
dev_t	dumpdev	= makedev(7, 1);	/* 1st swap area */
long	dumplo	= 20480 - 8 * 2048;	/* ok up to 8 megabytes */

/*
 * Nswap is the basic number of blocks of swap per
 * swap device, and is multiplied by nswdev after
 * nswdev is determined at boot.
 */
int	nswap = 20480;

struct	swdevt swdevt[] =
{
	makedev(7, 1),	0,		/* ra01 */
	makedev(7, 9),	0,		/* ra11 */
	makedev(7, 17),	0,		/* ra21 */
	makedev(7, 25), 0,		/* ra31 */
	0,		0,
};


#include "../h/param.h"
#include "../h/conf.h"
/*
 * two ra configuration
 *   (dumps up to 4 MB only)
 */
dev_t	rootdev	= makedev(7, 64);	/* 4K filesystem */
dev_t	pipedev	= makedev(7, 64);	/* 4K filesystem */
dev_t	argdev	= makedev(7, 1);	/* 1st swap area */
dev_t	dumpdev	= makedev(7, 1);	/* 1st swap area */
long	dumplo	= 20480 - 4 * 2048;	/* ok up to 4 megabytes */

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
	0,		0,
};


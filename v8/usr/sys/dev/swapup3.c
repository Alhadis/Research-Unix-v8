#include "../h/param.h"
#include "../h/conf.h"
/*
 * three storage module drives
 *	root on em00
*	paging on em01 em11 em21
 */
dev_t	rootdev	= makedev(2, 0);
dev_t	pipedev	= makedev(2, 0);
dev_t	argdev	= makedev(2, 1);
dev_t	dumpdev	= makedev(2, 1);
long	dumplo	= 20480 - 5 * 2048;

/*
 * Nswap is the basic number of blocks of swap per
 * swap device, and is multiplied by nswdev after
 * nswdev is determined at boot.
 */
int	nswap = 20480;

struct	swdevt swdevt[] =
{
	makedev(2, 1),	0,		/* em01 */
	makedev(2, 1+8), 0,		/* em11 */
	makedev(2, 1+16), 0,		/* em21 */
	0,		0,
};

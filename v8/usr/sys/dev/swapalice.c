#include "../h/param.h"
#include "../h/conf.h"
/*
 * Dual rp0?/rm?? configuration
 *	root on hp00
 *	paging on hp01, hp11, ra01, ra11, ra21, ra31
 */
dev_t	rootdev	= makedev(0, 0);
dev_t	pipedev	= makedev(0, 0);
dev_t	argdev	= makedev(0, 1);
dev_t	dumpdev = makedev(0, 1);
int	dumplo	= 33440 - 10 * 2048;

/*
 * Nswap is the basic number of sectors of swap per
 * swap device, and is multiplied by nswdev after
 * nswdev is determined at boot.
 */
int	nswap = 20480;	/* 10 Meg per swap dev */

struct	swdevt swdevt[] =
{
	makedev(0, 1),	0,		/* hp01 */
	makedev(0, 9),	0,		/* hp11 */
	makedev(7, 1),	0,		/* ra01 */
	makedev(7, 9),	0,		/* ra11 */
	makedev(7, 17),	0,		/* ra21 */
	makedev(7, 25),	0,		/* ra31 */
	0,		0,
};

#include "machine.h"
#include "mtype.h"
/*
 * sdb/adb - common definitions for old srb style code
 */

#define MAXCOM	64
#define MAXARG	32
#define LINSIZ	512

/*
 * miscellaneous bournese
 */
typedef	char	BOOL;
typedef	char	MSG[];


/*
 * file address maps
 * each open file has one per segment
 * if b <= address <= e, address is valid in space type sp
 * and may be found at address + f in the file
 */

#define	NMAP	5	/* text data stack u-area endmarker */

struct map {
	ADDR	b;		/* base */
	ADDR	e;		/* end */
	ADDR	f;		/* offset within file */
	int	sp;		/* type of space */
	char	*tag;		/* name for the segment mapped */
};
typedef	struct map	MAP;

struct bkpt {
	ADDR	loc;
	WORD	ins;
	int	count;
	int	initcnt;
	int	flag;
	char	comm[MAXCOM];
	struct bkpt *nxtbkpt;
};
typedef struct bkpt	BKPT;

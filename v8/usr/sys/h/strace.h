#define TR_NAME	17	/* 1 + max ioctl message size */

struct trc {
	char	tr_name[TR_NAME];
				/* user supplied name for trace messages */
	short	tr_state;	/* internal state */
	int	tr_mask;	/* block type trace mask */
};


/*
 * values for tr_state field:
 */

#define TR_USE	1	/* trc structure in use */


/*
 * Ioctl message format.
 */

struct trcioc {
	int	command;

	union {
		int	mask;
		char	name[16];
	} arg;
};

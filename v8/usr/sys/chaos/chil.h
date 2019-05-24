/*
 * Definitions for interlan ethernet interface used as chaos network interface
 */

/*
 * structure needed for each transmitter. (member of xminfo union).
 */
struct chilinfo	{
	struct chilsoft *il_soft;
};

#define CHILBASE	0764000		/* base UNIBUS address for ch11's */
#define CHILINC		010		/* ??? */


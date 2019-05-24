#ifdef	XT
#ifndef	MAXPCHAN
#include	"sys/xtproto.h"
#endif
#else	XT
/*
**	Mpx -- Blit packet protocol definition
*/

typedef	unsigned char	Pbyte;			/* The unit of communication */

#define	MAXPCHAN	16			/* Maximum channel number */
#define	CBITS		4			/* Bits for channel number */
#define	MAXPKTDSIZE	(64 * sizeof(Pbyte))	/* Maximum data part size */
#define	EDSIZE		(2 * sizeof(Pbyte))	/* Error detection part size */
#define	SEQMOD		4			/* Sequence number modulus */
#define	SBITS		2			/* Bits for sequence number */

/*
**	Packet header
**	(if only bit fields in C were m/c independant, sigh...)
*/

struct P_header
{
#	ifdef	vax
	Pbyte		seq	:SBITS,		/* Sequence number */
			channel	:CBITS,		/* Channel number */
			cntl	:1,		/* TRUE if control packet */
			ptyp	:1;		/* Always 1 */
	Pbyte		dsize;			/* Size of data part */
#	endif	vax
#	ifdef	Blit
	int		null	: 16,		/* null 16 bits because if m32*/
			ptyp	:1,		/* Always 1 */
			cntl	:1,		/* TRUE if control packet */
			channel	:CBITS,		/* Channel number */
			seq	:SBITS,		/* Sequence number */
			dsize	:8;		/* Size of data part */
#	endif	Blit
};

typedef	struct P_header	Ph_t;

/*
**	Packet definition for maximum sized packet for transmission
*/

struct Packet
{
	Ph_t		header;			/* Header part */
	Pbyte		data[MAXPKTDSIZE];	/* Data part */
	Pbyte		edb[EDSIZE];		/* Error detection part */
};

typedef struct Packet *	Pkt_p;

/*
**	Control codes
*/

#define	ACK		(Pbyte)(006)	/* Last packet with same sequence ok and in sequence */
#define	NAK		(Pbyte)(025)	/* Last packet with same sequence received out of sequence */
#define	PCDATA		(Pbyte)(002)	/* Data only control packet */
#endif	XT

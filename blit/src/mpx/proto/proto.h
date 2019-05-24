#ifdef	XT
#ifndef	MAXPCHAN
#include	"sys/xtproto.h"
#endif
#else	XT
/*
**	Mpx -- Blit packet protocol definition
*/

typedef	unsigned char	Pbyte;			/* The unit of communication */

#define	MAXPCHAN	8			/* Maximum channel number */
#define	CBITS		3			/* Bits for channel number */
#define	MAXPKTDSIZE	(64 * sizeof(Pbyte))	/* Maximum data part size */
#define	EDSIZE		(2 * sizeof(Pbyte))	/* Error detection part size */
#define	SEQMOD		8			/* Sequence number modulus */
#define	SBITS		3			/* Bits for sequence number */

/*
**	Packet header
**	(if only bit fields in C were m/c independent, sigh...)
*/

struct P_header
{
#	ifdef	vax
	Pbyte		channel	:CBITS,		/* Channel number */
			seq	:SBITS,		/* Sequence number */
			cntl	:1,		/* TRUE if control packet */
			ptyp	:1;		/* Always 1 */
	Pbyte		dsize;			/* Size of data part */
#	endif	vax
#	ifdef	mc68000
	short		ptyp	:1,		/* Always 1 */
			cntl	:1,		/* TRUE if control packet */
			seq	:SBITS,		/* Sequence number */
			channel	:CBITS,		/* Channel number */
			dsize	:8;		/* Size of data part */
#	endif	mc68000
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

#define	PPCNTL		0200			/* Packet protocol control code */
#define	ACK		(Pbyte)(PPCNTL|006)	/* Last packet with same sequence ok and in sequence */
#define	NAK		(Pbyte)(PPCNTL|025)	/* Last packet with same sequence received out of sequence */
#define	PCDATA		(Pbyte)(PPCNTL|0100)	/* Data only control packet */
#endif	XT

/*
 * Definitions for CHAOS-11 interface card used as Chaosnet link.
 */

/*
 * structure needed for each transmitter. (member of xminfo union).
 */
struct ch11info	{
	int		ch_rtries;	/* number of retries */
};

#define CH11BASE	0764140		/* base UNIBUS address for ch11's */
#define CH11INC		020		/* increment between ch11's (wrong!)*/
/*
 * Arrangement of chaos11 registers
 */
struct	ch11	{
	short	ch_csr;		/* Command and status register */
	short	ch_wbf;		/* Write buffer */
	short	ch_rbf;		/* Read buffer */
	short	ch_rbc;		/* Read bit counter */
	short	ch_nop;		/* Unused */
	short	ch_xmt;		/* Initiate transmission */
};
#define ch_myaddr	ch_wbf	/* When read, ch_wbf is interface address */
/*
 * ch_csr bit definitions
 */
#define CHBUSY		01		/* Transmitter busy */
#define CHLPBK		02		/* Loop back in interface */
#define CHSPY		04		/* Spy - accept any message */
#define CHREN		010		/* Receive enable */
#define CHRIEN		020		/* Receive interrupt enable */
#define CHTIEN		040		/* Transmit interrupt enable */
#define CHABRT		0100		/* Transmit aborted by conflict */
#define CHTDN		0200		/* Transmission done */
#define CHTCLR		0400		/* Transmitter clear */
#define CHLC		017000		/* Count of messages lost */
#define CHLCPOS		9		/* Bit position of lost count */
#define CHLCMASK	017		/* Mask after shift is messages lost */
#define CHRST		020000		/* I/O reset for interface */
#define CHCRC		040000		/* CRC error */
#define CHRDN		0100000		/* Input done */

/*
 * Definitions for DR11-C interface used as Chaosnet link.
 */

/*
 * Structure needed for each transmitter. (member of xminfo union).
 */
struct dr11cinfo	{
	struct dr11c	*dr_addr;	/* Actual UNIBUS address */
	short		*dr_tptr;	/* Next short in trans pkt */
	short		*dr_rptr;	/* Next short in rcv pkt */
	short		dr_tcnt;	/* # of shorts to send in pkt */
	short		dr_rcnt;	/* # of shorts to rcve in pkt */
	short		dr_tcheck;	/* Transmit block check word */
	short		dr_rcheck;	/* Receive block check word */
	char		dr_tstate;	/* Framing state of transmitter */
	char		dr_rstate;	/* Framing state of receiver */
	char		dr_intrup;	/* True during interrupt, used by
					   clock level to suppress checking
					   for hung interface if clock
					   interrupted a dr11c interrupt */
};

#ifdef vax
#define DR11CBASE	(0176770 + UBA0_DEV)	/* base UNIBUS address for first dr11-c */
#else
#define DR11CBASE	0167770	/* base UNIBUS address for first dr11-c */
#endif
#define DR11CINC	-010	/* increment for next dr11-c */
/*
 * Arrangement of dr11 registers
 */
struct dr11c	{
	short	dr_csr;
	short	dr_obuf;
	short	dr_ibuf;
};
/*
 * dr_csr bit definitions
 */
#define DROUT	02	/* I have sent output - set by xmitter (sets DRIRDY) */
#define DRORDY	0200	/* He has read output - interrupts (from DRIN) */
#define DROE	0100	/* Interrupt enable (allow DRORDY to interrupt me) */

#define DRIN	01	/* I have read input - set by receiver (sets DRORDY)*/
#define DRIRDY	0100000	/* He has sent input - interrupts (from DROUT) */
#define DRIE	040	/* Interrupt enable (allow DRIRDY to interrupt me) */

/*
 * Definitions of framing characters and CRC constants.
 */
#define DR11CHUNG	(HZ*2)
#define DRSYNC	(short)0137773
#define DRESC	(short)0167776
/*
 * Framing states
 */
#define DRIDLE	0	/* Idle, between packets */
#define DRSYN1	1	/* After one DRSYNC sent/received */
#define DRSYN2	2	/* After two DRSYNC's sent/received */
#define DRCNT1	3	/* After count sent/received */
#define DRDATA	4	/* In the middle of data */
#define DRESC1	5	/* After DRESC escape sent/received */
#define DRCHECK	6	/* After last data word sent/received */
#define DRTDONE	7	/* After check is transmitted (transmitter only) */

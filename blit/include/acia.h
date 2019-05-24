struct acia {
	char	a_control;	/* control register */
	char	filler;		/* NOTHING */
	char	a_data;		/* data register */
};

/* control bits */
#define	A_CDIV1		0x00	/* divide speed by 1 */
#define	A_CDIV16	0x01	/* divide speed by 16 */
#define	A_CDIV64	0x02	/* divide speed by 64 */
#define	A_RESET		0x03	/* reset acia */

#define	A_S2EB7		0x00	/* 2 stop bits, even parity, 7 bits */
#define	A_S2OB7		0x04	/* 2 stop bits, odd parity, 7 bits */
#define	A_S1EB7		0x08	/* 1 stop bit, even parity, 7 bits */
#define	A_S1OB7		0x0C	/* 1 stop bit, odd parity, 7 bits */
#define	A_S2NB8		0x10	/* 2 stop bit, no parity, 8 bits */
#define	A_S1NB8		0x14	/* 1 stop bit, no parity, 8 bits */
#define	A_S1EB8		0x18	/* 1 stop bit, even parity, 8 bits */
#define	A_S1OB8		0x1C	/* 1 stop bit, odd parity, 8 bits */

#define	A_RSBLTD	0x00	/* !RTS low, transmit intrpt disabled */
#define	A_RSBLTE	0x20	/* !RTS low, transmit intrpt enabled */
#define	A_RSBHTD	0x40	/* !RTS high, transmit intrpt disabled */
#define	A_RSBLBTD	0x60	/* !RTS low, send break, */
					/* transmit intrpt disabled */
#define	A_RE		0x80	/* receive intrpt enabled */

/* status bits */
#define	A_RDRF		0x01	/* receive data register full */
#define	A_TDRE		0x02	/* transmit data register empty */
#define	A_DCDBAR	0x04	/* ! data carrier detect */
#define	A_CTSBAR	0x08	/* ! clear to send */
#define	A_FE		0x10	/* framing error */
#define	A_OVRN		0x20	/* receiver overrun */
#define	A_PE		0x40	/* parity error */
#define	A_IRQ		0x80	/* interrupt request */

/* ACIA locations in Design Module */
#define	TERMACIA	0x3ff01
#define	HOSTACIA	0x3ff21

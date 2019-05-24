
/* Command word format
 *
 *	+-----------------------------------------------+
 *	|  |B |R |A |           |              |        |
 *	|  |C |S |B |    DID    |              |  FCN   |
 *	|  |T |T |T |           |              |        |
 *	+-----------------------------------------------+
 *	 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
 */
/* Function codes */
#define NOP  	0x0
#define BOARD_RESET	0x5
#define MASTER_CLEAR	0x6

#define ABORT(did)	(0x1000 | (did))
#define RESET(did)	(0x2000 | (did))
#define BROAD_CAST(did)	(0x4000 | (did))


/* SNET status word bits
 *
 *	+-----------------------------------------------+
 *	|                             |E |S |I |O |  |  |
 *	|                             |O |N |B |B |  |  |
 *	|                             |P |K |E |E |  |  |
 *	+-----------------------------------------------+
 *	 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
 */	
#define OUTBUF_E	0x4
#define INBUF_E	0x8
#define SNACK	0x10	/* transmitter error */
#define EOP	0x20	/* we have read and EOP frame */

#define	NET(dev)	((dev)&255)

#define		ANRESET		(('n'<<8) | 0)

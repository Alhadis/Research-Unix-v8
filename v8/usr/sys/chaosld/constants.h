#ifndef _CHCONSTANTS_
#define _CHCONSTANTS_
/*
 * This file contains constants defined in the basic chaos protocol,
 * including the built-in RFC's (status, etc.)
 */
#define CHMAXPKT	488
#define CHMAXDATA	488	/* Maximum data per packet */
#define CHSTATNAME	32	/* Length of node name in STATUS protocol */
#define CHDRWSIZE	5	/* Default receive window size */
#define CHNSUBNET	10	/* Number of subnets available */
#define CHSP	(040)
#define CHNL	(0200|'\r')
#define CHTAB	(0200|'\t')
#define CHFF	(0200|'\f')
#define CHBS	(0200|'\b')
#define CHLF	(0200|'\n')
/*
 * These are the connection states
 */
#define CSCLOSED	0	/* Closed */
#define CSLISTEN	1	/* Listening */
#define CSRFCRCVD	2	/* RFC received (used?) */
#define CSRFCSENT	3	/* RFC sent */
#define CSOPEN		4	/* Open */
#define CSLOST		5	/* Broken by receipt of a LOS */
#define CSINCT		6	/* Broken by incomplete transmission */
#define CSBRDSENT	7	/* BRD sent */
#define CSSTARTING	8	/* UNIX fd open, but conn not yet initted */

/*
 * These are the packet opcode types
 */
#define RFCOP	001		/* Request for connection */
#define OPNOP	002		/* Open connection */
#define CLSOP	003		/* Close connection */
#define FWDOP	004		/* Forward this packet */
#define ANSOP	005		/* Answer packet */
#define SNSOP	006		/* Sense packet */
#define STSOP	007		/* Status packet */
#define RUTOP	010		/* Routing information packet */
#define LOSOP	011		/* Losing connection packet */
#define LSNOP	012		/* Listen packet (never transmitted) */
#define MNTOP	013		/* Maintenance packet */
#define EOFOP	014		/* End of File packet  */
#define UNCOP	015		/* Uncontrolled data packet */
#define BRDOP	016		/* Broadcast request */
#define MAXOP	017		/* Maximum legal opcode */
#define DATOP	0200		/* Ordinary character data  */
#define DWDOP	0300		/* 16 bit word data */

/*
 * Modes available in CHIOCSMODE call.
 */
#define CHSTREAM	1
#define CHRECORD	2

/*
 * Chaos device IOCTL codes
 */
#define _IO(x,y)	(('x'<<8)|y)
#define _IOW(x,y,t)	_IO(x,y)
#define _IOR(x,y,t)	_IO(x,y)

#define CHIOCFLUSH	_IO(c,4)
#define CHIOCGSTAT	_IOR(c,5,struct chstatus)
#define CHIOCSWAIT	_IO(c,6)
#define CHIOCANSWER	_IO(c,7)
#define CHIOCREJECT	_IOW(c,8,struct chreject)
#define CHIOCACCEPT	_IO(c,9)
#define CHIOCOWAIT	_IO(c,10)
#define CHIOCRPKT	_IOR(c,16,struct chrpkt)

/*
 * Unmatched RFC server IOCTL codes
 */

#define CHIOCADDR	_IOW(c,11,int)
#define CHIOCNAME	_IOW(c,12,struct chstatname)

/*
 * Error codes
 */

#define CHNOCONN	-1


#define CHAOSDEV    "/dev/ch/ch"

#endif

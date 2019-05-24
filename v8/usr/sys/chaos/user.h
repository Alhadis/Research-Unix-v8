#include "../chaos/constants.h"
/*
 * Definitions needed by user programs.
 */
#define CHMAXPKT	488		/* Maximum data length in packet */
#define CHMAXRFC	CHMAXPKT	/* Maximum length of a rfc string */
#define CHMAXARGS	50		/* Maximum number of words in a RFC */
#define	CHRFCDEV	"/dev/chaos"	/* Path name for sending RFC's */
#define CHRFCADEV	"/dev/chaosa"	/* Path name for asynchronous RFC's */
#define CHLISTDEV	"/dev/chlisten"	/* Path name for listen */
#define CHURFCDEV	"/dev/churfc"	/* Path name for unmatched RFC list */
#define CHNETCHAR	'^'
#define CHSYSPREF	"/dev/ch"
#define CHCPRODEV	"/dev/chcproto"	/* Path name for channel driver
					 * prototype - for major device number
					 * and canonical invalid minor device
					 */
/*
 * This structure returned by the CHIOCGSTAT ioctl to return
 * connection status information.
 */
struct	chstatus	{
	short	st_fhost;		/* remote host */
	short	st_cnum;		/* local channel number */
	short	st_rwsize;		/* receive window size */
	short	st_twsize;		/* transmit window size */
	short	st_state;		/* connection state */
	short	st_ptype;		/* Opcode of next packet to read */
	short	st_plength;		/* Length of next packet to read */
	short	st_cmode;		/* Mode of connection */
	short	st_oroom;		/* Output window space left */
	/* etc - anything else useful? */
};
/*
 * Record mode packet structure.
 */
struct chpacket	{
	unsigned char	cp_op;
	char		cp_data[CHMAXDATA];
};
/*
 * FILE server login record structure.
 */
struct chlogin {
	int	cl_pid;		/* Process id of server */
	short	cl_cnum;	/* Chaos channel number of server */
	short	cl_haddr;	/* Host address of other end */
	long	cl_ltime;	/* Login time */
	long	cl_atime;	/* Last time used. */
	char	cl_user[8];	/* User name */
};
/*
 * Structure for CHIOCILADDR
 */
struct chiladdr {
	unsigned short	cil_device;
	unsigned short	cil_address;
};

/*
 * Chaos net io control commands
 */
#define CHIOCRSKIP	(('c'<<8)|1)	/* Skip the last read unmatched RFC */
#define CHIOCPREAD	(('c'<<8)|2)	/* Read my next data or control pkt */
#define CHIOCSMODE	(('c'<<8)|3)	/* Set the mode of this channel */
#define CHIOCFLUSH	(('c'<<8)|4)	/* flush current output packet */
#define CHIOCGSTAT	(('c'<<8)|5)	/* Make input reading like a tty */
#define CHIOCSWAIT	(('c'<<8)|6)	/* Wait for a different state */
#define CHIOCANSWER	(('c'<<8)|7)	/* Answer an RFC (in RFCRECVD state) */
#define CHIOCREJECT	(('c'<<8)|8)	/* Reject an RFC. Arg is string addr. */
#define CHIOCACCEPT	(('c'<<8)|9)	/* Accept an RFC, opening the connection. */
#define CHIOCOWAIT	(('c'<<8)|10)	/* Wait until all output acked. */
#define CHIOCADDR	(('c'<<8)|11)	/* Set my address */
#define CHIOCNAME	(('c'<<8)|12)	/* Set my name */
#define CHIOCILADDR	(('c'<<8)|13)	/* Set chaos address for Interlan ethernet */

#define DATESIZE 64		/* maximum size of a date field */
#define ADDRSIZE 400		/* maximum size of an address */
#define FROMLINESIZE 512	/* maximum length of a from line */
#define PATHSIZE 128		/* maximum size of a file path */
#define CMDSIZE 512		/* maximum size of a command */


/* tokens to parse mail headers */
#define FROM "From "
#define FSIZE sizeof(FROM)-1
#define ALTFROM ">From "
#define REMFROM " remote from "

/* mail file status */
#define MFTYPE	0x3	/* type of mail file */
#define MF_NORMAL 0
#define MF_FORWARD 1
#define MF_PIPE 2
#define MFEXTRA 0x4	/* extra stuff besides forward to or pipe to in mailfile */

#ifndef NULL
#define NULL 0
#endif NULL

#ifndef _CHDEV_
#define _CHDEV_

/*
 * This file defines how the UNIX minor device number is used by the UNIX
 * Chaos N.C.P.
 *
 * The minor device is divided into 2 fields: host and (contact) name
 * The low order CHNBITS specifies how to get the contact name,
 * the next higher order CHHBITS specifies how to get the host address.
 *
 * Host field definitions:
 */
#define CHHBITS		4		/* Number of bits of host specifier */
#define CHHMAX		((1 << CHHBITS) - 1)
					/*  Maximum value of host field */
#define CHHMASK		CHHMAX		/* Mask after shift for host */
#define CHHSHIFT	(8-CHHBITS-1)	/* Right shift of minor for host */
#define CHHOST(dev)	((minor(dev) >> CHHSHIFT) & CHHMASK)
					/* Extraction macro */
#define CHHSPEC		CHHMAX		/* Value for special devices */
#define CHHREAD		CHHMAX-1	/* Host number is in rest of path */
#define CHHUSE		CHHMAX-2	/* Host number is in last component */
#define CHHMAXN		CHHMAX-2	/* Max number of auto hosts numbers */

/*
 * Name field values for special devices:
 */
#define CHUNMATCHED	0	/* Minor device for unmatched RFC reader */
#define CHLISTEN	1	/* Listen */

/*
 * Contact name field definitions:
 */
#define CHNBITS		(8 - CHHBITS - 1)
#define CHNMAX		((1 << CHNBITS) - 1)	/* Max value of field */
#define CHNMASK		CHNMAX			/* Mask for extraction */
#define CHNAME(dev)	(minor(dev) & CHNMASK)	/* Extraction macro */
#define CHNREAD		CHNMAX		/* Contact name is rest of path */
#define CHNUSE		CHNMAX-1	/* Contact name is last component */
#define CHNMAXN		CHNMAX-1	/* Max number of auto contact names */

#define CHMKMIN(h,n)	((h << CHHSHIFT) | n)
#define CHURFCMIN	CHMKMIN(CHHSPEC, CHUNMATCHED)
#define CHLISTMIN	CHMKMIN(CHHSPEC, CHLISTEN)
#define CHRFCAMIN	CHMKMIN(CHHREAD, CHNREAD)
#define CHRFCMIN	(CHHANGMIN | (CHRFCAMIN))
#define CHHANGMIN	0200
#define CHHANGDEV(dev)	(minor(dev)&0200)

#define NONAME		((char *)0)

extern char *chnames[][3];
extern short chhosts[];		/* Array of "well known hosts" */

/*
 * For the channel driver, and some user programs, we need to know a
 * purposely invalid minor device number
 */
#define CHBADCHAN	255
#endif

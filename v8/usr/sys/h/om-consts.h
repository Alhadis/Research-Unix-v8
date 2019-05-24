/* ******************************************************************** */
/*									*/
/*			METHEUS Corporation				*/
/*									*/
/*		UNIX - DMA Interface Driver Source Code			*/
/*									*/
/*	This program and the subroutines implemented thereby are	*/
/*	proprietary information of Metheus Corporation and may not be	*/
/*	reproduced, or disclosed or released to, or used by any other	*/
/*	person without the express written consent of Metheus 		*/
/*	Corporation.							*/
/*									*/
/*	(c) 1983 Metheus Corporation					*/
/*	All rights reserved						*/
/*									*/
/*	UNIX DR11-W DMA Interface Driver Software by Ed Mills		*/
/*									*/
/*	Version: UNIX DMA Interface Driver constants			*/
/*	Release: 1.0							*/
/*	Date: May 15, 1983						*/
/*									*/
/* ******************************************************************** */


/* These constant definitions are for use by both the driver and the user. */

/* Definitions for ioctls for OMEGA 440. */
#define	OM_SETRSTALL	0
#define	OM_SETNORSTALL	1
#define	OM_GETRSTALL	2
#define	OM_SETRTIMEOUT	3
#define	OM_GETRTIMEOUT	4
#define	OM_SETWSTALL	5
#define	OM_SETNOWSTALL	6
#define	OM_GETWSTALL	7
#define	OM_SETWTIMEOUT	8
#define	OM_GETWTIMEOUT	9
#define	OM_WRITEREADY	10
#define	OM_READREADY	11
#define	OM_BUSY		12
#define	OM_RESET	13

/* The block size for buffering and DMA transfers. */
/* OM_BLOCKSIZE must be even and <= 32768. Multiples of 512 are prefered. */
#define	OM_BLOCKSIZE	32768

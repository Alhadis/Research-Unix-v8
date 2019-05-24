/*	v7.local.h	2.6	83/08/11	*/

/*
 * Declarations and constants specific to an installation.
 *
 * Vax/Unix version 7.
 */

#define	GETHOST				/* System has gethostname syscall */
#define	UNAME				/* 
					 * Use uname() instead of gethostname()
					 * Must have GETHOST defined, though
					 */
#ifdef	GETHOST
#define	LOCAL		EMPTYID		/* Dynamically determined local host */
#else
#define	LOCAL		'I'		/* Local host id */
#endif	GETHOST

#define	MAIL		"/bin/mail"	/* Name of mail sender */
/* #define SENDMAIL	"/etc/delivermail"
					/* Name of classy mail deliverer */
#define	EDITOR		"/usr/ucb/ex"	/* Name of text editor */
#define	VISUAL		"/usr/ucb/vi"	/* Name of display editor */
#define	SHELL		"/bin/sh"	/* Standard shell */
#define	MORE		"/usr/bin/p"	/* Standard output pager */
#define	HELPFILE	"/usr/lib/Mail.help"
					/* Name of casual help file */
#define	THELPFILE	"/usr/lib/Mail.help.~"
#define	POSTAGE		"/usr/adm/maillog"
					/* Where to audit mail sending */
					/* Name of casual tilde help */
#define	UIDMASK		0177777		/* Significant uid bits */
#define	MASTER		"/usr/lib/Mail.rc"
#define	APPEND				/* New mail goes to end of mailbox */
#define CANLOCK				/* Locking protocol actually works */
#define	UTIME				/* System implements utime(2) */

#ifndef VMUNIX
#include "sigretro.h"			/* Retrofit signal defs */
#endif VMUNIX

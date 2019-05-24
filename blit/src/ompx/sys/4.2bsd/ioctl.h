From research!ulysses!smb  Sat Jan 28 18:10:52 1984
Date: Sat, 28 Jan 84 18:10:52 est
From: ulysses!smb (Steven Bellovin)
Message-Id: <8401282310.AA25068@ulysses.UUCP>
Received: by ulysses.UUCP (4.12/3.7)
	id AA25068; Sat, 28 Jan 84 18:10:52 est
To: research!rob
Subject: h/ioctl.h

/*	ioctl.h	6.1	83/07/29	*/
/*
 * Ioctl definitions
 */
#ifndef	_IOCTL_
#define	_IOCTL_
#ifdef KERNEL
#include "../h/ttychars.h"
#include "../h/ttydev.h"
#else
#include <sys/ttychars.h>
#include <sys/ttydev.h>
#endif

#include <sgtty.h>
struct tchars {
	char	t_intrc;	/* interrupt */
	char	t_quitc;	/* quit */
	char	t_startc;	/* start output */
	char	t_stopc;	/* stop output */
	char	t_eofc;		/* end-of-file */
	char	t_brkc;		/* input delimiter (like nl) */
};
struct ltchars {
	char	t_suspc;	/* stop process signal */
	char	t_dsuspc;	/* delayed stop process signal */
	char	t_rprntc;	/* reprint line */
	char	t_flushc;	/* flush output (toggles) */
	char	t_werasc;	/* word erase */
	char	t_lnextc;	/* literal next character */
};

#ifndef _IO
/*
 * Ioctl's have the command encoded in the lower word,
 * and the size of any in or out parameters in the upper
 * word.  The high 2 bits of the upper word are used
 * to encode the in/out status of the parameter; for now
 * we restrict parameters to at most 128 bytes.
 */
#define	IOCPARM_MASK	0x7f		/* parameters must be < 128 bytes */
#define	IOC_VOID	0x20000000	/* no parameters */
#define	IOC_OUT		0x40000000	/* copy out parameters */
#define	IOC_IN		0x80000000	/* copy in parameters */
#define	IOC_INOUT	(IOC_IN|IOC_OUT)
/* the 0x20000000 is so we can distinguish new ioctl's from old */
#define	_IO(x,y)	(IOC_VOID|('x'<<8)|y)
#define	_IOR(x,y,t)	(IOC_OUT|((sizeof(t)&IOCPARM_MASK)<<16)|('x'<<8)|y)
#define	_IOW(x,y,t)	(IOC_IN|((sizeof(t)&IOCPARM_MASK)<<16)|('x'<<8)|y)
/* this should be _IORW, but stdio got there first */
#define	_IOWR(x,y,t)	(IOC_INOUT|((sizeof(t)&IOCPARM_MASK)<<16)|('x'<<8)|y)
#endif

/*
 * tty ioctl commands
 */
#define	TIOCGETD	_IOR(t, 0, int)		/* get line discipline */
#define	TIOCSETD	_IOW(t, 1, int)		/* set line discipline */
#define	TIOCHPCL	_IO(t, 2)		/* hang up on last close */
#define	TIOCMODG	_IOR(t, 3, int)		/* get modem control state */
#define	TIOCMODS	_IOW(t, 4, int)		/* set modem control state */
#define		TIOCM_LE	0001		/* line enable */
#define		TIOCM_DTR	0002		/* data terminal ready */
#define		TIOCM_RTS	0004		/* request to send */
#define		TIOCM_ST	0010		/* secondary transmit */
#define		TIOCM_SR	0020		/* secondary receive */
#define		TIOCM_CTS	0040		/* clear to send */
#define		TIOCM_CAR	0100		/* carrier detect */
#define		TIOCM_CD	TIOCM_CAR
#define		TIOCM_RNG	0200		/* ring */
#define		TIOCM_RI	TIOCM_RNG
#define		TIOCM_DSR	0400		/* data set ready */
#define	TIOCGETP	_IOR(t, 8,struct sgttyb)/* get parameters -- gtty */
#define	TIOCSETP	_IOW(t, 9,struct sgttyb)/* set parameters -- stty */
#define	TIOCSETN	_IOW(t,10,struct sgttyb)/* as above, but no flushtty */
#define	TIOCEXCL	_IO(t, 13)		/* set exclusive use of tty */
#define	TIOCNXCL	_IO(t, 14)		/* reset exclusive use of tty */
#define	TIOCFLUSH	_IOW(t, 16, int)	/* flush buffers */
#define	TIOCSETC	_IOW(t,17,struct tchars)/* set special characters */
#define	TIOCGETC	_IOR(t,18,struct tchars)/* get special characters */
#define		TANDEM		0x00000001	/* send stopc on out q full */
#define		CBREAK		0x00000002	/* half-cooked mode */
#define		LCASE		0x00000004	/* simulate lower case */
#define		ECHO		0x00000008	/* echo input */
#define		CRMOD		0x00000010	/* map \r to \r\n on output */
#define		RAW		0x00000020	/* no i/o processing */
#define		ODDP		0x00000040	/* get/send odd parity */
#define		EVENP		0x00000080	/* get/send even parity */
#define		ANYP		0x000000c0	/* get any parity/send none */
#define		NLDELAY		0x00000300	/* \n delay */
#define			NL0	0x00000000
#define			NL1	0x00000100	/* tty 37 */
#define			NL2	0x00000200	/* vt05 */
#define			NL3	0x00000300
#define		TBDELAY		0x00000c00	/* horizontal tab delay */
#define			TAB0	0x00000000
#define			TAB1	0x00000400	/* tty 37 */
#define			TAB2	0x00000800
#define		XTABS		0x00000c00	/* expand tabs on output */
#define		CRDELAY		0x00003000	/* \r delay */
#define			CR0	0x00000000
#define			CR1	0x00001000	/* tn 300 */
#define			CR2	0x00002000	/* tty 37 */
#define			CR3	0x00003000	/* concept 100 */
#define		VTDELAY		0x00004000	/* vertical tab delay */
#define			FF0	0x00000000
#define			FF1	0x00004000	/* tty 37 */
#define		BSDELAY		0x00008000	/* \b delay */
#define			BS0	0x00000000
#define			BS1	0x00008000
#define 	ALLDELAY	(NLDELAY|TBDELAY|CRDELAY|VTDELAY|BSDELAY)
#define		CRTBS		0x00010000	/* do backspacing for crt */
#define		PRTERA		0x00020000	/* \ ... / erase */
#define		CRTERA		0x00040000	/* " \b " to wipe out char */
#define		TILDE		0x00080000	/* hazeltine tilde kludge */
#define		MDMBUF		0x00100000	/* start/stop output on carrier intr */
#define		LITOUT		0x00200000	/* literal output */
#define		TOSTOP		0x00400000	/* SIGSTOP on background output */
#define		FLUSHO		0x00800000	/* flush output to terminal */
#define		NOHANG		0x01000000	/* no SIGHUP on carrier drop */
#define		L001000		0x02000000
#define		CRTKIL		0x04000000	/* kill line with " \b " */
#define		L004000		0x08000000
#define		CTLECH		0x10000000	/* echo control chars as ^X */
#define		PENDIN		0x20000000	/* tp->t_rawq needs reread */
#define		DECCTQ		0x40000000	/* only ^Q starts after ^S */
#define		NOFLSH		0x80000000	/* no output flush on signal */
/* locals, from 127 down */
#define	TIOCLBIS	_IOW(t, 127, int)	/* bis local mode bits */
#define	TIOCLBIC	_IOW(t, 126, int)	/* bic local mode bits */
#define	TIOCLSET	_IOW(t, 125, int)	/* set entire local mode word */
#define	TIOCLGET	_IOR(t, 124, int)	/* get local modes */
#define		LCRTBS		(CRTBS>>16)
#define		LPRTERA		(PRTERA>>16)
#define		LCRTERA		(CRTERA>>16)
#define		LTILDE		(TILDE>>16)
#define		LMDMBUF		(MDMBUF>>16)
#define		LLITOUT		(LITOUT>>16)
#define		LTOSTOP		(TOSTOP>>16)
#define		LFLUSHO		(FLUSHO>>16)
#define		LNOHANG		(NOHANG>>16)
#define		LCRTKIL		(CRTKIL>>16)
#define		LCTLECH		(CTLECH>>16)
#define		LPENDIN		(PENDIN>>16)
#define		LDECCTQ		(DECCTQ>>16)
#define		LNOFLSH		(NOFLSH>>16)
#define	TIOCSBRK	_IO(t, 123)		/* set break bit */
#define	TIOCCBRK	_IO(t, 122)		/* clear break bit */
#define	TIOCSDTR	_IO(t, 121)		/* set data terminal ready */
#define	TIOCCDTR	_IO(t, 120)		/* clear data terminal ready */
#define	TIOCGPGRP	_IOR(t, 119, int)	/* get pgrp of tty */
#define	TIOCSPGRP	_IOW(t, 118, int)	/* set pgrp of tty */
#define	TIOCSLTC	_IOW(t,117,struct ltchars)/* set local special chars */
#define	TIOCGLTC	_IOR(t,116,struct ltchars)/* get local special chars */
#define	TIOCOUTQ	_IOR(t, 115, int)	/* output queue size */
#define	TIOCSTI		_IOW(t, 114, char)	/* simulate terminal input */
#define	TIOCNOTTY	_IO(t, 113)		/* void tty association */
#define	TIOCPKT		_IOW(t, 112, int)	/* pty: set/clear packet mode */
#define		TIOCPKT_DATA		0x00	/* data packet */
#define		TIOCPKT_FLUSHREAD	0x01	/* flush packet */
#define		TIOCPKT_FLUSHWRITE	0x02	/* flush packet */
#define		TIOCPKT_STOP		0x04	/* stop output */
#define		TIOCPKT_START		0x08	/* start output */
#define		TIOCPKT_NOSTOP		0x10	/* no more ^S, ^Q */
#define		TIOCPKT_DOSTOP		0x20	/* now do ^S ^Q */
#define	TIOCSTOP	_IO(t, 111)		/* stop output, like ^S */
#define	TIOCSTART	_IO(t, 110)		/* start output, like ^Q */
#define	TIOCMSET	_IOW(t, 109, int)	/* set all modem bits */
#define	TIOCMBIS	_IOW(t, 108, int)	/* bis modem bits */
#define	TIOCMBIC	_IOW(t, 107, int)	/* bic modem bits */
#define	TIOCMGET	_IOR(t, 106, int)	/* get all modem bits */
#define	TIOCREMOTE	_IO(t, 105)		/* remote input editing */

/* begin CCA_PAGE */
#define TIOCSSCR	_IOW(t, 64, int)		/* Set screen length */
#define TIOCGSCR	_IOR(t, 65, int)		/* Get screen length */
#define TIOCPGET	_IOR(t, 66, struct pagechars)	/* Get page chars */
#define TIOCPSET	_IOW(t, 67, struct pagechars)	/* Set page chars */
/* end CCA_PAGE */
/* begin BTL_BLIT */
#include <sys/jioctl.h>
/* end BTL_BLIT */
/* begin _USESTAT */
#define TIOCSAUXC	_IOW(t, 68, struct tauxil)	/* Set aux char str */
#define TIOCGAUXC	_IOR(t, 69, struct tauxil)	/* Get aux char str */

#define Ust_load1	0x0001	/* one-minute load average */
#define Ust_load5	0x0002	/* five-minute load average */
#define Ust_load15	0x0004	/* fifteen-minute load average */
#define Ust_uptime	0x0008	/* time since last boot */
#define Ust_pgrp	0x0010	/* pgroup leader */
#define Ust_childs	0x0020	/* and N children */
#define Ust_pcpu	0x0040	/* %cpu they're getting */
#define Ust_incore	0x0080	/* size and rss of them */
#define Ust_flags	0x0100	/* OR of all their flags */
#define Ust_state	0x0200	/* rough state (in i/o wait, etc) */
#define Ust_rawcpu	0x0400	/* raw or cooked cpu %'age */
/* end _USESTAT */

#define	OTTYDISC	0		/* old, v7 std tty driver */
#define	NETLDISC	1		/* line discip for berk net */
#define	NTTYDISC	2		/* new tty discipline */
#define	TABLDISC	3		/* hitachi tablet discipline */
#define	NTABLDISC	4		/* gtco tablet discipline */

#define	FIOCLEX		_IO(f, 1)		/* set exclusive use on fd */
#define	FIONCLEX	_IO(f, 2)		/* remove exclusive use */
/* another local */
#define	FIONREAD	_IOR(f, 127, int)	/* get # bytes to read */
#define	FIONBIO		_IOW(f, 126, int)	/* set/clear non-blocking i/o */
#define	FIOASYNC	_IOW(f, 125, int)	/* set/clear async i/o */
#define	FIOSETOWN	_IOW(f, 124, int)	/* set owner */
#define	FIOGETOWN	_IOR(f, 123, int)	/* get owner */

/* socket i/o controls */
#define	SIOCSHIWAT	_IOW(s,  0, int)		/* set high watermark */
#define	SIOCGHIWAT	_IOR(s,  1, int)		/* get high watermark */
#define	SIOCSLOWAT	_IOW(s,  2, int)		/* set low watermark */
#define	SIOCGLOWAT	_IOR(s,  3, int)		/* get low watermark */
#define	SIOCATMARK	_IOR(s,  7, int)		/* at oob mark? */
#define	SIOCSPGRP	_IOW(s,  8, int)		/* set process group */
#define	SIOCGPGRP	_IOR(s,  9, int)		/* get process group */

#define	SIOCADDRT	_IOW(r, 10, struct rtentry)	/* add route */
#define	SIOCDELRT	_IOW(r, 11, struct rtentry)	/* delete route */

#define	SIOCSIFADDR	_IOW(i, 12, struct ifreq)	/* set ifnet address */
#define	SIOCGIFADDR	_IOWR(i,13, struct ifreq)	/* get ifnet address */
#define	SIOCSIFDSTADDR	_IOW(i, 14, struct ifreq)	/* set p-p address */
#define	SIOCGIFDSTADDR	_IOWR(i,15, struct ifreq)	/* get p-p address */
#define	SIOCSIFFLAGS	_IOW(i, 16, struct ifreq)	/* set ifnet flags */
#define	SIOCGIFFLAGS	_IOWR(i,17, struct ifreq)	/* get ifnet flags */
#define	SIOCGIFCONF	_IOWR(i,20, struct ifconf)	/* get ifnet list */
#endif


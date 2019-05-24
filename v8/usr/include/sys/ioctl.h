/*
 * Structure for stty and gtty system calls.
 */

#define	_IOCTL_
struct sgttyb {
	char	sg_ispeed;		/* input speed */
	char	sg_ospeed;		/* output speed */
	char	sg_erase;		/* erase character */
	char	sg_kill;		/* kill character */
	short	sg_flags;		/* mode flags */
};

/*
 * List of special characters
 */
struct tchars {
	char	t_intrc;	/* interrupt */
	char	t_quitc;	/* quit */
	char	t_startc;	/* start output */
	char	t_stopc;	/* stop output */
	char	t_eofc;		/* end-of-file */
	char	t_brkc;		/* input delimiter (like nl) */
};

/*
 * insld,
 */
struct	insld {
	short	ld;
	short	level;
};

/*
 * for passing files across streams
 */
struct	passfd {
	int	fd;
	short	uid;
	short	gid;
	short	nice;
	short	fill;
};

/*
 * Modes
 */
#define	TANDEM	01
#define	CBREAK	02
#define	LCASE	04
#define	ECHO	010
#define	CRMOD	020
#define	RAW	040
#define	ODDP	0100
#define	EVENP	0200
#define ANYP	0300
#define	NLDELAY	001400
#define	TBDELAY	006000
#define	XTABS	06000
#define	CRDELAY	030000
#define	VTDELAY	040000
#define BSDELAY 0100000
#define ALLDELAY 0177400

/*
 * Delay algorithms
 */
#define	CR0	0
#define	CR1	010000
#define	CR2	020000
#define	CR3	030000
#define	NL0	0
#define	NL1	000400
#define	NL2	001000
#define	NL3	001400
#define	TAB0	0
#define	TAB1	002000
#define	TAB2	004000
#define	FF0	0
#define	FF1	040000
#define	BS0	0
#define	BS1	0100000

/*
 * Speeds
 */
#define B0	0
#define B50	1
#define B75	2
#define B110	3
#define B134	4
#define B150	5
#define B200	6
#define B300	7
#define B600	8
#define B1200	9
#define	B1800	10
#define B2400	11
#define B4800	12
#define B9600	13
#define EXTA	14
#define EXTB	15

/*
 * tty ioctl commands
 */
#define	TIOCGETD	(('t'<<8)|0)
#define	TIOCSETD	(('t'<<8)|1)
#define	TIOCHPCL	(('t'<<8)|2)
#define	TIOCMODG	(('t'<<8)|3)
#define	TIOCMODS	(('t'<<8)|4)
#define	TIOCGETP	(('t'<<8)|8)
#define	TIOCSETP	(('t'<<8)|9)
#define	TIOCSETN	(('t'<<8)|10)
#define	TIOCEXCL	(('t'<<8)|13)
#define	TIOCNXCL	(('t'<<8)|14)
#define	TIOHMODE	(('t'<<8)|15)
#define	TIOCFLUSH	(('t'<<8)|16)
#define	TIOCSETC	(('t'<<8)|17)
#define	TIOCGETC	(('t'<<8)|18)
#define	TIOCSBRK	(('t'<<8)|19)
#define	TIOCSIGNAL	(('t'<<8)|21)
#define	TIOCUTTY	(('t'<<8)|22)
#define	TIOCSPGRP	(('t'<<8)|118)	/* set pgrp of tty */

/*
 * file ioctls
 */
#define	FIOCLEX		(('f'<<8)|1)
#define	FIONCLEX	(('f'<<8)|2)
#define	FIOPUSHLD	(('f'<<8)|3)
#define	FIOPOPLD	(('f'<<8)|4)
#define	FIOLOOKLD	(('f'<<8)|5)
#define FIOINSLD	(('f'<<8)|6)
#define	FIOSNDFD	(('f'<<8)|7)
#define	FIORCVFD	(('f'<<8)|8)
#define	FIOACCEPT	(('f'<<8)|9)
#define	FIOREJECT	(('f'<<8)|10)
#define	FIOAISLOCK	(('f'<<8)|124)
#define	FIOALOCK	(('f'<<8)|125)
#define	FIOAUNLOCK	(('f'<<8)|126)
#define	FIONREAD	(('f'<<8)|127)

/*
 * Datakit ioctls
 */
#define	DIOCLHN		(('d'<<8)|32)	/* announce mgr channel */
#define	DIOCHUP		(('d'<<8)|33)	/* tell ctlr to reinitialize */
#define	DIOCSTREAM	(('d'<<8)|34)	/* no input delimiters */
#define	DIOCRECORD	(('d'<<8)|35)	/* input delimiters */
#define	DIOCCHAN	(('d'<<8)|38)	/* suggest channel # */
#define	DIOCSTOP	(('d'<<8)|39)	/* delay input for cmcld */
#define	DIOCSTART	(('d'<<8)|40)	/* restart input for cmcld */

#define	KIOCISURP	(('k'<<8)|1)	/* is URP already turned on? */
#define	KIOCINIT	(('k'<<8)|2)	/* force transmitter reinit */
#define	KIOCSHUT	(('k'<<8)|3)	/* shut down all chans, force reinit */


/*
 * 'ntty' ioctls
 */

/*
 * local special characters
 */
struct ltchars {
	char	t_suspc;	/* stop process signal */
	char	t_dsuspc;	/* delayed stop process signal */
	char	t_rprntc;	/* reprint line */
	char	t_flushc;	/* flush output (toggles) */
	char	t_werasc;	/* word erase */
	char	t_lnextc;	/* literal next character */
};


/*
 * local undo special characters
 */
struct luchars {
	char	t_undoc;	/* erase/kill/werase undo character */
	char	t_urotc;	/* rotate undo stack character */
};


/*
 * local mode settings
 */
#define	LCRTBS	0000001		/* correct backspacing for crt */
#define	LPRTERA 0000002		/* printing terminal \ ... / erase */
#define	LCRTERA	0000004		/* do "\b \b" to wipe out character */
#define	LTILDE	0000010		/* IIASA - hazeltine tilde kludge */
#define	LMDMBUF	0000020		/* IIASA - start/stop output on carrier intr */
#define	LLITOUT	0000040		/* IIASA - suppress any output translations */
#define	LTOSTOP	0000100		/* send stop for any background tty output */
#define	LFLUSHO	0000200		/* flush output sent to terminal */
#define	LNOHANG 0000400		/* IIASA - don't send hangup on carrier drop */
#define	LETXACK 0001000		/* IIASA - diablo style buffer hacking */
#define	LCRTKIL	0002000		/* erase whole line ala LCRTERA */
#define	LINTRUP 0004000		/* interrupt on every input char - SIGTINT */
#define	LCTLECH	0010000		/* echo control characters as ^X */
#define	LPENDIN	0020000		/* tp->t_rawq is waiting to be reread */
#define	LDECCTQ 0040000		/* only ^Q starts after ^S */

/* local state */
#define	LSBKSL	01		/* state bit for lowercase backslash work */
#define	LSQUOT	02		/* last character input was \ */
#define	LSERASE	04		/* within a \.../ for LPRTRUB */
#define	LSLNCH	010		/* next character is literal */
#define	LSTYPEN	020		/* retyping suspended input (LPENDIN) */
#define	LSCNTTB	040		/* counting width of tab; leave LFLUSHO alone */

/*
 * tty ioctl commands
 */
#define	TIOCLBIS	(('t'<<8)|127)	/* bis local mode bits */
#define	TIOCLBIC	(('t'<<8)|126)	/* bic local mode bits */
#define	TIOCLSET	(('t'<<8)|125)	/* set entire local mode word */
#define	TIOCLGET	(('t'<<8)|124)	/* get local modes */
#define	TIOCSLTC	(('t'<<8)|117)	/* set local special characters */
#define	TIOCGLTC	(('t'<<8)|116)	/* get local special characters */
#define	TIOCOUTQ	(('t'<<8)|115)	/* number of chars in output queue */

#define TIOCSLUC	(('t'<<8)|113)	/* set local undo special characters */
#define TIOCGLUC	(('t'<<8)|112)	/* get local undo special characters */
#define	TIOCGPGRP	(('t'<<8)|119)	/* get pgrp of tty */

/*
 * stream tracer ioctls
 */
#define	TRCGNAME	(('T'<<8)|6)	/* get trace module name */
#define	TRCSNAME	(('T'<<8)|7)	/* set trace module name */
#define	TRCGMASK	(('T'<<8)|8)	/* get trace module mask */
#define	TRCSMASK	(('T'<<8)|9)	/* set trace module mask */

/*
 * stream tracer mask values
 */
#define	TR_DATA		0x1
#define	TR_BREAK	0x2
#define	TR_HANGUP	0x4
#define	TR_DELIM	0x8
#define	TR_ECHO		0x10
#define	TR_ACK		0x20
#define	TR_IOCTL	0x40
#define	TR_DELAY	0x80
#define	TR_CTL		0x100
#define	TR_SIGNAL	0x200
#define	TR_FLUSH	0x400
#define	TR_STOP		0x800
#define	TR_START	0x1000
#define	TR_IOCACK	0x2000
#define	TR_IOCNAK	0x4000
#define	TR_CLOSE	0x8000

/* internet goo */
#define IPIOHOST	(('i'<<8)|1)
#define IPIONET		(('i'<<8)|2)
#define IPIOLOCAL	(('i'<<8)|3)
#define IPIOARP		(('i'<<8)|4)
#define IPIORESOLVE	(('i'<<8)|5)
#define IPIOMTU		(('i'<<8)|6)
#define IPIOROUTE	(('i'<<8)|7)
#define IPIOGETIFS	(('i'<<8)|8)

/* generic ethernet */
#define ENIOTYPE	(('e'<<8)|1)	/* set receive packet type */
#define ENIOADDR	(('e'<<8)|2)	/* fetch physical addr */
#define ENIOCMD		(('e'<<8)|3)	/* perform interface board cmd */

/* ugly tcp ioctls */
#define TCPIOHUP	(('T'<<8)|1)	/* HANGUP on TH_FIN */
#define	TCPIOMAXSEG	(('T'<<8)|2)

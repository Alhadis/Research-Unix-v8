/*
 * stuff to be added to ioctl.h
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

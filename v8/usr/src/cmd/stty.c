#include <stdio.h>
#include <sgtty.h>

#define	CTRL(x)		('x'&037)


extern int	errno, ntty_ld, tty_ld;
int ttyfd;


struct {
	char	*s_name;
	int	s_define;
	int	s_speed;
}
	speeds[] = {
		"0",	B0,	0,
		"50",	B50,	50,
		"75",	B75,	75,
		"110",	B110,	110,
		"134",	B134,	134,
		"150",	B150,	150,
		"200",	B200,	200,
		"300",	B300,	300,
		"600",	B600,	600,
		"1200",	B1200,	1200,
		"1800",	B1800,	1800,
		"2400",	B2400,	2400,
		"4800",	B4800,	4800,
		"9600",	B9600,	9600,
		"exta",	EXTA,	19200,
		"extb",	EXTB,	0,
		"19200",	EXTA,	19200,		/* must follow extb */
		0,
	};


struct {
	char	*string;
	int	set;
	int	reset;
	int	ntty;
}
	modes[] = {
		{ "even",	EVENP,	0 },
		{ "-even",	0,	EVENP },	
		{ "odd",	ODDP,	0 },
		{ "-odd",	0,	ODDP },	
		{ "raw",	RAW,	0 },
		{ "-raw",	0,	RAW },	
		{ "cooked",	0,	RAW },	
		{ "-nl",	CRMOD,	0 },
		{ "nl",		0,	CRMOD },	
		{ "echo",	ECHO,	0 },
		{ "-echo",	0,	ECHO },	
		{ "LCASE",	LCASE,	0 },
		{ "lcase",	LCASE,	0 },
		{ "-LCASE",	0,	LCASE },	
		{ "-lcase",	0,	LCASE },	
		{ "-tabs",	XTABS,	0 },
		{ "tabs",	0,	XTABS },	
		{ "tandem",	TANDEM,	0 },
		{ "-tandem",	0,	TANDEM },	
		{ "cbreak",	CBREAK,	0 },
		{ "-cbreak",	0,	CBREAK },	
		{ "cr0",	CR0,	CR3 },	
		{ "cr1",	CR1,	CR3 },	
		{ "cr2",	CR2,	CR3 },	
		{ "cr3",	CR3,	CR3 },	
		{ "tab0",	TAB0,	XTABS },	
		{ "tab1",	TAB1,	XTABS },	
		{ "tab2",	TAB2,	XTABS },	
		{ "nl0",	NL0,	NL3 },	
		{ "nl1",	NL1,	NL3 },	
		{ "nl2",	NL2,	NL3 },	
		{ "nl3",	NL3,	NL3 },	
		{ "ff0",	FF0,	FF1 },	
		{ "ff1",	FF1,	FF1 },	
		{ "bs0",	BS0,	BS1 },	
		{ "bs1",	BS1,	BS1 },	
		{ "33",		CR1,	ALLDELAY },	
		{ "tty33",	CR1,	ALLDELAY },	
		{ "37",		FF1+CR2+TAB1+NL1,	ALLDELAY },	
		{ "tty37",	FF1+CR2+TAB1+NL1,	ALLDELAY },	
		{ "05",		NL2,	ALLDELAY },	
		{ "vt05",	NL2,	ALLDELAY },	
		{ "tn",		CR1,	ALLDELAY },	
		{ "tn300",	CR1,	ALLDELAY },	
		{ "ti",		CR2,	ALLDELAY },	
		{ "ti700",	CR2,	ALLDELAY },	
		{ "tek",	FF1,	ALLDELAY },	

		{ "crtbs",	LCRTBS,		LPRTERA,	1 },
		{ "-crtbs",	0,		LCRTBS, 	1 },
		{ "prterase",	LPRTERA,	LCRTBS+LCRTKIL+LCRTERA,	1 },
		{ "-prterase",	0,		LPRTERA,	1 },
		{ "crterase",	LCRTERA,	LPRTERA,	1 },
		{ "-crterase",	0,		LCRTERA,	1 },
		{ "crtkill",	LCRTKIL,	LPRTERA,	1 },
		{ "-crtkill",	0,		LCRTKIL,	1 },
		{ "ctlecho",	LCTLECH,	0,		1 },
		{ "-ctlecho",	0,		LCTLECH,	1 },
		{ "tilde",	LTILDE,		0,		1 },
		{ "-tilde",	0,		LTILDE,		1 },
		{ "litout",	LLITOUT,	0,		1 },
		{ "-litout",	0,		LLITOUT,	1 },
		{ "flusho",	LFLUSHO,	0,		1 },
		{ "-flusho",	0,		LFLUSHO,	1 },
		{ "nohang",	LNOHANG,	0,		1 },
		{ "-nohang",	0,		LNOHANG,	1 },
		{ "decctq",	LDECCTQ,	0,		1 },
		{ "-decctq",	0,		LDECCTQ,	1 },
/*
		{ "pendin",	LPENDIN,	0,		1 },
		{ "-pendin",	0,		LPENDIN,	1 },
		{ "mdmbuf",	LMDMBUF,	0,		1 },
		{ "-mdmbuf",	0,		LMDMBUF,	1 },
		{ "tostop",	LTOSTOP,	0,		1 },
		{ "-tostop",	0,		LTOSTOP,	1 },
		{ "etxack",	LETXACK,	0,		1 },
		{ "-etxack",	0,		LETXACK,	1 },
		{ "intrup",	LINTRUP,	0,		1 },
		{ "-intrup",	0,		LINTRUP,	1 },
*/
		0,
	};


struct sgttyb	mode;
struct tchars	tc;
struct ltchars	ltc;
struct luchars	luc;

struct	special {
	char	*name;
	char	*cp;
	char	def;
}
	special[] = {
		"erase",	&mode.sg_erase,		CTRL(h),
		"kill",		&mode.sg_kill,		'@',

		"intr",		&tc.t_intrc,		0177,
		"quit",		&tc.t_quitc,		CTRL(\\),
		"start",	&tc.t_startc,		CTRL(q),
		"stop",		&tc.t_stopc,		CTRL(s),
		"eof",		&tc.t_eofc,		CTRL(d),
		"brk",		&tc.t_brkc,		0377,

		"susp",		&ltc.t_suspc,		CTRL(z),
		"dsusp",	&ltc.t_dsuspc,		CTRL(y),
		"rprnt",	&ltc.t_rprntc,		CTRL(r),
		"flush",	&ltc.t_flushc,		CTRL(o),
		"werase",	&ltc.t_werasc,		CTRL(w),
		"lnext",	&ltc.t_lnextc,		CTRL(v),

		"undo",		&luc.t_undoc,		0377,
		"urot",		&luc.t_urotc,		0377,
		0
	};


char	*ego;
int	ldisc;		/* current line disc */
int	lmode;		/* new tty local mode bits */


main (argc, argv)
	int	argc;
	char	*argv[];
{
	char	*arg, obuf[BUFSIZ];
	int 	i;
	struct special *sp;

	ego = argv[0];
	setbuf (stderr, obuf);
	if ((ttyfd = open("/dev/tty", 1)) < 0) {
		fprintf(stderr, "%s: can't open /dev/tty\n", ego);
		exit(1);
	}
	(void) getmodes();
/*
	if (argc>=2 && ldisc!=tty_ld && ldisc!=ntty_ld)
		fprintf(stderr, "%s: warning: no tty line discipline.\n", ego);
*/
	if (argc < 2) {
		prmodes();
		exit (0);
	}

	for (arg = *++argv; --argc > 0; arg = *++argv) {

#define eq(x)	(!strcmp (arg, x))

		if (eq ("all")) {
			prmodes();
			continue;
		}

		if (eq ("old") || eq ("-new"))
			if (swdisc (tty_ld))
				goto done1;
			else	continue;

		if (eq ("new"))
			if (swdisc (ntty_ld))
				goto done1;
			else	continue;

		if (eq ("crt")) {
			lmode |= LCRTBS | LCTLECH;
			if (speeds[mode.sg_ispeed].s_speed >= 1200)
				lmode |= LCRTERA | LCRTKIL;
			continue;
		}

		if (eq ("ek")) {
			mode.sg_erase = '#';
			mode.sg_kill = '@';
			continue;
		}

		if (eq ("hup")) {
			(void) ioctl (ttyfd, TIOCHPCL, NULL);
			continue;
		}

		for (sp = special; sp->name; sp++)
			if (eq (sp->name)) {
				if (--argc > 0) {
					arg = *++argv;
					if (*arg == 'u')
						*sp->cp = 0377;
					else if (*arg == '^')
						*sp->cp = (arg[1] == '?') ?
						    0177 : arg[1] & 037;
					else	*sp->cp = *arg;
					goto cont;
				}
				fprintf(stderr, "%s: missing %s character\n",
				    ego, arg);
				goto done1;
			}

		for (i = 0; speeds[i].s_name; i++)
			if (eq (speeds[i].s_name)) {
				mode.sg_ispeed = mode.sg_ospeed = 
				    speeds[i].s_define;
				goto cont;
			}

		if (eq ("gspeed")) {
			mode.sg_ispeed = B300;
			mode.sg_ospeed = B9600;
			continue;
		}

		for (i = 0; modes[i].string; i++)
			if (eq (modes[i].string)) {
				if (modes[i].ntty) {
					lmode &= ~modes[i].reset;
					lmode |= modes[i].set;
				}
				else {
					mode.sg_flags &= ~modes[i].reset;
					mode.sg_flags |= modes[i].set;
				}
				goto cont;
			}

		fprintf(stderr, "%s: %s: unknown mode\n", ego, arg);
done1:
		setmodes();
		exit (1);
cont:
		;
	}

	setmodes();
	exit (0);
}


getmodes()
{
	int ret;
	ldisc = ioctl (ttyfd, FIOLOOKLD, 0);
	ret = ioctl (ttyfd, TIOCGETP, &mode) == -1;
	(void) ioctl (ttyfd, TIOCGETC, &tc);
	if (ldisc == ntty_ld) {
		(void) ioctl (ttyfd, TIOCLGET, &lmode);
		(void) ioctl (ttyfd, TIOCGLTC, &ltc);
		(void) ioctl (ttyfd, TIOCGLUC, &luc);
	}
	return ret;
}


pit (what, itsname, sep)
	unsigned char	what;
	char 	*itsname, *sep;
{
	printf("%s", itsname);
	if (what == 0377) {
		printf(" <undef>%s", sep);
		return;
	}
	printf(" = ");
	if (what & 0200) {
		printf("M-");
		what &= ~ 0200;
	}
	if (what == 0177) {
		printf("^");
		what = '?';
	}
	else if (what < ' ') {
		printf("^");
		what += '@';
	}
	printf("%c%s", what, sep);
	return;
}


prmodes()
{
	register int	m;

	if (mode.sg_ispeed != mode.sg_ospeed)
		printf("input speed: %d baud, output speed: %d baud\n",
		    speeds[mode.sg_ispeed].s_speed,
		    speeds[mode.sg_ospeed].s_speed);
	else
		printf("speed: %d baud\n",
		       speeds[mode.sg_ispeed].s_speed);
	pit (mode.sg_erase, "erase", "; ");
	pit (mode.sg_kill, "kill", "; ");
	pit (tc.t_intrc, "intr", "; ");
	pit (tc.t_quitc, "quit", "\n");
	pit (tc.t_startc, "start", "; ");
	pit (tc.t_stopc, "stop", "; ");
	pit (tc.t_eofc, "eof", "; ");
	pit (tc.t_brkc, "brk", "\n");

	if (ldisc == ntty_ld) {
		pit (ltc.t_werasc, "werase", "; ");
		pit (ltc.t_rprntc, "rprnt", "; ");
		pit (ltc.t_flushc, "flush", "; ");
		pit (ltc.t_lnextc, "lnext", "\n");
		pit (ltc.t_suspc, "susp", "; ");
		pit (ltc.t_dsuspc, "dsusp", "; ");
		pit (luc.t_undoc, "undo", "; ");
		pit (luc.t_urotc, "urot", "\n");
	}
	if (ldisc == tty_ld)
		printf("old ");
	else if (ldisc == ntty_ld)
		printf("new ");
	m = mode.sg_flags;

	if (m & EVENP)
		printf("even ");
	if (m & ODDP)
		printf("odd ");

#define	mpit(what,str)	printf(str + ((m & what) != 0))

	mpit (RAW, "-raw ");
	printf("-nl " + ((m & CRMOD) == 0));
	mpit (ECHO, "-echo ");
	mpit (LCASE, "-lcase ");
	printf("-tabs " + ((m & XTABS) != XTABS));
	mpit (CBREAK, "-cbreak ");
	mpit (TANDEM, "-tandem ");

#define delay(x,y)	printf("%s%d ", y, x)

	delay ((m & NLDELAY) / NL1, "nl");
	if ((m & TBDELAY) != XTABS)
		delay ((m & TBDELAY) / TAB1, "tab");
	delay ((m & CRDELAY) / CR1, "cr");
	delay ((m & VTDELAY) / FF1, "ff");
	delay ((m & BSDELAY) / BS1, "bs");
	printf("\n");

	if (ldisc == ntty_ld) {
		m = lmode;
		mpit (LCRTBS, "-crtbs ");
		mpit (LCRTERA, "-crterase ");
		mpit (LCRTKIL, "-crtkill ");
		mpit (LCTLECH, "-ctlecho ");
		mpit (LPRTERA, "-prterase ");
		printf("\n");

		mpit (LTILDE, "-tilde ");
		mpit (LFLUSHO, "-flusho ");
		mpit (LLITOUT, "-litout ");
		mpit (LNOHANG, "-nohang ");
		mpit (LDECCTQ, "-decctq ");
		printf("\n");
/*
		mpit (LMDMBUF, "-mdmbuf ");
		mpit (LTOSTOP, "-tostop ");
		mpit (LINTRUP, "-intrup ");
		mpit (LETXACK, "-etxack ");
		mpit (LPENDIN, "-pendin ");
		printf("\n");
*/
	}
	return;
}


setmodes()
{
	(void) ioctl (ttyfd, TIOCSETN, &mode);
	(void) ioctl (ttyfd, TIOCSETC, &tc);
	if (ldisc == ntty_ld) {
		(void) ioctl (ttyfd, TIOCSLTC, &ltc);
		(void) ioctl (ttyfd, TIOCSLUC, &luc);
		(void) ioctl (ttyfd, TIOCLSET, &lmode);
	}
	return;
}


int
swdisc (newld)
	int	newld;
{
	int curld = ioctl(ttyfd, FIOLOOKLD, 0);

	if ((curld==tty_ld || curld==ntty_ld) && 0 <= ioctl (ttyfd, FIOPOPLD, 0)) {
		if (0 <= ioctl (ttyfd, FIOPUSHLD, &newld)) {
			(void) ioctl (ttyfd, TIOCSETN, &mode);
			(void) ioctl (ttyfd, TIOCSETC, &tc);
			getmodes();
			return (0);
		}
		else {
			(void) ioctl (ttyfd, FIOPUSHLD, &ldisc);
			setmodes();
		}
	}
	fprintf(stderr, "%s: can't switch line disciplines\n", ego);
	return (-1);
}

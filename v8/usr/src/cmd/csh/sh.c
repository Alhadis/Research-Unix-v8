static	char *sccsid = "@(#)sh.c 4.2 3/11/81";

#include "sh.h"
#include <sys/ioctl.h>
/*
 * C Shell
 *
 * Bill Joy, UC Berkeley, California, USA
 * October 1978, May 1980
 *
 * Jim Kulp, IIASA, Laxenburg, Austria
 * April 1980
 */

extern int	ntty_ld, tty_ld;

char	*pathlist[] =	{ ".", "/usr/ucb", "/bin", "/usr/bin", 0 };
char	HIST = '!';
char	HISTSUB = '^';
bool	nofile;
bool	reenter;
bool	nverbose;
bool	nexececho;
bool	quitit;
bool	fast;
bool	prompt = 1;

main(c, av)
	int c;
	char **av;
{
	register char **v, *cp;
	register int f;

	settimes();			/* Immed. estab. timing base */
	v = av;
	if (eq(v[0], "a.out"))		/* A.out's are quittable */
		quitit = 1;
	uid = getuid();
	loginsh = **v == '-';
	if (loginsh)
		time(&chktim);

	/*
	 * Move the descriptors to safe places.
	 * The variable didfds is 0 while we have only FSH* to work with.
	 * When didfds is true, we have 0,1,2 and prefer to use these.
	 */
	initdesc();

	/*
	 * Initialize the shell variables.
	 * ARGV and PROMPT are initialized later.
	 * STATUS is also munged in several places.
	 * CHILD is munged when forking/waiting
	 */

	set("status", "0");
	dinit(cp = getenv("HOME"));	/* dinit thinks that HOME == cwd in a
					 * login shell */
	if (cp == NOSTR)
		fast++;			/* No home -> can't read scripts */
	else
		set("home", savestr(cp));
	/*
	 * Grab other useful things from the environment.
	 * Should we grab everything??
	 */
	if ((cp = getenv("USER")) != NOSTR)
		set("user", savestr(cp));
	if ((cp = getenv("TERM")) != NOSTR)
		set("term", savestr(cp));
	/*
	 * Re-initialize path if set in environment
	 */
	if ((cp = getenv("PATH")) == NOSTR)
		set1("path", saveblk(pathlist), &shvhed);
	else {
		register unsigned i = 0;
		register char *dp;
		register char **pv;

		for (dp = cp; *dp; dp++)
			if (*dp == ':')
				i++;
		pv = (char **)calloc(i+2, sizeof (char **));
		for (dp = cp, i = 0; ;)
			if (*dp == ':') {
				*dp = 0;
				pv[i++] = savestr(*cp ? cp : ".");
				*dp++ = ':';
				cp = dp;
			} else if (*dp++ == 0) {
				pv[i++] = savestr(*cp ? cp : ".");
				break;
			}
		pv[i] = 0;
		set1("path", pv, &shvhed);
	}
	set("shell", SHELLPATH);

	doldol = putn(getpid());		/* For $$ */
	shtemp = strspl("/tmp/sh", doldol);	/* For << */

	/*
	 * Record the interrupt states from the parent process.
	 * If the parent is non-interruptible our hand must be forced
	 * or we (and our children) won't be either.
	 * Our children inherit termination from our parent.
	 * We catch it only if we are the login shell.
	 */
	parintr = signal(SIGINT, SIG_IGN);	/* parents interruptibility */
	sigset(SIGINT, parintr);			/* ... restore */
	parterm = signal(SIGTERM, SIG_IGN);	/* parents terminability */
	signal(SIGTERM, parterm);			/* ... restore */

	/*
	 * Process the arguments.
	 *
	 * Note that processing of -v/-x is actually delayed till after
	 * script processing.
	 *
	 * We set the first character of our name to be '-' if we are
	 * a shell running interruptible commands.  Many programs which
	 * examine ps'es use this to filter such shells out.
	 */
	c--, v++;
	while (c > 0 && (cp = v[0])[0] == '-') {
		do switch (*cp++) {

		case 0:			/* -	Interruptible, no prompt */
			prompt = 0;
			setintr++;
			nofile++;
			break;

		case 'c':		/* -c	Command input from arg */
			if (c == 1)
				exit(0);
			c--, v++;
			arginp = v[0];
			prompt = 0;
			nofile++;
			break;

		case 'e':		/* -e	Exit on any error */
			exiterr++;
			break;

		case 'f':		/* -f	Fast start */
			fast++;
			break;

		case 'i':		/* -i	Interactive, even if !intty */
			intact++;
			nofile++;
			break;

		case 'n':		/* -n	Don't execute */
			noexec++;
			break;

		case 'q':		/* -q	(Undoc'd) ... die on quit */
			quitit = 1;
			break;

		case 's':		/* -s	Read from std input */
			nofile++;
			break;

		case 't':		/* -t	Read one line from input */
			onelflg = 2;
			prompt = 0;
			nofile++;
			break;

		case 'v':		/* -v	Echo hist expanded input */
			nverbose = 1;			/* ... later */
			break;

		case 'x':		/* -x	Echo just before execution */
			nexececho = 1;			/* ... later */
			break;

		case 'V':		/* -V	Echo hist expanded input */
			setNS("verbose");		/* NOW! */
			break;

		case 'X':		/* -X	Echo just before execution */
			setNS("echo");			/* NOW! */
			break;

		} while (*cp);
		v++, c--;
	}

	if (quitit)			/* With all due haste, for debugging */
		signal(SIGQUIT, SIG_DFL);

	/*
	 * Unless prevented by -, -c, -i, -s, or -t, if there
	 * are remaining arguments the first of them is the name
	 * of a shell file from which to read commands.
	 */
	if (nofile == 0 && c > 0) {
		nofile = open(v[0], 0);
		if (nofile < 0) {
			child++;		/* So this ... */
			Perror(v[0]);		/* ... doesn't return */
		}
		file = v[0];
		SHIN = dmove(nofile, FSHIN);	/* Replace FSHIN */
		prompt = 0;
		c--, v++;
	}
	/*
	 * Consider input a tty if it really is or we are interactive.
	 */
	intty = intact || isatty(SHIN);
	/*
	 * Decide whether we should play with signals or not.
	 * If we are explicitly told (via -i, or -) or we are a login
	 * shell (arg0 starts with -) or the input and output are both
	 * the ttys("csh", or "csh</dev/ttyx>/dev/ttyx")
	 * Note that in only the login shell is it likely that parent
	 * may have set signals to be ignored
	 */
	if (loginsh || intact || intty && isatty(SHOUT))
		setintr = 1;
#ifdef TELL
	settell();
#endif
	/*
	 * Save the remaining arguments in argv.
	 */
	setq("argv", v, &shvhed);

	/*
	 * Set up the prompt.
	 */
	if (prompt)
		set("prompt", uid == 0 ? "# " : "% ");

	/*
	 * If we are an interactive shell, then start fiddling
	 * with the signals; this is a tricky game.
	 */
	shpgrp = getpgrp(0);
	opgrp = tpgrp = -1;
	oldisc = -1;
	if (setintr) {
		**av = '-';
		if (!quitit)		/* Wary! */
			signal(SIGQUIT, SIG_IGN);
		sigset(SIGINT, pintr);
		sighold(SIGINT);
		signal(SIGTERM, SIG_IGN);
		if (quitit == 0 && arginp == 0) {
			signal(SIGTSTP, SIG_IGN);
			signal(SIGTTIN, SIG_IGN);
			signal(SIGTTOU, SIG_IGN);
			/*
			 * Wait till in foreground, in case someone
			 * stupidly runs
			 *	csh &
			 * dont want to try to grab away the tty.
			 */
			if (isatty(FSHDIAG))
				f = FSHDIAG;
			else if (isatty(FSHOUT))
				f = FSHOUT;
			else if (isatty(OLDSTD))
				f = OLDSTD;
			else
				f = -1;
retry:
			if (ioctl(f, TIOCGPGRP, &tpgrp) == 0 && tpgrp != -1) {
				int ldisc;
				if (tpgrp != shpgrp) {
					int old = sigsys(SIGTTIN, SIG_DFL);
					kill(0, SIGTTIN);
					sigsys(SIGTTIN, old);
					goto retry;
				}
				if (0 > (oldisc = ioctl (f, FIOLOOKLD, 0)))
					goto notty;
				if (oldisc == tty_ld) {
					ldisc = ntty_ld;
					if (0 > ioctl (f, FIOPOPLD, 0))
						goto notty;
					else if (0 > 
					    ioctl (f, FIOPUSHLD, &ldisc, 0)) {
						ioctl(f, FIOPUSHLD, &oldisc, 0);
						goto notty;
					}
					printf (
					    "Switching to new tty driver...\n");
				} else {
					int xx = 0;
					/* test for newtty */
					if (ioctl(f, TIOCLBIS, &xx) > 0)
						oldisc = ntty_ld;
					else
						oldisc = -1;
				}
				opgrp = shpgrp;
				shpgrp = getpid();
				tpgrp = shpgrp;
				ioctl(f, TIOCSPGRP, &shpgrp);
				setpgrp(0, shpgrp);
				dcopy(f, FSHTTY);
				ioctl(FSHTTY, FIOCLEX, 0);
			} else {
notty:
  printf("Warning: no access to tty; thus no job control in this shell...\n");
				tpgrp = -1;
			}
		}
	}
	sigset(SIGCHLD, pchild);		/* while signals not ready */

	/*
	 * Set an exit here in case of an interrupt or error reading
	 * the shell start-up scripts.
	 */
	setexit();
	haderr = 0;		/* In case second time through */
	if (!fast && reenter == 0) {
		reenter++;
		/* Will have value("home") here because set fast if don't */
		srccat(value("home"), "/.cshrc");
		if (!fast && !arginp && !onelflg)
			dohash();
		if (loginsh) {
			int ldisc;
			srccat(value("home"), "/.login");
		}
	}

	/*
	 * Now are ready for the -v and -x flags
	 */
	if (nverbose)
		setNS("verbose");
	if (nexececho)
		setNS("echo");

	/*
	 * All the rest of the world is inside this call.
	 * The argument to process indicates whether it should
	 * catch "error unwinds".  Thus if we are a interactive shell
	 * our call here will never return by being blown past on an error.
	 */
	process(setintr);

	/*
	 * Mop-up.
	 */
	if (loginsh) {
		printf("logout\n");
		close(SHIN);
		child++;
		goodbye();
	}
	exitstat();
}

untty()
{

	if (tpgrp > 0) {
		setpgrp(0, opgrp);
		ioctl(FSHTTY, TIOCSPGRP, &opgrp);
		if (oldisc != -1 && oldisc != ntty_ld) {
			printf("\nReverting to old tty driver...\n");
			ioctl (FSHTTY, FIOPOPLD, 0);
			ioctl (FSHTTY, FIOPUSHLD, &oldisc);
		}
	}
}

importpath(cp)
char *cp;
{
	register int i = 0;
	register char *dp;
	register char **pv;
	int c;
	static char dot[2] = {'.', 0};

	for (dp = cp; *dp; dp++)
		if (*dp == ':')
			i++;
	/*
	 * i+2 where i is the number of colons in the path.
	 * There are i+1 directories in the path plus we need
	 * room for a zero terminator.
	 */
	pv = (char **) calloc(i+2, sizeof (char **));
	dp = cp;
	i = 0;
	if (*dp)
	for (;;) {
		if ((c = *dp) == ':' || c == 0) {
			*dp = 0;
			pv[i++] = savestr(*cp ? cp : dot);
			if (c) {
				cp = dp + 1;
				*dp = ':';
			} else
				break;
		}
		dp++;
	}
	pv[i] = 0;
	set1("path", pv, &shvhed);
}

/*
 * Source to the file which is the catenation of the argument names.
 */
srccat(cp, dp)
	char *cp, *dp;
{
	register char *ep = strspl(cp, dp);
	register int unit = dmove(open(ep, 0), -1);

	/* ioctl(unit, FIOCLEX, NULL); */
	xfree(ep);
#ifdef INGRES
	srcunit(unit, 0);
#else
	srcunit(unit, 1);
#endif
}

/*
 * Source to a unit.  If onlyown it must be our file or our group or
 * we don't chance it.	This occurs on ".cshrc"s and the like.
 */
srcunit(unit, onlyown)
	register int unit;
	bool onlyown;
{
	/* We have to push down a lot of state here */
	/* All this could go into a structure */
	int oSHIN = -1, oldintty = intty;
	struct whyle *oldwhyl = whyles;
	char *ogointr = gointr, *oarginp = arginp;
	char *oevalp = evalp, **oevalvec = evalvec;
	int oonelflg = onelflg;
#ifdef TELL
	bool otell = cantell;
#endif
	struct Bin saveB;

	/* The (few) real local variables */
	jmp_buf oldexit;
	int reenter;

	if (unit < 0)
		return;
	if (didfds)
		donefds();
	if (onlyown) {
		struct stat stb;

		if (fstat(unit, &stb) < 0 || (stb.st_uid != uid && stb.st_gid != getgid())) {
			close(unit);
			return;
		}
	}

	/*
	 * There is a critical section here while we are pushing down the
	 * input stream since we have stuff in different structures.
	 * If we weren't careful an interrupt could corrupt SHIN's Bin
	 * structure and kill the shell.
	 *
	 * We could avoid the critical region by grouping all the stuff
	 * in a single structure and pointing at it to move it all at
	 * once.  This is less efficient globally on many variable references
	 * however.
	 */
	getexit(oldexit);
	reenter = 0;
	if (setintr)
		sighold(SIGINT);
	setexit();
	reenter++;
	if (reenter == 1) {
		/* Setup the new values of the state stuff saved above */
		copy((char *)&saveB, (char *)&B, sizeof saveB);
		fbuf = (char **) 0;
		fseekp = feobp = fblocks = 0;
		oSHIN = SHIN, SHIN = unit, arginp = 0, onelflg = 0;
		intty = isatty(SHIN), whyles = 0, gointr = 0;
		evalvec = 0; evalp = 0;
		/*
		 * Now if we are allowing commands to be interrupted,
		 * we let ourselves be interrupted.
		 */
		if (setintr)
			sigrelse(SIGINT);
#ifdef TELL
		settell();
#endif
		process(0);		/* 0 -> blow away on errors */
	}
	if (setintr)
		sigrelse(SIGINT);
	if (oSHIN >= 0) {
		register int i;

		/* We made it to the new state... free up its storage */
		/* This code could get run twice but xfree doesn't care */
		for (i = 0; i < fblocks; i++)
			xfree(fbuf[i]);
		xfree((char *)fbuf);

		/* Reset input arena */
		copy((char *)&B, (char *)&saveB, sizeof B);

		close(SHIN), SHIN = oSHIN;
		arginp = oarginp, onelflg = oonelflg;
		evalp = oevalp, evalvec = oevalvec;
		intty = oldintty, whyles = oldwhyl, gointr = ogointr;
#ifdef TELL
		cantell = otell;
#endif
	}

	resexit(oldexit);
	/*
	 * If process reset() (effectively an unwind) then
	 * we must also unwind.
	 */
	if (reenter >= 2)
		error(NOSTR);
}

goodbye()
{

	if (loginsh) {
		signal(SIGQUIT, SIG_IGN);
		sigset(SIGINT, SIG_IGN);
		signal(SIGTERM, SIG_IGN);
		setintr = 0;		/* No interrupts after "logout" */
		if (adrof("home"))
			srccat(value("home"), "/.logout");
	}
	exitstat();
}

exitstat()
{

	/*
	 * Note that if STATUS is corrupted (i.e. getn bombs)
	 * then error will exit directly because we poke child here.
	 * Otherwise we might continue unwarrantedly (sic).
	 */
	child++;
	exit(getn(value("status")));
}

char	*jobargv[2] = { "jobs", 0 };
/*
 * Catch an interrupt, e.g. during lexical input.
 * If we are an interactive shell, we reset the interrupt catch
 * immediately.  In any case we drain the shell output,
 * and finally go through the normal error mechanism, which
 * gets a chance to make the shell go away.
 */
pintr()
{
	pintr1(1);
}

pintr1(wantnl)
	bool wantnl;
{
	register char **v;

	if (setintr) {
		sigrelse(SIGINT);
		if (pjobs) {
			pjobs = 0;
			printf("\n");
			dojobs(jobargv);
			bferr("Interrupted");
		}
	}
	if (setintr)
		sighold(SIGINT);
	sigrelse(SIGCHLD);
	draino();

	/*
	 * If we have an active "onintr" then we search for the label.
	 * Note that if one does "onintr -" then we shan't be interruptible
	 * so we needn't worry about that here.
	 */
	if (gointr) {
		search(ZGOTO, 0, gointr);
		timflg = 0;
		if (v = pargv)
			pargv = 0, blkfree(v);
		if (v = gargv)
			gargv = 0, blkfree(v);
		reset();
	} else if (intty && wantnl)
		printf("\n");		/* Some like this, others don't */
	error(NOSTR);
}

/*
 * Process is the main driving routine for the shell.
 * It runs all command processing, except for those within { ... }
 * in expressions (which is run by a routine evalav in sh.exp.c which
 * is a stripped down process), and `...` evaluation which is run
 * also by a subset of this code in sh.glob.c in the routine backeval.
 *
 * The code here is a little strange because part of it is interruptible
 * and hence freeing of structures appears to occur when none is necessary
 * if this is ignored.
 *
 * Note that if catch is not set then we will unwind on any error.
 * If an end-of-file occurs, we return.
 */
process(catch)
	bool catch;
{
	register char *cp;
	jmp_buf osetexit;
	struct command *t;

	getexit(osetexit);
	for (;;) {
		pendjob();
		paraml.next = paraml.prev = &paraml;
		paraml.word = "";
		t = 0;
		setexit();
		justpr = 0;			/* A chance to execute */

		/*
		 * Interruptible during interactive reads
		 */
		if (setintr)
			sigrelse(SIGINT);

		/*
		 * For the sake of reset()
		 */
		freelex(&paraml), freesyn(t), t = 0;

		if (haderr) {
			if (!catch) {
				/* unwind */
				doneinp = 0;
				resexit(osetexit);
				reset();
			}
			haderr = 0;
			/*
			 * Every error is eventually caught here or
			 * the shell dies.  It is at this
			 * point that we clean up any left-over open
			 * files, by closing all but a fixed number
			 * of pre-defined files.  Thus routines don't
			 * have to worry about leaving files open due
			 * to deeper errors... they will get closed here.
			 */
			closem();
			continue;
		}
		if (doneinp) {
			doneinp = 0;
			break;
		}
		if (chkstop)
			chkstop--;
		if (neednote)
			pnote();
		if (intty && evalvec == 0) {
			mailchk();
			/*
			 * If we are at the end of the input buffer
			 * then we are going to read fresh stuff.
			 * Otherwise, we are rereading input and don't
			 * need or want to prompt.
			 */
			if (fseekp == feobp)
				if (!whyles)
					for (cp = value("prompt"); *cp; cp++)
						if (*cp == HIST)
							printf("%d", eventno + 1);
						else {
							if (*cp == '\\' && cp[1] == HIST)
								cp++;
							putchar(*cp | QUOTE);
						}
				else
					/*
					 * Prompt for forward reading loop
					 * body content.
					 */
					printf("? ");
			flush();
		}
		err = 0;

		/*
		 * Echo not only on VERBOSE, but also with history expansion.
		 * If there is a lexical error then we forego history echo.
		 */
		if (lex(&paraml) && !err && intty || adrof("verbose")) {
			haderr = 1;
			prlex(&paraml);
			haderr = 0;
		}

		/*
		 * The parser may lose space if interrupted.
		 */
		if (setintr)
			sighold(SIGINT);

		/*
		 * Save input text on the history list if it
		 * is from the terminal at the top level and not
		 * in a loop.
		 */
		if (catch && intty && !whyles)
			savehist(&paraml);

		/*
		 * Print lexical error messages.
		 */
		if (err)
			error(err);

		/*
		 * If had a history command :p modifier then
		 * this is as far as we should go
		 */
		if (justpr)
			reset();

		alias(&paraml);

		/*
		 * Parse the words of the input into a parse tree.
		 */
		t = syntax(paraml.next, &paraml, 0);
		if (err)
			error(err);

		/*
		 * Execute the parse tree
		 */
		execute(t, tpgrp);

		/*
		 * Made it!
		 */
		freelex(&paraml), freesyn(t);
	}
	resexit(osetexit);
}

dosource(t)
	register char **t;
{
	register char *f;
	register int u;

	t++;
	f = globone(*t);
	u = dmove(open(f, 0), -1);
	xfree(f);
	if (u < 0)
		Perror(f);
	srcunit(u, 0);
}

/*
 * Check for mail.
 * If we are a login shell, then we don't want to tell
 * about any mail file unless its been modified
 * after the time we started.
 * This prevents us from telling the user things he already
 * knows, since the login program insists on saying
 * "You have mail."
 */
mailchk()
{
	register struct varent *v;
	register char **vp;
	time_t t;
	int intvl, cnt;
	struct stat stb;
	bool new;

	v = adrof("mail");
	if (v == 0)
		return;
	time(&t);
	vp = v->vec;
	cnt = blklen(vp);
	intvl = (cnt && number(*vp)) ? (--cnt, getn(*vp++)) : MAILINTVL;
	if (intvl < 1)
		intvl = 1;
	if (chktim + intvl > t)
		return;
	for (; *vp; vp++) {
		if (stat(*vp, &stb) < 0)
			continue;
		new = stb.st_mtime > time0;
		if (stb.st_size == 0 || stb.st_atime > stb.st_mtime ||
		    (stb.st_atime < chktim && stb.st_mtime < chktim) ||
		    loginsh && !new)
			continue;
		if (cnt == 1)
			printf("You have %smail.\n", new ? "new " : "");
		else
			printf("%s in %s.\n", new ? "New mail" : "Mail", *vp);
	}
	chktim = t;
}

#include <pwd.h>
/*
 * Extract a home directory from the password file
 * The argument points to a buffer where the name of the
 * user whose home directory is sought is currently.
 * We write the home directory of the user back there.
 */
gethdir(home)
	char *home;
{
	register struct passwd *pp = getpwnam(home);

	if (pp == 0)
		return (1);
	strcpy(home, pp->pw_dir);
	return (0);
}

/*
 * Move the initial descriptors to their eventual
 * resting places, closin all other units.
 */
initdesc()
{

	didcch = 0;			/* Havent closed for child */
	didfds = 0;			/* 0, 1, 2 aren't set up */
	SHIN = dcopy(0, FSHIN);
	SHOUT = dcopy(1, FSHOUT);
	SHDIAG = dcopy(2, FSHDIAG);
	OLDSTD = dcopy(SHIN, FOLDSTD);
	closem();
}

exit(i)
	int i;
{

	untty();
#ifdef PROF
	IEH3exit(i);
#else
	_exit(i);
#endif
}

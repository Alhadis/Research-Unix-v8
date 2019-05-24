static	char *sccsid = "@(#)sh.sem.c 4.1 10/9/80";

#include "sh.h"
#include "sh.proc.h"
#include <sys/ioctl.h>

/*
 * C shell
 */

/*VARARGS 1*/
execute(t, wanttty, pipein, pipeout)
	register struct command *t;
	int wanttty, *pipein, *pipeout;
{
	bool forked = 0;
	struct biltins *bifunc;
	int pid = 0;
	int pv[2];

	if (t == 0)
		return;
	if ((t->t_dflg & FAND) && wanttty > 0)
		wanttty = 0;
	switch (t->t_dtyp) {

	case TCOM:
		if ((t->t_dcom[0][0] & (QUOTE|TRIM)) == QUOTE)
			strcpy(t->t_dcom[0], t->t_dcom[0] + 1);
		if ((t->t_dflg & FREDO) == 0)
			Dfix(t);		/* $ " ' \ */
		if (t->t_dcom[0] == 0)
			return;
		/* fall into... */

	case TPAR:
		if (t->t_dflg & FPOU)
			mypipe(pipeout);
		/*
		 * Must do << early so parent will know
		 * where input pointer should be.
		 * If noexec then this is all we do.
		 */
		if (t->t_dflg & FHERE) {
			close(0);
			heredoc(t->t_dlef);
			if (noexec)
				close(0);
		}
		if (noexec)
			break;

		set("status", "0");

		/*
		 * This mess is the necessary kludge to handle the prefix
		 * builtins: nice, nohup, time.  These commands can also
		 * be used by themselves, and this is not handled here.
		 * This will also work when loops are parsed.
		 */
		while (t->t_dtyp == TCOM)
			if (eq(t->t_dcom[0], "nice"))
				if (t->t_dcom[1])
					if (any(t->t_dcom[1][0], "+-"))
						if (t->t_dcom[2]) {
							setname("nice");
							t->t_nice = getn(t->t_dcom[1]);
							lshift(t->t_dcom, 2);
							t->t_dflg |= FNICE;
						} else
							break;
					else {
						t->t_nice = 4;
						lshift(t->t_dcom, 1);
						t->t_dflg |= FNICE;
					}
				else
					break;
			else if (eq(t->t_dcom[0], "nohup"))
				if (t->t_dcom[1]) {
					t->t_dflg |= FNOHUP;
					lshift(t->t_dcom, 1);
				} else
					break;
			else if (eq(t->t_dcom[0], "time"))
				if (t->t_dcom[1]) {
					t->t_dflg |= FTIME;
					lshift(t->t_dcom, 1);
				} else
					break;
			else
				break;
		/*
		 * Check if we have a builtin function and remember which one.
		 */
		bifunc = t->t_dtyp == TCOM ? isbfunc(t) : (struct biltins *) 0;

		/*
		 * We fork only if we are timed, or are not the end of
		 * a parenthesized list and not a simple builtin function.
		 * Simple meaning one that is not pipedout, niced, nohupped,
		 * or &'d.
		 * It would be nice(?) to not fork in some of these cases.
		 */
		if (((t->t_dflg & FTIME) || (t->t_dflg & FPAR) == 0 &&
		     (!bifunc || t->t_dflg & (FPOU|FAND|FNICE|FNOHUP))))
#ifdef VFORK
		    if (t->t_dtyp == TPAR || t->t_dflg&(FREDO|FAND) || bifunc)
#endif
			{ forked++; pid = pfork(t, wanttty); }
#ifdef VFORK
		    else {
			int vffree();
			int ochild, osetintr, ohaderr, odidfds, odidcch;
			int oSHIN, oSHOUT, oSHDIAG, oOLDSTD, otpgrp;

			sighold(SIGCHLD);
			ochild = child; osetintr = setintr;
			ohaderr = haderr; odidfds = didfds; odidcch = didcch;
			oSHIN = SHIN; oSHOUT = SHOUT;
			oSHDIAG = SHDIAG; oOLDSTD = OLDSTD; otpgrp = tpgrp;
			Vsav = Vdp = 0; Vav = 0;
			pid = vfork();
			if (pid < 0) {
				sigrelse(SIGCHLD);
				error("No more processes");
			}
			forked++;
			if (pid) {
				child = ochild; setintr = osetintr;
				haderr = ohaderr; didfds = odidfds;
				didcch = odidcch; SHIN = oSHIN;
				SHOUT = oSHOUT; SHDIAG = oSHDIAG;
				OLDSTD = oOLDSTD; tpgrp = otpgrp;
				xfree(Vsav); Vsav = 0;
				xfree(Vdp); Vdp = 0;
				xfree(Vav); Vav = 0;
				/* this is from pfork() */
				palloc(pid, t);
				sigrelse(SIGCHLD);
			} else {
				/* this is from pfork() */
				int pgrp;
				bool ignint = 0;

				if (setintr)
					ignint =
					    (tpgrp == -1 && (t->t_dflg&FINT))
					    || gointr && eq(gointr, "-");
				pgrp = pcurrjob ? pcurrjob->p_jobid : getpid();
				child++;
				if (setintr) {
					setintr = 0;
					sigsys(SIGCHLD, SIG_DFL);
					sigsys(SIGINT, ignint ? SIG_IGN : vffree);
					sigsys(SIGQUIT, ignint ? SIG_IGN : SIG_DFL);
					if (wanttty >= 0) {
						sigsys(SIGTSTP, SIG_DFL);
						sigsys(SIGTTIN, SIG_DFL);
						sigsys(SIGTTOU, SIG_DFL);
					}
					sigsys(SIGTERM, parterm);
				} else if (tpgrp == -1 && (t->t_dflg&FINT)) {
					sigsys(SIGINT, SIG_IGN);
					sigsys(SIGQUIT, SIG_IGN);
				}
				if (wanttty > 0)
					ioctl(FSHTTY, TIOCSPGRP, &pgrp);
				if (wanttty >= 0 && tpgrp >= 0)
					setpgrp(0, pgrp);
				if (tpgrp > 0)
					tpgrp = 0;
				if (t->t_dflg & FNOHUP)
					sigsys(SIGHUP, SIG_IGN);
				if (t->t_dflg & FNICE)
					nice(t->t_nice);
			}

		}
#endif
		if (pid != 0) {
			/*
			 * It would be better if we could wait for the
			 * whole job when we knew the last process
			 * had been started.  Pwait, in fact, does
			 * wait for the whole job anyway, but this test
			 * doesn't really express our intentions.
			 */
			if (didfds==0 && t->t_dflg&FPIN)
				close(pipein[0]), close(pipein[1]);
			if ((t->t_dflg & (FPOU|FAND)) == 0)
				pwait();
			break;
		}
		doio(t, pipein, pipeout);
		if (t->t_dflg & FPOU)
			close(pipeout[0]), close(pipeout[1]);

		/*
		 * Perform a builtin function.
		 * If we are not forked, arrange for possible stopping
		 */
		if (bifunc) {
			func(t, bifunc);
			if (forked)
				exitstat();
			break;
		}
		if (t->t_dtyp != TPAR) {
			doexec(t);
			/*NOTREACHED*/
		}
		/*
		 * For () commands must put new 0,1,2 in FSH* and recurse
		 */
		OLDSTD = dcopy(0, FOLDSTD);
		SHOUT = dcopy(1, FSHOUT);
		SHDIAG = dcopy(2, FSHDIAG);
		close(SHIN), SHIN = -1;
		didcch = 0, didfds = 0;
		wanttty = -1;
		t->t_dspr->t_dflg |= t->t_dflg & FINT;
		execute(t->t_dspr, wanttty);
		exitstat();

	case TFIL:
		t->t_dcar->t_dflg |= FPOU |
		    (t->t_dflg & (FPIN|FAND|FDIAG|FINT));
		execute(t->t_dcar, wanttty, pipein, pv);
		t->t_dcdr->t_dflg |= FPIN |
		    (t->t_dflg & (FPOU|FAND|FPAR|FINT));
		if (wanttty > 0)
			wanttty = 0;		/* got tty already */
		execute(t->t_dcdr, wanttty, pv, pipeout);
		break;

	case TLST:
		if (t->t_dcar) {
			t->t_dcar->t_dflg |= t->t_dflg & FINT;
			execute(t->t_dcar, wanttty);
			/*
			 * In strange case of A&B make a new job after A
			 */
			if (t->t_dcar->t_dflg&FAND && t->t_dcdr &&
			    (t->t_dcdr->t_dflg&FAND) == 0)
				pendjob();
		}
		if (t->t_dcdr) {
			t->t_dcdr->t_dflg |= t->t_dflg & (FPAR|FINT);
			execute(t->t_dcdr, wanttty);
		}
		break;

	case TOR:
	case TAND:
		if (t->t_dcar) {
			t->t_dcar->t_dflg |= t->t_dflg & FINT;
			execute(t->t_dcar, wanttty);
			if ((getn(value("status")) == 0) != (t->t_dtyp == TAND))
				return;
		}
		if (t->t_dcdr) {
			t->t_dcdr->t_dflg |= t->t_dflg & (FPAR|FINT);
			execute(t->t_dcdr, wanttty);
		}
		break;
	}
	/*
	 * Fall through for all breaks from switch
	 *
	 * If there will be no more executions of this
	 * command, flush all file descriptors.
	 * Places that turn on the FREDO bit are responsible
	 * for doing donefds after the last re-execution
	 */
	if (didfds && !(t->t_dflg & FREDO))
		donefds();
}

#ifdef VFORK
vffree()
{
	register char **v;

	if (v = gargv)
		gargv = 0, xfree(gargv);
	if (v = pargv)
		pargv = 0, xfree(pargv);
	_exit(1);
}
#endif

/*
 * Perform io redirection.
 * We may or maynot be forked here.
 */
doio(t, pipein, pipeout)
	register struct command *t;
	int *pipein, *pipeout;
{
	register char *cp;
	register int flags = t->t_dflg;

	if (didfds || (flags & FREDO))
		return;
	if ((flags & FHERE) == 0) {	/* FHERE already done */
		close(0);
		if (cp = t->t_dlef) {
			cp = globone(Dfix1(cp));
			xfree(cp);
			if (open(cp, 0) < 0)
				Perror(cp);
		} else if (flags & FPIN)
			dup(pipein[0]), close(pipein[0]), close(pipein[1]);
		else if ((flags & FINT) 
		    /*
		     * until stop on background read is implemented ...
		     */
		    /* && tpgrp == -1 */ )
			close(0), open("/dev/null", 0);
		else
			dup(OLDSTD);
	}
	close(1);
	if (cp = t->t_drit) {
		cp = globone(Dfix1(cp));
		xfree(cp);
		if ((flags & FCAT) && open(cp, 1) >= 0)
			lseek(1, 0l, 2);
		else {
			if (!(flags & FANY) && adrof("noclobber")) {
				if (flags & FCAT)
					Perror(cp);
				chkclob(cp);
			}
			if (creat(cp, 0666) < 0)
				Perror(cp);
		}
	} else if (flags & FPOU)
		dup(pipeout[1]);
	else
		dup(SHOUT);

	close(2);
	dup((flags & FDIAG) ? 1 : SHDIAG);
	didfds = 1;
}

mypipe(pv)
	register int *pv;
{

	if (pipe(pv) < 0)
		goto oops;
	pv[0] = dmove(pv[0], -1);
	pv[1] = dmove(pv[1], -1);
	if (pv[0] >= 0 && pv[1] >= 0)
		return;
oops:
	error("Can't make pipe");
}

chkclob(cp)
	register char *cp;
{
	struct stat stb;

	if (stat(cp, &stb) < 0)
		return;
	if ((stb.st_mode & S_IFMT) == S_IFCHR)
		return;
	error("%s: File exists", cp);
}

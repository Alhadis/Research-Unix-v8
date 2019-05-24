static	char *sccsid = "@(#)sh.proc.c	4.6 (Berkeley) 81/05/03";

#include "sh.h"
#include "sh.dir.h"
#include "sh.proc.h"
#include <wait.h>
#include <sys/ioctl.h>

/*
 * C Shell - functions that manage processes, handling hanging, termination
 */

#define BIGINDEX	9	/* largest desirable job index */

/*
 * pchild - called at interrupt level by the SIGCHLD signal
 *	indicating that at least one child has terminated or stopped
 *	thus at least one wait system call will definitely return a
 *	childs status.  Top level routines (like pwait) must be sure
 *	to mask interrupts when playing with the proclist data structures!
 */
pchild()
{
	register struct process *pp;
	register struct process	*fp;
	register int pid;
	union wait w;
	int jobflags;
#ifdef VMUNIX
	struct vtimes vt;
#endif

	if (!timesdone)
		timesdone++, times(&shtimes);
loop:
	pid = wait3(&w.w_status, (setintr ? WNOHANG|WUNTRACED:WNOHANG),
#ifndef VMUNIX
	    0);
#else
	    &vt);
#endif
	if (pid <= 0) {
		if (errno == EINTR) {
			errno = 0;
			goto loop;
		}
		pnoprocesses = pid == -1;
		return;
	}
	for (pp = proclist.p_next; pp != PNULL; pp = pp->p_next)
		if (pid == pp->p_pid)
			goto found;
	goto loop;
found:
	if (pid == atoi(value("child")))
		unsetv("child");
	pp->p_flags &= ~(PRUNNING|PSTOPPED|PREPORTED);
	if (WIFSTOPPED(w)) {
		pp->p_flags |= PSTOPPED;
		pp->p_reason = w.w_stopsig;
	} else {
		if (pp->p_flags & (PTIME|PPTIME) || adrof("time")) {
			time_t oldcutimes, oldcstimes;
			oldcutimes = shtimes.tms_cutime;
			oldcstimes = shtimes.tms_cstime;
			time(&pp->p_etime);
			times(&shtimes);
			pp->p_utime = shtimes.tms_cutime - oldcutimes;
			pp->p_stime = shtimes.tms_cstime - oldcstimes;
		} else
			times(&shtimes);
#ifdef VMUNIX
		pp->p_vtimes = vt;
#endif
		if (WIFSIGNALED(w)) {
			if (w.w_termsig == SIGINT)
				pp->p_flags |= PINTERRUPTED;
			else
				pp->p_flags |= PSIGNALED;
			if (w.w_coredump)
				pp->p_flags |= PDUMPED;
			pp->p_reason = w.w_termsig;
		} else {
			pp->p_reason = w.w_retcode;
#ifdef IIASA
			if (pp->p_reason >= 3)
#else
			if (pp->p_reason != 0)
#endif
				pp->p_flags |= PAEXITED;
			else
				pp->p_flags |= PNEXITED;
		}
	}
	jobflags = 0;
	fp = pp;
	do {
		if ((fp->p_flags & (PPTIME|PRUNNING|PSTOPPED)) == 0 &&
		    !child && adrof("time") &&
		    (fp->p_utime + fp->p_stime) / HZ >=
		     atoi(value("time")))
			fp->p_flags |= PTIME;
		jobflags |= fp->p_flags;
	} while ((fp = fp->p_friends) != pp);
	pp->p_flags &= ~PFOREGND;
	if (pp == pp->p_friends && (pp->p_flags & PPTIME)) {
		pp->p_flags &= ~PPTIME;
		pp->p_flags |= PTIME;
	}
	if ((jobflags & (PRUNNING|PREPORTED)) == 0) {
		fp = pp;
		do {
			if (fp->p_flags&PSTOPPED)
				fp->p_flags |= PREPORTED;
		} while((fp = fp->p_friends) != pp);
		while(fp->p_pid != fp->p_jobid)
			fp = fp->p_friends;
		if (jobflags&PSTOPPED) {
			if (pcurrent && pcurrent != fp)
				pprevious = pcurrent;
			pcurrent = fp;
		} else
			pclrcurr(fp);
		if (jobflags&PFOREGND) {
			if (jobflags & (PSIGNALED|PSTOPPED|PPTIME) ||
#ifdef IIASA
			    jobflags & PAEXITED ||
#endif
			    !eq(dcwd->di_name, fp->p_cwd->di_name)) {
				;	/* print in pjwait */
			}
/*
		else if ((jobflags & (PTIME|PSTOPPED)) == PTIME)
				ptprint(fp);
*/
		} else {
			if (jobflags&PNOTIFY || adrof("notify")) {
				printf("\215\n");
				pprint(pp, NUMBER|NAME|REASON);
				if ((jobflags&PSTOPPED) == 0)
					pflush(pp);
			} else {
				fp->p_flags |= PNEEDNOTE;
				neednote++;
			}
		}
	}
	goto loop;
}

pnote()
{
	register struct process *pp;
	int flags;

	neednote = 0;
	for (pp = proclist.p_next; pp != PNULL; pp = pp->p_next) {
		if (pp->p_flags & PNEEDNOTE) {
			sighold(SIGCHLD);
			pp->p_flags &= ~PNEEDNOTE;
			flags = pprint(pp, NUMBER|NAME|REASON);
			if ((flags&(PRUNNING|PSTOPPED)) == 0)
				pflush(pp);
			sigrelse(SIGCHLD);
		}
	}
}

/*
 * pwait - wait for current job to terminate, maintaining integrity
 *	of current and previous job indicators.
 */
pwait()
{
	register struct process *fp, *pp;

	/*
	 * Here's where dead procs get flushed.
	 */
	sighold(SIGCHLD);
	for (pp = (fp = &proclist)->p_next; pp != PNULL; pp = (fp = pp)->p_next)
		if (pp->p_pid == 0) {
			fp->p_next = pp->p_next;
			xfree(pp->p_command);
			if (pp->p_cwd && --pp->p_cwd->di_count == 0)
				if (pp->p_cwd->di_next == 0)
					dfree(pp->p_cwd);
			xfree((char *)pp);
			pp = fp;
		}
	sigrelse(SIGCHLD);
	if (setintr)
		sigignore(SIGINT);
	pjwait(pcurrjob);
}

/*
 * pjwait - wait for a job to finish or become stopped
 *	It is assumed to be in the foreground state (PFOREGND)
 */
pjwait(pp)
	register struct process *pp;
{
	register struct process *fp;
	int jobflags, reason;

	fp = pp;
	do {
		if ((fp->p_flags&(PFOREGND|PRUNNING)) == PRUNNING)
			printf("BUG: waiting for background job!\n");
	} while ((fp = fp->p_friends) != pp);
	/*
	 * Now keep pausing as long as we are not interrupted (SIGINT),
	 * and the target process, or any of its friends, are running
	 */
	fp = pp;
	for (;;) {
		sighold(SIGCHLD);
		jobflags = 0;
		do
			jobflags |= fp->p_flags;
		while((fp = (fp->p_friends)) != pp);
		if ((jobflags & PRUNNING) == 0)
			break;
		sigpause(SIGCHLD);
	}
	sigrelse(SIGCHLD);
	if (tpgrp > 0)
		ioctl(FSHTTY, TIOCSPGRP, &tpgrp);	/* get tty back */
	if ((jobflags&(PSIGNALED|PSTOPPED|PTIME)) ||
	     !eq(dcwd->di_name, fp->p_cwd->di_name)) {
		if (jobflags&PSTOPPED)
			printf("\n");
		pprint(pp, AREASON|SHELLDIR);
	}
	if ((jobflags&(PINTERRUPTED|PSTOPPED)) && setintr &&
	    (!gointr || !eq(gointr, "-"))) {
		if ((jobflags & PSTOPPED) == 0)
			pflush(pp);
		pintr1(0);
		/*NOTREACHED*/
	}
	reason = 0;
	fp = pp;
	do {
		if (fp->p_reason)
			reason = fp->p_flags & (PSIGNALED|PINTERRUPTED) ?
				fp->p_reason | QUOTE : fp->p_reason;
	} while ((fp = fp->p_friends) != pp);
	set("status", putn(reason));
	if (reason && exiterr)
		exitstat();
	pflush(pp);
}

/*
 * dowait - wait for all processes to finish
 */
dowait()
{
	register struct process *pp;

	pjobs++;
	if (setintr)
		sigrelse(SIGINT);
loop:
	sighold(SIGCHLD);
	for (pp = proclist.p_next; pp; pp = pp->p_next)
		if (pp->p_pid && pp->p_pid == pp->p_jobid &&
		    pp->p_flags&PRUNNING) {
			sigpause(SIGCHLD);
			goto loop;
		}
	sigrelse(SIGCHLD);
	pjobs = 0;
}

/*
 * pflushall - flush all jobs from list (e.g. at fork())
 */
pflushall()
{
	register struct process	*pp;

	for (pp = proclist.p_next; pp != PNULL; pp = pp->p_next)
		if (pp->p_pid)
			pflush(pp);
}

/*
 * pflush - flag all process structures in the same job as the
 *	the argument process for deletion.  The actual free of the
 *	space is not done here since pflush is called at interrupt level.
 */
pflush(pp)
	register struct process	*pp;
{
	register struct process *np;
	register int index;

	if (pp->p_pid == 0) {
		printf("BUG: process flushed twice");
		return;
	}
	while (pp->p_pid != pp->p_jobid)
		pp = pp->p_friends;
	pclrcurr(pp);
	if (pp == pcurrjob)
		pcurrjob = 0;
	index = pp->p_index;
	np = pp;
	do {
		np->p_index = np->p_pid = 0;
		np->p_flags &= ~PNEEDNOTE;
	} while ((np = np->p_friends) != pp);
	if (index == pmaxindex) {
		for (np = proclist.p_next, index = 0; np; np = np->p_next)
			if (np->p_index > index)
				index = np->p_index;
		pmaxindex = index;
	}
}

/*
 * pclrcurr - make sure the given job is not the current or previous job;
 *	pp MUST be the job leader
 */
pclrcurr(pp)
	register struct process *pp;
{

	if (pp == pcurrent)
		if (pprevious != PNULL) {
			pcurrent = pprevious;
			pprevious = pgetcurr(pp);
		} else {
			pcurrent = pgetcurr(pp);
			pprevious = pgetcurr(pp);
		}
	else if (pp == pprevious)
		pprevious = pgetcurr(pp);
}

/* +4 here is 1 for '\0', 1 ea for << >& >> */
char	command[PMAXLEN+4];
int	cmdlen;
char	*cmdp;
/*
 * palloc - allocate a process structure and fill it up.
 *	an important assumption is made that the process is running.
 */
palloc(pid, t)
	int pid;
	register struct command *t;
{
	register struct process	*pp;
	int i;

	pp = (struct process *)calloc(1, sizeof(struct process));
	pp->p_pid = pid;
	pp->p_flags = t->t_dflg & FAND ? PRUNNING : PRUNNING|PFOREGND;
	if (t->t_dflg & FTIME)
		pp->p_flags |= PPTIME;
	cmdp = command;
	cmdlen = 0;
	padd(t);
	*cmdp++ = 0;
	if (t->t_dflg & FPOU) {
		pp->p_flags |= PPOU;
		if (t->t_dflg & FDIAG)
			pp->p_flags |= PDIAG;
	}
	pp->p_command = savestr(command);
	if (pcurrjob) {
		struct process *fp;
		/* careful here with interrupt level */
		pp->p_cwd = 0;
		pp->p_index = pcurrjob->p_index;
		pp->p_friends = pcurrjob;
		pp->p_jobid = pcurrjob->p_pid;
		for (fp = pcurrjob; fp->p_friends != pcurrjob; fp = fp->p_friends)
			;
		fp->p_friends = pp;
	} else {
		pcurrjob = pp;
		pp->p_jobid = pid;
		pp->p_friends = pp;
		pp->p_cwd = dcwd;
		dcwd->di_count++;
		if (pmaxindex < BIGINDEX)
			pp->p_index = ++pmaxindex;
		else {
			struct process *np;

			for (i = 1; ; i++) {
				for (np = proclist.p_next; np; np = np->p_next)
					if (np->p_index == i)
						goto tryagain;
				pp->p_index = i;
				if (i > pmaxindex)
					pmaxindex = i;
				break;			
			tryagain:;
			}
		}
		if (pcurrent == PNULL)
			pcurrent = pp;
		else if (pprevious == PNULL)
			pprevious = pp;
	}
	pp->p_next = proclist.p_next;
	proclist.p_next = pp;
	time(&pp->p_btime);
}

padd(t)
	register struct command *t;
{
	char **argp;

	if (t == 0)
		return;
	switch (t->t_dtyp) {

	case TPAR:
		pads("( ");
		padd(t->t_dspr);
		pads(" )");
		break;

	case TCOM:
		for (argp = t->t_dcom; *argp; argp++) {
			pads(*argp);
			if (argp[1])
				pads(" ");
		}
		break;

	case TFIL:
		padd(t->t_dcar);
		pads(" | ");
		padd(t->t_dcdr);
		return;

	case TLST:
		padd(t->t_dcar);
		pads("; ");
		padd(t->t_dcdr);
		return;
	}
	if ((t->t_dflg & FPIN) == 0 && t->t_dlef) {
		pads((t->t_dflg & FHERE) ? " << " : " < ");
		pads(t->t_dlef);
	}
	if ((t->t_dflg & FPOU) == 0 && t->t_drit) {
		pads((t->t_dflg & FCAT) ? " >>" : " >");
		if (t->t_dflg & FDIAG)
			pads("&");
		pads(" ");
		pads(t->t_drit);
	}
}

pads(cp)
	char *cp;
{
	register int i = strlen(cp);

	if (cmdlen >= PMAXLEN)
		return;
	if (cmdlen + i >= PMAXLEN) {
		strcpy(cmdp, " ...");
		cmdlen = PMAXLEN;
		cmdp += 4;
		return;
	}
	strcpy(cmdp, cp);
	cmdp += i;
	cmdlen += i;
}

/*
 * psavejob - temporarily save the current job on a one level stack
 *	so another job can be created.  Used for { } in exp6
 *	and `` in globbing.
 */
psavejob()
{

	pholdjob = pcurrjob;
	pcurrjob = PNULL;
}

/*
 * prestjob - opposite of psavejob.  This may be missed if we are interrupted
 *	somewhere, but pendjob cleans up anyway.
 */
prestjob()
{

	pcurrjob = pholdjob;
	pholdjob = PNULL;
}

/*
 * pendjob - indicate that a job (set of commands) has been completed
 *	or is about to begin.
 */
pendjob()
{
	register struct process *pp, *tp;

	if (pcurrjob && (pcurrjob->p_flags&(PFOREGND|PSTOPPED)) == 0) {
		pp = pcurrjob;
		while (pp->p_pid != pp->p_jobid)
			pp = pp->p_friends;
		printf("[%d]", pp->p_index);
		tp = pp;
		do {
			printf(" %d", pp->p_pid);
			pp = pp->p_friends;
		} while (pp != tp);
		printf("\n");
	}
	pholdjob = pcurrjob = 0;
}

/*
 * pprint - print a job
 */
pprint(pp, flag)
	register struct process	*pp;
{
	register status, reason;
	struct process *tp;
	extern char *linp, linbuf[];
	int jobflags, pstatus;
	char *format;

	while (pp->p_pid != pp->p_jobid)
		pp = pp->p_friends;
	if (pp == pp->p_friends && (pp->p_flags & PPTIME)) {
		pp->p_flags &= ~PPTIME;
		pp->p_flags |= PTIME;
	}
	tp = pp;
	status = reason = -1; 
	jobflags = 0;
	do {
		jobflags |= pp->p_flags;
		pstatus = pp->p_flags & PALLSTATES;
		if (tp != pp && linp != linbuf && !(flag&FANCY) &&
		    (pstatus == status && pp->p_reason == reason ||
		     !(flag&REASON)))
			printf(" ");
		else {
			if (tp != pp && linp != linbuf)
				printf("\n");
			if(flag&NUMBER)
				if (pp == tp)
					printf("[%d]%s %c ", pp->p_index,
					    pp->p_index < 10 ? " " : "",
					    pp==pcurrent ? '+' :
						(pp == pprevious ? '-' : ' '));
				else
					printf("       ");
			if (flag&FANCY)
				printf("%5d ", pp->p_pid);
			if (flag&(REASON|AREASON)) {
				if (flag&NAME)
					format = "%-21s";
				else
					format = "%s";
				if (pstatus == status)
					if (pp->p_reason == reason) {
						printf(format, "");
						goto prcomd;
					} else
						reason = pp->p_reason;
				else {
					status = pstatus;
					reason = pp->p_reason;
				}
				switch (status) {

				case PRUNNING:
					printf(format, "Running ");
					break;

				case PINTERRUPTED:
				case PSTOPPED:
				case PSIGNALED:
					if (flag&REASON || reason != SIGINT ||
					    reason != SIGPIPE)
						printf(format, mesg[pp->p_reason].pname);
					break;

				case PNEXITED:
				case PAEXITED:
					if (flag & REASON)
						if (pp->p_reason)
							printf("Exit %-16d", pp->p_reason);
						else
							printf(format, "Done");
					break;

				default:
					printf("BUG: status=%-9o", status);
				}
			}
		}
prcomd:
		if (flag&NAME) {
			printf("%s", pp->p_command);
			if (pp->p_flags & PPOU)
				printf(" |");
			if (pp->p_flags & PDIAG)
				printf("&");
		}
		if (flag&(REASON|AREASON) && pp->p_flags&PDUMPED)
			printf(" (core dumped)");
		if (tp == pp->p_friends) {
			if (flag&AMPERSAND)
				printf(" &");
			if (flag&JOBDIR &&
			    !eq(tp->p_cwd->di_name, dcwd->di_name)) {
				printf(" (wd: ");
				dtildepr(value("home"), tp->p_cwd->di_name);
				printf(")");
			}
		}
		if (pp->p_flags&PPTIME && !(status&(PSTOPPED|PRUNNING))) {
			if (linp != linbuf)
				printf("\n\t");
#ifndef VMUNIX
			ptimes(pp->p_utime, pp->p_stime, pp->p_etime-pp->p_btime);
#else
			pvtimes(&zvms, &pp->p_vtimes, pp->p_etime - pp->p_btime);
#endif
		}
		if (tp == pp->p_friends) {
			if (linp != linbuf)
				printf("\n");
			if (flag&SHELLDIR && !eq(tp->p_cwd->di_name, dcwd->di_name)) {
				printf("(wd now: ");
				dtildepr(value("home"), dcwd->di_name);
				printf(")\n");
			}
		}
	} while ((pp = pp->p_friends) != tp);
	if (jobflags&PTIME && (jobflags&(PSTOPPED|PRUNNING)) == 0) {
		if (jobflags & NUMBER)
			printf("       ");
		ptprint(tp);
	}
	return (jobflags);
}

ptprint(tp)
	register struct process *tp;
{
	time_t tetime = 0;
#ifdef VMUNIX
	struct vtimes vmt;
#else
	time_t tutime = 0, tstime = 0;
#endif
	register struct process *pp = tp;

	vmt = zvms;
	do {
#ifdef VMUNIX
		vmsadd(&vmt, &pp->p_vtimes);
#else
		tutime += pp->p_utime;
		tstime += pp->p_stime;
#endif
		if (pp->p_etime - pp->p_btime > tetime)
			tetime = pp->p_etime - pp->p_btime;
	} while ((pp = pp->p_friends) != tp);
#ifdef VMUNIX
	pvtimes(&zvms, &vmt, tetime);
#else
	ptimes(tutime, tstime, tetime);
#endif
}

/*
 * dojobs - print all jobs
 */
dojobs(v)
	char **v;
{
	register struct process *pp;
	register int flag = NUMBER|NAME|REASON;
	int i;

	if (chkstop)
		chkstop = 2;
	if (*++v) {
		if (v[1] || !eq(*v, "-l"))
			error("Usage: jobs [ -l ]");
		flag |= FANCY|JOBDIR;
	}
	for (i = 1; i <= pmaxindex; i++)
		for (pp = proclist.p_next; pp; pp = pp->p_next)
			if (pp->p_index == i && pp->p_pid == pp->p_jobid) {
				pp->p_flags &= ~PNEEDNOTE;
				if (!(pprint(pp, flag) & (PRUNNING|PSTOPPED)))
					pflush(pp);
				break;
			}
}

/*
 * dofg - builtin - put the job into the foreground
 */
dofg(v)
	char **v;
{
	register struct process *pp;

	okpcntl();
	++v;
	do {
		pp = pfind(*v);
		pstart(pp, 1);
		if (setintr)
			sigignore(SIGINT);
		pjwait(pp);
	} while (*v && *++v);
}

/*
 * %... - builtin - put the job into the foreground
 */
dofg1(v)
	char **v;
{
	register struct process *pp;

	okpcntl();
	pp = pfind(v[0]);
	pstart(pp, 1);
	if (setintr)
		sigignore(SIGINT);
	pjwait(pp);
}

/*
 * dobg - builtin - put the job into the background
 */
dobg(v)
	char **v;
{
	register struct process *pp;

	okpcntl();
	++v;
	do {
		pp = pfind(*v);
		pstart(pp, 0);
	} while (*v && *++v);
}

/*
 * %... & - builtin - put the job into the background
 */
dobg1(v)
	char **v;
{
	register struct process *pp;

	pp = pfind(v[0]);
	pstart(pp, 0);
}

/*
 * dostop - builtin - stop the job
 */
dostop(v)
	char **v;
{

	pkill(++v, SIGSTOP);
}

/*
 * dokill - builtin - superset of kill (1)
 */
dokill(v)
	char **v;
{
	register int signum;
	register char *name;

	v++;
	if (v[0] && v[0][0] == '-') {
		if (v[0][1] == 'l') {
			for (signum = 1; signum <= NSIG; signum++) {
				if (name = mesg[signum].iname)
					printf("%s ", name);
				if (signum == 16)
					printf("\n");
			}
			printf("\n");
			return;
		}
		if (digit(v[0][1])) {
			signum = atoi(v[0]+1);
			if (signum < 1 || signum > NSIG)
				bferr("Bad signal number");
		} else {
			name = &v[0][1];
			for (signum = 1; signum <= NSIG; signum++)
			if (mesg[signum].iname &&
			    eq(name, mesg[signum].iname))
				goto gotsig;
			setname(name);
			bferr("Unknown signal; kill -l lists signals");
		}
gotsig:
		v++;
	} else
		signum = SIGTERM;
	pkill(v, signum);
}

pkill(v, signum)
	char **v;
	int signum;
{
	register struct process *pp, *np;
	register int jobflags = 0;
	int pid;
	extern char *sys_errlist[];
	int err = 0;

	if (setintr)
		sighold(SIGINT);
	sighold(SIGCHLD);
	while (*v) {
		if (**v == '%') {
			np = pp = pfind(*v);
			do
				jobflags |= np->p_flags;
			while ((np = np->p_friends) != pp);
			switch (signum) {

			case SIGSTOP:
			case SIGTSTP:
			case SIGTTIN:
			case SIGTTOU:
				if ((jobflags & PRUNNING) == 0) {
					printf("%s: Already stopped\n", *v);
					err++;
					goto cont;
				}
			}
			killpg(pp->p_jobid, signum);
			if (signum == SIGTERM || signum == SIGHUP)
				killpg(pp->p_jobid, SIGCONT);
		} else if (!digit(**v))
			bferr("Arguments should be jobs or process id's");
		else {
			pid = atoi(*v);
			if (kill(pid, signum) < 0) {
				printf("%d: ", pid);
				printf("%s\n", sys_errlist[errno]);
				err++;
				goto cont;
			}
			if (signum == SIGTERM || signum == SIGHUP)
				kill(pid, SIGCONT);
		}
cont:
		v++;
	}
	sigrelse(SIGCHLD);
	if (setintr)
		sigrelse(SIGINT);
	if (err)
		error(NOSTR);
}

/*
 * pstart - start the job in foreground/background
 */
pstart(pp, foregnd)
	register struct process *pp;
	int foregnd;
{
	register struct process *np;
	int jobflags = 0;

	sighold(SIGCHLD);
	np = pp;
	do {
		jobflags |= np->p_flags;
		if (np->p_flags&(PRUNNING|PSTOPPED)) {
			np->p_flags |= PRUNNING;
			np->p_flags &= ~PSTOPPED;
			if (foregnd)
				np->p_flags |= PFOREGND;
			else
				np->p_flags &= ~PFOREGND;
		}
	} while((np = np->p_friends) != pp);
	if (!foregnd)
		pclrcurr(pp);
	pprint(pp, foregnd ? NAME|JOBDIR : NUMBER|NAME|AMPERSAND);
	if (foregnd)
		ioctl(FSHTTY, TIOCSPGRP, &pp->p_jobid);
	if (jobflags&PSTOPPED)
		killpg(pp->p_jobid, SIGCONT);
	sigrelse(SIGCHLD);
}

panystop(neednl)
{
	register struct process *pp;

	chkstop = 2;
	for (pp = proclist.p_next; pp; pp = pp->p_next)
		if (pp->p_flags & PSTOPPED)
			error("\nThere are stopped jobs" + 1 - neednl);
}

struct process *
pfind(cp)
	char *cp;
{
	register struct process *pp, *np;

	if (cp == 0 || cp[1] == 0 || eq(cp, "%%") || eq(cp, "%+")) {
		if (pcurrent == PNULL)
			bferr("No current job");
		return (pcurrent);
	}
	if (eq(cp, "%-") || eq(cp, "%#")) {
		if (pprevious == PNULL)
			bferr("No previous job");
		return (pprevious);
	}
	if (digit(cp[1])) {
		int index = atoi(cp+1);
		for (pp = proclist.p_next; pp; pp = pp->p_next)
			if (pp->p_index == index && pp->p_pid == pp->p_jobid)
				return (pp);
		bferr("No such job");
	}
	np = PNULL;
	for (pp = proclist.p_next; pp; pp = pp->p_next)
		if (pp->p_pid == pp->p_jobid) {
			if (cp[1] == '?') {
				register char *dp;
				for (dp = pp->p_command; *dp; dp++) {
					if (*dp != cp[2])
						continue;
					if (prefix(cp+2, dp))
						goto match;
				}
			} else if (prefix(cp+1, pp->p_command)) {
match:
				if (np)
					bferr("Ambiguous");
				np = pp;
			}
		}
	if (np)
		return (np);
	if (cp[1] == '?')
		bferr("No job matches pattern");
	else
		bferr("No such job");
}

/*
 * pgetcurr - find most recent job that is not pp, preferably stopped
 */
struct process *
pgetcurr(pp)
	register struct process *pp;
{
	register struct process *np;
	register struct process *xp = PNULL;

	for (np = proclist.p_next; np; np = np->p_next)
		if (np != pcurrent && np != pp && np->p_pid &&
		    np->p_pid == np->p_jobid) {
			if (np->p_flags & PSTOPPED)
				return (np);
			if (xp == PNULL)
				xp = np;
		}
	return (xp);
}

/*
 * donotify - flag the job so as to report termination asynchronously
 */
donotify(v)
	char **v;
{
	register struct process *pp;

	pp = pfind(*++v);
	pp->p_flags |= PNOTIFY;
}

/*
 * Do the fork and whatever should be done in the child side that
 * should not be done if we are not forking at all (like for simple builtin's)
 * Also do everything that needs any signals fiddled with in the parent side
 *
 * Wanttty tells whether process and/or tty pgrps are to be manipulated:
 *	-1:	leave tty alone; inherit pgrp from parent
 *	 0:	already have tty; manipulate process pgrps only
 *	 1:	want to claim tty; manipulate process and tty pgrps
 * It is usually just the value of tpgrp.
 */
pfork(t, wanttty)
	struct command *t;	/* command we are forking for */
	int wanttty;
{
	register int pid;
	bool ignint = 0;
	int pgrp;

	/*
	 * A child will be uninterruptible only under very special
	 * conditions. Remember that the semantics of '&' is
	 * implemented by disconnecting the process from the tty so
	 * signals do not need to ignored just for '&'.
	 * Thus signals are set to default action for children unless:
	 *	we have had an "onintr -" (then specifically ignored)
	 *	we are not playing with signals (inherit action)
	 */
	if (setintr)
		ignint = (tpgrp == -1 && (t->t_dflg&FINT))
		    || (gointr && eq(gointr, "-"));
	/*
	 * Hold SIGCHLD until we have the process installed in our table.
	 */
	sighold(SIGCHLD);
	while ((pid = fork()) < 0)
		if (setintr == 0)
			sleep(FORKSLEEP);
		else {
			sigrelse(SIGINT);
			sigrelse(SIGCHLD);
			error("No more processes");
		}
	if (pid == 0) {
		settimes();
		pgrp = pcurrjob ? pcurrjob->p_jobid : getpid();
		pflushall();
		pcurrjob = PNULL;
		timesdone = 0;
		child++;
		if (setintr) {
			setintr = 0;		/* until I think otherwise */
			sigrelse(SIGCHLD);
			/*
			 * Children just get blown away on SIGINT, SIGQUIT
			 * unless "onintr -" seen.
			 */
			signal(SIGINT, ignint ? SIG_IGN : SIG_DFL);
			signal(SIGQUIT, ignint ? SIG_IGN : SIG_DFL);
			if (wanttty >= 0) {
				/* make stoppable */
				signal(SIGTSTP, SIG_DFL);
				signal(SIGTTIN, SIG_DFL);
				signal(SIGTTOU, SIG_DFL);
			}
			signal(SIGTERM, parterm);
		} else if (tpgrp == -1 && (t->t_dflg&FINT)) {
			signal(SIGINT, SIG_IGN);
			signal(SIGQUIT, SIG_IGN);
		}
		if (wanttty > 0)
			ioctl(FSHTTY, TIOCSPGRP, &pgrp);
		if (wanttty >= 0 && tpgrp >= 0)
			setpgrp(0, pgrp);
		if (tpgrp > 0)
			tpgrp = 0;		/* gave tty away */
		/*
		 * Nohup and nice apply only to TCOM's but it would be
		 * nice (?!?) if you could say "nohup (foo;bar)"
		 * Then the parser would have to know about nice/nohup/time
		 */
		if (t->t_dflg & FNOHUP)
			signal(SIGHUP, SIG_IGN);
		if (t->t_dflg & FNICE) {
/* sigh...
			nice(20);
			nice(-10);
*/
			nice(t->t_nice);
		}

	} else {
		palloc(pid, t);
		sigrelse(SIGCHLD);
	}

	return (pid);
}

okpcntl()
{

	if (tpgrp == -1)
		error("No job control in this shell");
	if (tpgrp == 0)
		error("No job control in subshells");
}

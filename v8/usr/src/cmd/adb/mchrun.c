/*
 * machine-specific functions for running the debugged process
 * particular to VAX and /proc
 */

#include "defs.h"
#include <sys/types.h>
#include <sys/pioctl.h>
#include <sys/vtimes.h>
#include <sys/proc.h>
#include <signal.h>
#include <sys/psl.h>
#include <errno.h>
#include "regs.h"

MSG NOFORK;
MSG BADWAIT;
MSG ENDPCS;

char lastc, peekc;
ADDR txtsize;

static int child;
static long tsigs = -1;

/*
 * kill process
 */

killpcs()
{
	long sig = SIGKILL;

	ioctl(fcor, PIOCCSIG, 0);
	ioctl(fcor, PIOCKILL, &sig);
	ioctl(fcor, PIOCRUN, 0);
}

/*
 * grab the process already opened (but not traced);
 * stop it so we can look at it
 */

grab()
{
	struct proc p;
	int f;

	if ((f = open(corfil, 2)) < 0)
		error("no write access");
	close(fcor);
	fcor = f;
	if (ioctl(fcor, PIOCGETPR, &p) < 0)
		error("not a process");
	pid = p.p_pid;
	child = 0;
	ioctl(fcor, PIOCSMASK, &tsigs);
	if (p.p_stat != SSTOP)
		ioctl(fcor, PIOCSTOP, 0);
	bpwait();
}

/*
 * turn off tracing & let it go
 */

ungrab()
{
	long zero = 0;

	if (signo == 0)
		ioctl(fcor, PIOCCSIG, 0);
	ioctl(fcor, PIOCSMASK, &zero);
	ioctl(fcor, PIOCRUN, 0);
	pid = 0;
}

/*
 * get the program to be debugged ready to run
 * program is left stopped at the beginning (so we can poke in breakpoints)
 */

startpcs()
{
	int fd;
	char *procname();
	extern int (*sigint)(), (*sigqit)();

	fd = procopen(getpid());
	if (ioctl(fd, PIOCSEXEC, 0) < 0) {
		close(fd);
		error("no process ioctl");
	}
	if ((pid = fork()) == 0) {
		close(fd);
		close(fsym);
		close(fcor);
		signal(SIGINT, sigint);
		signal(SIGQUIT, sigqit);
		doexec();
		exit(0);
	}
	ioctl(fd, PIOCREXEC, 0);
	close(fd);
	if (pid == -1)
		error(NOFORK);
	child++;
	fcor = procopen(pid);
	corfil = procname(pid);
	ioctl(fcor, PIOCSMASK, &tsigs);
	bpwait();
	ioctl(fcor, PIOCREXEC, 0);
	if (adrflg)
		rput(PC, wtoa(adrval));
	while (rdc() != EOR)
		;
	reread();
}

int
procopen(pid)
int pid;
{
	char *pn;
	int fd;
	char *procname();

	pn = procname(pid);
	if ((fd = open(pn, 2)) < 0) {
		printf("%s: cannot open\n", pn);
		error(0);
	}
	return (fd);
}

char *
procname(pid)
int pid;
{
	static char name[20];

	sprintf(name, "/proc/%d", pid);
	return (name);
}

/*
 * set process running, single-stepped
 */

runstep(keepsig)
int keepsig;
{

	delbp();
	rput(PSL, rget(PSL) | PSL_T);
	rrest();
	if (keepsig == 0)
		ioctl(fcor, PIOCCSIG, 0);
	ioctl(fcor, PIOCRUN, 0);
}

/*
 * set process running
 */

runrun(keepsig)
int keepsig;
{

	if (keepsig == 0)
		ioctl(fcor, PIOCCSIG, 0);
	ioctl(fcor, PIOCRUN, 0);
}

/*
 * exec the program to be debugged
 * opening standard input and output as requested
 */

doexec()
{
	char *argl[MAXARG];
	char args[LINSIZ];
	register char *p;
	register char **ap;
	register char *thisarg;
	extern char **environ;

	ap = argl;
	p = args;
	*ap++ = symfil;
	for (rdc(); lastc != EOR;) {
		thisarg = p;
		while (lastc != EOR && lastc != SP && lastc != TB) {
			*p++ = lastc;
			readchar();
		}
		if (lastc == SP || lastc == TB)
			rdc();
		*p++ = 0;
		if (*thisarg == '<') {
			close(0);
			if (open(&thisarg[1], 0) < 0) {
				printf("%s: cannot open\n", &thisarg[1]);
				_exit(0);
			}
		}
		else if (*thisarg == '>') {
			close(1);
			if (creat(&thisarg[1], 0666) < 0) {
				printf("%s: cannot create\n", &thisarg[1]);
				_exit(0);
			}
		}
		else
			*ap++ = thisarg;
	}
	*ap = NULL;
	execve(symfil, argl, environ);
	perror(symfil);
}

/*
 * install (f != 0) or remove (f == 0) a breakpoint
 */

#define	BPT	03

bkput(bk, f)
register BKPT *bk;
{
	register int sp;

	if (bk->loc < txtsize)
		sp = CORF | INSTSP;
	else
		sp = CORF | DATASP;
	if (f == 0)
		cput(bk->loc, sp, wtoc(bk->ins));
	else {
		bk->ins = ctow(cget(bk->loc, sp));
		cput(bk->loc, sp, wtoc(BPT));
		if (errflg) {
			printf("cannot set breakpoint: ");
			/* stuff */
			prints(errflg);
		}
	}
}

/*
 * wait for the process to stop;
 * pick up status and registers when it does
 */

#define	WSLEEP	10

bpwait()
{
	register int w;
	int stat;
	int (*isig)();
	int nulsig();
	extern int errno;

	isig = signal(SIGINT, SIG_IGN);
	/*
	 * alarm stuff is just in case
	 */
	for (;;) {
		signal(SIGALRM, nulsig);
		alarm(WSLEEP);
		if (ioctl(fcor, PIOCWSTOP, 0) >= 0)
			errno = 0;
		alarm(0);
		if (errno == 0) {
			signal(SIGINT, isig);
			mapimage();
			if (signo == SIGTRAP || signo == SIGSTOP)
				signo = 0;
			else {
				sigprint();
				newline();
			}
			return;		/* should set stuff? */
		}
		if (errno == ENOENT)
			break;
		/* still there, still running.  try again. */
	}
	if (child == 0) {
		close(fcor);
		pid = 0;
		corfil = NULL;
		errflg = ENDPCS;
		return;
	}
	/*
	 * process has died; wait and report status
	 * should check if it's really our child
	 */
	signal(SIGALRM, nulsig);
	alarm(WSLEEP);
	while ((w = wait(&stat)) != -1 && w != pid)
		;
	alarm(0);
	pid = 0;
	signal(SIGINT, isig);
	close(fcor);
	pid = 0;
	corfil = NULL;
	errflg = ENDPCS;
	if (w == -1)
		errflg = BADWAIT;
	else {
		if ((stat & 0177) == 0177)
			printf("trace status?  0%o\n", stat);
		else {
			sigcode = 0;
			if ((signo = stat & 0177) != 0)
				sigprint();
			if (stat & 0200) {
				prints(" - core dumped");
				corfil = "core";
				setcor();
			}
		}
	}
}

static
nulsig()
{
}

#include <pads.pri>
#include <CC/signal.h>
SRCFILE("popen.c")
static int popen_pid[_NFILE];

int pipe(int[2]), wait(int*), vfork(), _exit(int);

FILE *Popen(char *cmd, char *mode)
{
	int parent = (*mode == 'r') ? 0 : 1;
	int child  = (*mode == 'r') ? 1 : 0;
	int p[2];

	if( pipe(p) < 0 ) return NULL;
	int pid = vfork();
	if( pid == 0) {
		dup2(p[child], child);
		setuid(getuid());
		setgid(getgid());
		for( int i = 0; i < _NFILE; ++i )
			if( i != child ) close(i);
		execl("/bin/sh", "sh", "-c", cmd, 0);
		_exit(1);
	}
	if(pid == -1)
		return NULL;
	close(p[child]);
	popen_pid[p[parent]] = pid;
	return(fdopen(p[parent], mode));
}

int Pclose(FILE *ptr)
{
	SIG_TYP hstat, istat, qstat;
	int f, r, status;

	f = fileno(ptr);
	fclose(ptr);
	istat = signal( SIGINT, (SIG_ARG_TYP)SIG_IGN);
	qstat = signal(SIGQUIT, (SIG_ARG_TYP)SIG_IGN);
	hstat = signal( SIGHUP, (SIG_ARG_TYP)SIG_IGN);
	while((r = wait(&status)) != popen_pid[f] && r != -1)
		;
	if(r == -1)
		status = -1;
	signal( SIGINT, (SIG_ARG_TYP)istat);
	signal(SIGQUIT, (SIG_ARG_TYP)qstat);
	signal( SIGHUP, (SIG_ARG_TYP)hstat);
	return(status);
}

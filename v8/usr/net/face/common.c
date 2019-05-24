#include "fserv.h"

char dbgbuf[128];

/*VARARGS*/
debug(s, a, b, c, d, e, f)
{	/* no buffering, shared fd with children */
	long x;
	extern char *ctime();

	x = time(0);
	sprintf(dbgbuf, s, a, b, c, d, e, f);
	strcat(dbgbuf, " ");
	strcat(dbgbuf, ctime(&x));
	write(dbgfd, dbgbuf, strlen(dbgbuf));
}

extern int sys_nerr;
extern char *sys_errlist[];
perror(s)
char *s;
{	register char *c;
	c = "Unknown error";
	if(errno < sys_nerr)
		c = sys_errlist[errno];
	debug("%s: %s (%d)", s, c, errno);
}

debugreset()
{
	dptr = 0;
}

xdebug(s, a, b, c, d, e, f, g)
{
	sprintf(debugbuf[dptr], s, a, b, c, d, e, f, g);
	dptr++;
	if(dptr >= NDBG) {
		debugreset();
		xdebug("buffer wrapped");
	}
}

/* detach from our environment */
detach (logfile)
	char *logfile;
{
	int i;

	/* detach from the process group */	
	setpgrp (0, 0);

	/* ignore some signals */
	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	/* detach from old i/o */
	switch (fork()) {
	case -1:
		perror ("couldn't fork");
		exit (-1);
	case 0:
		for (i = 0; i < NOFILE; i++)
			close (i);
		i = creat (logfile, 0666);
		dup2(i, 2);
		dbgfd = 2;
		close(i);
		break;
	default:
		_exit (0);
	}
}


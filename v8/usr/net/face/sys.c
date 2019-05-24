#include "stdio.h"
#include "fserv.h"
#include "signal.h"
#include "wait.h"
#include "errno.h"

extern int dev;
extern struct stat statb;
extern char *ctime();
extern char *buf, *nbuf;

dummy()
{
	signal(SIGCHLD, dummy);
}

signals()
{	extern int profit();

/*
	signal(SIGINT, SIG_IGN);
*/
	signal(SIGCHLD, dummy);
	signal(SIGTERM, profit);
	signal(SIGPIPE, SIG_IGN);
}

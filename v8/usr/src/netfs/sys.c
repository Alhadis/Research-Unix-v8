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

hup()
{
	signal(SIGHUP, hup);
	fprintf(stderr, "hup %d\n", getpid());
}

signals()
{	extern int profit();
	hup();
	signal(SIGINT, SIG_IGN);
	signal(SIGCHLD, dummy);
	signal(SIGTERM, profit);
	signal(SIGPIPE, SIG_IGN);
	setpgrp(0, getpid());
}

reapchild()
{	int pid, i;
	int status;
	pid = wait3(&status, WNOHANG, 0);
	if(pid <= 0)
		return;
	for(i = 0; i < NORMAN; i++)
		if(children[i].pid == pid) {
			children[i].pid = 0;
			if(status) {	/* bye bye */
				debug("child fd %d status 0x%x", i, status);
				close(children[i].fd);
				children[i].fd = 0;
				children[i].flags = 0;
				if(children[i].who)
					free(children[i].who);
				children[i].who = 0;
				return;
			}
			children[i].flags = CONN;
			children[i].lastheard = time(0);
			FD_SET(i, active);
			debug("wait got %d %s", pid, children[i].who);
			return;
		}
	debug("wait got pid %d, not found", pid);
}

serve(n, argp)	/* ok child[n] */
char *argp;	/* for ps */
{	int i;
	long now;
	if(children[n].flags == UNINIT) {
		doinit(n);
		return;
	}
	myfd = n;
	children[n].pid = fork();
	if(children[n].pid == -1) {	/*can't fork, what a pain*/
		perror(children[n].who);
		debug("couldn't fork, bye");
		return;	/* not enough */
	}
	if(children[n].pid == 0) {
		for(i = 0; i < NOFILE-1; i++)
			if(i != myfd && i != dbgfd)
				close(i);
		debug("child %d started for %s", getpid(), children[n].who);
		dev = children[n].dev;
		newdev(statb.st_dev);
		(void) newnetf("/", -1, 0);
		for(i = 0; argp[i]; i++)
			argp[i] = children[n].who[i];
		work();
	}
	FD_CLR(n, active);
	/* should close n, but how would we know who to kill? */
}

xstat(s, b)
char *s;
struct stat *b;
{	int i;
	i = lstat(s, b);
	if(i < 0)
		return(i);
	if((b->st_mode & S_IFMT) == S_IFCHR || (b->st_mode & S_IFMT) == S_IFBLK)
		b->st_mode &= ~0777;
	if(isremote(b->st_dev)) {
		errno = ELOOP;
		return(-1);
	}
	return(i);
}

isremote(n)	/* temporary, maybe should read /etc/net/friends, or ask sys */
{
	/*if(((n>>8) & 0xff) >= 48)
		return(1);*/	/* let's see what happens with it off */
	return(0);
}

lcllink(q)
netf *q;
{
	getbuf(q->statb.st_size + 1);
	readlink(q->name, buf, q->statb.st_size);
	if(buf[0] != '/')
		return(0);
	buf[q->statb.st_size] = 0;
	getnbuf(q->statb.st_size + 1);
	clrnetf(q->tag);
	strcpy(nbuf, buf);
	return(1);
}

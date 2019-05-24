#ifndef lint
static char sccsid[] = "@(#)rogind.c	4.18 83/07/01";
#endif

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <sgtty.h>
#include <stdio.h>
#include <wait.h>
#include <sys/inet/in.h>
#include "config.h"

extern	errno;
int	reapchild();
int	alrmcatch();
struct	passwd *getpwnam();
struct passwd *pwd;
struct	passwd nouser = {"", "nope", -1, -1, -1, "", "", "", "" };
char	*crypt(), *rindex(), *index(), *malloc();
extern char **environ;
char term[20] = "TERM=";
static char *envinit[] = {
	term,
	0
};
/*
 * remote login server:
 *	remuser\0
 *	locuser\0
 *	terminal type\0
 *	data
 */
main(argc, argv)
	int argc;
	char **argv;
{
	int f, dev;
	unsigned long faddr;
	int myport, fport;
	struct in_service *sp;

	sp = in_service("login", "tcp", 0);
	if(sp == 0){
		fprintf(stderr, "rogind: tcp/rogin: unknown service\n");
		exit(1);
	}
	myport = sp->port;
	close(3);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	f = tcp_sock();
	if (f < 0) {
		perror("rogind: socket");
		exit(1);
	}
	signal(SIGCHLD, reapchild);
	signal(SIGALRM, alrmcatch);
	tcp_listen(f, myport, 0, 0);
	for (;;) {
		int s;

		s = tcp_accept(f, &faddr, &fport, &dev);
		if (s < 0) {
			if (errno == EINTR)
				continue;
			perror("rogind: accept");
			continue;
		}
		doit(s, faddr, fport, dev);
		close(s);
	}
}

reapchild()
{
	union wait status;
	int pid;

	signal(SIGCHLD, SIG_IGN);
	while((pid = wait3(&status, WNOHANG, 0)) > 0)
		rmut(pid);
	signal(SIGCHLD, reapchild);
}

char rusername[32], lusername[32];
char	buf[BUFSIZ];
int	child;
int	cleanup();
int	netf;
extern	errno;

doit(f, faddr, fport, dev)
	int f;
	unsigned long faddr;
{
	extern tty_ld;
	extern char *ttyname();
	char c;
	int pid, ret;
	char *host, line[32];

	sprintf(line, "/dev/tcp%02d", dev);
	alarm(20);
	c = 1;
	read(f, &c, 1);
	alarm(0);
	if(c != 0){
		fprintf(stderr, "non-zero initial read from %x %d\n", faddr, fport);
		return;
	}
	host = (char *)in_host(faddr);
	if (host == 0) {
		msg(f, "Don't know your host name\n");
		fprintf(stderr, "didn't know his host name\n");
		return;
	}
	if(fport >= 1024){
		msg(f, "Permission denied\n");
		fprintf(stderr, "rogind: denied him permission\n");
		return;
	}
	if(access(line, 0) < 0){
		msg(f, "No tty\n");
		fprintf(stderr, "rogind: couldn't access line %s\n", line);
		return;
	}
	pid = fork();
	if (pid < 0){
		msg(f, "Fork problem\n");
		fprintf(stderr, "rogind: fork problen\n");
		return;
	}
	if (pid) {
		record(pid, line);
		return;
	}

	write(f, "", 1);

	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_DFL);
	signal(SIGALRM, SIG_DFL);

	dup2(f, 0);
	dup2(f, 1);
	dup2(f, 2);
	dup2(0, 3);
	close(f);
	ioctl(0, TIOCSPGRP, 0);
	ioctl(0, TCPIOHUP, 0);

	environ = envinit;

	ret = doremotelogin(host);
	ioctl(0, FIOPUSHLD, &tty_ld);
	sttyek(0);
	if(ret < 0)
		execl(LOGIN, "login", 0);
	else
		execl(LOGIN, "login", "-f", pwd->pw_name, 0);
	printf("%s not found\n", LOGIN);
	exit(1);
	/*NOTREACHED*/
}



struct foo{
	int pid;
	char line[16];
};
#define NFOO 50
struct foo foo[NFOO];

record(pid, line)
char *line;
{
	int i;

	for(i = 0; i < NFOO; i++){
		if(foo[i].pid == 0){
			foo[i].pid = pid;
			strncpy(foo[i].line, line+5, 8);
			return;
		}
	}
}

#include <utmp.h>

struct	utmp wtmp;
char	wtmpf[]	= "/usr/adm/wtmp";
char	utmp[] = UTMP;
#define SCPYN(a, b)	strncpy(a, b, sizeof(a))
#define SCMPN(a, b)	strncmp(a, b, sizeof(a))

rmut(pid)
{
	register f;
	int found = 0;
	char *line;
	int i;
	char file[32];

	for(i = 0; i < NFOO; i++){
		if(pid == foo[i].pid){
			line = foo[i].line;
			foo[i].pid = 0;
			break;
		}
	}
	if(i >= NFOO)
		return;
	f = open(utmp, 2);
	if (f >= 0) {
		while(read(f, (char *)&wtmp, sizeof(wtmp)) == sizeof(wtmp)) {
			if (SCMPN(wtmp.ut_line, line) || wtmp.ut_name[0]==0)
				continue;
			lseek(f, -(long)sizeof(wtmp), 1);
			SCPYN(wtmp.ut_name, "");
			time(&wtmp.ut_time);
			write(f, (char *)&wtmp, sizeof(wtmp));
			found++;
		}
		close(f);
	}
	if (found) {
		f = open(wtmpf, 1);
		if (f >= 0) {
			SCPYN(wtmp.ut_line, line+5);
			SCPYN(wtmp.ut_name, "");
			time(&wtmp.ut_time);
			lseek(f, (long)0, 2);
			write(f, (char *)&wtmp, sizeof(wtmp));
			close(f);
		}
	}
	sprintf(file, "/dev/%s", line);
	chmod(file, 0600);
	chown(file, 0, 0);
}

msg(fd, s)
char *s;
{
	write(fd, "\001", 1);
	write(fd, s, strlen(s));
}

alrmcatch()
{
	signal(SIGALRM, alrmcatch);
	return(-1);
}

sttyek(fd)
{
	struct sgttyb vec;

	gtty(fd, &vec);
	vec.sg_erase = '#';
	vec.sg_kill = '@';
	vec.sg_flags |= XTABS;
	stty(fd, &vec);
}

doremotelogin(host)
	char *host;
{
	FILE *hostf;
	int first = 1;
	char *p;

	getstr(rusername, sizeof (rusername), "remuser");
	getstr(lusername, sizeof (lusername), "locuser");
	getstr(term+5, sizeof(term)-5, "Terminal type");
	if(p = (char *)rindex(term, '/'))
		*p = '\0';
	if (getuid()) {
		pwd = &nouser;
		goto bad;
	}
	setpwent();
	pwd = getpwnam(lusername);
	endpwent();
	if (pwd == NULL) {
		pwd = &nouser;
		goto bad;
	}
	hostf = pwd->pw_uid ? fopen(EQUIV, "r") : 0;
again:
	if (hostf) {
		char ahost[32];

		while (fgets(ahost, sizeof (ahost), hostf)) {
			char *user;

			if ((user = index(ahost, '\n')) != 0)
				*user++ = '\0';
			if ((user = index(ahost, ' ')) != 0)
				*user++ = '\0';
			if (!strcmp(host, ahost) &&
			    !strcmp(rusername, user ? user : lusername)) {
				fclose(hostf);
				return (1);
			}
		}
		fclose(hostf);
	}
	if (first == 1) {
		char *rhosts = ".rhosts";
		struct stat sbuf;

		first = 0;
		if (chdir(pwd->pw_dir) < 0)
			goto again;
		if (lstat(rhosts, &sbuf) < 0)
			goto again;
		if ((sbuf.st_mode & S_IFMT) == S_IFLNK) {
			printf("login: .rhosts is a soft link.\r\n");
			goto bad;
		}
		hostf = fopen(rhosts, "r");
		fstat(fileno(hostf), &sbuf);
		if (sbuf.st_uid && sbuf.st_uid != pwd->pw_uid) {
			printf("login: Bad .rhosts ownership.\r\n");
			fclose(hostf);
			goto bad;
		}
		goto again;
	}
bad:
	return (-1);
}

getstr(buf, cnt, err)
	char *buf;
	int cnt;
	char *err;
{
	char c;

	do {
		if (read(0, &c, 1) != 1)
			exit(1);
		if (--cnt < 0) {
			printf("%s too long\r\n", err);
			exit(1);
		}
		*buf++ = c;
	} while (c != 0);
}

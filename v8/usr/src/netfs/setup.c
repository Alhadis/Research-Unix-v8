#include "stdio.h"
#include "sys/param.h"
#include "sys/stat.h"
#include "sys/neta.h"
#include "errno.h"
#include "sys/ioctl.h"
#include "signal.h"
#include "setjmp.h"

#define RMFSTYP 1	/* file system type for remote file system */

struct friend {
	char *who;
	char *mount;
	int dev;
	int silent;
	int status;
	int chan;
} friends[NOFILE];
int nfriends;
#define DEAD	0
#define OK	1
#define PROBE	2
#define KILL 	3

char *states[] = { "dead", "ok", "probe", "kill", "?"};

struct senda x;
struct rcva y;
extern int errno;
extern char *ctime();
extern char *malloc();
struct stat stb;
long looked;
jmp_buf jmpbuf;

main()
{	int i;
/* any signal but SIGALRM (14) will start it over.  SIGALRM is used to
 * interrupt hung reads.  SIGCHLD is ignored too */
	sigalrm(0);
	if(setjmp(jmpbuf) == 1)
		rdfile();
	else
		sigcatch();
	probe();
}
sigalrm(n)
{
	fprintf(stderr, "sigalrm %d\n", n);
	fflush(stderr);
	if(n == SIGALRM || n == 0)	/* otherwise might get SIGCHLD in sleep */
		signal(SIGALRM, sigalrm);
	signal(SIGCHLD, sigalrm);
}
sigcatch(n)
{	int i;
	for(i = 1; i < NSIG; i++)
		if(i != SIGALRM && i != SIGCHLD)	/* alarm and tdkdial */
			(void) signal(i, sigcatch);
	fprintf(stderr, "sigcatch %d\n", n);
	fflush(stderr);
	switch(i) {
	case SIGILL: case SIGBUS: case SIGSEGV:
		restart();
	}
	longjmp(jmpbuf, 1);
}
restart()
{
	fprintf(stderr, "execing\n");
	fflush(stderr);
	execl("/etc/net/setup", "setup", 0);
	execl("/usr/net/setup", "setup", 0);
	/* if that doesn't work */
	fprintf(stderr, "oops, calling main\n");
	fflush(stderr);
	main();		/* ho ho */
}
probe()
{	int i;
	static int lastdone;
	long now;
loop:
	if(lastdone >= nfriends) {
		lastdone = 0;
		sleep(30);
		if(stat("/etc/net/friends", &stb) == 0 && stb.st_mtime > looked)
			rdfile();
		if(stat("/usr/net/friends", &stb) == 0 && stb.st_mtime > looked)
			rdfile();
		if(time((long *)0) > looked + 1200)
			rdfile();
	}
	i = lastdone++;
	switch(friends[i].status) {
	default:
		break;
	case PROBE:
	case OK:
		friends[i].status++;
		doprobe(friends + i);
		if(friends[i].status != OK) {
			now = time(0);
			fprintf(stderr, "hmm %s %s %s\n", friends[i].who,
				states[friends[i].status], ctime(&now));
		}
		break;
	case KILL:	/* over the limit */
		now = time(0);
		fprintf(stderr, "%s timed out %s 0x%x %s", friends[i].who,
			states[friends[i].status], friends[i].chan, ctime(&now));
		errno = 0;
		gmount(RMFSTYP, friends[i].dev, 1, 0, 0);
		if(errno == 0 || errno == EINVAL) {
			friends[i].status = DEAD;
			fprintf(stderr, "dead now\n");
		}
		else
			perror("xmount");
		break;
	}
	goto loop;
}	
doprobe(x)
struct friend *x;
{	struct stat stb;
	if(stat(x->mount, &stb) == 0 && stb.st_dev == x->dev)
		x->status = OK;
}
startup(p)
struct friend *p;
{	int fd, i;
	char version;
	fd = callfs(p->who);
	if(fd < 0) {
		fprintf(stderr, "callfs %s failed\n", p->who);
		return;
	}
	(void) fstat(fd, &stb);
	fprintf(stderr, "starting %s (0x%x)", p->who, stb.st_rdev);
	gmount(RMFSTYP, p->dev, 1, 0, 0);	/* just in case */
	perror("xunmount");
	x.trannum = 0;
	version = x.version = NETVERSION;
	x.cmd = NSTART;
	x.uid = p->silent;
	x.dev = p->dev;
	alarm(30);
	x.ta = time(0);
	if((write(fd, &version, 1) != 1) || write(fd, (char *)&x, sizeof(x))
		!= sizeof(x)) {
			perror("write in setup");
			fprintf(stderr, "%s\n", p->who);
			close(fd);
			alarm(0);
			return;
	}
	if((i = read(fd, (char *)&y, sizeof(y))) != sizeof(y)) {
		if(y.trannum == -1)
			fprintf(stderr, "version mismatch %s\n", p->who);
		else {
			fprintf(stderr, "read %d chars for %s\n", i, p->who);
			perror((char *)&y);
		}
		close(fd);
		return;
	}
	alarm(0);
	if(y.errno != 0) {
		errno = y.errno;
		perror("nak in setup");
		close(fd);
		return;
	}
	if(gmount(RMFSTYP, p->dev, 0, fd, p->mount) != 0) {
		perror("xmount");
		fprintf(stderr, "unmounting it\n");	/* garbage collection*/
		gmount(RMFSTYP, p->dev, 1, 0, 0);
		perror("xunmount");
		close(fd);
		/* and try again later */
		return;
	}
	p->status = OK;
	fprintf(stderr, "started %s on %s dev %d silent %d chan 0x%x\n", p->who,
		p->mount, p->dev/256, p->silent, p->chan = stb.st_rdev);
	close(fd);
}

#define skip for(; *p == ' ' || *p == '\t'; p++)
#define note for(s = p; *p != ' ' && *p != '\t' && *p != '\n'; p++)
struct friend *
cnvt(s)
char *s;
{	static struct friend x;
	char *p;
	s[strlen(s)] = '\n';
	p = s;
	skip;
	if(*p == '\n') {
		x.who = "#";
		return(&x);
	}
	note;
	if(*p == '\n')
		return(0);
	*p++ = 0;
	x.who = malloc(strlen(s) + 1);
	strcpy(x.who, s);
	s = p;
	skip;
	note;
	if(*p == '\n')
		return(0);
	*p++ = 0;
	x.mount = malloc(strlen(s) + 1);
	strcpy(x.mount, s);
	s = p;
	skip;
	note;
	if(*p == '\n')
		return(0);
	*p++ = 0;
	x.dev = 256 * atoi(s);
	s = p;
	skip;
	note;
	*p++ = 0;
	x.silent = atoi(s);
	x.status = DEAD;
	return(&x);
}

rdfile()
{	FILE *fd;
	int i;
	char line[128];
	struct friend *p;
	alarm(0);		/* disable the sleep in probe */
	fd = fopen("/etc/net/friends", "r");
	if(fd == NULL) {
		fd = fopen("/usr/net/friends","r");
		if(fd == NULL) {
			perror("friends");
			exit(1);
		}
	}
	for(;;) {
onward:
		(void) fgets(line, sizeof(line), fd);
		if(feof(fd))
			break;
		p = cnvt(line);
		if(p == 0) {
			fprintf(stderr, "weird friends line %s\n", line);
			continue;
		}
		if(p->who[0] == '#')
			continue;
		for(i = 0; i < nfriends; i++) {
			if(strcmp(p->who, friends[i].who))
				continue;
			if(friends[i].status != OK) {
				looked = time(0);
				fprintf(stderr, "\nrdfile %s (%s) %s",
					states[friends[i].status],
					friends[i].who, ctime(&looked));
				/* off the mother */
				if(friends[i].status != DEAD) {
					friends[i].status = KILL;
					fprintf(stderr, "bye %s\n", friends[i].who);
				}
			}
			if(friends[i].status != DEAD)
				goto onward;
			/* one could free the space */
			fprintf(stderr, "new friend %s\n", p->who);
			friends[i] = *p;
			startup(friends + i);
			goto onward;
		}
		for(i = 0; i < nfriends; i++)
			if(friends[i].status == DEAD) {
				friends[i] = *p;
				fprintf(stderr, "new friend %s\n", p->who);
				startup(friends + i);
				goto onward;
			}
		if(i < nfriends)
			continue;
		if(nfriends >= NOFILE) {
			fprintf(stderr, "friends overflow\n");
			abort();
		}
		fprintf(stderr, "new friend %s\n", p->who);
		friends[nfriends++] = *p;
		startup(friends + nfriends - 1);
	}
	fclose(fd);
	looked = time(0);
	fprintf(stderr, "ok %s", ctime(&looked));
}

callfs(srvr)
char *srvr;
{
	int rem, v;
	extern int pk_ld, dkp_ld;
	long x;

	alarm(30);
	x = time(0);
	fprintf(stderr, "%d tdkdial %s %s", getpid(), srvr, ctime(&x));
	fflush(stderr);
	rem = _tdkdial(3, 2, srvr);
	x = time(0);
	alarm(0);
	if (rem < 0) {
		fprintf(stderr, " %d can't rexec err = %d\n", getpid(), errno);
		fflush(stderr);
		return(-1);
	}
	else
		fprintf(stderr, "tdkdial ok %s", ctime(&x));
	/* v = ioctl(rem, FIOPUSHLD, &dkp_ld);	/* should be dkproto */
	v = dkproto(rem, dkp_ld);
	x = time(0);
	if (v < 0) {
		perror("can't start protocol");
		fprintf(stderr, "\t%s", ctime(&x));
		close(rem);
		return(-1);
	}
	else
		fprintf(stderr, "protocol started\n");
	return(rem);
}

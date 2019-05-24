/* file service */
#include "fserv.h"
#include "errno.h"
#include "sys/neta.h"

int lflag;	/* means /-prefixed symbolic links are interpreted by server */
char me[16];
char *service;
char cmdbuf[256];
struct rcva y, nilrcv;
struct stat statb, rootstat;
int intrcnt, timecnt, okcnt;
fd_set svrdmask;

main(argc, argv)
char **argv;
{	int n, i;
	fd_set rdmask;
	if(argc > 1 && strcmp(argv[1], "-l") == 0)
		lflag = 1;
	dbgfd = 2;
	close(0);
	close(1);
	whoami();
	service = malloc(strlen(me) + 3);
	strcpy(service, me);
	strcat(service, "F");
	debug1("%s", service);
	umask(0);
	perminit();
	signals();
	(void) stat("/", &rootstat);
	statb = rootstat;
	announce(service);
loop:
	if(cntlfd < 0) {	/* datakit died */
		debug("announcing");
		for(i = 0; i < 32; i++)
			if(i != dbgfd)
				close(i);
		announce(service);
	}
	reapchild();
	errno = 0;
	rdmask = active;
	debug2("selecting x%x", rdmask);
	n = select(NOFILE, &rdmask, 0, 20000);
	if(n == -1 && errno == EINTR) {
		intrcnt++;
		goto loop;
	}
	else if(n == -1) {
		perror("select");
		goto loop;		/* probably an infinite loop? */
	}
	else if(n == 0) {
		debug2("timeout");
		timecnt++;
		permredo();
		goto timer;		/* nothing's going on? */
	}
	else {
		okcnt++;
		svrdmask = rdmask;
		debug2("got rdmask %lx %lx", rdmask.fds_bits[0], rdmask.fds_bits[1]);
	}
	for(i = NOFILE - 1; i >= 0; i--)
		if(FD_ISSET(i, rdmask))
			if(i == cntlfd)
				newone();
			else {
				children[i].lastheard = time(0);
				serve(i, argv[0]);
			}
timer:
	for(i = 0; i < NOFILE; i++)
		if(FD_ISSET(i, active)) {
			if(i == cntlfd)
				continue;
			if(children[i].lastheard + 500 > time(0))
				continue;
			debug("%s 500 seconds", children[i].who);
			/*close(children[i].fd);
			children[i].fd = children[i].flags = 0;
			if(children[i].who)
				free(children[i].who);
			children[i].who = 0;
			FD_CLR(i, active);
			*/
			children[i].lastheard = time(0);
		}
	goto loop;
}

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

whoami()
{	int fd, n;
	dbgfd = 2;	/* until logging starts */
	fd = open("/etc/whoami", 0);
	if(fd < 0) {
foof:
		perror("/etc/whoami");
		exit(1);
	}
	if((n = read(fd, me, sizeof(me))) == -1)
		goto foof;
	if(n >= sizeof(me)) {
		debug("/etc/whoami too long");
		exit(1);
	}
	close(fd);
	if(me[n - 1] == '\n')
		me[n - 1] = 0;
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


/* we expect one byte of version number followed by a senda with 
 * initialization stuff */
doinit(n)
{	struct senda *x;
	int i;
	i = read(children[n].fd, cmdbuf, sizeof(cmdbuf));
	x = (struct senda *)cmdbuf;
	debug("doinit read %d on %d for %d", i, children[n].fd, n);
	if(i != 1 || x->version != NETVERSION) {
		debug("read %d chars [0x%x] for child %d on fd %d",
			i, x->version, n, children[n].fd);
		goto awful;
	}
	i = read(children[n].fd, cmdbuf, sizeof(cmdbuf));
	x = (struct senda *)cmdbuf;
	debug("doinit read %d on %d for %d", i, children[n].fd, n);
	if(i != sizeof(struct senda) || x->version != NETVERSION) {
		if(x->version != NETVERSION) {
			y.trannum = -1;
			write(children[n].fd, (char *)&y, sizeof(struct rcva));
			debug("got version %d for %s", x->version, NETVERSION);
		}
		else
			debug("doinit, read %d on %d", i, children[n].fd);
awful:
		/* panic, we may get an endless stream */
		respond(0);
		close(children[n].fd);
		FD_CLR(children[n].fd, active);
		return;
	}
	children[n].dtime = x->ta - time(&children[n].lastheard);
	children[n].dev = x->dev;
	children[n].silent = x->uid;
	y.trannum = x->trannum;
	myfd = children[n].fd;
	respond(0);
	children[n].flags = CONN;
	debug("%s dev %d silent %d", children[n].who, children[n].dev / 256,
		children[n].silent);
}

respond(n)
{
	if((y.errno = n) && silent) {
		errno = n;
		perror("respond");
	}
	(void) write(children[myfd].fd, (char *)&y, sizeof(y));
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

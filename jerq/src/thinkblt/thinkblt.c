#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include "thinkblt.h"

#define Signal(s, f)	((signal(s, SIG_IGN) == SIG_IGN) ? SIG_IGN : signal(s, f))

#define SELTIMO	0x7fff
#define PARGVAL	((*argv)[2] ? (*argv)+2 : --argc ? *++argv : (char *)0)

#define putch(c)	(cbuf[0]=(c), write(1, cbuf, 1))

char *jpgm = { "/usr/jerq/mbin/thinkblt.m" }; int zflag = 0;

extern int errno;

char ptname[32], linkbuf[64], *linkname = 0;
char *getenv(), *strcat(), *strcpy();
unsigned char cbuf[240];

fd_set fdset, fdbits;
int infd, keepfd, ready, sighup();

main(argc,argv)
int argc; char **argv;
{
	int n;
	Signal(SIGHUP, sighup);
	while (--argc > 0) {
		if ((*++argv)[0] == '-') switch ((*argv)[1]) {
		case 'j':
			jpgm = PARGVAL; break;
		case 'z':
			zflag++; break;
		default:
			fprintf(stderr,"unknown option %s\n",*argv);
			exit(1);
		} else {
			if (linkname == 0)
				linkname = *argv;
		}
	}

	ptinit();
	if (jload(jpgm,zflag))
		Exit(1);
	rawtty(1);
	putch(0); putch(255);
	n = strlen(linkname);
	cbuf[0] = n>>8;
	cbuf[1] = n;
	strcpy(&cbuf[2], linkname);
	cbuf[n+2] = 0;
	cbuf[n+3] = 0;
	write(1, cbuf, n+4);
	ready = 1;
	n = 0;
	FD_SET(0, fdset);
	for (;;) {
		fdbits = fdset;
		if (ready)
			FD_SET(infd, fdbits);
		while (select(infd + 1, &fdbits, (fd_set *)0, SELTIMO) == -1)
			if (errno != EINTR)
				Exit(1);
		if (FD_ISSET(0, fdbits)) {
			read(0, cbuf, sizeof cbuf);
			switch (*cbuf) {
			case EXIT:
				Exit(0);
			case ABORT:
				ptinit();
				if (n > 0) {
					cbuf[0] = cbuf[1] = 0;
					write(1, cbuf, 2);
				}
				ready = 1;
				continue;
			case READY:
				ready = 1;
				break;
			}
		}
		if (FD_ISSET(infd, fdbits)) {
			n = read(infd, &cbuf[2], (sizeof cbuf)-2);
			if (n < 0)
				n = 0;
			cbuf[0] = n>>8;
			cbuf[1] = n;
			write(1, cbuf, n+2);
			if (n > 0)
				ready = 0;
		}
	}
}

ptinit()
{
	int ptfd;
	if ((ptfd = ptopen(ptname)) < 0) {
		fprintf(stderr, "no streams\n");
		Exit(1);
	}
	if (keepfd)
		close(keepfd);
	if (infd)
		close(infd);
	infd = ptfd;
	if ((keepfd = open(ptname, 2)) < 0) {
		fprintf(stderr, "cannot open %s\n", ptname);
		Exit(1);
	}
	if (linkname == 0)
		linkname = getenv("THINK");
	if (linkname == 0 && (linkname = getenv("HOME")))
		linkname = strcat(strcpy(linkbuf, linkname), "/.THINK");
	if (linkname) {
		unlink(linkname);
		if (symlink(ptname, linkname) < 0) {
			fprintf(stderr, "cannot symlink %s to %s\n",
			    ptname, linkname);
			Exit(1);
		}
	} else
		linkname = ptname;
}

Exit(n)
{
	if (linkname && linkname != ptname)
		unlink(linkname);
	rawtty(0);
	exit(n);
}

sighup(s)
{
	if (linkname && linkname != ptname)
		unlink(linkname);
	_exit(1);
}

jload(prog,zflag)
char *prog; int zflag;
{
	static char *cmd[] = { "32ld", (char *)0, (char *)0, (char *)0 };
	if (zflag) { cmd[1] = "-z"; cmd[2] = prog; }
	else { cmd[1] = prog; cmd[2] = (char *)0; }
	if (systemv("/usr/jerq/bin/32ld",cmd)) return 1;
	sleep(2);
	return 0;
}

systemv(name,argv)
char *name, **argv;
{
	int status, pid, l;
	if ((pid = fork()) == 0) { execv(name,argv); _exit(127); }
	while ((l = wait(&status)) != pid && l != -1);
	return (l == -1) ? -1 : status;
}

#include <sgtty.h>

rawtty(flag)
{
	static struct sgttyb ttyb;
	static int tty_flags, tty_raw = 0, was_ld;
	extern int tty_ld;

	if (flag == tty_raw) return;

	if (flag) {
		ioctl(0, TIOCGETP, &ttyb);
		tty_flags=ttyb.sg_flags;
		ttyb.sg_flags |=  RAW;
		ttyb.sg_flags &= ~ECHO;
		ioctl(0, TIOCSETP, &ttyb);
		if ((was_ld = ioctl(0, FIOLOOKLD, 0)) == tty_ld)
			ioctl(0, FIOPOPLD, 0);
	} else {
		if (was_ld == tty_ld)
			ioctl(0, FIOPUSHLD, &tty_ld);
		ttyb.sg_flags=tty_flags;
		ioctl(0, TIOCSETP, &ttyb);
	}
	tty_raw = flag;
}

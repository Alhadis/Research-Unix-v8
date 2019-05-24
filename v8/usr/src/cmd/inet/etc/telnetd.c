#ifndef lint
static char sccsid[] = "@(#)telnetd.c	4.26 (Berkeley) 83/08/06";
#endif

/*
 * Stripped-down telnet server.
 */
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
#include <sys/ttyld.h>
#include <sys/stream.h>
#include "config.h"
#include "telnet.h"

#define	BELL	'\07'
#define DEBUG if (debug)

/* option settings (remote and local) */
static char	hisopts[256];
static char	myopts[256];

/* formats for option messages */
static char	doopt[] = { IAC, DO, '%', 'c', 0 };
static char	dont[] = { IAC, DONT, '%', 'c', 0 };
static char	will[] = { IAC, WILL, '%', 'c', 0 };
static char	wont[] = { IAC, WONT, '%', 'c', 0 };

static int	ptno;		/* number of the pt */
static int 	ptfd, netfd;	/* fd's to pt and tcp */
static int	done;		/* true if session is to be ended */
static int	debug;		/* true if debugging is to be output */
static struct sgttyb ptb;	/* result of an IOCGETP on the pt */
static struct tchars ptt;	/* result of an IOCGETC on the pt */

/* predefined */
static 	reapchild();
static	catchint();	

/* imported */
extern	char **environ;
extern	int errno;
extern	char *strrchr(), *strchr();
 

/*
 *	The following macros and routines are used to
 *	manage the I/O buffers.  They're a cross between
 *	stream buffers and standard I/O.
 */
struct buffer {
	char *b_rp;		/* read pointer */
	char *b_wp;		/* write pointer */
	char b_buf[BUFSIZ];	/* the buffer */
};
struct buffer ptibuf, *ptin = &ptibuf;
struct buffer ptobuf, *ptout = &ptobuf;
struct buffer netibuf, *netin = &netibuf;
struct buffer netobuf, *netout = &netobuf;

#define binit(bp) (bp->b_rp = bp->b_wp = bp->b_buf)
#define bytes_filled(bp) (bp->b_wp - bp->b_rp)
#define space_left(bp) (bp->b_buf+sizeof(bp->b_buf) - bp->b_wp)
#define bput(bp, c) (*(bp->b_wp++) = c)
#define bget(bp) (*(bp->b_rp++) & 0377)

/* read whatever the buffer can take */
static int
bread(bp, fd)
	struct buffer *bp;
	int fd;
{
	int cc;

	/* normalize the buffer */
	if (bytes_filled(bp) == 0)
		bp->b_rp = bp->b_wp = bp->b_buf;

	/* fill it */
	cc = read(fd, bp->b_wp, space_left(bp));
	if (cc > 0)
		bp->b_wp += cc;
	return cc;
}

/* read at most n bytes */
static int
breadn(bp, fd, n)
	struct buffer *bp;
	int fd, n;
{
	int cc;

	/* normalize the buffer */
	if (bytes_filled(bp) == 0)
		bp->b_rp = bp->b_wp = bp->b_buf;
	if (n > space_left(bp))
		n = space_left(bp);

	/* fill it */
	cc = read(fd, bp->b_wp, n);
	if (cc > 0)
		bp->b_wp += cc;
	return cc;
}

/* empty the buffer */
static int
bwrite(bp, fd)
	struct buffer *bp;
	int fd;
{
	int cc;

	
	cc = write(fd, bp->b_rp, bytes_filled(bp));

	/* normalize the buffer */
	if (cc == bytes_filled(bp))
		binit(bp);
	return cc;
}
static int
bputs(bp, s)
	struct buffer *bp;
	char *s;
{
	while (*s)
		bput(bp, *s++);
}

/*
 *	Establish a tcp socket and fork off a process for each connection.
 */
main(argc, argv)
	char *argv[];
{
	int pid;
	int f, dev;
	unsigned long faddr;
	int myport, fport;
	struct in_service *sp;

	sp = in_service("telnet", "tcp", 0);
	if (sp == 0) {
		fprintf(stderr, "telnetd: tcp/telnet: unknown service\n");
		exit(1);
	}
	myport = sp->port;
	argc--, argv++;
	if (argc > 0 && !strcmp(*argv, "-d")) {
		debug++;
		argc--, argv++;
	}
	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	do {
		f = tcp_sock();
		if (f < 0) {
			perror("telnetd: socket");;
			sleep(5);
			continue;
		}
		if (tcp_listen(f, myport, 0, 0) < 0) {
			close(f);
			f = -1;
			sleep(30);
			continue;
		}
	} while (f < 0);

	signal(SIGCHLD, reapchild);
	for (;;) {
		int s;

		s = tcp_accept(f, &faddr, &fport, &dev);
		if (s < 0) {
			if (errno == EINTR)
				continue;
			perror("telnetd: accept");
			sleep(1);
			continue;
		}
		switch(fork()) {
		case -1:
			printf("Out of processes\n");
			break;
		case 0:
			doit(s, faddr, fport, dev);
			exit(0);
		}
		close(s);
	}
	/*NOTREACHED*/
}

static
reapchild()
{
	union wait status;

	while (wait3(&status, WNOHANG, 0) > 0)
		;
}

char	*envinit[] = { "TERM=network", 0 };

/*
 *	Get a pt.  Put a login on one side and a telnet receiver on
 *	the other.
 */
static
doit(f, faddr, fport, dev)
	int f;
	unsigned long faddr;
{
	char *host, *in_ntoa(), *ptname();
	int pfd[2];
#	define TERMEND pfd[0]
#	define PROCEND pfd[1]
	extern int tty_ld, mesg_ld;

	/* get a pt pair */
	if ((ptno = ptpipe(pfd)) < 0)
		fatalperror(f, "out of pts", errno);

	host = (char *)in_host(faddr);
	if (host == 0)
		host = in_ntoa(faddr);
	DEBUG fprintf(stderr, "telnet from %s: login on %s\n", host, ptname(ptno));
	DEBUG fprintf(stderr, "telnetd pid = %d\n", getpid());


	/* get the pt's characteristics */
	ioctl(PROCEND, TIOCGETC, &ptt);
	ioctl(PROCEND, TIOCGETP, &ptb);

	/* make it really look like a terminal */
	ioctl(TERMEND, FIOPUSHLD, (struct sgttyb *)&mesg_ld);
	ioctl(PROCEND, FIOPUSHLD, (struct sgttyb *)&tty_ld);

	/* prepare for the death of a child */
	signal(SIGCHLD, catchint);
	switch (fork()) {
	case -1:
		fatalperror(f, "fork", errno);
	case 0:
		/* a process which is the remote login */
		getty(PROCEND);
	default:
		/* the protocol process */
		close(PROCEND);
		netfd = f;
		ptfd = TERMEND;
		telnet();
	}

	/*NOTREACHED*/
}

static
fatal(f, msg)
	int f;
	char *msg;
{
	char buf[BUFSIZ];

	(void) sprintf(buf, "telnetd: %s.\n", msg);
	(void) write(f, buf, strlen(buf));
	exit(1);
}

static
fatalperror(f, msg, errno)
	int f;
	char *msg;
	int errno;
{
	char buf[BUFSIZ];
	extern char *sys_errlist[];

	(void) sprintf(buf, "%s: %s", msg, sys_errlist[errno]);
	fatal(f, buf);
}

static
terminate(s)
	char *s;
{
	DEBUG fprintf(stderr, "session on %s terminated because of %s\n",
			ptname(ptno), s);
	done = 1;
}

/* loop on input from the pt and the network */
static
telnet()
{
	register int c;
	int n;
	fd_set ibits, obits;

	binit(ptin);
	binit(ptout);
	binit(netin);
	binit(netout);

	/* Request to do remote echo. */
	dooption(TELOPT_ECHO);
	myopts[TELOPT_ECHO] = 1;

	while(!done) {
		FD_ZERO(ibits);
		FD_ZERO(obits);

		/* process anything sitting in the input buffers */
		if (bytes_filled(ptin) > 0)
			ptprocess();
		if (bytes_filled(netin) > 0)
			netprocess();

		/* select for read only if there's room to read into */
		if (space_left(netin) > 0)
			FD_SET(netfd, ibits);
		if (space_left(ptin) > 0)
			FD_SET(ptfd, ibits);

		/* select for write only if there's something to write */
		if (bytes_filled(ptout) > 0)
			FD_SET(ptfd, obits);
		if (bytes_filled(netout) > 0)
			FD_SET(netfd, obits);

		n = select(NOFILE, &ibits, &obits, 100000);
		if (n < 0)
			break;
		else if (n == 0)
			continue;

		/* fill input buffers */
		if (FD_ISSET(netfd, ibits))
			netrcv();
		if (FD_ISSET(ptfd, ibits))
			ptrcv();

		/* flush output buffers */
		if (FD_ISSET(netfd, obits) && bytes_filled(netout) > 0)
			netflush();
		if (FD_ISSET(ptfd, obits) && bytes_filled(ptout) > 0)
			ptflush();
	}
	rmut();
	close(netfd);
	close(ptfd);
	exit(0);
}

struct mesg {
	short type;
	short size;
};
static struct mesg m;

/* read from the pt */
static
ptrcv()
{
	/* get the header if we don't already have one */
	if (m.size <= 0)
		if (read(ptfd, &m, sizeof(m)) != sizeof(m)) {
			terminate("pt read");
			return;
		}
	switch(m.type) {
	case M_HANGUP:
		terminate("pt hangup");
		break;
	case M_DATA:
		datamesg();
		break;
	case M_IOCTL:
		ioctlmesg();
		break;
	case M_IOCACK:
		fprintf(stderr, "IOCACK\n");
		flushmesg();
		break;
	case M_IOCNAK:
		fprintf(stderr, "IOCNAK\n");
		flushmesg();
		break;
	default:
		othermesg();
		break;
	}
}

/* flush a message from the pt */
static
flushmesg()
{
	union stmsg s;
	int cc;

	/* flush it */
	while (m.size > 0) {
		fprintf(stderr, "flush\n");
		cc = read(ptfd, &s, sizeof(s));
		if (cc < 0) {
			terminate("pt read");
			return;
		}
		m.size -= cc;
	}
}

/* handle an ioctl message from the pt */
static
ioctlmesg()
{
	struct mesg rm;
	union stmsg s;
	int cc;

	rm.size = m.size;
	if (m.size > 0) {
		cc = read(ptfd, &s, sizeof(s));
		if (cc < 0) {
			terminate("pt read");
			return;
		}
		m.size -= cc;
	}
	rm.type = M_IOCACK;
	switch (s.ioc0.com) {

	case TIOCSETN:
	case TIOCSETP:
		ptb = s.ioc1.sb;
		rm.size = 0;
		break;

	case TIOCGETP:
		s.ioc1.sb.sg_ispeed = B9600;
		s.ioc1.sb.sg_ospeed = B9600;
		ptb = s.ioc1.sb;
		break;

	default:
		rm.type = M_IOCNAK;
		rm.size = 0;
		break;
	}
	if (write(ptfd, &rm, sizeof(rm)) != sizeof(rm)) {
		terminate("pt write");
		return;
	}
	if (rm.size > 0)
		if (write(ptfd, &s, rm.size) != rm.size) {
			terminate("pt write");
			return;
		}

	flushmesg();
}

/* read bytes from the pt and write to the network connection */
static
datamesg()
{
	int cc;

	cc = breadn(ptin, ptfd, m.size);
	if (cc < 0)
		terminate("pt read");
	else
		m.size -= cc;
}

/* handle an unrecognized type of message */
static
othermesg()
{
	struct mesg rm;
	char buf[132];
	int rcc, wcc;

	wcc = write(ptfd, &m, sizeof(m));
	if (wcc != sizeof(m)) {
		terminate("pt write");
		return;
	}
	while (m.size > 0) {
		rcc = read(ptfd, buf, sizeof(buf));
		if (rcc < 0) {
			terminate("pt read");
			return;
		}
		wcc = write(ptfd, buf, rcc);
		if (wcc != rcc) {
			terminate("pt write");
			return;
		}
		m.size -= rcc;
	}
}

/* set pt mode */
static
mode(on, off)
	int on, off;
{
	ptflush();
	ptb.sg_flags |= on;
	ptb.sg_flags &= ~off;
	if(ioctl(ptfd, TIOCSETP, &ptb)<0)
		terminate("setting mode of pt");
}

/* flush the pt's output buffer */
ptflush()
{
	struct mesg rm;

	rm.type = M_DATA;
	rm.size = bytes_filled(ptout);
	if (rm.size > 0) {
		if (write(ptfd, &rm, sizeof(rm)) < sizeof(rm))
			terminate("pt write");
		else if (bwrite(ptout, ptfd) <= 0)
			terminate("pt write");
	}
}

/* process bytes from the pt */
static
ptprocess()
{
	register int c;

	while(bytes_filled(ptin) && space_left(netout) > 1) {
		c = bget(ptin);
		if (c == IAC)
			bput(netout, c);
		bput(netout, c);
	}
}

/* read bytes from the net */
static
netrcv()
{
	if (bread(netin, netfd) < 0)
		terminate("net read");
}

/* flush the netwrk's output buffer */
netflush()
{
	if (bwrite(netout, netfd) < 0)	
		terminate("net write");
}
	
/*
 * State for recv fsm
 */
#define	TS_DATA		0	/* base state */
#define	TS_IAC		1	/* look for double IAC's */
#define	TS_CR		2	/* CR-LF ->'s CR */
#define	TS_BEGINNEG	3	/* throw away begin's... */
#define	TS_ENDNEG	4	/* ...end's (suboption negotiation) */
#define	TS_WILL		5	/* will option negotiation */
#define	TS_WONT		6	/* wont " */
#define	TS_DO		7	/* do " */
#define	TS_DONT		8	/* dont " */

static
netprocess()
{
	static int state = TS_DATA;
	register int c;
	char buf[10];

	while (bytes_filled(netin)) {
		c = bget(netin);
		switch (state) {

		case TS_DATA:
			if (c == IAC) {
				state = TS_IAC;
				break;
			}
			bput(ptout, c);
			if (!myopts[TELOPT_BINARY] && c == '\r')
				state = TS_CR;
			break;

		case TS_CR:
			if (c && c != '\n')
				bput(ptout, c);
			state = TS_DATA;
			break;

		case TS_IAC:
			switch (c) {

			/*
			 * Send the process on the pty side an
			 * interrupt.  Do this with a NULL or
			 * interrupt char; depending on the tty mode.
			 */
			case BREAK:
			case IP:
				interrupt();
				break;

			/*
			 * Are You There?
			 */
			case AYT:
				bput(ptout, BELL);
				break;

			/*
			 * Erase Character and
			 * Erase Line
			 */
			case EC:
			case EL:
				ptflush();
				bput(ptout, (c == EC) ? ptb.sg_erase : ptb.sg_kill);
				break;

			/*
			 * Check for urgent data...
			 */
			case DM:
				break;

			/*
			 * Begin option subnegotiation...
			 */
			case SB:
				state = TS_BEGINNEG;
				continue;

			case WILL:
			case WONT:
			case DO:
			case DONT:
				state = TS_WILL + (c - WILL);
				continue;

			case IAC:
				bput(ptout, c);
				break;
			}
			state = TS_DATA;
			break;

		case TS_BEGINNEG:
			if (c == IAC)
				state = TS_ENDNEG;
			break;

		case TS_ENDNEG:
			state = c == SE ? TS_DATA : TS_BEGINNEG;
			break;

		case TS_WILL:
			if (!hisopts[c])
				willoption(c);
			state = TS_DATA;
			continue;

		case TS_WONT:
			if (hisopts[c])
				wontoption(c);
			state = TS_DATA;
			continue;

		case TS_DO:
			if (!myopts[c])
				dooption(c);
			state = TS_DATA;
			continue;

		case TS_DONT:
			if (myopts[c]) {
				myopts[c] = 0;
				sprintf(buf, wont, c);
				bputs(netout, buf);
			}
			state = TS_DATA;
			continue;

		default:
			printf("telnetd: panic state=%d\n", state);
			exit(1);
		}
	}
}

static
willoption(option)
	int option;
{
	char *fmt;
	char buf[10];

	switch (option) {

	case TELOPT_BINARY:
		mode(RAW, 0);
		goto common;

	case TELOPT_ECHO:
		mode(0, ECHO|CRMOD);
		/*FALL THRU*/

	case TELOPT_SGA:
	common:
		hisopts[option] = 1;
		fmt = doopt;
		break;

	case TELOPT_TM:
		fmt = dont;
		break;

	default:
		fmt = dont;
		break;
	}
	sprintf(buf, fmt, option);
	bputs(netout, buf);
}

static
wontoption(option)
	int option;
{
	char *fmt;
	char buf[10];

	switch (option) {

	case TELOPT_ECHO:
		mode(ECHO|CRMOD, 0);
		goto common;

	case TELOPT_BINARY:
		mode(0, RAW);
		/*FALL THRU*/

	case TELOPT_SGA:
	common:
		hisopts[option] = 0;
		fmt = dont;
		break;

	default:
		fmt = dont;
	}
	sprintf(buf, fmt, option);
	bputs(netout, buf);
}

static
dooption(option)
	int option;
{
	char *fmt;
	char buf[10];

	switch (option) {

	case TELOPT_TM:
		fmt = wont;
		break;

	case TELOPT_ECHO:
		mode(ECHO|CRMOD, 0);
		goto common;

	case TELOPT_BINARY:
		mode(RAW, 0);
		/*FALL THRU*/

	case TELOPT_SGA:
	common:
		fmt = will;
		break;

	default:
		fmt = wont;
		break;
	}
	sprintf(buf, fmt, option);
	bputs(netout, buf);
}

/*
 * Send interrupt to process on other side of pty.
 * If it is in raw mode, just write NULL;
 * otherwise, write intr char.
 */
static
interrupt()
{
	ptflush();
	ioctl(ptfd, TIOCGETC, &ptt);
	bput(ptout, ptb.sg_flags & RAW ? '\0' : ptt.t_intrc);
}

static
catchint()
{
	terminate("child death");
}

#include <utmp.h>

struct	utmp wtmp;
char	wtmpf[]	= "/usr/adm/wtmp";
char	utmp[] = "/etc/utmp";
#define SCPYN(a, b)	strncpy(a, b, sizeof (a))
#define SCMPN(a, b)	strncmp(a, b, sizeof (a))

static
rmut()
{
	register f;
	int found = 0;
	char *line, *dev;

	dev = ptname(ptno);
	line = dev + 5;

	DEBUG fprintf(stderr, "closing connection on %s\n", line);

	f = open(utmp, 2);
	if (f >= 0) {
		while(read(f, (char *)&wtmp, sizeof (wtmp)) == sizeof (wtmp)) {
			if (SCMPN(wtmp.ut_line, line) || wtmp.ut_name[0]==0)
				continue;
			lseek(f, -(long)sizeof (wtmp), 1);
			SCPYN(wtmp.ut_name, "");
			time(&wtmp.ut_time);
			write(f, (char *)&wtmp, sizeof (wtmp));
			found++;
		}
		close(f);
	}
	if (found) {
		f = open(wtmpf, 1);
		if (f >= 0) {
			SCPYN(wtmp.ut_line, line);
			SCPYN(wtmp.ut_name, "");
			time(&wtmp.ut_time);
			lseek(f, (long)0, 2);
			write(f, (char *)&wtmp, sizeof (wtmp));
			close(f);
		}
	}
	chown(dev, 0, 0);
	chmod(dev, 0666);
}

static
getty(f)
{
	int i;
	struct sgttyb b;
	char banner[128];

	ioctl(f, TIOCSPGRP, 0);
	signal(SIGTERM, SIG_DFL) ;
	signal(SIGPIPE, SIG_DFL) ;
	signal(SIGQUIT, SIG_DFL) ;
	signal(SIGINT, SIG_DFL) ;
	signal(SIGALRM, SIG_DFL) ;
	signal(SIGHUP, SIG_DFL) ;
	signal(SIGCHLD, SIG_DFL) ;
	close(0) ;
	close(1) ;
	close(2) ;
	close(3) ;
	dup(f) ;
	dup(f) ;
	dup(f) ;
	dup(f);		/* for /dev/tty */
	for (i=4; i<NOFILE; i++)
		close(i) ;
	ioctl(0, TIOCGETP, &b);
	b.sg_flags |= CRMOD|XTABS|ANYP|ECHO;
	b.sg_erase = '#';
	b.sg_kill = '@';
	ioctl(0, TIOCSETP, &b);
	sprintf(banner, "%s (%s) ", whoami(), strrchr(ptname(ptno), '/')+1);
	write (netfd, banner, strlen(banner));
	execl(LOGIN, "login", 0);
	fatalperror(2, LOGIN, errno);
	exit(1);

}

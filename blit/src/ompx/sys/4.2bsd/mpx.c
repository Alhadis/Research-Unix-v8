From research!ulysses!smb  Mon Jan 16 09:04:36 1984
Date: Mon, 16 Jan 84 09:04:36 est
From: ulysses!smb (Steven Bellovin)
Message-Id: <8401161404.AA08106@ulysses.UUCP>
Received: by ulysses.UUCP (4.12/3.7)
	id AA08106; Mon, 16 Jan 84 09:04:36 est
To: research!rob
Subject: part 1 of 2 -- mpx.c

#include <stdio.h>

#include <sys/types.h>
#include <sgtty.h>
#include <signal.h>
#include <errno.h>
#include <jioctl.h>
#include <time.h>
#include <utmp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "mpxstats.h"
#include "msgs.h"
#include "pconfig.h"
#include "proto.h"
#include "packets.h"
#include "pstats.h"

#define	NLAYERS	8	/* Same as in jerq itself */
#define	NSELFD	16	/* Maximum file descriptors for 'select' */

struct timeval select_time = {1L, 0};	/* Select timeout interval */

char *	jerqprog	= "/usr/jerq/lib/mpxterm";
char	umesgf[100];
char	*shell;

int	enabled	= 1;
int	rdfd_set;

struct utmp myut;
struct stat statb;
int statok = 0;

char buf[MAXPKTDSIZE+1];

struct{
	short	speed;
	short	bytes;
} speeds[] = {
	EXTA, 1920,
	B9600, 960,
	B4800, 480,
	B1200, 120,
	B300, 30,
	0, 960,		/* default */
};
#define	NSPEEDS	((sizeof speeds)/(sizeof speeds[0]))
#define	max(A,B)	(((A)>(B))?(A):(B))

/*
 *	One layer structure per file-descriptor
 */

struct layer {
	char	lay;
	char	busy;
	struct	winsize wnsz;
	int	lay_pgrp;	/* Initial process group for layer */
	char	ptyn[40];	/* Name of pty slave */
	int	slot;		/* Slot in /etc/ttys for pty */
} layer[NSELFD];

struct Pchannel	pconvs[NLAYERS];

extern char *	itoa();
extern char *	strcpy();
extern char *	strcat();
extern int	strlen();
extern int	write();
int		receive();
int		creceive();
void		wrmesgb();
void		dosig();
#ifdef	TRACING
int		twrite();
#endif

struct Pconfig	pconfig	= {
#	ifndef	TRACING
	 write
#	else
	 twrite
#	endif
	,receive
	,(void(*)())creceive
};

/* Assorted tty mode stuff */
struct sgttyb	sttymodes, sttysave;
int ldisc;
struct tchars tc;
int lmode;
struct ltchars ltc;

/*	CCA page-mode stuff */
#ifdef	TIOCSSCR
int	screen;
int	zero = 0;
struct pagechars pchars;
#endif

/* USESTAT stuff */
#ifdef TIOCSAUXC
struct tauxil achars;
#endif

short	quitflag;
short	booted;
short	hupped = 0;
char cmdttyb[100];
enum LTYPE {UT_IN, UT_OUT};

#ifndef	TRACING
#define trace(a,b)
#define	tread(a,b)
#else
FILE	*tracefd;
void	trace();
void	tread();
#endif
#if	TRACING == 1 || PDEBUG == 1
char 	tracefile[]	= "traces";
#define	_exit	exit
#endif

extern int	errno;
extern char *	sys_errlist[];
extern int	sys_nerr;

sighup()
{
	hupped = 1;
	quit("hangup");
}

main(argc, argv)
	char *argv[];
{
	register int	n;
	char *getenv(), *rindex();
	char *cmdname, *p;
	int dowaits();

	cmdname = argv[0];
	p = rindex(cmdname, '/');
	if (p && *++p) cmdname = p;
	while(argc>1) {
		argc--;
		argv++;
		if(argv[0][0] == '-') {
			switch(argv[0][1]) {
			default:
				quit("bad flag");
			}
		} else
			jerqprog=argv[0];
	}

	if (fstat(0, &statb) == 0) statok++;

	if ((shell = getenv("SHELL")) == (char *)0) shell = "/bin/sh";
	signal(SIGHUP, sighup);
	ioctl(0, TIOCGETP, &sttymodes);
	sttysave=sttymodes;
	ioctl(0, TIOCGETC, &tc);
	ioctl(0, TIOCGLTC, &ltc);
	ioctl(0, TIOCLGET, &lmode);
#ifdef	TIOCSSCR
	ioctl(0, TIOCGSCR, &screen);
	ioctl(0, TIOCPGET, &pchars);
#endif
#ifdef	TIOCSAUXC
	ioctl(0, TIOCGAUXC, &achars);
#endif
	ioctl(0, TIOCGETD, &ldisc);
	sttymodes.sg_flags|=RAW;
	sttymodes.sg_flags&=~ECHO;
	ioctl(0, TIOCSETP, &sttymodes);
	ioctl(0, TIOCEXCL, 0);	/* Exclusive use */
#ifdef	TRACING
	tracefd=fopen(tracefile, "w");
#	ifdef	PDEBUG
	ptracefd = tracefd;
#	endif
#endif
#	if	TRACING != 1 && PDEBUG == 1
	ptracefd=fopen(tracefile, "w");
#	endif
	trace("orig screensize=%d\n", screen);
	trace("page char=%o\n", pchars.tc_pagec);
	utinit(cmdname);
	if(boot(jerqprog))
		quit("can't boot terminal program");
	booted++;
	ioctl(0, TIOCEXCL, 0);
	trace(0, 0);
	trace("start\n", 0);

	for(n=0; n < NSPEEDS; n++)
		if(speeds[n].speed <= sttymodes.sg_ospeed)
			break;
	n=speeds[n].bytes;
	Pxtimeout=max((((NLAYERS-2)*sizeof(struct Packet)*NPCBUFS+n-1)/n), 3);
	Prtimeout=max(((sizeof(struct Packet)+n-1)/n), 2);
	Pscanrate=1;
	trace("speed = %d", n);
	trace(" xtimo = %d", Pxtimeout);
	trace(" rtimo = %d\n", Prtimeout);

	Pxfdesc = 1;

	if(pinit(NLAYERS)==-1)
		quit("bad protocol initialisation");

	buf[0] = JTIMO;
	buf[1] = Prtimeout;
	buf[2] = Pxtimeout;
	(void)psend(0, buf, 3);

	signal(SIGCHLD, dowaits);
	for(;;){
		if(scan()==-1)
			break;
		if(quitflag)
			quit("exit");
#		ifdef	TRACING
		fflush(tracefd);
#		endif
	}

	trace("errno = %d\n", errno);
	quit("select");
}

dowaits()
{
	union wait status;

	while (wait3(&status, WNOHANG, 0) > 0)
		;
}

scan()
{
	register int	fd, bit, n, ret;
	int selret;

	trace(0, 0);
	trace("enabled %o\n", enabled);
	ret = 0;
	rdfd_set = enabled;
	while((selret = select(NSELFD, &rdfd_set, (int *)0, (int *)0, &select_time))
	    == -1 )
		if ( errno != EINTR )
			return -1;
		else {
			ret++;
			rdfd_set = enabled;
		}
	trace(0, 0);
	trace("selected %o\n", rdfd_set);
	if ( selret == 0 ) {
		if ( Ptflag )
			ptimeout(SIGALRM);
	}else for (fd = 0, bit = 1 ; fd < NSELFD ; fd++, bit <<= 1)
		if (bit & rdfd_set) {
			while ((n = read(fd, buf, sizeof buf)) == -1)
				if (errno == EIO) {
					if (fd) n = 0;
					break;
				}
				else if (errno == EINTR) {
					trace("read error, errno=%d\n", errno);
					ret++;
					break;
				}
				else return -1;
			if(n == 0) {
				ret++;
#ifdef	TRACING
				trace("0 byte read", 0);
				fflush(tracefd);
#endif
			}
			if (fd == 0) {
				tread(buf, n);
				precv(buf, n);
			} else if (unpack(fd, buf, n))
				enabled &= ~bit;
		}
	return ret;
}

quit(s)
	register char *s;
{
	register l, i;

#	ifdef	TRACING
	trace("\nmpx: %s\n", s);
	trace(0, 0);
	fflush(tracefd);
#	endif

	signal(SIGHUP, SIG_IGN);
	if(booted){
		for(i=0; i<NSELFD; i++)
			if(layer[i].busy) {
				utrelog(i, UT_OUT);
				(void)close(i);
			}
		layer[0].lay = 0;
		sendioctl(0, JTERM);	/* kill demux ==> boot terminal */
		if (!hupped)
			for(i=(Pxtimeout+1); Ptflag && i>0;) {
				enabled = 1;
				if((l = scan()) == -1)
					break;
				i -= l;
			}
		alarm(0);
		ioctl(0, TIOCSETP, &sttysave);
		ioctl(0, TIOCNXCL, 0);
	}
	sleep(2);
	write(2, "mpx: ", 5);
	write(2, s, strlen(s));
	write(2, "\n", 1);
#	ifdef	PSTATISTICS
	for (i = 0, l = 0 ; i < PS_NSTATS ; i++)
		if (pstats[i].count) {
			if (l++ == 0)
				fprintf(stderr, "\nPacket protocol statistics:\n");
			fprintf(stderr, "%6ld %s\n"
				,pstats[i].count
#				ifdef	PSTATSDESC
				,pstats[i].descp
#				else
				,""
#				endif
				);
			trace("%6ld ", pstats[i].count);
			trace("%s\n", pstats[i].descp);
		}
	fflush(stderr);
#	endif
#	ifdef	NMPXSTATS
	for (i = 0, l = 0 ; i < NMPXSTATS ; i++)
		if (mpxstats[i].count) {
			if (l++ == 0)
				fprintf(stderr, "\nMpx statistics:\n");
			fprintf(stderr, "%6ld %s\n", mpxstats[i].count, mpxstats[i].descp);
			trace("%6ld ", mpxstats[i].count);
			trace("%s\n", mpxstats[i].descp);
		}
	fflush(stderr);
#	endif
#	if	TRACING == 1 || PDEBUG == 1
	fprintf(stderr, "\nThere are traces in '%s'\n", tracefile);
#	endif
#	ifdef	TRACING
	fflush(tracefd);
	abort();
#	endif
#	ifdef	MONITOR
	monitor(0);
#	endif
	_exit(0);
}


/*
 *	Unpack a message buffer bp of length n.
 */
unpack(fd, bp, n)
	register int fd;
	register char *	bp;
	int n;
{
	trace("unpack fd %d", fd);
	trace(" size %d\n", n);

	if ( n <= 0 ) {
		trace("shell died\n", 0);
		if(layer[fd].busy)
			(void)psend(layer[fd].lay, "Shell died.\n", 12);
		layer[fd].busy = 0;
		close(fd);
		utrelog(fd, UT_OUT);
		enabled &= ~(1<<fd);
		return 1;
	}

	if (*bp != 0) {

		trace("unpack ioctl type '%c'", *bp & 0xff);
		switch ((*bp) & 0xff) {

			case JTERM & 0xff:
			case JBOOT & 0xff:
			case JZOMBOOT & 0xff:
			case JTIMO & 0xff:
			case JTIMOM & 0xff:
				sendioctl(fd, *bp);
				break;
			default:
				trace("Unknown ioctl\n", 0);
				break;
		}
		return 0;
	}
	return sendchars(fd, bp+1, n-1);
}

sendioctl(fd, cmd)
{
	char ioctlvec[2];

	ioctlvec[0]=cmd;
	ioctlvec[1]=layer[fd].lay;
	if(psend(0, ioctlvec, sizeof ioctlvec)==-1){
		trace("layer 0 blocked for ioctl\n", (char *)0);	/* BUG */
		MPXSTATS(L0BLOCKED);
	}
	unblock(fd);
}

int
sendchars(fd, s, cc)
	char *s;
	int cc;
{
	register int	l = layer[fd].lay;

#	ifdef	TRACING
	char		buf[256];
#	endif
	register int	n;

#	ifdef TRACING
	trace("write %d chars ", cc);
	trace("to layer %d\n", l);
	if(layer[fd].busy==0)
		return 0;		/* layer was deleted, but there's still data */
	strncpy(buf, s, cc);
	buf[cc]=0;
	trace("<%s>\n", buf);
#	endif

	if(cc > 0){
		do{
			if((n=cc)>MAXPKTDSIZE)
				n=MAXPKTDSIZE;
			if(psend(l, s, n)==-1){
				trace("layer %d blocked\n", l);
				MPXSTATS(LBLOCKED);
				return fd;	/* BUG */
			}
		}while(s+=n, (cc-=n)>0);
		unblock(fd);
	}
	return 0;
}

unblock(fd)
	int fd;
{
	register Pch_p	pcp = &pconvs[layer[fd].lay];

	trace("unblock for layer %d", layer[fd].lay);
	trace(" freepkts=%d\n", pcp->freepkts);

	if(fd==0)
		return;
	if(pcp->freepkts>=1)
		enabled |= 1<<fd;
	else
		enabled &= ~(1<<fd);
}

void
lerror(l, s, t)
	int l; char *s; char t;
{
	char	ebuf[128];

	strcpy(ebuf, s);
	if(errno){
		strcat(ebuf, ": ");
		if(errno < sys_nerr)
			strcat(ebuf, sys_errlist[errno]);
		else{
			strcat(ebuf, "error ");
			strcat(ebuf, itoa(errno));
		}
		errno = 0;
	}
	strcat(ebuf, "\n");

	trace("receive type %d", t);
	trace(" for layer %d", l);
	trace(" %s\n", ebuf);

	layer[0].lay = l;
	sendchars(0, ebuf, strlen(ebuf));
}

int
creceive(l, s, n)
	char *s;
{
	if(s[0]!=C_UNBLK||n!=1)
		quit("bad control type");
	(void)receive(l, s, n);
}

int
receive(l, s, cc)
	int l;
	register char *s;
	register int cc;
{
	register int i;
	register char s2;

	if((i=ltofd(l))==-1)
		switch(*s){
		case C_NEW: case C_EXIT:
			break;
		default:
			errno = 0;
			lerror(l, "inactive layer", *s);
		case C_UNBLK:
			return 0;
		}

	while(cc--){
		trace("receive C type %d", *s);
		trace(" for layer %d", l);
		trace(" fd %d\n", i);

		switch(s2 = *s++){
		case C_SENDCHAR:	/* send layer char */
			wrmesgb(i, s++, 1);
			cc--;
			break;
		case C_NEW:		/* make layer */
			if((i=doshell())==-1){
				lerror(l, umesgf, C_NEW);
				trace("can't open %s\n", umesgf);
				cc-=6;
				break;
			}
			MPXSTATS(NEWLAYER);
			layer[i].busy=1;
			layer[i].lay=l;
			trace("new fd %d ", i);
			trace("layer %d ", l);
			enabled |= (1<<i);
			/* fall through */

		case C_RESHAPE:
			layer[i].wnsz.bytesx= *s++;
			trace("x wid %d ", layer[i].wnsz.bytesx);
			layer[i].wnsz.bytesy= *s++;
			trace("y wid %d\n", layer[i].wnsz.bytesy);
			layer[i].wnsz.bitsx = *(unsigned char *)s++;
			layer[i].wnsz.bitsx |= (*s++)<<8;
			layer[i].wnsz.bitsy = *(unsigned char *)s++;
			layer[i].wnsz.bitsy |= (*s++)<<8;
			ioctl(i, JSWINSIZE, &layer[i].wnsz);
#ifdef	TIOCSSCR
			{
				int nscr;

				if (s2 == C_RESHAPE) ioctl(i, TIOCGSCR, &nscr);
				else nscr = screen;

				/* Watch out for race -- see comments in doshell */
				if (nscr) nscr = layer[i].wnsz.bytesy;
				ioctl(i, TIOCSSCR, &nscr);
			}
#endif
			cc-=6;
			break;
		case C_UNBLK:		/* unblock layer */
			unblock(i);
			break;
		case C_DELETE:		/* delete layer */
			dosig(i, SIGHUP);
			utrelog(i, UT_OUT);
			layer[i].busy=0;
			pconvs[layer[i].lay].freepkts=1;	/* hack */
			unblock(i);
			break;
		case C_EXIT:		/* exit */
			quitflag++;
			return 0;
		case C_BRAINDEATH:{	/* jerq prog died... */
			dosig(i, SIGTERM);
			ioctl(i, TIOCSETN, &sttysave);
			ioctl(i, TIOCSETC, &tc);
			ioctl(i, TIOCSLTC, &ltc);
			ioctl(i, TIOCLSET, &lmode);
			ioctl(i, TIOCSETD, &ldisc);
#ifdef	TIOCSSCR
			ioctl(i, TIOCSSCR, &zero);
			ioctl(i, TIOCPSET, &pchars);
#endif
#ifdef	TIOCSAUXC
			ioctl(i, TIOCSAUXC, &achars);
#endif
			break;
		}
		case C_SENDNCHARS:	/* send cc characters */
			wrmesgb(i, s, cc);
			return 0;
		default:
			quit("unknown state in case 0");
		}
#		ifdef	TRACING
		if(cc<0)
			quit("bad count in receive");
#		endif	TRACING
	}
	return 0;
}

int
ltofd(l)
{
	register i;

	if(l==0)
		return 0;
	for(i=1; i<NSELFD; i++)
		if(layer[i].busy && layer[i].lay==l)
			return i;
	trace("unknown layer %d\n", l);
	return -1;
}

void
dosig(fd, sig)		/* Interrupt shell */
{
	int ttypg;

	if (layer[fd].lay_pgrp > 0) killpg(layer[fd].lay_pgrp, sig);
	if (ioctl(fd, TIOCGPGRP, &ttypg) == 0 &&
	    ttypg > 0 && ttypg != layer[fd].lay_pgrp)
		killpg(ttypg, sig);
}

void
wrmesgb(fd, cp, n)
	register char *cp;
	register int n;
{
#	ifdef	TRACING
	fprintf(tracefd, "mesg to fd %d: <%.*s>\n", fd, n, cp);
#	endif	TRACING
	write(fd, cp, n);
}

int
doshell()
{
	register int	fd, i;
	register char c;
	int bliton = 1;
#ifdef TIOCSSCR
	struct	winsize zwnsz;
	int nscr;
#endif

	trace("do shell\n", 0);

	for (c = 'p'; c <= 'u'; c++) {
		strcpy(umesgf, "/dev/ptyXX");
		umesgf[strlen("/dev/pty")] = c;
		umesgf[strlen("/dev/ptyp")] = '0';
		if (access(umesgf, 0) < 0)
			break;
		for (i = 0; i < 16; i++) {
			umesgf[strlen("/dev/ptyp")] = "0123456789abcdef"[i];
			fd = open(umesgf, 2);
			if (fd > 0)
				goto gotpty;
		}
	}
	return (-1);

gotpty:

	trace("opened %s\n", umesgf);
	if (ioctl(fd, JSMPX, &bliton) < 0 || ioctl(fd, FIOCLEX, 0) < 0) {
		close(fd);
		return(-1);
	};
	umesgf[strlen("/dev/")] = 't';
	strcpy(layer[fd].ptyn, umesgf);
	layer[fd].slot = slotno(umesgf);
	if (statok) {
		chown(umesgf, statb.st_uid, statb.st_gid);
		chmod(umesgf, statb.st_mode & 06777);
	}


#ifdef TRACING
	fflush(tracefd);
#endif
#ifdef TIOCSSCR
	/* Zero window size synchronously to C_RESHAPE code -- see below */
	zwnsz.bytesx = 0;
	zwnsz.bytesy = 0;
	zwnsz.bitsx = 0;
	zwnsz.bitsy = 0;
	ioctl(fd, JSWINSIZE, &zwnsz);
#endif
	switch(layer[fd].lay_pgrp = fork()){
	 case 0:
		ioctl(0, TIOCNXCL, 0);
		fd = open("/dev/tty", 2);
		ioctl(0, TIOCEXCL, 0);
		if (fd > 0) {
			ioctl(fd, TIOCNOTTY, 0);
			close(fd);
		}
		else trace("/dev/tty err %d\n", errno);

		close(0); close(1); close(2);
		setgid(getgid()); setuid(getuid());
		if (open(umesgf, 2) < 0) {
			trace("Can't open %s\n", umesgf);
			perror(umesgf, errno);
			exit(1);
		}
		ioctl(0, TIOCSETD, &ldisc);
		ioctl(0, TIOCSETN, &sttysave);
		ioctl(0, TIOCSETC, &tc);
		ioctl(0, TIOCSLTC, &ltc);
		ioctl(0, TIOCLSET, &lmode);
#ifdef	TIOCSSCR
		/* We have a race condition possible here.  If
		 * some screen size is used by the real tty, we
		 * assume that the user wants it in a window, too.
		 * So we want to set it here. But we don't have
		 * access to the window size yet; that hasn't been
		 * extracted yet.  It will, when C_NEW falls through
		 * into C_RESHAPE.  So we could, in theory, just let
		 * C_RESHAPE handle it.  Unfortunately, that's in
		 * another process, which may run to that point before
		 * we even open the pty.  If that happens, our open
		 * will reset the size to 0.  We avoid that by taking
		 * advantage of the fact (bug?) that window sizes aren't
		 * reset by the kernel.  So some code in the main branch
		 * zeros it, and C_RESHAPE doesn't set it till it's done
		 * with the pagesize code.  So -- if this child process
		 * is delayed, the screen size and the window size will
		 * be set first by the main branch.  Our open will reset
		 * the screen, but not the window.  We'll use the info
		 * returned then to set the screen size again.  If, on the
		 * other hand, we run first, the window size will be zero
		 * here, so we do nothing.  But the screen size can then
		 * be set successfully in the main branch, because our open
		 * request, with all the trauma that causes the data
		 * structures, will be done.  Note that it doesn't matter
		 * if both branches set the screensize; it's essentially
		 * impossible for there to be a conflict.  One final point --
		 * we can skip all this nonsense if screen size was zero
		 * to start with.
		 */
		if (screen) {
			ioctl(0, JWINSIZE, &zwnsz);
			if (zwnsz.bytesy) {
				nscr = zwnsz.bytesy;
				ioctl(0, TIOCSSCR, &nscr);
			}
			/* no else -- handled by RESHAPE */
		}
		else ioctl(0, TIOCSSCR, &screen);
		ioctl(0, TIOCPSET, &pchars);
#endif
#ifdef	TIOCSAUXC
		ioctl(0, TIOCSAUXC, &achars);
#endif

		dup(0); dup(0);
#ifdef TRACING
		fflush(tracefd);
#endif
		execlp(shell, shell, "-i", 0);
		perror(shell);
		exit(1);
		break;

	 case -1:
		layer[fd].lay_pgrp =  0;
		umesgf[strlen("/dev/")] = 'p';
		close(fd);
		chown(umesgf, 0, 0);
		chmod(umesgf, 0666);
		return -1;
	}

	utrelog(fd, UT_IN);

	umesgf[strlen("/dev/")] = 'p';
	return fd;
}

int
boot(s)
	char *s;
{
	if(system("/usr/jerq/bin/68ld", "68ld", "-e", s))
		return 1;
	sleep(2);
	return 0;
}

int
system(s, t, u, v)
char *s, *t, *u, *v;
{
	int status, pid, l;

	if ((pid = fork()) == 0) {
		setgid(getgid()); setuid(getuid());
		execl(s, t, u, v, 0);
		_exit(127);
	}
	while ((l = wait(&status)) != pid && l != -1)
		;
	if (l == -1)
		status = -1;
	return(status);
}

char *
itoa(i)
	register int i;
{
	static char str[11];
	register char *	sp = &str[sizeof str];

	*--sp = '\0';
	if(i > 0){
		do
			*--sp = i%10 + '0';
		while((i/=10) > 0);
	}else
		*--sp = '0';

	return sp;
}

#ifdef	TRACING
/*VARARGS1*/
void
trace(s, a)
	char *s, *a;
{
	long		t;
	extern long	time();
	extern char *	ctime();

	if(s == (char *)0){
		(void)time(&t);
		fprintf(tracefd, "%.9s", ctime(&t)+11);
	}else
		fprintf(tracefd, s, a);
}

int
twrite(fd, s, n)
	unsigned char *	s;
{
	register int	i;

	fprintf(tracefd, "to jerq: ");
	for(i=0; i<n; i++)
		fprintf(tracefd, "<%o>", s[i]);
	fprintf(tracefd, "\n");

	return write(fd, s, n);
}

void
tread(s, n)
	unsigned char *	s;
{
	register int	i;

	fprintf(tracefd, "from jerq: ");
	for(i=0; i<n; i++)
		fprintf(tracefd, "<%o>", s[i]);
	fprintf(tracefd, "\n");
}
#endif

int	ufd = -1;	/* Fd for /etc/utmp */
int	slot;		/* Our slot in /etc/utmp */
char nbuf[sizeof myut.ut_name+1];
char lbuf[100];

utinit(cmd) char *cmd;
{
	struct passwd *pw;
	register char **ep, **nep;
	extern char **environ;
	register int i;
	char *p;
	char *ttyname();

	/* Find and validate utmp slot. */
	p = ttyname(0);
	if (p == NULL) {
		trace("ttyname: %d\n", errno);
		return;
	}
	trace("ttyn: %s\n", p);
	slot = slotno(p);
	if (slot < 0) {
		trace("slotno: %d\n", slot);
		return;
	}

	ufd = open("/etc/utmp", 2);
	if (ufd < 0) {
		trace("utmp open: %d\n", errno);
		return;
	}

	ioctl(ufd, FIOCLEX, 0);
	if (lseek(ufd, (long)slot * sizeof myut, 0) < 0L ||
	    read(ufd, (char *)&myut, sizeof myut) != sizeof myut) {
		trace("lseek/read: %d\n", errno);
		close(ufd);
		ufd = -1;
		return;
	}

	sprintf(cmdttyb, "%.*s!%s", sizeof myut.ut_line, myut.ut_line, cmd);
	strncpy(nbuf, myut.ut_name, sizeof myut.ut_name);
	if ((pw = getpwnam(nbuf)) == (struct passwd *) 0 || getuid() != pw->pw_uid) {
		if (pw) trace("login name security: %d\n", pw->pw_uid);
		else trace("login name no match\n", 0);
		endpwent();
		close(ufd);
		ufd = -1;
		return;
	}
	endpwent();

	/* Now stick the line name into the environment */
	i = 0;
	for (ep = environ; *ep; ep++) i++;
	nep = (char **) calloc(i+2, sizeof (char *));
	if (nep == (char **) 0) return;
	strcpy(lbuf, "LINE=");
	strncat(lbuf, myut.ut_line, sizeof myut.ut_line);
	*nep = lbuf;
	ep = environ; environ = nep++;
	/* Rember that calloc zeroed last elem */
	while (*ep) {
		if (strncmp(*ep, "LINE=", 5) != 0) *nep++ = *ep;
		ep++;
	}
}

utrelog(fd, io) int fd; enum LTYPE io;
{
	struct utmp ut;
	char *ttn;
	
	trace("utrelog %d", fd);
	trace(" %d", io);
	trace(" ufd=%d\n", ufd);

	if (io == UT_IN) {
		ttn = layer[fd].ptyn;
		if (strncmp(ttn, "/dev/", strlen("/dev/")) == 0)
			ttn += strlen("/dev/");
		strncpy(ut.ut_line, ttn, sizeof ut.ut_line);
		strncpy(ut.ut_host, cmdttyb, sizeof ut.ut_host);
		strncpy(ut.ut_name, myut.ut_name, sizeof ut.ut_name);
		time(&ut.ut_time);
	}
	else {
		strcpy(ut.ut_line, "");
		strcpy(ut.ut_name, "");
		strcpy(ut.ut_host, "");
		ut.ut_time = 0;
		chown(layer[fd].ptyn, 0, 0);
		chmod(layer[fd].ptyn, 0666);
	}

	if (ufd < 0) return(-1);
	if (lseek(ufd, (long)layer[fd].slot * sizeof ut, 0) < 0L ||
	    write(ufd, (char *)&ut, sizeof ut) != sizeof ut) {
		close(ufd);
		ufd = -1;
		return(-1);
	}
	trace("utrelog exit 0\n", 0);
	return(0);
}

/*
 * Return the number of the slot in the utmp file
 * corresponding to the paramter passed.
 * Definition is the line number in the /etc/ttys file.
 */


static	char	ttys[]	= "/etc/ttys";

static
slotno(p) register char *p;
{
	register s;
	register FILE *tf;
	char lname[100];

	if ((tf=fopen(ttys, "r")) == NULL)
		return(errno>0 ? -errno : -1);
	s = 0;
	if (strncmp(p, "/dev/", strlen("/dev/")) == 0) p += strlen("/dev/");
	while (fgets(lname, sizeof lname, tf) != NULL) {
		s++;
		lname[strlen(lname)-1] = '\0';
		if (strcmp(p, lname+2)==0) {
			fclose(tf);
			return(s);
		}
	}
	fclose(tf);
	return(-1);
}


#if	TRACING | PSTATISTICS | MPXSTATS
#include <stdio.h>
#endif

#include <sys/types.h>
#include <sys/stream.h>
#include <sgtty.h>
#include <signal.h>
#include <errno.h>
#include <jioctl.h>

#include "mpxstats.h"
#include "msgs.h"
#include "pconfig.h"
#include "proto.h"
#include "packets.h"
#include "pstats.h"

#define	NLAYERS	8	/* Same as in jerq itself */
#define	NSELFD	16	/* Maximum file descriptors for 'select' */
#define	SELTIMO	(1000*1)	/* 'select' timeout is in milisecs */

char *	jerqprog	= "/usr/blit/lib/mpxterm";
char	umesgf[]	= "/dev/pt/pt01";
char	*shell;
/*
 *	The following two line discipline declarations are inherently unsafe!
 */
int	ttyld		= 0;
int	mesgld		= 4;
#ifdef RFC
int	trcld		= 7;
#endif

/*
 *	Structure returned by "mesgld"
 *	(should be in a system header)
 */

struct mesgb{
	struct mesg{
		short	type;
		short	size;
	} h;
	char	buf[MAXPKTDSIZE];
} buf;

fd_set	rdfd_set;
int	enabled	= 1;

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
	char	dx, dy;		/* Window size in characters */
	short	bitsx, bitsy;	/* Window size in bits */
	int	more;
} layer[NSELFD];

struct Pchannel	pconvs[NLAYERS];

extern char *	itoa();
extern char *	strcpy();
extern char *	strcat();
extern int	strlen();
extern int	write();
int		receive();
int		creceive();
void		dosig();
void		wrmesgb();
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

struct sgttyb	sttymodes, sttysave;
struct tchars	tcharssave;
short	booted;

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
extern int	tty_ld;

int	quitflag;

main(argc, argv)
	char *argv[];
{
	register int	n;
	char *getenv();

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
	if((shell=getenv("SHELL")) == 0)
		shell="sh";
	ioctl(0, TIOCGETP, &sttymodes);
	ioctl(0, TIOCGETC, &tcharssave);
	sttysave=sttymodes;
	sttymodes.sg_flags|=RAW;
	sttymodes.sg_flags&=~ECHO;
	ioctl(0, TIOCSETP, &sttymodes);
#ifdef	TRACING
	tracefd=fopen(tracefile, "w");
#	ifdef	PDEBUG
	ptracefd = tracefd;
#	endif
#endif
#	if	TRACING != 1 && PDEBUG == 1
	ptracefd=fopen(tracefile, "w");
#	endif
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

	buf.buf[0] = JTIMO;
	buf.buf[1] = Prtimeout;
	buf.buf[2] = Pxtimeout;
	(void)psend(0, buf.buf, 3);

	for(;;){
		if(scan()==-1)
			break;
		if(quitflag)
			quit("you made it!");
#		ifdef	TRACING
		fflush(tracefd);
#		endif
	}

	trace("errno = %d\n", errno);
	quit("select");
}

scan()
{
	register int	fd, bit, n, ret;

	trace(0, 0);
	trace("enabled %o\n", enabled);
	ret = 0;
	rdfd_set.fds_bits[0] = enabled;
	while ( select(NSELFD, &rdfd_set, (fd_set *)0, SELTIMO) == -1 )
		if ( errno != EINTR )
			return -1;
		else
			ret++;
	trace(0, 0);
	trace("selected %o\n", rdfd_set.fds_bits[0]);
	if ( rdfd_set.fds_bits[0] == 0 ) {
		if ( Ptflag )
			ptimeout(SIGALRM);
	}else for (fd = 0, bit = 1 ; fd < NSELFD ; fd++, bit <<= 1)
		if (bit & rdfd_set.fds_bits[0]) {
			while ((n = read(fd, (char *)&buf, sizeof buf)) == -1)
				if (errno != EINTR)
					return -1;
				else{
					trace("read error, errno=%d\n", errno);
					ret++;
				}
#ifdef	TRACING
			if(n == 0)
				trace("0 byte read", 0);
#endif
			if (fd == 0) {
				if (n == 0)
					quit("EOF on blit");
				tread((char *)&buf, n);
				precv((char *)&buf, n);
			} else if (unpack(fd, &buf, n))
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

	if(booted){
		for(i=0; i<NSELFD; i++)
			if(layer[i].busy)
				(void)close(i);
		layer[0].lay = 0;
		sendioctl(0, JTERM);	/* kill demux ==> boot terminal */
		for(i=(Pxtimeout+1); Ptflag && i>0;) {
			enabled = 1;
			if((l = scan()) == -1)
				break;
			i -= l;
		}
		alarm(0);
	}
	ioctl(0, TIOCSETP, &sttysave);
	ioctl(0, TIOCSETC, &tcharssave);
	ioctl(0, TIOCNXCL, 0);
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
	register struct mesgb *	bp;
	int n;
{
	trace("unpack fd %d", fd);
	trace(" size %d\n", n);

	if ( n <= 0 )
		bp->h.type = M_HANGUP;
	else if (layer[fd].more > 0) {
		layer[fd].more -= n;
		return sendchars(fd, (char *)bp, n);
	}

	switch (bp->h.type) {
	 case M_HANGUP:
		trace("shell died\n", 0);
		wait((int *)bp);
		if(layer[fd].busy)
			(void)psend(layer[fd].lay, "Shell died.\n", 12);
		layer[fd].busy = 0;
		close(fd);
		enabled &= ~(1<<fd);
		return 1;

	 case M_DELIM:
	 case M_DELAY:
	 default:
		trace("ignore type 0%o\n", bp->h.type);
		return 0;

	 case M_DATA:
		break;

	 case M_IOCTL:
		bp->h.type = M_IOCACK;

		switch (*(int *)bp->buf) {
		 case TIOCSETP:
		 case TIOCSETN:
		 case JMPX:
			bp->h.size = 0;
		 case TIOCGETP:
			break;

		 case JWINSIZE:
			*((int *)bp->buf) = JWINSIZE;	/* answering JWINSIZE ioctl */
			((struct winsize *)(bp->buf+sizeof(int)))->bytesx = layer[fd].dx;
			((struct winsize *)(bp->buf+sizeof(int)))->bytesy = layer[fd].dy;
			((struct winsize *)(bp->buf+sizeof(int)))->bitsx = layer[fd].bitsx;
			((struct winsize *)(bp->buf+sizeof(int)))->bitsy = layer[fd].bitsy;
			bp->h.size = sizeof(struct winsize) + sizeof(int);
			break;

		 case JTERM:
		 case JBOOT:
		 case JZOMBOOT:
			sendioctl(fd, *(int *)bp->buf);
			bp->h.size = 0;
			break;

/*
		 case ('j'<<8)|69:
			*(int *)bp->buf=('j'<<8)|(69-1);
			sendioctl(fd, *(int *)bp->buf);
			bp->h.size = 0;
			break;
*/
		 default:
			bp->h.type = M_IOCNAK;
			bp->h.size = 0;
		}

		write(fd, (char *)bp, sizeof(struct mesg)+bp->h.size);
		trace("unpack ioctl type '%c'", *(int *)bp->buf>>8);
		trace(" %d\n", *(int *)bp->buf&0xff);
		return 0;
	}
	if(bp->h.size > sizeof(bp->buf)){
		layer[fd].more = bp->h.size - sizeof(bp->buf);
		bp->h.size = sizeof(bp->buf);
	}
	return sendchars(fd, bp->buf, bp->h.size);
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
	struct mesg hupmsg;

	if((i=ltofd(l))==-1)
		switch(*s){
		case C_NEW:
		case C_EXIT:
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

		switch(*s++){
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
		case C_RESHAPE:
			layer[i].dx= *s++;
			trace("x wid %d ", layer[i].dx);
			layer[i].dy= *s++;
			trace("y wid %d\n", layer[i].dy);
			layer[i].bitsx = *(unsigned char *)s++;
			layer[i].bitsx |= (*s++)<<8;
			layer[i].bitsy = *(unsigned char *)s++;
			layer[i].bitsy |= (*s++)<<8;
			cc-=6;
			break;
		case C_UNBLK:		/* unblock layer */
			unblock(i);
			break;
		case C_DELETE:		/* delete layer */
			hupmsg.size=0;
			hupmsg.type=M_HANGUP;
			write(i, (char *)&hupmsg, sizeof (struct mesg));
			layer[i].busy=0;
			pconvs[layer[i].lay].freepkts=1;	/* hack */
			unblock(i);
			break;
		case C_EXIT:		/* exit */
			quitflag++;
			return 0;
		case C_BRAINDEATH:{	/* jerq prog died... */
			struct mesgb mb;
			dosig(i, SIGHUP);
			mb.h.type=M_IOCTL;
			mb.h.size=sizeof sttysave;
			*(struct sgttyb *)mb.buf = sttysave;
			write(i, (char *)&mb, sizeof(struct mesg)+sizeof sttysave);
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
	struct mesgb sigbuf;

	sigbuf.h.type = M_SIGNAL;
	sigbuf.h.size = sizeof(int);
	*(int *)sigbuf.buf = sig;
	write(fd, (char *)&sigbuf, sizeof(struct mesg)+sigbuf.h.size);
}

void
wrmesgb(fd, cp, n)
	register char *cp;
	register int n;
{
	struct mesgb wrbuf;
	register char *bp;

#	ifdef	TRACING
	fprintf(tracefd, "mesg to fd %d: <%.*s>\n", fd, n, cp);
#	endif	TRACING
	wrbuf.h.type = M_DATA;
	wrbuf.h.size = n;
	bp = wrbuf.buf;
	while ( n-- ) *bp++ = *cp++;
	write(fd, (char *)&wrbuf, sizeof(struct mesg)+wrbuf.h.size);
}

int
doshell()
{
	register int	fd, i;

	trace("do shell\n", 0);

	i = strlen(umesgf) - 1;
	umesgf[i-1] = '0';
	umesgf[i] = '1';

	while((fd = open(umesgf, 2)) == -1){
		trace("can't open %s\n", umesgf);
		if ( errno != ENXIO )
			return -1;
		if(umesgf[i] == '9'){
			umesgf[i] = '1';
			umesgf[i-1] += 1;
		}else
			umesgf[i] += 2;
	}
	trace("opened %s\n", umesgf);

#ifdef RFC
#define TRCSNAME	(('T'<<8)|7)	/*set trace module name */
	ioctl (fd, FIOPUSHLD, &trcld);
	ioctl (fd, TRCSNAME, "msg-bottom");
#endif
	if(ioctl(fd, FIOPUSHLD, &mesgld) == -1){
		close(fd);
		return -1;
	}
#ifdef RFC
/*
	ioctl (fd, FIOPUSHLD, &trcld);
	ioctl (fd, TRCSNAME, "msg-top");
*/
#endif

	switch(fork()){
	 case 0:
		for ( fd = 0 ; close(fd) != -1 ; fd++ );

		umesgf[i] -= 1;
		if(open(umesgf, 2) == -1 ||
		    ioctl(0, FIOPUSHLD, &ttyld) == -1){
			perror(umesgf);
			exit(1);
			break;
		}
		dup(0); dup(0);
		ioctl(0, TIOCSPGRP, 0);
		ioctl(0, TIOCSETN, &sttysave);
		ioctl(0, TIOCSETC, &tcharssave);
		execlp(shell, shell, 0);
		perror(shell);
		exit(1);
		break;

	 case -1:
		close(fd);
		return -1;
	}

	return fd;
}

int
boot(s)
	char *s;
{
	if(system("/usr/blit/bin/68ld", "68ld", "-e", s))
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

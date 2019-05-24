#if	TRACING | PSTATISTICS | MPXSTATS
#include <stdio.h>
#endif

#include <sys/param.h>
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
#include "/usr/blit/include/tty.h"

extern int	tty_ld;
extern int	mesg_ld;
extern int	buf_ld;
extern int	errno;
extern char	*sys_errlist[];
extern int	sys_nerr;

#define	NLAYERS	8	/* Same as in jerq itself */
#define	NSELFD	16	/* Maximum file descriptors for 'select' */
#define	SELTIMO	(1000*3)	/* 'select' timeout is in millisecs */
#define	CDSIZE	(sizeof(struct sgttyb)-1)

char	*jerqprog = "/usr/blit/lib/muxterm";
char	umesgf[] = "/dev/pt/pt01";
char	*progname;
char	*shell;
int	quitflag;
int	start_ld;
char	*getenv();

/*
 *	Structure returned by "mesg_ld"
 *	(should be in a system header)
 */

struct mesgb{
	struct mesg{
		short	type;
		short	size;
	}h;
	char	buf[MAXPKTDSIZE];
}buf;

fd_set	rdfd;
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
	char		lay;
	char		busy;
	char		dx, dy;		/* Window size in characters */
	short		bitsx, bitsy;	/* Window size in bits */
	int		more;
	struct ttychars	ttychars;
	char		ptfile[14];	/* file name of slave */
} layer[NSELFD];

struct Pchannel	pconvs[NLAYERS];

extern char	*itoa();
extern char	*strcpy();
extern char	*strcat();
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
	write,
#	else
	twrite,
#	endif
	receive,
	(void(*)())creceive,
};

struct sgttyb	sttymodes, sttysave;
struct tchars	tcharssave;
struct ttychars	ttychars;
struct ttychars zerochars;
short		booted;

#ifndef	TRACING
#define trace(a,b)
#define	tread(a,b)
#else
FILE	*tracefd;
void	trace();
void	tread();
#endif
#if	TRACING == 1 || PDEBUG == 1
char 	tracefile[] = "traces";
#define	_exit	exit
#endif

main(argc, argv)
	char *argv[];
{
	register int	n;
	char *getenv();

	progname=argv[0];
	if(argc>1){
		if(argc>2 && strcmp(argv[1], "-f")==0){
			jerqprog=argv[2];
		}else
			return service(argc-1, argv+1);
	}
	if((shell=getenv("SHELL")) == 0)
		shell="sh";
	ioctl(0, TIOCGETP, &sttymodes);
	sttysave=sttymodes;
	ioctl(0, TIOCGETC, &tcharssave);
	setmodes(&ttychars, &sttymodes);
	settchars(&ttychars, &tcharssave);
	sttymodes.sg_flags|=RAW;
	sttymodes.sg_flags&=~ECHO;
	ioctl(0, TIOCSETP, &sttymodes);
/*	ioctl(0, FIOPOPLD, 0);
	ioctl(0, FIOPUSHLD, &buf_ld);*/
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
/*
	if(ioctl(0, FIOLOOKLD, (struct sgttyb *)0)==tty_ld){
		ioctl(0, FIOPOPLD, (struct sgttyb *)0);
		start_ld=tty_ld;
	}else
*/
		start_ld= -1;
	trace(0, 0);
	trace("start\n", 0);
	for(n=0; n<NSPEEDS; n++)
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
		quit("bad protocol initialization");
	buf.buf[0] = JTIMO;
	buf.buf[1] = Prtimeout;
	buf.buf[2] = Pxtimeout;
	(void)psend(0, buf.buf, 3);
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

scan()
{
	register int	fd, bit, n, ret;

	trace(0, 0);
	trace("enabled %o\n", enabled);
	ret = 0;
	rdfd.fds_bits[0] = enabled;
	while(select(NSELFD, &rdfd, (fd_set *)0, SELTIMO) == -1)
		if(errno != EINTR)
			return -1;
		else
			ret++;
	trace(0, 0);
	trace("selected %o\n", rdfd.fds_bits[0]);
	if(rdfd.fds_bits[0] == 0) {
		if(Ptflag)
			ptimeout(SIGALRM);
	}else for (fd = 0, bit = 1 ; fd < NSELFD ; fd++, bit <<= 1)
		if (bit & rdfd.fds_bits[0]) {
			while ((n = read(fd, (char *)&buf, sizeof buf)) == -1)
				if (errno != EINTR)
					return -1;
				else{
					trace("read error, errno=%d\n", errno);
					return 1;
				}
#ifdef	TRACING
			if(n == 0)
				trace("0 byte read", 0);
#endif
			if (fd == 0) {
				if (n == 0)
					quit("EOF on jerq");
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
	trace("\nmux: %s\n", s);
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
		if(start_ld>=0)
			ioctl(0, FIOPUSHLD, (struct sgttyb *)&start_ld);
	}
/*	ioctl(0, FIOPOPLD, 0);
	ioctl(0, FIOPUSHLD, &tty_ld); */
	ioctl(0, TIOCSETP, &sttysave);
	ioctl(0, TIOCNXCL, 0);
	sleep(2);
	write(2, progname, strlen(progname));
	write(2, ": ", 2);
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
	register struct mesgb *bp;
	int n;
{
	struct ttychars tempchars;
	static char cdbuf[256];
	char *s;
	trace("unpack fd %d", fd);
	trace(" size %d", n);
	trace(" count %d\n", bp->h.size);

	if(n <= 0)
		bp->h.type = M_HANGUP;
	else if (layer[fd].more > 0) {
		layer[fd].more -= n;
		return sendchars(fd, (char *)bp, n);
	}

	switch (bp->h.type) {
	case M_HANGUP:
		trace("shell died\n", 0);
		wait((int *)bp);
		if(layer[fd].busy){
			sendioctl(fd, JDELETE);
			/*(void)psend(layer[fd].lay, "Shell died.\n", 12);*/
		}
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
			tempchars=layer[fd].ttychars;
			setmodes(&tempchars, (struct sgttyb *)(bp->buf+sizeof(int)));
			ttyset(fd, &tempchars);
			bp->h.size = 0;
			break;
		case TIOCGETP:
			tempchars=layer[fd].ttychars;
			getmodes(&tempchars, (struct sgttyb *)(bp->buf+sizeof(int)));
			bp->h.size=sizeof (struct sgttyb) + sizeof (int);
			ttyset(fd, &tempchars);
			break;
		case TIOCSETC:
			tempchars=layer[fd].ttychars;
			settchars(&tempchars, (struct tchars *)(bp->buf+sizeof(int)));
			ttyset(fd, &tempchars);
			bp->h.size = 0;
			break;
		case TIOCGETC:
			gettchars(&layer[fd].ttychars, (struct tchars *)(bp->buf+sizeof(int)));
			bp->h.size=sizeof (struct tchars) + sizeof (int);
			break;
		case JMPX:
			bp->h.size = 0;
			break;

		case JWINSIZE:
			*((int *)bp->buf) = JWINSIZE;	/* answering JWINSIZE ioctl */
#define	BP	((struct winsize *)(bp->buf+sizeof(int)))
			BP->bytesx = layer[fd].dx;
			BP->bytesy = layer[fd].dy;
			BP->bitsx = layer[fd].bitsx;
			BP->bitsy = layer[fd].bitsy;
			bp->h.size = sizeof(struct winsize) + sizeof(int);
			break;
		case JTERM:
		case JBOOT:
		case JZOMBOOT:
			sendioctl(fd, *(int *)bp->buf);
			bp->h.size = 0;
			break;
		case JEXIT:
			sendioctl(fd, *(int *)bp->buf);
			bp->h.size = 0;
			break;
		case JCHDIR:
			s=bp->buf+sizeof(int);
			if(*s==0){
				if(chdir(cdbuf)!=0) 
					bp->h.type = M_IOCNAK;
				cdbuf[0]=0;
			}else
				strcat(cdbuf, s);
			bp->h.size = 0;
			break;
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

getmodes(tp, bp)
	register struct ttychars *tp;
	register struct sgttyb *bp;
{
	bp->sg_ispeed=sttysave.sg_ispeed;
	bp->sg_ospeed=sttysave.sg_ospeed;
	bp->sg_flags=(tp->flags1<<8)|(tp->flags0&0xFF);
	bp->sg_erase=tp->erase;
	bp->sg_kill=tp->kill;
}

setmodes(tp, bp)
	register struct ttychars *tp;
	register struct sgttyb *bp;
{
	tp->flags0=bp->sg_flags;
	tp->flags1=bp->sg_flags>>8;
	tp->erase=bp->sg_erase;
	tp->kill=bp->sg_kill;
}

gettchars(tp, bp)
	register struct ttychars *tp;
	register struct tchars *bp;
{
	bp->t_intrc=tp->intrc;
	bp->t_quitc=tp->quitc;
	bp->t_startc=tp->startc;
	bp->t_stopc=tp->stopc;
	bp->t_eofc=tp->eofc;
	bp->t_brkc=tp->brkc;
}

settchars(tp, bp)
	register struct ttychars *tp;
	register struct tchars *bp;
{
	tp->intrc=bp->t_intrc;
	tp->quitc=bp->t_quitc;
	tp->startc=bp->t_startc;
	tp->stopc=bp->t_stopc;
	tp->eofc=bp->t_eofc;
	tp->brkc=bp->t_brkc;
}

ttyset(fd, tp)
	struct ttychars *tp;
{
	register char *p, *q;
	register i;
	static struct ttycmesg m={ JTTYC };
	for(i=0, p=(char *)tp, q=(char *)&layer[fd].ttychars; i<sizeof(struct ttychars); i++)
		if(*p++!=*q++)
			goto out;
	return;
   out:
	m.chan=layer[fd].lay;
	layer[fd].ttychars= *tp;
	m.ttychars= *tp;
	(void)psend(0, (char *)&m, sizeof m);	/* the void is a bug! */
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
	if(cc>0)
		do {
			if((n=cc)>MAXPKTDSIZE)
				n=MAXPKTDSIZE;
			if(psend(l, s, n)==-1){
				trace("layer %d blocked\n", l);
				MPXSTATS(LBLOCKED);
				return fd;	/* BUG */
			}
		}while(s+=n, (cc-=n)>0);
	unblock(fd);
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
			ttyset(i, &ttychars);
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
		case C_PUSHLD:		/* push ld onto stream */
			pushld(i);
			break;
		case C_POPLD:		/* pop ld from stream */
			popld(i);
			break;
		case C_DELETE:		/* delete layer */
			hupmsg.size=0;
			hupmsg.type=M_HANGUP;
			write(i, (char *)&hupmsg, sizeof (struct mesg));
			layer[i].busy=0;
			pconvs[layer[i].lay].freepkts=1;	/* hack */
			unblock(i);
			layer[i].ttychars=zerochars;
			break;
		case C_EXIT:		/* exit */
			quitflag++;
			return 0;
		case C_SENDNCHARS:	/* send cc characters */
			wrmesgb(i, s, cc);
			return 0;
		case C_KILL:	/* send layer signal */
			ioctl(i, TIOCFLUSH, (struct sgttyb *)0);
			dosig(i, *s++);
			cc--;
			break;
		default:
			quit("unknown state incase 0");
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
	while(n--) *bp++ = *cp++;
	write(fd, (char *)&wrbuf, sizeof(struct mesg)+wrbuf.h.size);
	wrbuf.h.size = 0;
	wrbuf.h.type = M_DELIM;
	write(fd, (char *)&wrbuf, sizeof(struct mesg));
}

popld(fd)
{
	register f, i;
	errno=0;
	if((f=open(layer[fd].ptfile, 2))>0){
		if((i=ioctl(f, FIOLOOKLD, 0))==tty_ld)
			ioctl(f, FIOPOPLD, (struct sgttyb *)0);
		else if(i>0)
			lerror(layer[fd].lay, "mux warning: unknown line discipline", 0);
	}
	close(f);
	trace("popld file %s\n", layer[fd].ptfile);
	trace("popld file descriptor %d\n", f);
}

pushld(fd)
{
	register f, i;
	int tty;
	struct tchars tc;
	struct sgttyb tb;
	if((f=open(layer[fd].ptfile, 2))>0){
		if((i=ioctl(f, FIOLOOKLD, 0))==-1){
			ioctl(f, FIOPUSHLD, (struct sgttyb *)&tty_ld);
			getmodes(&layer[fd].ttychars, &tb);
			gettchars(&layer[fd].ttychars, &tc);
			ioctl(f, TIOCSETP, &tb);
			ioctl(f, TIOCSETC, &tc);
		}else
			; /* can't warn; program could be e.g. jim! */
	}
	close(f);
	trace("pushld file descriptor %d\n", f);
}
int
doshell()
{
	register int	fd, i, slave;

	trace("do shell\n", 0);

	i = strlen(umesgf) - 1;
	umesgf[i-1] = '0';
	umesgf[i] = '1';

	while((fd = open(umesgf, 2)) == -1){
		trace("can't open %s\n", umesgf);
		if(errno != ENXIO)
			return -1;
		if(umesgf[i] == '9'){
			umesgf[i] = '1';
			umesgf[i-1] += 1;
		}else
			umesgf[i] += 2;
	}
	umesgf[i] -= 1;
	if((slave=open(umesgf, 2)) == -1){
		trace("can't open %s\n", umesgf);
		close(fd);
		return -1;
	}
	trace("opened %s\n", umesgf);
	strcpy(layer[fd].ptfile, umesgf);

#ifdef RFC
#define TRCSNAME	(('T'<<8)|7)	/*set trace module name */
	ioctl (fd, FIOPUSHLD, &trcld);
	ioctl (fd, TRCSNAME, "msg-bottom");
#endif
	if(ioctl(fd, FIOPUSHLD, &mesg_ld) == -1){
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
		close(0); close(1); close(2); close(3);
		dup(slave); dup(slave); dup(slave); dup(slave);
		for(fd=NSYSFILE; close(fd)!=-1; fd++)
			;
		ioctl(0, TIOCSPGRP, 0);
		execlp(shell, shell, 0);
		perror(shell);
		exit(1);
		break;

	case -1:
		close(fd);
		return -1;
	}
	close(slave);
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
service(argc, argv)
	char *argv[];
{
	if(strcmp(argv[0], "cd")==0){
		char *where=argv[1];
		char buf[CDSIZE+1];
		buf[CDSIZE]=0;
		if(where==0 && (where=getenv("HOME"))==0){
			write(2, "cd: no HOME set\n", 16);
			return 1;
		}
		while(*where){
			strncpy(buf, where, CDSIZE);
			ioctl(0, JCHDIR, buf);
			where+=strlen(buf);
		}
		if(ioctl(0, JCHDIR, where)!=0){
			write(2, "cd: bad directory\n", 18);
			return 1;
		}
		return 0;
	}
	if(strcmp(argv[0], "exit")==0)
		return ioctl(0, JEXIT, 0);
	write(2, "mux: no such command ", 21);
	write(2, argv[0], strlen(argv[0]));
	return 1;
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

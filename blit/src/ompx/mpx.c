#if	TRACING == 1 || PSTATISTICS == 1
#include <stdio.h>
#endif

#include <sys/mx.h>
#include <sgtty.h>
#include <signal.h>
#include <errno.h>
#include <jioctl.h>

#include "queue.h"
#include "mpxstats.h"
#include "msgs.h"
#include "pconfig.h"
#include "proto.h"
#include "packets.h"
#include "pstats.h"


#define	NLAYERS	8	/* Same as in jerq itself */
#define	BUFSIZE	(MAXPKTDSIZE+sizeof(struct rh))	/* must be no larger than buffer in jerq process */

char *	mytty;
char *	jerqprog	= "/usr/jerq/lib/mpxterm";
struct{
	short	speed;
	short	bytes;
}
	speeds[] =
{
	 EXTA, 1920
	,B9600, 960
	,B4800, 480
	,B1200, 120
	,B300, 30
	,0, 120		/* default */
};
#define	NSPEEDS	((sizeof speeds)/(sizeof speeds[0]))
#define	max(A,B)	(((A)>(B))?(A):(B))

struct layer{
	char	busy;
	short	chan;
	short	lay;
	struct clist queue;
	struct sgttyb sttybuf;
	char	dx, dy;		/* Window size in characters */
	short	bitsx, bitsy;	/* Window size in bits */
}
	layer[NLAYERS];

struct Pchannel	pconvs[NLAYERS];
#define	unblocks	user

int	receive(), wchan();
int	creceive();

struct Pconfig	pconfig	=
{
	 wchan
	,receive
	,(void(*)())creceive
};

int	xd;	/* Multiplexor file descriptor */

short	eotcmd[2] = {M_EOT, 0};
short	ublkcmd=M_UBLK;

struct sgttyb sttymodes, sttysave, sttychars;

char	buf[BUFSIZE];
short	quitflag;
short	booted;

void	canon(), sendline(), dosig(), sendeot();

#ifndef	TRACING
#define trace(a,b)
#else
#define	_exit	exit
FILE	*tracefd;
char 	tracefile[]	= "traces";
void	trace();
#endif

extern int	errno;



sighup(){
	quit("hangup");
}

main(argc, argv)
	char *argv[];
{
	short cc;
	char *ttyname();
	qinit();
	xd=mpx("", 0666);
	if(xd<0){
		write(2, "can't create mpx file\n", 22);
		_exit(1);
	}
	mytty=ttyname(0);
	Pxfdesc=join(0, xd);
	if(Pxfdesc==-1){
		write(2, "can't join standard input to mpx file\n", 38);
		_exit(1);
	}
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
	signal(SIGHUP, sighup);
	ioctl(0, TIOCGETP, &sttymodes);
	sttysave=sttymodes;
	sttymodes.sg_flags|=RAW;
	sttymodes.sg_flags&=~ECHO;
	ioctl(0, TIOCSETP, &sttymodes);
	ioctl(0, TIOCGETC, &sttychars);
	ioctl(0, TIOCEXCL, 0);	/* Exclusive use */
#ifdef	TRACING
	tracefd=fopen(tracefile, "w");
#	ifdef	PDEBUG
	ptracefd = tracefd;
#	endif
#endif
	if(boot(jerqprog))
		quit("can't boot terminal program");
	booted++;
	trace("start\n", 0);
	if(ioctl(xd, MXAUTOBLK, 0)<0)
		quit("ioctl MXAUTOBLK fails");

	for(cc=0; cc < NSPEEDS; cc++)
		if(speeds[cc].speed <= sttymodes.sg_ospeed)
			break;
	Pxtimeout=max((((NLAYERS-2)*sizeof(struct Packet)*NPCBUFS)/speeds[cc].bytes), 3);
	Prtimeout=max((sizeof(struct Packet)/speeds[cc].bytes), 2);
	Pscanrate=1;
	trace("speed = %d", speeds[cc].bytes);
	trace(" xtimo = %d", Pxtimeout);
	trace(" rtimo = %d\n", Prtimeout);

	if(pinit(NLAYERS)==-1)
		quit("bad protocol initialisation");

	buf[0] = JTIMO;
	buf[1] = Prtimeout;
	buf[2] = Pxtimeout;
	psend(0, buf, 3);

	for(;;){
		if((cc=read(xd, buf, BUFSIZE))>=0)
			unpack(buf, cc);
		else if(errno!=EINTR)
			break;
		if(quitflag)
			quit("exit");
#		ifdef	TRACING
		fflush(tracefd);
#		endif
/*		if(cc<BUFSIZE)
			nap(2);
*/
	}

	quit("error on master group");
}

quit(s)
	register char *s;
{
	register l, i;

	if(booted){
		for(l=0; l<NLAYERS; l++)
			if(layer[l].busy)
				(void)ckill(layer[l].chan, xd, SIGHUP);
		sendioctl(JTERM, 0);	/* kill demux ==> boot terminal */
		for(i=(Pxtimeout+1); Ptflag && i>0;)
		{
			if((l=read(xd, buf, BUFSIZE))>=0)	/* allow protocol to complete */
				unpack(buf, l);
			else
			if(errno==EINTR)
				--i;
			else
				break;
		}
	}
	ioctl(0, TIOCSETP, &sttysave);
	ioctl(0, TIOCNXCL, 0);
	trace("\rmpx: %s\n", s);
#	ifdef	TRACING
	fflush(tracefd);
#	endif
	sleep(2);
	write(2, "mpx: ", 5);
	write(2, s, strlen(s));
	write(2, "\n", 1);
#	ifdef	PSTATISTICS
	for ( i = 0, l = 0 ; i < PS_NSTATS ; i++ )
		if ( pstats[i].count )
		{
			if ( l++ == 0 )
				fprintf(stderr, "\nPacket protocol statistics:\n");
			fprintf(stderr, "%6ld %s\n", pstats[i].count, pstats[i].descp);
			trace("%6ld ", pstats[i].count);
			trace("%s\n", pstats[i].descp);
		}
	fflush(stderr);
#	endif
#	ifdef	NMPXSTATS
	for ( i = 0, l = 0 ; i < NMPXSTATS ; i++ )
		if ( mpxstats[i].count )
		{
			if ( l++ == 0 )
				fprintf(stderr, "\nMpx errors:\n");
			fprintf(stderr, "%6ld %s\n", mpxstats[i].count, mpxstats[i].descp);
			trace("%6ld ", mpxstats[i].count);
			trace("%s\n", mpxstats[i].descp);
		}
	fflush(stderr);
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
 * unpack an mpx buffer at
 * bp of length cc.
 */
unpack(bp, cc)
	register char *bp;
{
	register char *end;
	register struct rh *rp;

	end=bp+cc;
	while(bp<end){
		rp=(struct rh *)bp;
		bp+=sizeof(struct rh);
		rp->index=map((unsigned short)rp->index);
		if(rp->count==0){
			control(rp->index, bp);
			bp+=rp->ccount;
		}else{
			if(rp->index==0)
			{
#				ifdef	TRACING
				register int	i;

				trace("from jerq:", (char *)0);
				for(i=0; i < rp->count; i++)
					trace("<%o>", bp[i]&0xff);
				trace("\n", (char *)0);
#				endif
				precv(bp, rp->count);
			}
			else		/* From a process; send to Jerq */
				sendchars(rp->index, bp, rp->count);
			bp+=rp->count;
		}
		if((int)bp&1)
			bp++;
	}
}
control(l, cb)
	register char *cb;
{
	int sig;
	int cmd;
	cmd= *cb++;
	trace("control %d\n", cmd);
	switch(cmd){
	case M_WATCH:
		quit("M_WATCH");
	case M_CLOSE:
		trace("close %d\n", map((unsigned short)l));
		detach(layer[l].chan, xd);
		trace("wait\n", 0);
		wait(&sig);		/* safe? */
		break;
	case M_EOT:
		cb+=3;
		if(l&1)
			break;
		sendeot(l);
		break;
	case M_BLK:
		cb+=sizeof(short)+1;
		break;
	case M_UBLK:
		break;
	case M_SIG:
		cb++;
		sig= *(short *)cb;
		cb+=sizeof(short);
		if(sig==SIGINT){
			dosig(l, SIGINT);
			return;
		}else if (sig==SIGHUP)
			quit("hangup");
		break;
	case M_IOCTL:
		cb++;
		doioctl(l, *((short *)cb),
				(struct sgttyb *)(cb+sizeof(short)));
		cb+=sizeof(short) + sizeof (struct sgttyb);
		break;
	default:
		trace("botch cmd %d\n", cmd);
		quit("mpx botch");
	}
}
doioctl(chan, cmd, buf)
	short cmd;
	struct sgttyb *buf;
{
	register struct sgttyb *bp;
	struct winsize jbuf;

	bp= &layer[chan].sttybuf;
	switch(cmd){
	case TIOCSETP:
	case TIOCFLUSH:
		qclear(&layer[chan].queue);
		if(cmd==TIOCFLUSH)
			break;
		/* fall through */
	case TIOCSETN:
		*bp = *buf;
		break;
	case TIOCSETC:
	case TIOCGETC:
		bp= &sttychars;
		break;
	case TIOCGETP:
		break;
	case JWINSIZE:
		jbuf.bytesx=layer[chan].dx;
		jbuf.bytesy=layer[chan].dy;
		jbuf.bitsx=layer[chan].bitsx;
		jbuf.bitsy=layer[chan].bitsy;
		bp= (struct sgttyb *)&jbuf;
		break;
	case JMPX:
		break;
	default:
		if((cmd>>8)=='j'){
			trace("jerq ioctl %d", cmd&0xFF);
			trace(" of lay %d\n", layer[chan].lay);
			sendioctl(cmd, layer[chan].lay);
		}
		/* fall through to return answer */
		break;
	}
	wioctl(layer[chan].chan, bp);
}
wctl(chan,obuf,count)
	char *obuf;
{
	struct wh msg;
	msg.index=chan;
	msg.count=0;
	msg.ccount=count;
	msg.data=obuf;
	write(xd, &msg, sizeof msg);
#	ifdef	TRACING
	{
		register int	i;

		if(chan == Pxfdesc)
			trace("wctl to jerq:", (char *)0);
		else
			trace("wctl to UNIX chan %d:", chan);
		for(i=0; i < count; i++)
			trace("<%o>", obuf[i]&0xff);
		trace("\n", (char *)0);
	}
#	endif
}

wchan(chan,obuf,count)
	char *obuf;
{
	struct wh msg;
	msg.index = chan;
	msg.count = count;
	msg.ccount = 0;
	msg.data = obuf;
	write(xd,&msg,sizeof msg);
#	ifdef	TRACING
	{
		register int	i;

		if(chan == Pxfdesc)
			trace("to jerq:", (char *)0);
		else
			trace("to UNIX chan %d:", chan);
		for(i=0; i < count; i++)
			trace("<%o>", obuf[i]&0xff);
		trace("\n", (char *)0);
	}
#	endif
}
wioctl(index, vec)
	struct sgttyb *vec;
{
	struct {
		short cmd;
		struct sgttyb wvec;
	} wcmd;
	wcmd.wvec = *vec;
	wcmd.cmd = M_IOANS;
	wctl(index, (char *)&wcmd, sizeof wcmd);
}

sendioctl(cmd, l)
	char cmd, l;
{
	char ioctlvec[2];

	ioctlvec[0]=cmd;
	ioctlvec[1]=l;
	if(psend(0, ioctlvec, sizeof ioctlvec)==-1)
	{
		trace("layer 0 blocked for ioctl\n", (char *)0);	/* BUG */
		MPXSTATS(L0BLOCKED);
	}
}

sendchars(l, s, cc)
	char *		s;
	int		cc;
{
#	ifdef	TRACING
	char		buf[BUFSIZE+1];
#	endif
	register int	n;

#	ifdef TRACING
	verify(l);
	trace("write %d chars ", cc);
	trace("to layer %d\n", layer[l].lay);
	strncpy(buf, s, cc);
	buf[cc]=0;
	trace("<%s>\n", buf);
#	endif

	if (cc==0)
		sendeot(l);
	else{
		register int	channel = layer[l].lay;

		if(--pconvs[channel].unblocks<0)
			pconvs[channel].unblocks = 0;
		do{
			if((n=cc)>MAXPKTDSIZE)
				n=MAXPKTDSIZE;
			if(psend(channel, s, n)==-1)
			{
				trace("layer %d blocked\n", channel);
				MPXSTATS(LBLOCKED);
				return;	/* BUG */
			}
		} while(s+=n, (cc-=n)>0);
		unblock(l);
	}
}

unblock(l)
	int	l;
{
	register Pch_p	pcp = &pconvs[layer[l].lay];

	trace("unblock for layer %d", layer[l].lay);
	trace(" freepkts=%d", pcp->freepkts);
	trace(" unblocks=%d\n", pcp->unblocks);
	if(pcp->freepkts>=1 && pcp->unblocks<NPCBUFS)
	{
		pcp->unblocks++;
		wctl(layer[l].chan, &ublkcmd, sizeof ublkcmd);
	}
}

#ifdef	TRACING
verify(l){
	if(l<0 || NLAYERS<=l)
		quit("layer out of range");
	if(layer[l].chan==0){
		trace("no layer %d\n", l);
		quit("no such layer!");
	}
}
#endif

void
lerror(l, s, t)
	int l; char *s; char t;
{
	trace("receive type %d", t);
	trace(" for layer %d", l);
	trace(" error: %s\n", s);

	psend(l, s, strlen(s));
}

int
creceive(l, s, n)
	char *	s;
{
	if(s[0]!=C_UNBLK||n!=1)
		quit("bad control type");
	(void)receive(l, s, n);
}

int
receive(l, s, cc)
	int		l;
	register char *	s;
	register int	cc;
{
	register int	i, chan;

	if((i=ltoindex(l))==-1)
		switch(*s){
		case C_NEW: case C_EXIT:
			break;
		default:
			lerror(l, "inactive layer!\n", *s);
		case C_UNBLK:
			return 0;
		}

	while(cc--){
		trace("receive C type %d", *s);
		trace(" for layer %d", l);
		trace(" index %d\n", i);

		switch(*s++){
		case C_SENDCHAR:	/* send layer char */
			canon(i, *s++);	/* char typed in layer l */
			cc--;
			break;
		case C_SENDNCHARS:	/* send cc characters */
			while(cc--)
				canon(i, *s++);	/* char typed in layer l */
			return 0;
		case C_NEW:		/* make layer */
			if((chan=doshell())==-1){
				lerror(l, "out of channels!\n", C_NEW);
				cc-=6;
				break;
			}
			i=map((unsigned short)chan);
			layer[i].lay=l;
			layer[i].chan=chan;
			trace("new %d ", i);
			trace("chan %d ", layer[i].chan);
			trace("layer %d ", l);
			layer[i].sttybuf=sttysave;
			layer[i].busy=1;
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
			dosig(i, SIGHUP);
			layer[i].busy=0;
			break;
		case C_EXIT:		/* exit */
			quitflag++;
			return 0;
		case C_BRAINDEATH:	/* jerq prog died... */
			dosig(i, SIGTERM);
			layer[i].sttybuf=sttysave;
			break;
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
ltoindex(l){
	register i;

	for(i=0; i<NLAYERS; i++)
		if(layer[i].busy && layer[i].lay==l)
			return i;
	trace("unknown layer %d\n", l);
	return -1;
}

void
canon(l, c)
	register int	l;
	char		c;
{
	register struct clist *qp;
	register int	echo, rawmode, crmode, raw;
	static long	timev[2];

	qp= &layer[l].queue;
	echo=layer[l].sttybuf.sg_flags&ECHO;
	rawmode=layer[l].sttybuf.sg_flags&(RAW|CBREAK);
	raw=layer[l].sttybuf.sg_flags&RAW;
	crmode=layer[l].sttybuf.sg_flags&CRMOD;
	c &= (raw? 0xFF : 0x7F);
	switch(c) {
	case 4:	/* ^D */
		if(rawmode)
			break;
		sendeot(l);
		return;
	case 010:  /* ^H */
		if(rawmode)
			break;
		qtakec(qp);
		if(echo)
			Pcdata = c;
		return;
	case '\r':
		if(!rawmode && crmode)
			c = '\n';
		break;
	case '@':
		if(!rawmode){
			qclear(qp);
			if(echo)
				psend(layer[l].lay, "@\n", 2);
			return;
		}
		break;
	case 0177:	/* ^? */
	case 034:	/* ^\ : FS */
		if((layer[l].sttybuf.sg_flags&RAW)==0){
			qclear(qp);
			dosig(l, c==0177? SIGINT : SIGQUIT);
			return;
		}
		break;
	}
	if(echo)
		Pcdata = c;
	if(rawmode)
		wchan(layer[l].chan, &c, 1);
	else{
		qputc(qp, c);
		if(c=='\n'){
			sendline(l);
			qclear(qp);
			/* maintain idle time (mpx funny) */
			timev[1]=time((long *)0);
			if(timev[1]>timev[0]+60){
				timev[0]=timev[1];
				utime(mytty, timev);	/* thanks to dmr for fix */
			}
		}
	}
}

void
sendline(l){
	register struct clist *q;
	register struct cbuf *p;
	register char *s;
	char strbuf[NCHARS];

	trace("sendline from %d\n", layer[l].lay);
	q= &layer[l].queue;
	p=q->c_head;
	s=strbuf;
	while(p){
		*s++=p->word;
		p=p->next;
	}
	if(s>strbuf)
		wchan(layer[l].chan, strbuf, s-strbuf);
	qclear(q);
}

int
map(x)
	register unsigned short x;
{
	unsigned short y;
	for(y=017; y; y<<=4) {
		if ((x&y) == y)
			x &= ~y;
	}
	return(x);
}

void
dosig(l, sig){		/* Interrupt shell */
	if(ckill(layer[l].chan, xd, sig)==-1)
	{
		trace("ckill(%d) fails\n", layer[l].chan);
		MPXSTATS(BADCKILL);
	}
}

void
sendeot(l){
	wctl(layer[l].chan, (char *)eotcmd, sizeof eotcmd);
}

int
doshell()
{
	register fd, i;
	int shindx, shellid;
	trace("do shell\n", 0);
	shindx = chan(xd);
	if (shindx== -1)
		return -1;
	fd = extract(shindx,xd,1);
	if (fd<0)
	{
		detach(shindx, xd);
		return -1;
	}
	shellid=fork();
	if (shellid==0) {
		npgrp(shindx, xd, 0);
		for(i=0; i<20; i++)
			if(i!=fd)
				close(i);
		dup(fd);
		close(fd);
		dup(0);
		dup(0);
		execlp("sh", "sh", "-i");
		write(2, "mpx: can't exec shell\n", 22);
		_exit(1);
	}
	close(fd);
	if(shellid==-1)
	{
		detach(shindx, xd);
		return -1;
	}
	return(shindx);
}

int
boot(s)
	char *s;
{
	if(system("/usr/jerq/bin/68ld", "68ld", "-e", s))
		return 1;
	/* Give terminal program a while to wake up */
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

#ifdef	TRACING
void
trace(s, t)
	char *s;
{
	fprintf(tracefd, s, t);
}
#endif

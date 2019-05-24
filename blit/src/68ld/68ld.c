/*
 *	MC68000 loader
 */

char	Usage[]	= "Usage: 68ld [-b addr] [-d] [-e] [-p] [-r] [-z] objectfile";

/*
 * swapw	words must be swapped between host and 68000
 * swapb	bytes must be swapped between host and 68000
 */
#ifdef	pdp11
#define	swapb	1
#define	swapw	0
#endif
#ifdef	vax
#define	swapb	1
#define	swapw	1
#endif
#ifdef	u3b
#define	swapb	0
#define	swapw	0
#endif

#include "a.out.h"
#include <stdio.h>
#include <sgtty.h>
#include <errno.h>
#include <jioctl.h>
#include "proto.h"

#define	MAXRETRIES	10
#define	DATASIZE	512

struct sgttyb sgttyb;
struct sgttyb sttysave;
int	obj;		/* File descriptor for object file */
int	mpx;		/* Running under mpx */
long	location;
char	file[128];	/* Name of file */
int	nargchars;	/* Number of characters, including nulls, in args */
struct exec	header;
long	longbuf[3];
int	debug;		/* Show sizes etc. */
long	bflag;		/* -b option (cf. mld) */
long	base_location=0x100; char *bpt;
int	eflag;		/* Use error detecting loader */
int	psflag;		/* Print error detection statistics */
short	maxpktdsize;
int	rflag;		/* relocate? */
int	xflag;		/* Use one-shot error detection */
int	zflag;		/* Do a JZOMBOOT */
int	booted;
int	errfile;
int	retries;
int	open();
int	access();
char 	*malloc();

short speeds[16]={
	 1,	5,	7,	10,	13,	15,	20,	30,
	60,	120,	180,	240,	480,	960,	1920,	1
};

unsigned char sizes[16]={
	 16,	16,	16,	16,	16,	16,	16,	16,
	 16,	32,	32,	56,	56,	120,	120,	16
};

void	Psend();
void	Precv();

extern int	errno;

main(argc, argv)
	char *argv[];
{
	ioctl(1, TIOCGETP, &sttysave);

	while(argc>1 && argv[1][0]=='-'){
		switch(argv[1][1]){
		case 'b':
			if (argv[1][2] != 0) bpt = &argv[1][2];
			else { argv++; argc--; bpt = &argv[1][0]; }
			bflag=sscanf(bpt,"%ld",&base_location);
			if (!bflag) error(0,"bad address:",bpt);
			break;
		case 'd':
			debug++;
			break;
		case 'e':
			eflag++;
			break;
		case 'p':
			psflag++;
			break;
		case 'r':
			rflag++;
			break;
		case 'x':
			xflag++;
			eflag++;
			break;
		case 'z':
			zflag++;
			break;
		case '\0':
			break;
		default:
			error(0, Usage, (char *)0);
			return 1;
		}
		argv++; argc--;
	}
	if(argc<2){
		error(0, Usage, (char *)0);
		return 2;
	}
	sgttyb = sttysave;
	sgttyb.sg_flags = RAW|ANYP;
	ioctl(1, TIOCSETP, &sgttyb);
	if(jpath(argv[1], access, 4)!=0)
		error(1, "no such file", argv[1]);
	if(boot() && rflag==0)
		rflag++;
	if(!mpx){
		ioctl(1, TIOCEXCL, 0);
		maxpktdsize = min(sizes[sttysave.sg_ospeed&017], (long)MAXPKTDSIZE);
		pinit(speeds[sttysave.sg_ospeed&017], maxpktdsize,
			eflag?(xflag?ACKOFF:ACKON):NOCRC);
	}
	load(argv[1], argc-1, argv+1);
	if(!mpx && eflag){	/* ACKON or ACKOFF */
		buzz();
		ioctl(0, TIOCFLUSH, (struct sgttyb *)0);
	}
	ioctl(1, TIOCNXCL, 0);
	ioctl(1, TIOCSETP, &sttysave);
	if(psflag)
		pstats(stderr);
	return(0);
}
char *
bldargs(argc, argv)
	char *argv[];
{
	register i;
	register char *argp, *p, *q;
	for(nargchars=0, i=0; i<argc; i++)
		nargchars+=strlen(argv[i])+1;
	if((argp=malloc(nargchars))==0)
		error(0, "can't allocate argument chars", (char *)0);
	/* this loop is probably not necessary, but it's safe */
	for(i=0, q=argp; i<argc; i++){
		p=argv[i];
		do; while(*q++ = *p++);
	}
	return argp;
}
load(f, argc, argv)
	char *f;
	char *argv[];
{
	char *argp;
	long largc;
	long l;
	obj=jpath(f, open, 0);
	if(obj<0)
		error(1, "can't open", file);
	Read((char *)&header, sizeof (struct exec));
	if(debug){
		fprintf(stderr, "%s:\ttext: %ld, data: %ld, bss: %ld\n",
			 file, header.a_text, header.a_data, header.a_bss);
		buzz();
	}
	swaw(&header.a_magic, &l, 4);
	if(l==0406 || l==0407){
		header.a_magic=l;
		swaw(&header.a_text, &l, 4);
		header.a_text=l;
		swaw(&header.a_data, &l, 4);
		header.a_data=l;
		swaw(&header.a_bss, &l, 4);
		header.a_bss=l;
	}
	if(header.a_magic!=0406 && header.a_magic!=0407)
		error(0, file, "not a 68000 a.out");
	if((header.a_magic==0406) ^ mpx)
		error(0, file, mpx? "compiled stand-alone": "compiled for mpx");
	if(mpx){
		argp=bldargs(argc, argv);
		largc=argc;
		writeswap((char *)&largc, 4);	/* number of arguments */
		largc=nargchars;
		writeswap((char *)&largc, 4);	/* number of chars in arguments */
		writeswap((char *)&header.a_text, 12);
	}
	if(rflag)
		relocate();
	else
		location = base_location;
	if(mpx)
		Write(argp, nargchars);
	sendfile();
	if(!mpx){
		long	startaddr;

		retries = 0;
		while(freepkts != NPBUFS)
			Precv();
		location = base_location;
		swaw(&location, &startaddr, PKTASIZE);
		if(xflag)
			pinit(speeds[sttysave.sg_ospeed&017], maxpktdsize, ACKON);
		psend((char *)&startaddr, PKTASIZE);
		retries = 0;
		while(freepkts != NPBUFS)
			Precv();
	}
}

jpath(f, fn, a)
	register char *f;
	register int (*fn)();
{
	char *getenv(), *strcpy();
	register char *jp, *p;
	register o;
	if (*f != '/' && strncmp(f, "./", 2) && strncmp(f, "../", 3) && 
	    (jp=getenv("JPATH"))!=0){
		while(*jp){
			for(p=file; *jp && *jp!=':'; p++,jp++)
				*p= *jp;
			if(p!=file)
				*p++='/';
			if(*jp)
				jp++;
			(void)strcpy(p, f);
			if((o=(*fn)(file, a))!=-1)
				return o;
		}
	}
	return((*fn)(strcpy(file, f), a));
}

error(pflag, s1, s2)
	char *s1, *s2;
{
	register n;
	char buf[BUFSIZ];
	if(booted){
		ioctl(1, JTERM, 0);
		if(errfile>0){
			buzz();
			while((n=read(errfile, buf, sizeof buf))>0)
				write(1, buf, n);
		}
	}
	ioctl(1, TIOCNXCL, 0);
	ioctl(1, TIOCSETP, &sttysave);
	if(pflag)
		perror(s2);
	fprintf(stderr, "\r68ld: %s %s\r\n", s1, s2);
	if(psflag)
		pstats(stderr);
	exit(1);
}
int
Read(a, n)
	char *a;
{
	register i;
	i=read(obj, a, n);
	if(i<0)
		error(1, "read error", file);
	return(i);
}
void
Write(a, n)
	char *a;
{
	if(write(1, a, n)!=n)
		error(1, "write error", "jerq");
	if(psflag && !mpx)
		trace(a);
}
writeswap(a, n)
	char *a;
{
	char buf1[DATASIZE+PKTASIZE], buf2[DATASIZE+PKTASIZE];
	swaw(a, buf1, n);
	swab(buf1, buf2, n);
	Write(buf2, n);
}
trace(a)
	char *a;
{
	register int	i;

	for(i=0; i<(PKTHDRSIZE+PKTASIZE); i++)
		fprintf(stderr, "<%o>", a[i]&0xff);
	fprintf(stderr, "\n");
}

sendfile(){
	sendseg(location+header.a_text);
	sendseg(location+header.a_data);
}

sendseg(endloc)
	long endloc;
{
	char buf[DATASIZE+PKTASIZE], buf2[DATASIZE];
	register n;
	while((n=Read(&buf[PKTASIZE], min(!mpx?maxpktdsize:DATASIZE, endloc-location)))>0){
		if(mpx){
			swab(&buf[PKTASIZE], buf2, n);
			Write(buf2, n);
		}else{
			swaw((short *)&location, (short *)&buf[0], PKTASIZE);
			Psend(buf, n+PKTASIZE);
		}
		location+=n;
	}
}
void
Psend(bufp, count)
	char *bufp;
	int count;
{
	retries = 0;
	while(freepkts == 0)
		Precv();
	psend(bufp, count);
}
void
Precv()
{
	char c;

	alarm(3);	/* min. 2 sec, more time than a packet at 1200 baud */
	if(read(0, &c, 1) == 1){
		alarm(0);
		if(psflag)
			fprintf(stderr, "recv <%o>\n", c&0xff);
		precv(c);
	}else if(errno != EINTR )
		error(1, "read error", "jerq");
	else if(++retries >= MAXRETRIES)
		error(0, "load protocol failed", "");
	else if(psflag)
		fprintf(stderr, "recv timeout\n");
}

min(a, b)
	long b;	/* not your average min() */
{
	return(a<b? a : (int)b);
}

swab(a, b, n)
	register char *a, *b;
	register n;
{
#	if(swapb)
	register char *s, *t;
	n/=2;	/* n in bytes */
	s=b+1;
	t=b;
	while(n--){
		*s= *a++;
		*t= *a++;
		s+=2;
		t+=2;
	}
#	else
	while(n--)
		*b++= *a++;
#	endif
}

swaw(a, b, n)
	register short *a, *b;
	register n;
{
#	if(swapw)
	register short *s, *t;
	n/=4;	/* n in bytes */
	s=b+1;
	t=b;
	while(n--){
		*s= *a++;
		*t= *a++;
		s+=2;
		t+=2;
	}
#	else
	n>>=1;
	while(n--)
		*b++= *a++;
#	endif
}
relocate(){
	long address;
	char buf[100];
	char *mktemp();
	long caddress;
	register i;
	register char *p=(char *)&address;
	char	*tmpname;	/* name of temporary file for mld */
	char	*errname;	/* name of error file for mld */
	for(i=0; i<4; i++)
		read(0, p++, 1);
	ioctl(1, TIOCEXCL, 0);	/* must be here so PUSHLD of tty_ld can work from mux */
	swab(&address, &caddress, 4);
	swaw(&caddress, &address, 4);
	location=address;
	if(location==0)
		error(0, "no memory in jerq", "");
	sprintf(buf, "/usr/blit/bin/mld -s -o %s -b %ld %s>%s 2>&1",
		tmpname=mktemp("/tmp/6XXXXXX"),
		address, file, errname=mktemp("/tmp/6EXXXXXX"));
	if(system(buf)){
		errfile=open(errname, 0);
		unlink(errname);
		unlink(tmpname);
		error(0, "mld errors", (char *)0);
	}
	close(obj);
	obj=open(tmpname, 0);
	if(obj<0)	/* HELP!! */
		error(1, "tmp file vanished!", tmpname);
	unlink(tmpname);
	unlink(errname);
	Read(&header, sizeof (struct exec));
}

boot(){
	if(mpx=(ioctl(1, JMPX, 0)!=-1 || ioctl(0, JMPX, 0)!=-1)){
		ioctl(0, TIOCFLUSH, 0);	/* throw away type-ahead! */
		ioctl(1, zflag?JZOMBOOT:JBOOT, 0);
	}else{
		write(1, "\020", 1);
		buzz();
	}
	booted++;
	return mpx;
}

buzz(){
	/* sleep for a time >~0.5 sec; nice if we had nap! */
	sleep(2);	/* sleep(1) not necessarily long enough */
}

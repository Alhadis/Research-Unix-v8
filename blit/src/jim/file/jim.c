#define	MONITOR(a)
#include <stdio.h>
#include <signal.h>
#include <sgtty.h>
#include "/usr/blit/include/jioctl.h"
#include <setjmp.h>
#include "file.h"
#include "msgs.h"
jmp_buf	jmpbuf;
int initflag=1;
char *zflag="-";
extern Message m;
struct sgttyb sttybuf, sttysave;
char *argv0;
void rescue();
int rescuing=FALSE;
String	*jerqname();
char	tempname[]="/tmp/jim.XXXXXX";
int	tempfile;
int	diagnewline;	/* last char in diagnostic line was a `\n' */
int	nfile=1;	/* initially, just DIAG */
extern	char pattern[];
extern	char unixcmd[];
int	searchdir;
int	unixtype;
main(argc, argv)
	char *argv[];
{
	register i;
	SIG_TYP onhup;
	char *mktemp();

	argv0=argv[0];
	close(creat(mktemp(tempname), 0600));
	if((tempfile=open(tempname, 2))==-1){
		fprintf(stderr, "%s: tmp file %s vanished!\n", argv0, tempname);
		exit(1);
	}
	ioctl(0, TIOCGETP, &sttysave);
	sttybuf=sttysave;
	sttybuf.sg_flags=RAW|ANYP;
	ioctl(0, TIOCSETP, &sttybuf);
	if(argc>1 && strcmp(argv[1], "-z")==0){
		zflag=argv[1];
		argv++; --argc;
	}
	if(!boot(argv[0][0]=='x'? "/usr/blit/src/jim/xr.m": "/usr/blit/mbin/jim.m"))
		quit("68ld errors");
	ioctl(0, TIOCEXCL, 0);	/* Exclusive use */
	buffer=newstring();
	transmit=newstring();
	(void)Fcreat(&file[0], bldstring(""));	/* DIAG */
	for(i=1; i<argc && i<NFILE; i++){
		nfile++;
		Fcreat(&file[i], jerqname(&file[i], argv[i]));
		unmodified(&file[i]);
	}
	
	initflag=0;
	onhup=signal(SIGHUP, rescue);
	if(onhup==SIG_IGN)
		signal(SIGHUP, SIG_IGN);
	setjmp(jmpbuf);
	send(0, O_DONE, 0, 0, (char *)0);
	for(;;){
		rcv();
		message();
	}
}
String *
jerqname(cf, s)
	register File *cf;
	register char *s;
{
	register File *f;
	register oldmaxlen=0, maxlen=0;
	char buf[128+1];
	namecompact(s, buf);
	sendstr(cf-file, O_FILENAME, 0, buf);
	for(f=file+1; f<&file[NFILE]; f++)
		if(f->str && oldmaxlen<f->namelen)
			oldmaxlen=f->namelen;
	cf->namelen=strlen(buf);
	for(f=file+1; f<&file[NFILE]; f++)
		if((f->str || f==cf) && maxlen<f->namelen)
			maxlen=f->namelen;
	if(maxlen!=oldmaxlen || cf->str==0)
		send(0, O_NAMELENGTH, 0, 2, data2(maxlen));
	return bldstring(s);
}
/*
 * Compact path elements in as, result in ad
 */
namecompact(as, ad)
	char *as, *ad;
{
	register char *s=as, *d=ad, *slash;
	register len;
	extern char *index();
	
	if(strlen(as)>NAMELEN){
		/* copy to first '/' */
		while(*s && *s!='/')
			*d++ = *s++;
		while(*s){
			*d++ = *s++;	/* copy the slash */
			while(*s=='/')	/* skip multiple slashes */
				s++;
			if(*s)
				*d++ = *s++;	/* an interesting non-slash */
			if((slash=index(s, '/')) == 0)
				break;
			s=slash;
		}
	}
	strcpy(d, s);
	if((len=strlen(ad))>NAMELEN){	/* desperation */
		strncpy(d=ad+len-15-2, "<-", 2);	/* 15 gives last element */
		strcpy(ad, d);	/* only works because of order in strcpy */
	}
}
void
rescue()
{
	register fd;
	register File *f;
	char buf[512];
	signal(SIGHUP, SIG_IGN);
	rescuing=TRUE;
	fd=creat("jim.recover", 0777);
	if(fd>0){
		for(f=file+1; f<&file[NFILE]; f++)
			if(f->str){
				sprintf(buf,
					"/usr/blit/lib/jimunpack %s %s \"$@\" <<'-%@$ %s'\n",
					f->changed? "m" : "-m", f->name->s,
					f->name->s);
				write(fd, buf, strlen(buf));
				Fwrite(f, (String *)0, fd);
				sprintf(buf, "-%@$ %s\n", f->name->s);
				write(fd, buf, strlen(buf));
			}
	}
	quit("HUP");
}
message()
{
	register File *f= &file[m.file];
	register n, op;
	register posn;
	register unsigned char *data;
	register data2;
	n=m.nbytes;
	op=m.op;
	posn=m.posn;
	data=(unsigned char *)m.data;
	data2=data[0]|(data[1]<<8);
	data[n]=0;
	switch(op){
	case O_DIAGNOSTIC:	/* end of input */
		if(diagnewline)	/* input is just '\n' */
			Freset(DIAG);
		diagnewline=TRUE;
		commands(f);	/* f==DIAG if no frames open */
		send(0, O_DONE, 0, 0, (char *)0);
		break;
	case O_SEARCH:		/* execute remembered command */
		if(posn==1 || searchdir==0)
			Unix(f, unixtype, 0);
		else{
			if(searchdir && execute(f, searchdir))
				moveto(f, loc1, loc2);
			else
				dprintf("%s not found\n", pattern);
		}
		send(0, O_DONE, 0, 0, (char *)0);
		break;
	case O_BACKSPACE:
	case O_INSERT:
		if(f==DIAG && diagnewline){
			Freset(f);
			diagnewline=FALSE;
		}
		if(op==O_BACKSPACE){
			Fdeltext(f, f->origin+posn, (long)data2);
			break;
		}
		Finstext(f, f->origin+posn, (char *)data, n);
		/* it's always typing, so... */
		f->selloc=f->origin+posn+n;
		f->nsel=0;
		if(f==DIAG && n>0 && data[n-1]=='\n')
			diagnewline=TRUE;
		break;
	case O_CUT:
		if(f->nsel>0)	/* Jerq doesn't change buffer if nsel<=0 */
			Fsave(f, buffer, f->selloc, f->nsel);
		Fdeltext(f, f->selloc, f->nsel);
		f->nsel=0;
		break;
	case O_PASTE1:
		Fdeltext(f, f->selloc, f->nsel);
		break;
	case O_PASTE2:
		Finstext(f, f->selloc, buffer->s, buffer->n);
		f->nsel=buffer->n;
		break;
	case O_SELECT:
		f->selloc=f->origin+posn;
		f->nsel=data2;
		break;
	case O_SNARF:
		Fsave(f, buffer, f->selloc, f->nsel);
		break;
	case O_REQUEST:
		Fsave(f, transmit, f->origin+posn, (int)min((long)data2, (long)NDATA));
		/* transmit->n==0 if at EOF */
		send(m.file, O_INSERT, posn, transmit->n, transmit->s);
		break;
	case O_SEEK:
		f->origin=Forigin(f, (long)posn);
		f->selloc=f->origin;
		f->nsel=0;
		tellseek(f);
		break;
	case O_SCROLL:	/* Request scroll posn lines; answer with posn=# of chars */
		f->nsel=0;
		if(posn>0)	/* Forward */
			posn=Fforwnl(f, f->origin, posn);
		else{
			if(f->origin<=0)
				posn=0;
			else
				posn= -Fbacknl(f, f->origin-1, -posn)-1;
		}
		f->origin+=posn;
		tellseek(f);
		/* he'll give me a SELECT if it's different now */
		send(0, O_SCROLL, posn, 0, (char *)0);
		break;
	case O_FILENAME:	/* Tell jerq which file to open */
		n=newfile();
		if(n <= 0){
			sendstr(NFILE, O_FILENAME, 0, (char *)0);
			error("too many files open", (char *)0);
		}
		jerqname(&file[n], "");
		unmodified(&file[n]);
		Fcreat(&file[n], bldstring(""));
		break;
	case O_WRITE:
		if(f->name->s[0]==0)
			error("no file name", (char *)0);
		Fwrite(f, f->name, 0);
		mesg("wrote", f->name->s);
		send(0, O_DONE, 0, 0, (char *)0);
		unmodified(f);
		break;
	default:
		error("unix unk", (char *)data);
	}
}
newfile(){
	register File *f;
	for(f=file+1; f<&file[NFILE]; f++)
		if(length(f)==0 && (f->name==0 || f->name->n==0)){
			Fclose(f);	/* in case */
			return f-file;
		}
	return 0;
}
commands(f)
	register File *f;
{
	register char *p=DIAG->str->s;
	char fname[64];
	register c, i;
	int wholefile=FALSE;
	p[DIAG->str->n]=0;
	switch(c = *p){
	case 0:
	case '\n':
	case ' ':
	case '\t':
		break;
	case '?':
	case '/':
		if(*++p){
			strncpy(pattern, p, 128);
			compile(c);
		}else
			dprintf("%c%s\n", c, pattern);
		send(0, O_SEARCH, 0, 0, (char *)0);
		if(execute(f, searchdir=c))
			moveto(f, loc1, loc2);
		else
			dprintf("%s not found\n", pattern);
		break;
	case '=':
		dprintf("%d\n", Fcountnl(f, f->selloc));
		break;
	case '*':
		if((c=p[1])!='<' && c!='|' && c!='>')
			goto Default;
		c= *++p;
		wholefile=TRUE;
		/* fall through */
	case '<':
	case '>':
	case '|':
		if(*++p){
			strncpy(unixcmd, p, 128);
		}else
			dprintf("%c%s\n", c, unixcmd);
		send(0, O_SEARCH, 1, 0, (char *)0);
		Unix(f, unixtype=c, wholefile);
		break;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		i=atoi(p);
		if(i<=0){
			loc1=0;
			loc2=0;
		}else{
			loc1=Fforwnl(f, 0L, i-1);	/* 1-indexed */
			loc2=loc1+Fforwnl(f, loc1, 1);
		}
		moveto(f, loc1, loc2);
		break;
	case 'c':
		if(*++p=='d' && *++p==' ' && *++p){
			if(chdir(p)==-1)
				error(p, ": bad directory");
			else for(f=file; f<&file[NFILE]; f++)
				if(f->str)
					f->date=DUBIOUS;
		}else
			goto Default;
		break;
	case 'e':
		if(f->changed && fileschanged){
			fileschanged = FALSE;
			error(f->name->s, "changed");
		}
		/* fall through */
	case 'E':
	case 'w':
	case 'f':
		if(f==DIAG)
			error("no frame open", (char *)0);
		if(p[1]==0)
			strcpy(fname, f->name->s);
		else if(*++p!=' ')
			error("syntax", (char *)0);
		else
			strncpy(fname, p+1, sizeof fname -1);
		if(c=='f'){
			if(strcmp(fname, f->name->s)!=0){
				f->date=DUBIOUS;
				jerqname(f, fname);
				modified(f);
			}
			dupstring(bldstring(fname), f->name);
			dprintf("%c. %s\n", " '"[f->changed], fname);
			break;
		}
		if(fname[0]==0)
			error("no file name", (char *)0);
		if(c=='e' || c=='E'){
			Fclose(f);
			unmodified(f);
			(void)Fcreat(f, jerqname(f, fname));
			tellseek(f);
			send(f-file, O_RESET, 0, 0, (char *)0);
		}else if(c=='w'){
			if(f->name->s[0]==0){
				f->date=DUBIOUS;
				dupstring(jerqname(f, fname), f->name);
			}
			Fwrite(f, bldstring(fname), 0);
			mesg("wrote", fname);
			if(strcmp(fname, f->name->s)==0)
				unmodified(f);
		}
		break;
	case 'q':
		if(fileschanged)
			for(f=file; f<&file[NFILE]; f++)
				if(f->changed){
					fileschanged=FALSE;
					error("files changed", (char *)0);
				}
		/* fall through */
	case 'Q':
		if(p[1])
			error("syntax?", (char *)0);
		quit("q");
	default:
	Default:
		dprintf("you typed: '%s'\n", DIAG->str->s);
	}
}
char *
data2(n){
	static char x[2];
	x[0]=n;
	x[1]=n>>8;
	return x;
}
long
labs(a)
	long a;
{
	if(a<0)
		return -a;
	return a;
}
moveto(f, p1, p2)
	register File *f;
	register long p1, p2;
{
	register long p0=p1-Fbacknl(f, p1-1, 2)-1;
	register long nseen;	/* number of chars on screen */
	register posn, ntosend;
	register long x;
	if(p0<0)
		p0=0;
	/* Try to save some screen redrawing */
	send(f-file, O_CHARSONSCREEN, 0, 0, (char *)0);
	rcv();
	nseen=9*m.posn/10;
	if(labs(f->origin-p1)>nseen){	/* just redraw */
		posn= -1;
		ntosend=32767;
	}else if(f->origin<p1){		/* on screen now */
		posn=1;
		ntosend=0;
		p0=f->origin;
	}else{				/* back a little */
		posn=0;
		ntosend=f->origin-p0;
	}
	f->origin=p0;
	f->selloc=p1;
	f->nsel=p2-p1;
	x=p2-p1;
	if(x>32000)
		x=32000;	/* jerq'll truncate it, anyway */
	send(f-file, O_SELECT, (int)(p1-p0), 2, data2(x));
	send(f-file, O_MOVE, posn, 2, data2(ntosend));
	tellseek(f);
}
tellseek(f)
	register File *f;
{
	register n;
	register l=length(f);
	if(l>0)
		n=f->origin*YMAX/l;
	else
		n=0;
	if(n<0)
		n=0;
	if(n>YMAX)
		n=YMAX;
	send(f-file, O_SEEK, n, 0, (char *)0);
}
error(s, t)
	register char *s, *t;
{
	if(rescuing)
		return;
	mesg(s, t);
	/*?????tellseek(f);???why is this here??*/
	send(0, O_DONE, 0, 0, (char *)0);
	if(!initflag)
		longjmp(jmpbuf, 0);
}
quit(s)
	char *s;
{
	register File *f;
	if(strcmp(s, "68ld errors")!=0 && strcmp(s, "HUP")!=0)
		ioctl(0, JTERM, (struct sgttyb *)0);
	for(f=file; f<&file[NFILE]; f++)
		if(f->str > 0)
			Fclose(f);
	unlink(tempname);
	ioctl(0, TIOCSETP, &sttysave);
	ioctl(0, TIOCNXCL, 0);
	if(s)
		printf("%s: %s\n", argv0, s);
	MONITOR(0);
	exit(0);
}
ioerr(s, t)
	register char *s, *t;
{
	extern errno;
	dprintf("ioerr (%s) on file %s: errno %d\n", s, t, errno);
}
mesg(s, t)
	register char *s, *t;
{
	dprintf(t? "%s %s\n": "%s\n", s, t);
}
sendstr(f, op, posn, d)
	unsigned f;
	register char *d;
{
	register n=strlen(d), l;
	do{
		if((l=n)>NDATA)
			l=NDATA;
		send(f, op, posn, l, d);
		posn+=l;
		d+=l;
		n-=l;
	}while(n>0);
}
/*VARARGS1*/
dprintf(a, b, c, d, e, f, g)
	char *a;
{
	register s;
	char buf[128];
	sprintf(buf, a, b, c, d, e, f, g);
	sendstr(0, O_DIAGNOSTIC, 0, buf);
	if(diagnewline)
		Freset(DIAG);
	s=strlen(buf);
	if(s>0 && buf[s-1]=='\n'){
		diagnewline=TRUE;
		buf[--s]=0;
	}else
		diagnewline=FALSE;
	Finstext(DIAG, (long)DIAG->str->n, buf, strlen(buf));
}
boot(s)
	char *s;
{
	if(system("/usr/blit/bin/68ld", "68ld", zflag, s))
		return(0);
	return(1);
}
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
modified(f)
	File *f;
{
	if(f>file && !f->changed){	/* we can change DIAG all we want */
		f->changed=TRUE;
		fileschanged=TRUE;
		send(f-file, O_MODIFIED, TRUE, 0, (char *)0);
	}
}
unmodified(f)
	File *f;
{
	if(f>file && f->changed){	/* we can change DIAG all we want */
		f->changed=FALSE;
		send(f-file, O_MODIFIED, FALSE, 0, (char *)0);
	}
}

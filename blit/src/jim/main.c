#include "r.h"
extern Message m;
Rectangle diagrect;
int diagclr;
short newlnsz;
Textframe *current;
int usualtest();
Point loadpt;	/* location of next text to be received from host */
int ntoload;	/* number of characters to read from unix */
int (*donetest)()=usualtest;	/* test to decide when load is done */
int snarfhuge, selecthuge;
/*
 * Event flags to wait for in protocol
 */
int iodone, diagdone, scrolllines, filedone;
int reqlimit, nrequested, reqposn;
#define	INFINITY	32767
main()
{
	register got;
	request(KBD|MOUSE|RCV|SEND);
	newlnsz=defont.height;
	init();
	zerostring(BUF);
	waitunix(&diagdone);	/* when menu is loaded */
	for(got=0; ; got=wait(MOUSE|KBD|RCV)){
		if(P->state&RESHAPED){
			stipple(Drect);
			closeall();
			init();
			P->state&=~RESHAPED;
		}
		/* NOTE: cursor is OFF at all times... */
		if((got&RCV) && rcv()){
			curse(current);
			(void)message();
			curse(current);
		}
		if((got&MOUSE) && button123()){
			curse(current);
			buttonhit(mouse.xy, mouse.buttons);
			curse(current);
		}
		if((got&KBD) && current)	/* ...except in type */
			type(current);	/* manages cursor itself */
	}
}
closeall(){
	register Textframe *t;
	for(t=frame; t<&frame[NFRAME]; t++){
		if(t->file)
			setchar(t->file, STARDOT, ' ');
		closeframe(t);
		delobs(t);
	}
	current=0;
}
init(){
	diagrect=Drect;
	diagrect.origin.y=diagrect.corner.y-(defont.height+2*M)-1;
	diagclr=F_CLR;
	diagrect.origin.y++;
	(void)newframe(diagrect);	/* DIAG */
	initframe(DIAG);
	curse(DIAG);
	screenrect=Drect;
	screenrect.corner.y=diagrect.origin.y;
	current=0;
	workframe=0;
}
/*ARGSUSED*/
seek(t, pt, but)
	register Textframe *t;
	Point pt;
{
	Rectangle r;
	register n;
	register y=pt.y;
	if(t==DIAG)
		return;
	initframe(t);
	ontop(t);
	r=t->scrollrect;
	n=muldiv(y-r.origin.y, YMAX, r.corner.y-r.origin.y);
	if(n<0)
		n=0;
	if(n>YMAX)
		n=YMAX;
	send(t->file, O_SEEK, n, 0, (char *)0);
	loadfile(t, 0, INFINITY);
	setsel(t, 0);
}
tellseek(t, y)
	register Textframe *t;
{
	Rectangle r;
	t->scrolly=y;
	r=t->scrollrect;
	y=muldiv(y, r.corner.y-r.origin.y-2*M, YMAX)+r.origin.y+M;
	Rectf(r, F_OR);
	r.origin.y=y;
	r.corner.y=y+2;
	r.origin.x++;
	r.corner.x--;
	rXOR(r);
}
usualtest()
{
	return(!inscomplete);
}
loadfile(t, posn, n)
	register Textframe *t;
	register posn;
{
	loadpt=ptofchar(t, posn);
	reqlimit=n;
	ntoload=n;
	nrequested=0;
	reqposn=posn;
	urequest(t->file);
	urequest(t->file);	/* double buffering */
	waitunix(&iodone);
	if(n==INFINITY){	/* the stuff's gotta die down! */
		reqlimit=0;
		while(wait(RCV)){
			if(rcv()){
				message();
				break;
			}
		}
	}
}
urequest(f)
	int f;
{
	register n;
	if(nrequested <= reqlimit){
		n=min(NDATA, reqlimit-nrequested);
		send(f, O_REQUEST, reqposn, 2, data2(n));
		reqposn+=n;
		nrequested+=n;
	}
}
move(t, pt, but)
	register Textframe *t;
	Point pt;
{
	Rectangle r;
	register n;
	if(t==DIAG)
		return;
	selectf(t, F_XOR);
	r=t->scrollrect;
	n=muldiv(pt.y-r.origin.y-M, t->nlines, r.corner.y-r.origin.y-2*M);
	if(n<0)
		n=0;
	if(but==B1)	/* backwards */
		n= -n;
	pt=ptofchar(t, t->s1);
	(void)scroll(t, n);
	setsel(t, charofpt(t, pt));
	send(t->file, O_SELECT, t->s1, 2, data2(0));
}
Texture deadmouse = {
	 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x000C, 0x0082, 0x0441,
	 0xFFE1, 0x5FF1, 0x3FFE, 0x17F0, 0x03E0, 0x0000, 0x0000, 0x0000,
};
waitunix(flag)
	register *flag;
{
	cursswitch(&deadmouse);
	*flag=FALSE;
	while(*flag==FALSE){
		wait(RCV);
		if(rcv())
			message();
	}
	cursswitch((Texture *) 0);
}
message()
{
	register f, n, op, posn;
	register char *data;
	static s1, s2;
	String rcvstr;
	register Textframe *t;
	register numdata;
	static maxlength=0;	/* maximum length of a file name */
	f=m.file;
	t=frameoffile(f);
	n=m.nbytes;
	op=m.op;
	posn=m.posn;
	data=m.data;
	data[n]=0;
	numdata=((unsigned char)data[0])+(data[1]<<8);
	switch(op){
	case O_DIAGNOSTIC:
		mesg(data, FALSE);
		break;
	case O_INSERT:	/* only comes after a REQUEST */
		if(n==0 || t==0){	/* t==0 is rare, but can come from a race */
	    Done:
			iodone=TRUE;
			break;
		}
		rcvstr.n=n;
		rcvstr.s=data;
		instext(t, &rcvstr, posn);
		ntoload-=n;
		if((*donetest)())
			goto Done;
		urequest(f);
		break;
	case O_SEARCH:
		keepsearch(posn);
		break;
	case O_SEEK:
		tellseek(t, posn);
		break;
	case O_RESET:
		seek(t, Pt(0, 0), 0);
		if(t!=current)
			curse(t);
		break;
	case O_SELECT:
		curse(t);
		s1=posn;
		s2=s1+numdata;
		break;
	case O_MOVE:
		if(posn==-1)	/* it's out in left field; reset */
			initframe(t);
		else
			selectf(t, F_XOR);
		if(posn<=0)	/* 0==> just before top of screen */
			loadfile(t, 0, integer(data));
		/* otherwise it's on the screen now */
		t->s1=s1;
		t->s2=s2;
		t->selecthuge=0;
		if(s2>t->str->n){
			s2=t->str->n;
			t->selecthuge=1;
		}
		selectf(t, F_XOR);
		curse(t);
		break;
	case O_SCROLL:
		scrolllines=TRUE;
		break;
	case O_DONE:
		diagdone=TRUE;
		break;
	case O_FILENAME:
		if(f<NFRAME){	/* could be greater if all full */
			setname(f, data);
			if(n<=maxlength)
				adjustnames(maxlength);
		}
		filedone=f;
		break;
	case O_MODIFIED:
		modified(t, posn);
		break;
	case O_NAMELENGTH:
		adjustnames(maxlength=numdata);
		break;
	case O_CHARSONSCREEN:
		send(0, O_CHARSONSCREEN, charofpt(t, Pt(XMAX, YMAX)), 0, (char *)0);
		break;
	default:
		mesg("unk\n", TRUE);
	}
}
integer(s)
	register char *s;
{
	return (unsigned char)s[0]+(s[1]<<8);
}
int diagnewline=TRUE;
mesg(s, sendit)
	register char *s;
{
	char buf[80+1];
	register char *p=buf, *q=s;
	String diagstr;
	register Textframe *t=DIAG;
	register i;
	if(diagnewline){	/* last thing caused a newline */
		Rectf(DIAG->rect, current==t? diagclr: F_CLR);
		zerostring(t->str);
		setsel(t, 0);
	}
	diagnewline=FALSE;
	while(*p= *q++)
		p++;
	if(sendit)
		sendstr(DIAG, O_INSERT, t->s1, p-buf, s);
	if(p>&buf[0] && p[-1]=='\n'){
		diagnewline=TRUE;
		*--p=0;
	}
	diagstr.n=i=p-buf;
	diagstr.s=buf;
	instext(t, &diagstr, t->s1);
	setsel(t, t->s1+i);
	if(p>&buf[0] && sendit==FALSE){
		rXOR(DIAG->rect);
		sleep(10);
		rXOR(DIAG->rect);
	}
}
/*VARARGS1*/
/*
dprintf(a,b,c,d,e,f,g,h)
	char *a;
{
	char buf[81];
	sprintf(buf, a, b, c, d, e, f, g, h);
	mesg(buf, TRUE);
}
*/
sendstr(f, op, posn, n, d)
	Textframe *f;
	register n;
	register char *d;
{
	register unsigned m;
	do{
		if((m=n)>NDATA)
			m=NDATA;
		send(f->file, op, posn, m, d);
		posn+=m;
		d+=m;
		n-=m;
	}while(n > 0);
}
scrolltest()
{
	return(ntoload<=0);
}
scroll(t, nlines)
	register Textframe *t;
	register nlines;
{
	register nchars;
	if(nlines==0 || t==DIAG)
		return 0;
	send(t->file, O_SCROLL, nlines, 0, (char *)0);
	waitunix(&scrolllines);
	nchars=m.posn;
	if(nchars>0){	/* scroll forwards */
		t->s1=0;
		if(nchars>t->str->n)
			t->s2=t->str->n;
		else
			t->s2=nchars;
		t->selecthuge=0;
		deltext(t, F_CLR);
	}else{	/* scroll backwards */
		setsel(t, 0);
		donetest=scrolltest;
		loadfile(t, 0, -nchars);
		donetest=usualtest;
	}
	return nchars;
}
char *
data2(n){
	static char x[2];
	x[0]=n;
	x[1]=n>>8;
	return x;
}
Send(op, posn, n, s)
	register op;
	register posn;
	register n;
	register char *s;
{
	send(current->file, op, posn, n, s);
}

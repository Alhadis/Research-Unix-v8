#include <jerq.h>
#include "layer.h"
#include "queue.h"
#include "jerqproc.h"
#include "../msgs.h"
#include "pconfig.h"
#include "proto.h"
#include "packets.h"

struct Pchannel pconvs[NPROC];
struct Proc *debugger;
extern int recvchars(), sendpkt();
#define	crecvchars	recvchars

struct Pconfig pconfig={
	sendpkt,
	recvchars,
	(void(*)())crecvchars
};

extern short second;

#define	INSET	2
Layer *whichlayer();
struct Proc *kbdproc=0;
#define	DEMUX	0
#define	CONTROL 1
#define	UP	0
#define	DOWN	1
int control(), windowstart(), demux();
struct Mouse mouse;
main(){
	register struct Proc *p;
	extern int end;
	/* jinit() by hand... */
	*DADDR=(156*1024L/4);
	*DSTAT=1;
	qinit();
	aciainit();
	binit();
	kbdinit();
	Lgrey(display.rect);
	cursinit();
	allocinit(&end, (int *)(90*1024L));
	if(pinit(NPROC)==-1)
		error("pinit", "-1");
	bufinit();
	spl0();
	(void)newproc(demux);	/* process 0 */
	p=newproc(control);	/* process 1 */
	setrun(p);
	resume(p);
}
buttons(updown)
{
	do; while((button123()!=0) != updown);
}
Sw()
{
	if(second){
		second=0;
		if(Ptflag)
			ptimeout();
	}
	sw(P!=&proctab[CONTROL]);	/* if control, clock will restart us */
}
short usermouse=0; /* kbdproc (a USER proc) has the mouse under its paw */
control(){
	register Layer *lp;
	register struct Proc *p, *pp;
	for(;;){
		pp=0;
		lp=whichlayer();
		for(p=proctab+CONTROL+1; p<&proctab[NPROC]; p++){
			if(p->state&WAKEUP){
				p->state&=~WAKEUP;
				setrun(p);
			}
			if(lp && p->layer==lp)
				pp=p;		/* pp pointed at by mouse */
		}
		if(usermouse && (pp!=kbdproc || (pp->state&GOTMOUSE)==0)){
			usermouse=0;
			cursswitch((Texture *)0);
			cursallow();
		}else if(!usermouse && pp){
	Check_mouse:	if(pp==kbdproc && (pp->state&GOTMOUSE)){
				usermouse=1;
				cursswitch(pp->cursor);
				if(pp->inhibited)
					cursinhibit();
			}
		}
		if(button123()){
			if(lp==0 || (pp->state&GOTMOUSE)==0){
				dobutton(whichbutton());
				/* make sure kbdproc doesn't think
				   buttons are down */
				if(kbdproc){
					givemouse(kbdproc);
					goto Check_mouse;	/* usermouse==0 */
				}
			}
			if(pp && pp->state&GOTMOUSE)
				givemouse(pp);
		}
		if(RCVQUEUE.c_cc)
			setrun(&proctab[DEMUX]);
		Sw();
	}
}
Texture bullseye = {
	 0x07E0, 0x1FF8, 0x399C, 0x63C6, 0x6FF6, 0xCDB3, 0xD99B, 0xFFFF,
	 0xFFFF, 0xD99B, 0xCDB3, 0x6FF6, 0x63C6, 0x399C, 0x1FF8, 0x07E0,
};
Texture boxcurs = {
	0x43FF, 0xE001, 0x7001, 0x3801, 0x1D01, 0x0F01, 0x8701, 0x8F01,
	0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0xFFFF,
};
New(){
	newwindow(windowstart);
}
Psend(a, b, c, d)
	char *b;
{
	while(psend(a, b, c, d)==-1)
		Sw();
}
Delete(){
	delete(whichlayer());
}
delete(l)
	register Layer *l;
{
	register struct Proc *p;
	register w;
	if(l){
		w=whichproc(l);
		p= &proctab[w];
		mpxmesg(w, C_DELETE);
		delproc(p);
		dellayer(l);
		if(kbdproc==p)
			kbdproc=0;
	}
}
delproc(p)
	register struct Proc *p;
{
	p->state=0;		/* exit(w) */
	p->nticks=0;
	qclear(&p->kbdqueue);
	freemem(p);
	p->layer=0;		/* sigh */
}
Top(){
	upfront(whichlayer());
}
Bottom(){
	register Layer *lp;
	lp=whichlayer();
	if(lp){
		upfront(lp);
		while(lback!=lp)
			upfront(lback);
	}
}
Current(){
	register Layer *l;
	l=whichlayer();
	if(l)
		tolayer(l);
}
Rectangle
canon(p1, p2)
	Point p1, p2;
{
	Rectangle r;
	r.origin.x = min(p1.x, p2.x);
	r.origin.y = min(p1.y, p2.y);
	r.corner.x = max(p1.x, p2.x);
	r.corner.y = max(p1.y, p2.y);
	return(r);
}
Rectangle
getrect(){
	Rectangle r;
	Point p1, p2;
	cursswitch(&boxcurs);
	buttons(UP);
	buttons(DOWN);
	p1=mouse.xy;
	p2=p1;
	r=canon(p1, p2);
	outline(r);
	for(; button3(); nap(2)){
		outline(r);
		p2=mouse.xy;
		r=canon(p1, p2);
		outline(r);
	}
	outline(r);	/* undraw for the last time */
	cursswitch((P->state&USER)? P->cursor : (Texture *)0);
	return r;
}
Reshape(){
	register Layer *l;
	register struct Proc *p;
	Rectangle r;
	l=whichlayer();
	if(l==0)
		return;
	p= &proctab[whichproc(l)];
	r=getrect();
	if(r.corner.x-r.origin.x>100 && r.corner.y-r.origin.y>40){
		Rectangle save;
		save=l->rect;
		dellayer(l);
		p->state&=~MOVED;
		p->state|=RESHAPED;
		if((l=newlayer(r))==0 && (l=newlayer(r=save))==0){
			/* one desperate try... */
			r.corner=add(r.origin, Pt(100, 40));
			if((l=newlayer(r))==0){	/* oh shit */
				delproc(p);
				mpxmesg((int)(p-proctab), C_DELETE);
				return;
			}
		}
		p->layer=l;
		p->rect=inset(r, INSET);
		if(p!=kbdproc)
			shade(l);
	}
	if(p->state&USER)
		setdata(p);
	mpxnewwind(p, C_RESHAPE);
}
Move(){
	register Layer *l, *nl;
	register struct Proc *procp;
	Point p, op, dp;
	l=whichlayer();
	if(l==0)
		return;
	procp= &proctab[whichproc(l)];
	dp=sub(l->rect.corner, l->rect.origin);
	cursset(l->rect.origin);
	cursswitch(&boxcurs);
	p=l->rect.origin;
	while(button3()){
		if(button12())
			goto Return;
		outline(Rpt(p, add(p, dp)));
		nap(2);
		op=p;
		p=mouse.xy;
		/* using boxcurs, can't get off top or left! */
		if(p.x+dp.x >= XMAX-9)
			p.x=XMAX-9-dp.x;
		if(p.y+dp.y >= YMAX-9)
			p.y=YMAX-9-dp.y;
		outline(Rpt(op, add(op, dp)));
		cursset(p);
	}
	nl=newlayer(Rpt(p, add(p, dp)));
	if(nl==0)
		goto Return;
	if(procp!=kbdproc)
		shade(l);
	Ubitblt(l, l->rect, nl, p, F_STORE);
	procp->layer=nl;
	procp->rect=inset(nl->rect, INSET);
	dellayer(l);
	if(procp!=kbdproc)
		shade(nl);
	if(procp->state&USER)
		setdata(procp);
	if((procp->state&RESHAPED) == 0)
		procp->state|=MOVED|RESHAPED;	/* turn on RESHAPED for old progs */
	l=nl;
   Return:
	cursset(div(add(l->rect.origin, l->rect.corner), 2));
	/* No C_RESHAPE required */
}
Memory(){	/* Pretty show */
	Point p;
	p=mouse.xy;
	cursswitch((Texture *)0);
	cursinhibit();
	*DSTAT=0;
	while (button123()) {
		long y = mouse.xy.y;
		if (y<0 || y>1023)
			y = 0;
		y = ((y*156)/100)*25;
		*DADDR=y;
		nap(2);
	}
	*DADDR=(156*1024L/4);
	*DSTAT=1;
	cursallow();
	cursset(p);
}
/* button hit to indicate which process, invoked by debugger */
struct Proc *
debug(){
	extern Texture bullseye;
	register Layer *l;
	struct Proc *z=0;
	debugger=P;
	cursswitch(&bullseye);
	buttons(DOWN);
	if(button3() && (l=whichlayer()))
		z=&proctab[whichproc(l)];
	buttons(UP);
	return z;
}
char *menutext[]={
	"New", "Reshape", "Move", "Top", "Bottom", "Current",
	"Memory", "Delete",  0
};
int (*menufn[])()={
	New,	Reshape,Move,	Top,	Bottom,	Current,
	Memory,	Delete,	0,
};
Menu windowmenu={ menutext };
dobutton(b)
{
	register hit;
	register Layer *l;
	switch(b){
	case 1:
		if(l=whichlayer()){
			upfront(l);
			tolayer(l);
		}
		break;
	case 2:
		break;	/* dunno... */
	case 3:
		if((hit=menuhit(&windowmenu, 3))>=0){
			if(hit==0)	/* a little different because of getrect */
				New();
			else{
				cursswitch(&bullseye);
				buttons(DOWN);
				if(button3())
					(*menufn[hit])();
				cursswitch((Texture *)0);
			}
		}
		break;
	default:
		break;
	}
	buttons(UP);
}
whichproc(l)
	register Layer *l;
{
	register struct Proc *p;
	for(p=proctab+CONTROL+1; p<proctab+NPROC; p++)
		if(p->layer==l && (p->state&BUSY))
			return((int)(p-proctab));
	return(CONTROL+1);	/* HELP?? */
}
whichbutton()
{
	static int which[]={0, 3, 2, 2, 1, 1, 2, 2, };
	return which[mouse.buttons&7];
}
newwindow(fn)
	int (*fn)();
{
	register struct Proc *p;
	Rectangle r;

	r=getrect();
	if(r.corner.x-r.origin.x>100 && r.corner.y-r.origin.y>40){
		if(p=newproc(fn)){	/* Assignment = */
			p->rect=inset(r, INSET);
			if(p->layer=newlayer(r)){
				shade(p->layer);
				mpxnewwind(p, C_NEW);
				tolayer(p->layer);
				setrun(p);
			}else
				p->state=0;
		}
	}
}

void
sendnchars(n, p)
	int n; char * p;
{
	register int	cc;

	do{
		if((cc=n)>MAXPKTDSIZE-1)
			cc=MAXPKTDSIZE-1;
		Psend((int)(P-proctab), p, cc, C_SENDNCHARS);
	}while(p+=cc, (n-=cc)>0);
}

short	sendbusy;

int
sendpkt(p, n)
	register char *	p;
	register int	n;
{
	register int	sr;

	while(sendbusy) sw(1);
	sendbusy=1;
	while(OUTQUEUE.c_cc>CBSIZE/2) sw(1);
	sr=spl1();
	trdisable();
	do
		qputc(&OUTQUEUE, *(unsigned char *)p++);
	while(--n);
	trenable();
	splx(sr);
	sendbusy=0;
	aciatrint();
	return 0;
}

mpxnewwind(p, c)
	register struct Proc *p;
	char c;
{
	char	mesg[6];
	register int	dx, dy;
	register char *	cp = mesg;

	dx=p->rect.corner.x-p->rect.origin.x;
	dy=p->rect.corner.y-p->rect.origin.y;
	*cp++=(dx-6)/9;
	*cp++=(dy-6)/16;
	*cp++=dx;
	*cp++=(dx>>8);
	*cp++=dy;
	*cp++=(dy>>8);
	Psend((int)(p-proctab), mesg, sizeof mesg, c);
}
int
mpxsendchar(c, p)
	char c;
	struct Proc *p;
{
	if(sendbusy || (OUTQUEUE.c_cc >= (CBSIZE/2)))
		return -1;	/* avoid "sw" in "sendpkt" */
	return psend((int)(p-proctab), &c, 1, C_SENDCHAR);
}
int
mpxkill(s, p)
	char s;
	struct Proc *p;
{
	Psend((int)(p-proctab), &s, 1, C_KILL);
}
mpxmesg(w, m){
	Psend(w, (char *)0, 0, m);
}
mpxublk(p)
	register struct Proc * p;
{
	register int l = p-proctab;

	while(pconvs[l].user > 0){
		pconvs[l].user--;
		Psend(l, (char *)0, 0, C_UNBLK);
	}
}
outline(r)
	Rectangle  r;
{
	register dx=r.corner.x-r.origin.x-1, dy=r.corner.y-r.origin.y-1;
	jmoveto(r.origin);
	jline(Pt(dx, 0), F_XOR);
	jline(Pt(0, dy), F_XOR);
	jline(Pt(-dx,0), F_XOR);
	jline(Pt(0,-dy), F_XOR);
}
min(a, b){
	return(a<b? a : b);
}
max(a, b){
	return(a>b? a : b);
}
Layer *
whichlayer()
{
	register Layer *lp;
	for(lp=lfront; lp; lp=lp->back)
		if(ptinrect(mouse.xy, lp->rect))
			return(lp);
	return(0);
}
tolayer(l)
	register Layer *l;
{
	register struct Proc *p;
	for(p=proctab; p<&proctab[NPROC]; p++)
		if((p->state&BUSY) && l==p->layer){
			if(kbdproc!=p){
				if(kbdproc){
					shade(kbdproc->layer);
					kbdproc->state&=~GOTMOUSE;
				}
				shade(p->layer);
				kbdproc=p;
			}
			if(p->state&MOUSELOCAL){
				p->state|=GOTMOUSE;
				setrun(p);
			}
			break;
		}
}
Texture shademap={
	0x1000, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
};
shade(l)
	Layer *l;
{
#ifndef	NOSTIPPLES
	ltexture(l, inset(l->rect, INSET), &shademap, F_XOR);
#endif
}
clear(r, inh)
	Rectangle r;
{
	if(inh)
		cursinhibit();
#ifndef	NOSTIPPLES
	if(P==kbdproc)
#endif
		lrectf(P->layer, r, F_CLR);
#ifndef	NOSTIPPLES
	else
		ltexture(P->layer, r, &shademap, F_STORE);
#endif
	if(inh)
		cursallow();
}

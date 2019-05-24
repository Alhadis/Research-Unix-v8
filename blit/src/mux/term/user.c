#include <jerq.h>
#include <layer.h>
#include "jerqproc.h"
#include <font.h>
static inhibited;
Texture *Ucursswitch();

Point
transform(p)
	Point p;
{
#	define	o P->rect.origin
#	define	c P->rect.corner
	p.x=muldiv(c.x-o.x, p.x, XMAX)+o.x;
	p.y=muldiv(c.y-o.y, p.y, YMAX)+o.y;
#undef	o
#undef	c
	return p;
}
Rectangle
rtransform(r)
	Rectangle r;
{
	r.origin=transform(r.origin);
	r.corner=transform(r.corner);
	return r;
}
char *
Ualloc(n)
	unsigned n;
{
	extern char *realalloc();
	return realalloc(n, (char *)P);
}
Bitmap *
Uballoc(r)
	Rectangle r;
{
	extern Bitmap *realballoc();
	return realballoc(r, (char *)P);
}
Ujinit(){
	P->inhibited=0;
	P->cursor=0;
}
Ujline(p, f)
	Point p;
	Code f;
{
	Point next;
	next=add(P->curpt, p);
	Ujsegment(P->curpt, next, f);
}
Ujlineto(p, f)
	Point p;
	Code f;
{
	Ujsegment(P->curpt, p, f);
}
Ujmove(p)
	Point p;
{
	P->curpt=add(P->curpt, p);
}
Ujmoveto(p)
	Point p;
{
	P->curpt=p;
}
Ujpoint(p, f)
	Point p;
	Code f;
{
	Upoint(P->layer, transform(p), f);
}
Ujrectf(r, f)
	Rectangle r;
	Code f;
{
	Urectf(P->layer, rtransform(r), f);
}
Ujsegment(p, q, f)
	Point p, q;
	Code f;
{
	Point pp,qq;
	pp=transform(p);
	qq=transform(q);
	if(!eqpt(pp, qq))
		Usegment(P->layer, transform(p), transform(q), f);
	P->curpt=q;
}
extern Font defont;
Point
Ujstring(s)
	char *s;
{
	int x;
	Point p;
	p=string(&defont, s, P->layer, transform(P->curpt), F_XOR);
	x=p.x;
	P->curpt.x+=muldiv(jstrwidth(s),XMAX,P->rect.corner.x-P->rect.origin.x);
	while(transform(P->curpt).x<x)
		P->curpt.x++;
	return P->curpt;
}
Ujtexture(r, t, f)
	Rectangle r;
	Texture *t;
	Code f;
{
	Utexture(P->layer, rtransform(r), t, f);
}
Uexit(){
	extern windowstart();
	shutdown(P);
	exec(windowstart);
}
Urequest(r)
{
	extern struct Proc *kbdproc;
	register struct Proc *p=P;
	if(r&KBD)
		p->state|=KBDLOCAL;
	else
		p->state&=~KBDLOCAL;
	if(r&MOUSE){
		p->state|=MOUSELOCAL;
		if(kbdproc==p){
			p->state|=GOTMOUSE;
			if(p->state&USER)	/* if not, it's windowproc */
				givemouse(p);
		}
		sleep(1);	/* Let control() update the mouse */
	}else{
		Ucursallow();
		Ucursswitch((Texture *) 0);
		p->state&=~(GOTMOUSE|MOUSELOCAL);
	}
	if(r&ALARM)
		p->state|=ALARMREQD;
	else
		p->state&=~ALARMREQD;
}
Urcvchar(){
	if(P->nchars==0)
		return -1;
	return(getchar());
}
Ukbdchar(){
	if((P->state&KBDLOCAL)==0)
		return -1;
	return(qgetc(&P->kbdqueue));
}
Uown()
{
	register got=CPU|SEND;
	if(P->state&GOTMOUSE)
		got|=MOUSE;
	if(P->kbdqueue.c_cc>0)
		got|=KBD;
	if(P->nchars>0)
		got|=RCV;
	if(P->state&ALARMREQD && P->nticks== 0)
		got|=ALARM;
	return got;
}
Uwait(r)
	register r;
{
	register u;

	if((r&RCV) && P->nchars==0)
		mpxublk(P);
	sw(1);
	if(r==0)
		return;	/* dumb person */
	spl1();
	while((u=Uown()&r)==0 && (r&CPU)==0){
		spl0();
		sw(0);
		spl1();
	}
	spl0();
	return u;
}
Ucursallow(){
	if(P->state&MOUSELOCAL){
		if(ptinrect(mouse.xy, P->layer->rect) && (P->state&GOTMOUSE))
			cursallow();
		P->inhibited=0;
	}
}
Ucursset(p)
	Point p;
{
	if(P->state&GOTMOUSE){
		cursset(p);
		givemouse(P);
	}
}
Ucursinhibit(){
	if(P->state&MOUSELOCAL){
		if(ptinrect(mouse.xy, P->layer->rect) && (P->state&GOTMOUSE))
			cursinhibit();
		P->inhibited=1;
	}
}
Texture *
Ucursswitch(t)
	register Texture *t;
{
	Texture *ot;
	ot=0;
	if(P->state&MOUSELOCAL){
		ot=P->cursor;
		if(ptinrect(mouse.xy, P->layer->rect) && (P->state&GOTMOUSE))
			cursswitch(t);
		P->cursor=t;
	}
	sleep(1);
	return ot;
}
Point
string(f,s,b,p,fc)
	register Font *f;
	unsigned char *s;
	register Layer *b;
	Point p;
	int fc;
{
	register c;
	int full = (fc == F_STORE);
	Point q;
	Rectangle r;
	register Fontchar *i;
	inhibited=0;
	if(fc!=F_XOR && b->base>=display.base){
		cursinhibit();
		inhibited=1;
	}
	if (full) {
		r.origin.y = 0;
		r.corner.y = f->height;
	}
	for (; c = *s++; p.x += i->width) {
		i = f->info + c;
		if (!full) {
			r.origin.y = i->top;
			r.corner.y = i->bottom;
		}
		r.origin.x = i->x;
		r.corner.x = (i+1)->x;
		q.x = p.x+i->left;
		q.y = p.y+r.origin.y;
		if (b->obs == 0)
			bitblt(f->bits,r,b,q,fc);
		else
			lblt(f->bits,r,b,q,fc);
	}
	if(inhibited)
		cursallow();
	return(p);
}
Usendchar(c)
	char c;
{
	while(mpxsendchar(c, P) == -1)
		sw(1);
}
Upoint(l, p, f)
	register Layer *l;
	Point p;
{
	inhibited=0;
	if(f!=F_XOR && l->base>=display.base){
		cursinhibit();
		inhibited=1;
	}
	lpoint(l, p, f);
	if(inhibited)
		cursallow();
}
Ubitblt(s, r, d, p, f)
	Layer *s, *d;
	Rectangle r;
	Point p;
{
#define	ROMBASE	(Word *)(256*1024L)
	inhibited=0;
	if((f!=F_XOR && d->base>=display.base) || (s->base>=display.base && s->base<ROMBASE)){
		cursinhibit();
		inhibited=1;
	}
	lbitblt(s, r, d, p, f);
	if(inhibited)
		cursallow();
}
Urectf(l, r, f)
	Layer *l;
	Rectangle r;
{
	inhibited=0;
	if(f!=F_XOR && l->base>=display.base){
		cursinhibit();
		inhibited=1;
	}
	/* speed hack: rectf clips */
	if(l->obs)
		lrectf(l, r, f);
	else
		rectf(l, r, f);
	if(inhibited)
		cursallow();
}
Usegment(l, p, q, f)
	Layer *l;
	Point p, q;
{
	inhibited=0;
	if(f!=F_XOR && l->base>=display.base){
		cursinhibit();
		inhibited=1;
	}
	lsegment(l, p, q, f);
	if(inhibited)
		cursallow();
}
Utexture(l, r, t, f)
	Layer *l;
	Rectangle r;
	Texture *t;
{
	inhibited=0;
	if(f!=F_XOR && l->base>=display.base){
		cursinhibit();
		inhibited=1;
	}
	/* speed hack; texture clips anyway */
	if(l->obs)
		ltexture(l, r, t, f);
	else
		texture(l, r, t, f);
	if(inhibited)
		cursallow();
}
Uscreenswap(b, r, s)
	Bitmap *b;
	Rectangle r, s;
{
	cursinhibit();
	screenswap(b, r, s);
	cursallow();
}
struct Proc *
Unewproc(f)
	int (*f)();
{
	extern windowstart();
	if(f==0)
		f=windowstart;
	return newproc(f);
}

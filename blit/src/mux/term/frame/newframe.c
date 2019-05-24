#include <jerq.h>
#include <layer.h>
#include <queue.h>
#include "jerqproc.h"
#include "frame.h"

Rectangle Null /* ={0,0,0,0} */;
Frame *
newframe(r)
	Rectangle r;
{
	register Frame *f;
	if((f=(Frame *)Ualloc(sizeof(Frame)))==0)
		return 0;
	if((f->str=(String *)Ualloc(sizeof(String)))==0){
		free((char *)f);
		return 0;
	}
	insure(f->str, MINCHARS, P);
	setrects(f, r);
	setcpl(f, 0, f->nlines-1);
	f->scroll.x=pixtoclix(f, f->rect.origin.y+10);
	f->scroll.y=pixtoclix(f, f->rect.corner.y-10);
	drawframe(f);
	return f;
}
setrects(f, r)
	register Frame *f;
	Rectangle r;
{
	f->totalrect=r;
	f->rect=inset(r, M);
	f->rect.corner.y-=(f->rect.corner.y-f->rect.origin.y)%newlnsz;
	f->scrollrect=f->rect;
	f->scrollrect.origin.x+=2;
	f->scrollrect.corner.x=f->scrollrect.origin.x+SCROLLWIDTH;
	f->rect.origin.x+=3*SCROLLWIDTH/2;
	f->nlines=(f->rect.corner.y-f->rect.origin.y)/newlnsz;
}
drawframe(f)
	register Frame *f;
{
	Urectf(D, f->totalrect, F_OR);
	clear(inset(f->totalrect, 2), 1);
	drawscrollbar(f);
	draw(f, f->rect.origin, f->str->s, f->str->n);
	selectf(f, F_XOR);
}
closeframe(t)
	register Frame *t;
{
	if(t==0 || t->str==0)
		return;
	gcfree(t->str->s);
	t->str->s=0;
	t->str->n=t->str->size=0;
	t->totalrect=Null;
	t->rect=Null;
	t->scrollrect=Null;
	t->str=0;	/* Mark it available */
}
setsel(t, n)
	register Frame *t;
	register n;
{
	t->s1=t->s2=n;
}

#include "frame.h"

Rectangle Null /* ={0,0,0,0} */;
Textframe frame[NFRAME];
Textframe *
newframe(r)
	Rectangle r;
{
	register short i;
	register Textframe *t;
	for (i = 0; frame[i].str!=0 && i<NFRAME; i++)	/* find an empty Textframe */
		;
	if(i >= NFRAME){
		mesg("couldn't allocate a frame\n", FALSE);
		for(;;)
			sw(0);
	}
	t = &frame[i];
	t->str = TEXT(i);		/* get it a string index */
	setrects(t, r, t==&frame[0]);
	return t;
}
setrects(t, r, isdiag)
	register Textframe *t;
	Rectangle r;
{
	t->totalrect = r;
	t->rect = inset(r, M);
	t->rect.corner.y -= (t->rect.corner.y-t->rect.origin.y)%newlnsz;
	t->nlines = (t->rect.corner.y-t->rect.origin.y)/newlnsz;
	if(!isdiag){
		t->scrollrect = t->rect;
		t->scrollrect.origin.x += 2;
		t->scrollrect.corner.x = t->scrollrect.origin.x+SCROLLWIDTH;
		t->rect.origin.x += SCROLLWIDTH+SCROLLWIDTH/2;
	}
}
initframe(t)
	register Textframe *t;
{
	box(t);
	t->obscured = FALSE;
	zerostring(t->str);
	setcpl(t, 0, t->nlines-1);
	setsel(t, 0);
}
closeframe(t)
	register Textframe *t;
{
	if(t==0 || t->str==0)
		return;
	gcfree(t->str->s);
	t->str->s=0;
	t->str->n=t->str->size=0;
	t->totalrect=Null;
	t->rect=Null;
	t->scrollrect=Null;
	t->obscured=1;	/* force workinframe() to redraw */
	t->file=0;
	t->str=0;	/* Mark it available */
}

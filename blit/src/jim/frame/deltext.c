#include "frame.h"

#define	INFINITY	32767

cut(t, save, f_clr)
	register Textframe *t;
{
	register n;
	extern snarfhuge;
	if ((n=t->s1) != t->s2) {
		if (save) {
			snarfhuge=t->selecthuge;
			snarf(t->str, n, t->s2, BUF);
		}
		deltext(t, f_clr);
		setsel(t, n);
	}
}
/*			BULLSHIT
 * Note the great asymmetry between the calling conventions for instext()
 * and deltext().  This is because deltext really needs two Spots to work
 * well, and is only called twice, whereas instext has an easier time of
 * it and is called in difficult places.
 */
deltext(t, f_clr)
	register Textframe *t;
{
	register char *p, *q;
	register n, y1, y2;
	Point pt;
	selectf(t, f_clr);	/* also sets F_rectf */
	p=t->str->s+t->s2;
	for(n=0,q=p; *q && *q!='\n'; q++)
		n++;
	/* Found the '\n' at the end of this line; clear out */
	frameop(t, oprectf, ptofchar(t, t->s2), p, n);
	y1=endpoint.y;	/* current y position of '\n' at p[n] */
	/* Draw rest of line at new place */
	pt=ptofchar(t, t->s1);
	draw(t, pt, p, n);
	y2=endpoint.y;
	if(y1 != y2){	/* More housekeeping */
		/* NOTE: y1 > y2 */
		y1+=newlnsz;
		y2+=newlnsz;
		/* Scroll up */
		bitblt(D, Rect(t->rect.origin.x, y1,
			t->rect.corner.x, t->rect.corner.y),
			D, Pt(t->rect.origin.x, y2), F_STORE);
		/* Clear the rest */
		rectf(D, Rpt(Pt(t->rect.origin.x,
			t->rect.corner.y-(y1-y2)),
			t->rect.corner), f_clr);
		y1-=newlnsz;
		y2-=newlnsz;
	}
	y1=lineno(t, y1);
	y2=lineno(t, y2);
	delstring(t->str, t->s1, t->s2);
	if(y1 == y2)
		setcpl(t, lineno(t, pt.y), y1);
	else
		setcpl(t, 0, t->nlines-1);	/* SLOW, correct */
	t->s2=t->s1;
	if(y1 != y2){
		n=charofpt(t, Pt(XMAX, (t->nlines-(y1-y2)-1)
			*newlnsz+t->rect.origin.y));	/* Re-use of n */
		/* n is last visible char */
		draw(t, ptofchar(t, n), t->str->s+n, t->str->n-n);
		if(complete)	/* There's more to get from file */
			loadfile(t, t->str->n, INFINITY);
	}
}

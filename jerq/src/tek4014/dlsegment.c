#include <jerq.h>
#include <layer.h>
#include "dline.h"

/*ARGSUSED*/
static void
dLsegment(l, r, db, fp, o)
	Layer *l;
	Rectangle r;
	Bitmap *db;
	register struct{
		Point p0, p1;
		Code f;
	}*fp;
	Obscured *o;
{
	Jdclipline(db, r, fp->p0, fp->p1, fp->f);
}

dlsegment(l, p, q, f)
	Layer *l;
	Point p, q;
	Code f;
{
	Point p0, p1;
	p0=p;
	if(p.x==q.x && p.y==q.y)
		return;
	p1=Jdsetline(p, q);
	finder(l, dLsegment, l->rect, p0, p1, f);
}
#define	out(r)	Rfinder(l, fn, r, fp, o->next)	/* r outside box */
#define in(r)	(*fn)(l, r, o->bmap, fp, o);	/* r inside box */
Rfinder(l, fn, r, fp, o)
	register Layer *l;
	void (*fn)();
	Rectangle r;	/* Screen coords */
	int *fp;
	register Obscured *o;
{
	register Rectangle *rp= &r, *bp;

Top:
	bp= &o->rect;
	if(o==0)
		(*fn)(l, r, l, fp, o);
	else if(rectXrect(*rp, *bp)==0){
		o=o->next;
		goto Top;
	}else{
		if(rp->origin.x<bp->origin.x){
			out(Rpt(rp->origin, Pt(bp->origin.x, rp->corner.y)));
			rp->origin.x=bp->origin.x;
		}
		if(rp->origin.y<bp->origin.y){
			out(Rpt(rp->origin, Pt(rp->corner.x, bp->origin.y)));
			rp->origin.y=bp->origin.y;
		}
		if(rp->corner.x>bp->corner.x){
			out(Rpt(Pt(bp->corner.x, rp->origin.y), rp->corner));
			rp->corner.x=bp->corner.x;
		}
		if(rp->corner.y>bp->corner.y){
			out(Rpt(Pt(rp->origin.x, bp->corner.y), rp->corner));
			rp->corner.y=bp->corner.y;
		}
		in(*rp);
	}
}

finder(l, fn, r, f)
	register Layer *l;
	void (*fn)();
	Rectangle r;	/* Screen coords */
	int f;
{
	if(rectclip(&r, l->rect)==0)
		return;
	Rfinder(l, fn, r, &f, l->obs);
}

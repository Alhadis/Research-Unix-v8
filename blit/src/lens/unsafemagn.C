#include <jerq.h>
/*
 * Like bitblt, only magnifies.  fac = (xscale, yscale), expansion factors.
 * F_STORE into tb only.  Assumptions: b != tb != db; tb offscreen and trashable;
 * db is the ultimate destination (0 if none desired) bitmap, and dp is the
 * destination point in that bitmap; the ultimate bitblt is done in mode mode.  If
 * db is 0, the magnified image is found in tb, in a rectangle based at (0,0).
 * nmagnify does the last bitblt for you so that it can freely 'reshape' the
 * bitmap, and noone will notice.  If tb is exactly the right size, this is
 * not important, so feel free to set db = (Bitmap *)0.  tb->rect.origin MUST
 * equal (0,0).
 */
nmagnify(b, r, tb, db, dp, fac, mode)
	register Bitmap *b, *tb, *db;
	Rectangle r;
	Point dp, fac;
	int mode;
{
	Bitmap temp;
	register i, shift;
	Point d,q,o;
	int s,w;
	if(fac.x<1 || fac.y<1)
		return;
	d=sub(q=r.corner, o=r.origin);
	if ((d.x <= 0) || (d.y <= 0))
		return;
	temp = *tb;
	if (db)
		tb->width = (d.x * fac.x + 15) / 16;
	tb->rect.corner.y = d.y;
	w = tb->width * 16;
	tb->width *= fac.y;
	tb->rect.corner.x = tb->width * 16;
	s=fac.x*d.x;
	rectf(tb, tb->rect, F_CLR);
	/* 1: expand horizontally (and vertically, cheating) */
	if (fac.x==1)
		bitblt(b,r,tb,Pt(0,0),F_OR);
	else
		for(i=0; i<d.x; i++){
			bitblt(b, Rect(o.x+i, o.y, o.x+i+1, q.y),
			       tb, Pt(i*fac.x, 0), F_OR);
		}
	/* 2: smear horizontally */
	for(i=1; i<fac.x; i<<=1){
		shift=min(i, fac.x-i);
		Bitblt(tb, Rect(0, 0, s-shift, d.y),
		       tb, Pt(shift, 0), F_OR);
	}
	/* 3: smear 'vertically' */
	for(i=1; i<fac.y; i<<=1){
		shift=min(i, fac.y-i)*w;
		bitblt(tb, Rect(0, 0, shift, d.y),
		       tb, Pt(i*w, 0), F_OR);
	}
	if (db) {
		tb->rect.corner.y *= fac.y;
		tb->rect.corner.x = d.x * fac.x;
		tb->width /= fac.y;
		bitblt(tb, tb->rect, db, dp, mode);
	}  
	*tb = temp;	
}

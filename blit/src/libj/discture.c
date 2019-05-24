#include <jerq.h>
#ifndef	MPX
#define	Jcursinhibit()	cursinhibit()
#define	Jcursallow()	cursallow()
#define	lrectf(l, r, f)	rectf(l, r, f)
#endif
/*	Fill a disc of radius r centered at x1,y1
 *	The boundary is a sequence of vertically, horizontally,
 *	or diagonally adjacent points that minimize 
 *	abs(x^2+y^2-r^2).
 *
 *	The circle is guaranteed to be symmetric about
 *	the horizontal, vertical, and diagonal axes
 */
short inhibited;
discture(b, p, r, t, f)
	Bitmap *b;
	Point p;
	Texture *t;
{
	int eps,exy,dxsq,dysq;
	register x0,y0,x1,y1;
	r--;
	eps = 0;
	dxsq = 1;
	dysq = 1 - 2*r;
	x0 = p.x-1;
	x1 = p.x+1;
	y0 = p.y-r-1;
	y1 = p.y+r;
	inhibited=0;
	if(f!=F_XOR && b->base>=(Word *)(156*1024)){
		Jcursinhibit();
		inhibited=1;
	}
	while(y1 > y0) {
		exy = eps + dxsq + dysq;
		if(-exy <= eps+dxsq) {
			ltexture(b, Rect(x0, y0, x1, y0+1), t, f);
			ltexture(b, Rect(x0, y1, x1, y1+1), t, f);
			y1--;
			y0++;
			eps += dysq;
			dysq += 2;
		}
		if(exy <= -eps) {
			x1++;
			x0--;
			eps += dxsq;
			dxsq += 2;
		}
	}
	if(inhibited)
		Jcursallow();
}

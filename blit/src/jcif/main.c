#include "cif.h"

#define BIN 1000
int *bin,bincnt,binmax;
int symcnt,linecnt = 1;
#define SYMS 300
Symbol syms[SYMS];
Point thispt;
Rectangle thisrect;
Transform thistrans,nulltrans = {
	1,0,
	0,1,
	0,0,
};
#define c corner
#define o origin

Rectangle pttobox(p,w)
Point p;
int w;
{
	int v = w/2;
	return(newrect(newpt(p.x-v,p.y-v),newpt(p.x+v,p.y+v)));
}

Rectangle segtorect(w,p1,p2)
int w;
Point p1,p2;
{
	int v = w/2;
	return(newrect(newpt(min(p1.x,p2.x)-v,min(p1.y,p2.y)-v),
	    newpt(max(p1.x,p2.x)+v,max(p1.y,p2.y)+v)));
}

Point minpt(p1,p2)
Point p1,p2;
{
	return(newpt(min(p1.x,p2.x),min(p1.y,p2.y)));
}

Point maxpt(p1,p2)
Point p1,p2;
{
	return(newpt(max(p1.x,p2.x),max(p1.y,p2.y)));
}

Rectangle mbbpt(r,p)
Rectangle r,p;
{
	return(newrect(minpt(r.origin,p),maxpt(r.corner,p)));
}

Rectangle mbbrect(r1,r2)
Rectangle r1,r2;
{
	return(newrect(minpt(r1.origin,r2.origin),maxpt(r1.corner,r2.corner)));
}

Rectangle inset(r,n)
Rectangle r;
register n;
{
	r.origin.x += n;
	r.origin.y += n;
	r.corner.x -= n;
	r.corner.y -= n;
	return(r);
}
rectltrect(r,s)		/* r is smaller than s */
Rectangle r,s;
{
	return((r.c.x-r.o.x) <= (s.c.x-s.o.x) &&
		(r.c.y-r.o.y) <= (s.c.y-s.o.y));
}

rectXrect(r,s)		/* r intersects s */
Rectangle r,s;
{
	return(r.o.x<s.c.x && s.o.x<r.c.x && r.o.y<s.c.y && s.o.y<r.c.y);
}

Point ptatbin(i)
int i;
{
	Point p;
	p.x = bin[i++];
	p.y = bin[i];
	return(p);
}

Rectangle rectatbin(i)
int i;
{
	Rectangle r;
	r.origin = ptatbin(i);
	r.corner = ptatbin(i + 2);
	return(r);
}

Transform transatbin(i)
int i;
{
	Transform t;
	t.t11 = bin[i++];
	t.t12 = bin[i++];
	t.t21 = bin[i++];
	t.t22 = bin[i++];
	t.tx = bin[i++];
	t.ty = bin[i];
	return(t);
}

printpt(p)
Point p;
{
	printf("%d,%d",p.x,p.y);
}

printrect(r)
Rectangle r;
{
	printpt(r.origin);
	printf(" : ");
	printpt(r.corner);
	printf("\n");
}

printtrans(t)
Transform t;
{
	printf("11 12 %d %d\n21 22 %d %d\n x  y %d %d\n",
	    t.t11,t.t12,t.t21,t.t22,t.tx,t.ty);
}

markbos(i)
int i;
{
	syms[i].pc = bincnt;
}

inttobin(i)
int i;
{
	register int j,*newbin;
	if (bincnt >= binmax) {
		binmax = (binmax == 0) ? BIN : 4 * binmax;
		if ((newbin = (int *) alloc(sizeof(int) * binmax)) == (int *) -1)
			yyerror("bin alloc failed!");
		for (j = 0; j < bincnt; j++)
			newbin[j] = bin[j];
		bin = newbin;
	}
	bin[bincnt++] = i;
}

pttobin(p)
Point p;
{
	inttobin(p.x);
	inttobin(p.y);
}

recttobin(r)
Rectangle r;
{
	pttobin(r.origin);
	pttobin(r.corner);
}

transtobin(t)
Transform t;
{
	inttobin(t.t11);
	inttobin(t.t12);
	inttobin(t.t21);
	inttobin(t.t22);
	inttobin(t.tx);
	inttobin(t.ty);
}

Point newpt(x,y)
int x,y;
{
	Point p;
	p.x = x;
	p.y = y;
	return(p);
}

Rectangle newrect(p1,p2)
Point p1,p2;
{
	Rectangle r;
	r.origin = p1;
	r.corner = p2;
	return(r);
}

Rectangle boxtorect(x,y,p)
int x,y;
Point p;
{
	int w,h;
	w = x / 2;
	h = y / 2;
	return(newrect(newpt(p.x-w,p.y-h),newpt(p.x+w,p.y+h)));
}

Transform newtrans(t11,t12,t21,t22,tx,ty)
int t11,t12,t21,t22,tx,ty;
{
	Transform t;
	t.t11 = t11;
	t.t12 = t12;
	t.t21 = t21;
	t.t22 = t22;
	t.tx = tx;
	t.ty = ty;
	return(t);
}

Point transpt(t,p)
Transform t;
Point p;
{
	extern int scale;
	return(newpt((t.t11*p.x + t.t21*p.y + t.tx) >> scale,
	    (t.t12*p.x + t.t22*p.y + t.ty) >> scale));
}

Rectangle transrect(t,r)
Transform t;
Rectangle r;
{
	return(sortfrompt(transpt(t,r.origin),transpt(t,r.corner)));
}

Rectangle sortfrompt(p1,p2)
Point p1,p2;
{
	return(newrect(newpt(min(p1.x,p2.x),
	    min(p1.y,p2.y)),
	    newpt(max(p1.x,p2.x),
	    max(p1.y,p2.y))));
}

Transform transtrans(t,s)
Transform t,s;
{
	return(newtrans(t.t11*s.t11 + t.t21*s.t12,
	    t.t12*s.t11 + t.t22*s.t12,
	    t.t11*s.t21 + t.t21*s.t22,
	    t.t12*s.t21 + t.t22*s.t22,
	    t.t11*s.tx  + t.t21*s.ty  + t.tx,
	    t.t12*s.tx  + t.t22*s.ty  + t.ty));
}


#include "cip.h"

#define DOT 2
#define DASH 10
#define ARROWwid 10
#define ARROWht 5

Rectangle saveScreenmap;
Word *saveBase;
extern Rectangle brushes[];
struct thing addOffset();

draw(t,offset,f) register struct thing *t; Point offset; register int f;
{	register struct thing *s; Rectangle rc; Point p1,p2; register int u;
	register Font *ft;
    if (t != (struct thing *) NULL) {
	cursinhibit();
	display.base = addr(&display,brushes[PIC].origin);
	display.rect = brushes[PIC];
	switch(t->type) {
	case CIRCLE:
		jcircle(add(offset,t->origin),t->otherValues.radius,f);
		break;
	case BOX:
		rc = raddp(t->bb,offset);
		if (t->border == DOTTED) dashedBox(rc,DOT,f);
		else if (t->border == DASHED) dashedBox(rc,DASH,f);
		else box(rc,f);
		break;
	case ELLIPSE:
		Ellipse(add(offset,t->origin),t->otherValues.ellipse.ht,
			t->otherValues.ellipse.wid,f);
		break;
	case LINE: 
		p1 = add(t->origin,offset);
		p2 = add(t->otherValues.end,offset);
		if (t->border == DOTTED) dashedLine(p1,p2,DOT,f);
		else if (t->border == DASHED) dashedLine(p1,p2,DASH,f);
		else jsegment(p1,p2,f);
		if ((t->arrow==startARROW)||(t->arrow==doubleARROW))
			arrow(p2,p1,f);
		if ((t->arrow==endARROW)||(t->arrow==doubleARROW))
			arrow(p1,p2,f);
		break;
	case ARC:
		jarc(add(offset,t->origin),add(offset,t->otherValues.arc.start),
			add(offset,t->otherValues.arc.end),f);
		break;
	case TEXT:
		ft = t->otherValues.text.f->f;
		u = strwidth(ft,t->otherValues.text.s);
		switch (t->otherValues.text.just) {
		case CENTER:
		    	p1 = add(offset,sub(t->origin,Pt(u>>1,0)));
		    	break;
		case LEFTJUST:
		    	p1 = add(offset,t->origin);
		    	break;
		case RIGHTJUST:
		    	p1 = add(offset,sub(t->origin,Pt(u,0)));
		    	break;
		}
		string(ft,t->otherValues.text.s,&display,p1,F_XOR);
		break;
	case SPLINE:
		u = t->otherValues.spline.used;
	    	jspline(offset,t->otherValues.spline.plist,u,f);
		if ((t->arrow==startARROW)||(t->arrow==doubleARROW))
			arrow(add(offset,t->otherValues.spline.plist[2]),
			add(offset,t->origin),f);
		if ((t->arrow==endARROW)||(t->arrow==doubleARROW))
			arrow(add(offset,t->otherValues.spline.plist[u-2]),
			add(offset,t->otherValues.spline.plist[u]),f);
	    	break;
	case MACRO:
		if ((s=t->otherValues.list->parts) != (struct thing *)NULL) 
			do {
				draw(s,add(offset,t->origin),f);
				s = s->next;
			} while (s != t->otherValues.list->parts);
		break;
	}
	display.base = saveBase;
	display.rect = saveScreenmap;
	cursallow();
    }
}

box(r,f) Rectangle r; register int f;
{
	jsegment(r.origin,Pt(r.corner.x,r.origin.y),f);
	jsegment(Pt(r.corner.x,r.origin.y),r.corner,f);
	jsegment(r.corner,Pt(r.origin.x,r.corner.y),f);
	jsegment(Pt(r.origin.x,r.corner.y),r.origin,f);
}

dashedBox(r,dashsize,f) Rectangle r; register int dashsize, f;
{
	dashedLine(r.origin,Pt(r.corner.x,r.origin.y),dashsize,f);
	dashedLine(Pt(r.corner.x,r.origin.y),r.corner,dashsize,f);
	dashedLine(r.corner,Pt(r.origin.x,r.corner.y),dashsize,f);
	dashedLine(Pt(r.origin.x,r.corner.y),r.origin,dashsize,f);
}

dashedLine(s,end,dashsize,f) Point s, end; int dashsize, f;
{	register int e, dx, dy, i, toDraw, yinc, xinc, swit;
	dx = abs(end.x - s.x);
	dy = abs(end.y - s.y);
	xinc = ((end.x-s.x)>0)? 1 : -1;
	yinc = ((end.y-s.y)>0)? 1 : -1;
	swit = (dy>dx);
	toDraw = 1;
	e = (swit)? (2*dx - dy) : (2*dy - dx);
	for (i=0; i < ((swit) ? dy : dx); i++) {
		if (i>0 && i%dashsize==0) toDraw = (toDraw==1)?0:1;
		if (toDraw) jpoint(s,f);
		if (e>0) {
			if (swit) s.x += xinc;
			else s.y += yinc;
			e += (swit)? (2*dx - 2*dy) : (2*dy - 2*dx);
		}
		else e += (swit)? 2*dx : 2*dy;
		if (swit) s.y += yinc;
		else s.x += xinc;
	}
}

int degrees(d)  register int d;
{
	while (d>360) d -= 360;
	while (d<0) d += 360;
	return(d);
}

arrow(a, b, f) Point a,b; register int f;	/* draw arrow (without line) */
{
	register int alpha, rot, hyp;
	register int dx, dy;

	rot = atan2( ARROWwid / 2, ARROWht);
	hyp = norm(ARROWwid,ARROWht,0);
	alpha = atan2(b.y-a.y, b.x-a.x);
	dx = muldiv(hyp,cos(degrees(alpha + 180 + rot)),1024);
	dy = muldiv(hyp,sin(degrees(alpha + 180 + rot)),1024);
	/* line(x1+dx, y1+dy, x1, y1); */
	if ((b.x==a.x) && (b.y < a.y)) dy = -dy;
	cursinhibit(); jsegment(add(b,Pt(-dx,dy)),b,f); cursallow();
	dx = muldiv(hyp,cos(degrees(alpha + 180 - rot)),1024);
	dy = muldiv(hyp,sin(degrees(alpha + 180 - rot)),1024);
	/* line(x1+dx, y1+dy, x1, y1); */
	if ((b.x==a.x) && (b.y < a.y)) dy = -dy;
	cursinhibit(); jsegment(add(b,Pt(dx,-dy)),b,f); cursallow();
}

centeredText(p,s) Point p; register char *s; 
{
	jmoveto(Pt(p.x-(jstrwidth(s)>>1),p.y)); 
	jstring(s);
}

flash(b,offset) register Rectangle *b; Point offset;
{
    if (b != (Rectangle *) NULL) {
	cursinhibit();
	jrectf(inset(raddp(*b,offset),2),F_XOR);
	cursallow();
    }
}

flashThing(t,offset) register struct thing *t; Point offset;
{	
    if (t != (struct thing *) NULL) {
	cursinhibit();
	switch (t->type) {
	case CIRCLE:
	case BOX:
	case ELLIPSE:
	case LINE:
	case ARC:
	case SPLINE:
		draw(t,offset,F_XOR);
		break;
	case MACRO:
	case TEXT:
		flash(&t->bb,offset);
		break;
	}
	cursallow();
    }
}

Ellipse(p,h,w,f) Point p; register int h, w, f;
{
	jellipse(p, w>>1, h>>1, f);
}

drawZigZag(offset,p,n,f) Point offset;  register Point *p;  int n,f; 
{ 	register int i;
    	if (p != (Point *) NULL) {
		cursinhibit();
		if (n>0) {
			jmoveto(add(offset,p[1]));
			for (i=2; i<=n; i++) jlineto(add(offset,p[i]),f);
		}
		cursallow();
    	}
}

jspline(offset,p, n, f) Point offset; register Point *p; int n, f;
{	register long w, t1, t2, t3, scale=1000; 
	register int i, j, steps=10; 
	Point q;
    	if (p != (Point *) NULL) {
		p[0] = p[1];
		p[n] = p[n-1];
		cursinhibit();
		jmoveto(add(offset,p[0]));
		for (i = 0; i < n-1; i++) {
			for (j = 0; j < steps; j++) {
				w = scale * j / steps;
				t1 = w * w / (2 * scale);
				w = w - scale/2;
				t2 = 3*scale/4 - w * w / scale;
				w = w - scale/2;
				t3 = w * w / (2*scale);
				q.x = (t1*p[i+2].x + t2*p[i+1].x + 
					t3*p[i].x + scale/2) / scale;
				q.y = (t1*p[i+2].y + t2*p[i+1].y + 
					t3*p[i].y + scale/2) / scale;
				jlineto(add(offset,q), f);
			}
		}
		cursallow();
	}
}


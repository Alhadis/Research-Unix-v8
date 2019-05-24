#include "cip.h"

editThing(m,p,t)  Point m,p; register struct thing *t;
{   register int d,dx,dy,u;  Point o, np; 
    if (t != (struct thing *) NULL) {
    	o = t->origin;
	switch(t->type) {
	case CIRCLE:
		d = norm(o.x-m.x,o.y-m.y,0);
		if (d< (t->otherValues.radius>>1)) track(m,p,MOVE,t); 
		else track(m,p,GROWCIRCLE,t);
		d = t->otherValues.radius;
		break;
	case BOX:
		np = t->otherValues.corner;
		dx = (np.x - o.x)>>2;
		dy = (np.y - o.y)>>2;
		if (ptinrect(m,Rect(o.x+dx,o.y+dy,np.x-dx,np.y-dy))) {
			if (t->border != SOLID) {
				draw(t,p,F_CLR);
				box(raddp(t->bb,p),F_STORE);
			}
			t->bb = moveBox(m,t->bb,t,p);
			t->otherValues.corner = t->bb.corner;
			t->origin = t->bb.origin;
			box(raddp(t->bb,p),F_XOR);
		}
		else {
			draw(t,p,F_CLR);
			t->origin.x = 0;
			t->origin.y = 0;
			t->bb.origin = t->origin;
			t->bb.corner = t->origin;
			t->otherValues.corner = t->origin;
			if (ptinrect(m,Rect(o.x-dx,o.y-dy,o.x+dx,o.y-dy))) ;
			else if (ptinrect(m,Rect(o.x-dx,np.y-dy,o.x+dx,np.y+dy))) 
				np.y = o.y;
			else if (ptinrect(m,Rect(np.x-dx,np.y-dy,np.x+dx,np.y+dy))) 
				np = o;
			else if (ptinrect(m,Rect(np.x-dx,o.y-dy,np.x+dx,o.y+dy))) 
				np.x = o.x;
			o = track(np,p,BOX,t);
			t->origin.x = min(o.x,np.x);
			t->origin.y = min(o.y,np.y);
			t->otherValues.corner.x = max(o.x,np.x);
			t->otherValues.corner.y = max(o.y,np.y);
		}		
		break;
	case ELLIPSE:
		dx = abs(m.x - o.x);  
		dy = abs(m.y - o.y);
		d = norm(dx,dy,0);
		if ((dx > dy) && (d > (t->otherValues.ellipse.wid>>2))) 
			track(m,p,GROWEWID,t); 
		else if ((dx < dy) && (d > (t->otherValues.ellipse.ht>>2))) 
			track(m,p,GROWEHT,t); 
		else track(m,p,MOVE,t); 
		break;
	case LINE:
		np=t->otherValues.end;
		draw(t,p,F_CLR);
		if (distance(m,o)<distance(m,np)) 
			t->origin = track(np,p,LINE,t);
		else t->otherValues.end = track(o,p,LINE,t);
		break;
	case ARC:
		d = (distance(o,t->otherValues.arc.start))>>1;
		if (distance(o,m)<d)  arcOrigin(t,p);
		else if (distance(m,t->otherValues.arc.start)<
			distance(m,t->otherValues.arc.end)) arcStart(t,p);
		else arcEnd(t,p);
		break;
	case MACRO:
		draw(t,p,F_CLR);
		box(raddp(t->bb,p),F_XOR);
		t->bb = moveBox(m,t->bb,t,p);
		t->origin = t->bb.origin;
		box(raddp(t->bb,p),F_XOR);
		break;
	case TEXT:
	    	track(m,p,MOVE,t);
	    	break;
	case SPLINE:
		u = t->otherValues.spline.used;
		d=findNearestPoint(m,t->otherValues.spline.plist,u);
		if ((t->arrow==startARROW)||(t->arrow==doubleARROW))
			arrow(add(p,t->otherValues.spline.plist[2]),
			add(p,t->origin),F_XOR);
		if ((t->arrow==endARROW)||(t->arrow==doubleARROW))
			arrow(add(p,t->otherValues.spline.plist[u-2]),
			add(p,t->otherValues.spline.plist[u]),F_XOR);
		if ((d == u-1) || (d==1)) {
			jsegment(add(p,t->otherValues.spline.plist[(d==1)?1:(d-1)]),
				add(p,t->otherValues.spline.plist[(d==1)?2:d]),F_XOR);
			np=track(t->otherValues.spline.plist[(d==1)?2:(d-1)],
				p,LINE,t);
		}
		else np=track2(p,t->otherValues.spline.plist[d-1],
			t->otherValues.spline.plist[d+1],
			t->otherValues.spline.plist[d]);
		jspline(p,t->otherValues.spline.plist,u,F_CLR);
		t->otherValues.spline.plist[d]=np;
		break;
	} /* end switch */
	BoundingBox(t);
	if (t->type!=TEXT) draw(t,p,F_STORE);	
    }
} 
		
struct thing *copyThing(t,p,offset) register struct thing *t; Point p, offset;
{	register struct thing *c; Point *pl; int i, n; 
    if (t != (struct thing *) NULL) {
	switch(t->type) {
	case CIRCLE:
		c = newCircle(p);
		c->otherValues.radius = t->otherValues.radius;
		break;
	case BOX:
		c = newBox(p,add(p,sub(t->bb.corner,t->origin)));
		c->border = t->border;
		break;
	case ELLIPSE:
		c = newEllipse(p);		
		c->otherValues = t->otherValues;
		break;
	case LINE:
		c = newLine(p,add(p,sub(t->otherValues.end,t->origin)));
		c->border = t->border;
		c->arrow = t->arrow;
		break;
	case ARC:
		c = newArc(p,add(p,
			sub(t->otherValues.arc.end,t->otherValues.arc.start)));
		break;
	case SPLINE:
		n = t->otherValues.spline.used;
		if ((pl = (Point *) alloc((n+2)*sizeof(Point)))==(Point *)NULL)
			outOfSpace();
		p = sub(p, t->origin);
		for (i=1; i<=n; i++) 
			pl[i]=add(p,t->otherValues.spline.plist[i]);
		c = newSpline(n,n,pl);		
		break;
	case TEXT:
		c = newText(p,t->otherValues.text.s);
		c->otherValues.text.f = t->otherValues.text.f;
		c->otherValues.text.just = t->otherValues.text.just;
		break;
	case MACRO:
		c = newMacro(p,t->otherValues.list);
		break;
	}
	BoundingBox(c);
	draw(c,offset,F_STORE);
	return(c);
    }
    else return(t);
}

int findNearestPoint(o,p,n) register Point *p; int n; Point o;
{	register int i; int f, d, mind = -1;
    if (p != (Point *) NULL) {
	for (i=1; i<=n; i++) {
		d = norm(o.x-p[i].x, o.y-p[i].y, 0);
		if ((mind<0) || (mind>d)) {
			mind = d;
			f = i;
		}
	}
	return(f);
    }
    else return(-1);
}

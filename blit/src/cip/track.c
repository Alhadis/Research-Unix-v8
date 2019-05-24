#include "cip.h"

Point track(p,offset,b,th) Point p,offset; int b; register struct thing *th;
{	Point t, ot, r;
	p = add(p,offset);
	ot = p;
	if ((th != (struct thing *) NULL) || (b==BOX || b==LINE || b==ARC))
	do {
		t = mouse.jxy;
		if ((t.x!=ot.x)||(t.y!=ot.y)) {
			cursinhibit();
			switch (b) {
			case BOX:
				r = near(t,th,offset);
				if (r.x != 0) t = r;
				else {
					r=near(Pt(p.x,t.y),th,offset);
					if (r.x!=0) t.y = r.y;
					else {
						r=near(Pt(t.x,p.y),th,offset);
						if (r.x != 0) t.x = r.x;
					}
				}
				box(Rect(min(p.x,ot.x),min(p.y,ot.y),
					max(p.x,ot.x),max(p.y,ot.y)),F_XOR);
				box(Rect(min(p.x,t.x),min(p.y,t.y),
					max(p.x,t.x),max(p.y,t.y)),F_XOR);
				break;
			case LINE:
				r=near(t,th,offset);
				if (r.x!=0) t=r;
				else if (abs(p.x-t.x)<nearlyStraight) t.x=p.x;
				else if (abs(p.y-t.y)<nearlyStraight) t.y=p.y;
				jsegment(p,ot,F_XOR);
				jsegment(p,t,F_XOR);
				break;
			case GROWCIRCLE:
				draw(th,offset,F_XOR);
				th->otherValues.radius=distance(add(th->origin,offset),t);
				draw(th,offset,F_XOR);
				break;
			case MOVE:
				r=near(t,th,offset);
				if (r.x != 0) t=r;
				draw(th,offset,F_XOR);
				th->origin = sub(t,offset);
				draw(th,offset,F_XOR);
				break;	
			case GROWEHT:
				draw(th,offset,F_XOR);
				th->otherValues.ellipse.ht =  
				    	abs(th->origin.y-t.y+offset.y)<<1;
				draw(th,offset,F_XOR);
				break;
			case GROWEWID:
				draw(th,offset,F_XOR);
				th->otherValues.ellipse.wid = 
					abs(th->origin.x-t.x+offset.x)<<1;
				draw(th,offset,F_XOR);
				break;
			case ELLIPSE:
				draw(th,offset,F_XOR);
				th->otherValues.ellipse.wid = 
					(abs(th->origin.x-t.x+offset.x))<<1;
				th->otherValues.ellipse.ht =  
				    	abs(th->origin.y-t.y+offset.y)<<1;
				draw(th,offset,F_XOR);
				break;
			case ARC:
				break;
			}
			ot = t;
			cursallow();
		}
		nap(2);
	} while (button2());
	cursinhibit();
	switch (b) {
	case BOX:
		box(Rect(min(p.x,ot.x),min(p.y,ot.y),
			max(p.x,ot.x),max(p.y,ot.y)),F_XOR);
		break;
	case LINE:
		jsegment(p,ot,F_XOR);
		break;
	}
	cursallow();
	return(sub(t,offset));
}

Point track2(offset,o1, o2, p) Point offset, o1, o2, p;
{	Point op;
	o1 = add(offset,o1);
	o2 = add(offset,o2);
	op = add(offset,p);
	do {
		p = mouse.jxy;
		cursinhibit();
		if ((p.x!=op.x)||(p.y!=op.y)) {
			jsegment(o1,op,F_XOR);
			jsegment(op,o2,F_XOR);
			jsegment(o1,p,F_XOR);
			jsegment(p,o2,F_XOR);
		}
		op = p;
		cursallow();
		nap(2);
	} while (button2());
	return(sub(p,offset));
}

Rectangle moveBox(p,r,h,offset)  Point p, offset; Rectangle r; 
register struct thing *h;
{	Rectangle or; Point dor, dc; Point op, q;
	or = r;
	dor= sub(or.origin,p);
	dc = sub(or.corner,p);
	op = add(p,offset);
	do {
		p = mouse.jxy;	
		r.origin = add(or.origin,sub(p,op));
		q = near(add(r.origin,offset),h,offset);
		if (q.x!=0) p = sub(q,dor);
		else {
			r.corner = add(or.corner,sub(p,op));
			q = near(add(r.corner,offset),h,offset);
			if (q.x!=0) p = sub(q,dc);
		}
		r.origin = add(or.origin,sub(p,op));
		r.corner = add(or.corner,sub(p,op));
		cursinhibit();
		box(raddp(or,offset),F_XOR);
		box(raddp(r,offset),F_XOR);
		op = p;
		or = r;
		cursallow();
		nap(2);
	} while (button2()); 
	return(r);
}
		
arcOrigin(t,offset) register struct thing *t; Point offset;
{	Point oc, c, c1, c2, s, e, om, m, org, mid; 
	s = add(offset,t->otherValues.arc.start);
	e = add(offset,t->otherValues.arc.end);
	org = add(offset,t->origin);
	mid = sub(org,div(add(s,e),2));
	oc = org;
	om = oc;
	do {
		cursinhibit();
		m = mouse.jxy;
		if (distance(m,om)>2) {
			c1.x = m.x;
			c1.y = muldiv(mid.y, c1.x-org.x, mid.x) + org.y;
			c2.y = m.y;
			c2.x = muldiv(c2.y-org.y, mid.x,mid.y) + org.x;
			c = (distance(oc,c1)<distance(c,c2))?c1:c2;
			eraseAndDrawArc(oc,e,s,c,e,s);
			oc = c;
			om = m;
		}
		cursallow();
		nap(2);
	} while (button2());
	t->origin = sub(c,offset);
}

arcStart(t,offset) register struct thing *t; Point offset;
{	Point oc, s, e, os; 
	os = add(offset,t->otherValues.arc.start);
	e = add(offset,t->otherValues.arc.end);
	oc = add(offset,t->origin);
	do {
		cursinhibit();
		s = mouse.jxy;
		if (distance(s,os)>2) {
			eraseAndDrawArc(oc,e,os,oc,e,s);
			os = s;
		}
		cursallow();
		nap(2);
	} while (button2());
	t->otherValues.arc.start=sub(s,offset);
}

arcEnd(t,offset) register struct thing *t; Point offset;
{	Point oc, s, e, oe; 
	s = add(offset,t->otherValues.arc.start);
	oe = add(offset,t->otherValues.arc.end);
	oc = add(offset,t->origin);
	do {
		cursinhibit();
		e = mouse.jxy;
		if (distance(e,oe)>2) {
			eraseAndDrawArc(oc,oe,s,oc,e,s);
			oe = e;
		}
		cursallow();
		nap(2);
	} while (button2());
	t->otherValues.arc.end=sub(e,offset);
}

eraseAndDrawArc(oc,oe,os,c,e,s) Point oc, oe, os, c, e, s;
{
	jarc(oc,os,oe,F_XOR);
	jsegment(oc,os,F_XOR);
	jsegment(oc,oe,F_XOR);
	jarc(c,s,e,F_XOR);
	jsegment(c,s,F_XOR);
	jsegment(c,e,F_XOR);
}

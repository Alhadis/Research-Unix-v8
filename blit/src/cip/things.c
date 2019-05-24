#include "cip.h"

extern struct macro *macroList;
extern int nextMacroName;
extern Texture hourglass;

struct thing *newCircle(p) Point p;
{	register struct thing *b;
	if ((b = (struct thing *) alloc(sizeof(struct thing)))== (struct thing *)NULL)
		outOfSpace();
	b->type = CIRCLE;
	b->origin = p;
	b->otherValues.radius = RADdefault;
	BoundingBox(b);
	b->arrow = 0;
	return(b);
}

struct thing *newBox(o,c) Point o, c;
{	register struct thing *b;
	if ((b = (struct thing *) alloc(sizeof(struct thing)))== (struct thing *)NULL)
		outOfSpace();
	b->type = BOX;
	b->origin = o;
	b->otherValues.corner = c;
	BoundingBox(b);
	b->border = SOLID;
	b->arrow = 0;
	return(b);
}

struct thing *newEllipse(p) Point p;
{	register struct thing *b;
	if ((b = (struct thing *) alloc(sizeof(struct thing)))== (struct thing *)NULL)
		outOfSpace();
	b->type = ELLIPSE;
	b->origin = p;
	b->otherValues.ellipse.ht = HTdefault;
	b->otherValues.ellipse.wid = WIDdefault;
	BoundingBox(b);
	b->border = SOLID;
	b->arrow = 0;
	return(b);
}

struct thing *newLine(o,c) Point o, c;
{	register struct thing *b;
	if ((b = (struct thing *) alloc(sizeof(struct thing)))== (struct thing *)NULL)
		outOfSpace();
	b->type = LINE;
	b->origin = o;
	b->otherValues.end = c;
	BoundingBox(b);
	b->border = SOLID;
	b->arrow = 0;
	return(b);
}

struct thing *newArc(s,e) Point s, e;
{	register struct thing *b; 
	if ((b = (struct thing *) alloc(sizeof(struct thing)))== (struct thing *)NULL)
		outOfSpace();
	b->type = ARC;
	b->otherValues.arc.start = s;
	b->otherValues.arc.end = e;
	b->origin = computeArcOrigin(s,e);
	BoundingBox(b);
	b->border = SOLID;
	b->arrow = 0;
	return(b);
}

struct thing *newText(p,s) Point p; char *s;
{	register struct thing *b; fontBlk *f;
	if ((b = (struct thing *) alloc(sizeof(struct thing)))== (struct thing *)NULL)
		outOfSpace();
	b->type = TEXT;
	b->origin = p;
	f = findFont(10,ROMAN);
	b->otherValues.text.f = f;
	b->otherValues.text.just = CENTER;
	b->otherValues.text.s = s;
	BoundingBox(b);
	b->border = SOLID;
	b->arrow = 0;
	return(b);
}

struct thing *newSpline(u,s,p) int u, s; Point *p;
{	register struct thing *b;
	if ((b=(struct thing *)alloc(sizeof(struct thing)))==(struct thing *)NULL)
		outOfSpace();
	b->type = SPLINE;
	b->origin = p[1];
	b->otherValues.spline.used = u;
	b->otherValues.spline.size = s;
	b->otherValues.spline.plist = p;
	BoundingBox(b);
	b->border = SOLID;
	b->arrow = 0;
	return(b);
}

struct thing *newMacro(p,l) Point p; struct macro *l; 
{	register struct thing *b;
	if ((b = (struct thing *) alloc(sizeof(struct thing)))== (struct thing *)NULL)
		outOfSpace();
	b->type = MACRO;
	b->origin = p;
	b->otherValues.list = l;
	l->useCount++;
	BoundingBox(b);
	b->border = SOLID;
	b->arrow = 0;
	return(b);
}		

struct thing *insert(t,list) register struct thing *t, *list;
{
    	if (t != (struct thing *) NULL) {
		if (list!=(struct thing*)NULL) {
			t->next = list;
			t->last = list->last;
			list->last->next = t;
			list->last=t;
		}
		else {
			t->last = t;
			t->next = t;
		}
    	}
	return(t);
}

fontBlk *insertFont(t) register fontBlk *t;
{
    	if (t != (fontBlk *) NULL) {
		if (fonts!=(fontBlk *)NULL) {
			t->next = fonts;
			t->last = fonts->last;
			fonts->last->next = t;
			fonts->last=t;
		}
		else {
			t->last = t;
			t->next = t;
		}
    	}
	return(t);
}

struct thing *remove(t) register struct thing *t;
{
	if (t != (struct thing *) NULL) {
		t->last->next = t->next;
		t->next->last = t->last;
	}
	return(( (t==(struct thing *)NULL) || (t==t->next)) ? 
		(struct thing *) NULL : t->next);
}

struct thing *deleteThing(t,p) register struct thing *t; Point p;
{	register struct thing *t1;
	if (t!=(struct thing *)NULL) {
		draw(t,p,F_CLR);  
		t1 = remove(t);
		if (t->type==MACRO) t->otherValues.list->useCount--;
		else if (t->type==TEXT) t->otherValues.text.f->useCount--;
		free((char *) t);
	}
	else t1 = t;
	return(t1);
}

struct thing *deleteAllThings(list) struct thing *list;
{	register struct thing *t, *h; register struct macro *m;
	/* I don't use deleteThing() 'cuz I don't want to undraw them all */
	if ((h = list) != (struct thing *) NULL) 
		do {
			t = h;
			h = h->next;
			free((char *) t);
		} while (h != list);
	while (macroList != (struct macro *) NULL) {
		m = macroList;
		macroList = m->next;
		free(m);
	}
	nextMacroName = 0;
	return((struct thing *) NULL);
}

struct thing *selectThing(m,list) Point m; register struct thing *list;
{  	register struct thing *t, *h = (struct thing *)NULL; register int d, hd=0;
	if ((t = list) != (struct thing *) NULL) 
		do {
			d = distance(t->bb.origin,t->bb.corner);
			if ( ptinrect(m,t->bb) && ((hd==0) || (d<hd)) ) {
				h = t;
				hd = d;
			} 
			t=t->next;
		} while (t != list);
	return(h);
}

int distance(p,q) Point p, q;
{	
	return ( norm(q.x-p.x,q.y-p.y,0) );
}

BoundingBox(t) register struct thing *t;
{	Point p, q, r; register int i, h; Font *f; 
	switch(t->type) {
	case CIRCLE:
		p.x = t->otherValues.radius;
		p.y = p.x;
		t->bb.origin = sub(t->origin,p);
		t->bb.corner = add(t->origin,p);
		break;	
	case BOX:
		t->bb.origin = t->origin;
		t->bb.corner = t->otherValues.corner;
		break;
	case ELLIPSE:
		p.x = (t->otherValues.ellipse.wid)>>1;
		p.y = (t->otherValues.ellipse.ht)>>1;
		t->bb.origin = sub(t->origin,p);
		t->bb.corner = add(t->origin,p);
		break;
	case LINE:
		p = t->origin;
		q = t->otherValues.end;
		if (abs(p.x-q.x)<10) 
			{p.x -= nearlyStraight; q.x += nearlyStraight;} 
		else if (abs(p.y-q.y)<10) 
			{p.y -= nearlyStraight; q.y += nearlyStraight;} 
		t->bb.origin.x = min(p.x,q.x);
		t->bb.origin.y = min(p.y,q.y);
		t->bb.corner.x = max(p.x,q.x);
		t->bb.corner.y = max(p.y,q.y);
		break;
	case ARC:
		p = t->origin;
		i = distance(p,t->otherValues.arc.start);
		t->bb.origin = sub(p,Pt(i,i));
		t->bb.corner = add(p,Pt(i,i));
		break;
	case TEXT:
		p = t->origin;
		f = t->otherValues.text.f->f;
		h = f->height;
		i = strwidth(f,t->otherValues.text.s);
		switch(t->otherValues.text.just) {
		case CENTER:
			t->bb.origin = sub(p,Pt(i>>1,0));
			t->bb.corner = add(p,Pt(i>>1,h));
			break;
		case LEFTJUST:
			t->bb.origin = p;
			t->bb.corner = add(p,Pt(i,h));
			break;
		case RIGHTJUST:
			t->bb.origin = sub(p,Pt(i,0));
			t->bb.corner = add(p,Pt(0,h));
			break;
		}
		break;
	case SPLINE:
		p.x = Xmin; p.y=YPIC; q.x=Xmax; q.y=YBOT;
		for (i=1; i<t->otherValues.spline.used; i++) {
			r = t->otherValues.spline.plist[i];
			p.x = max(p.x,r.x);  p.y = max(p.y,r.y);
			q.x = min(q.x,r.x);  q.y = min(q.y,r.y);
		}
		t->bb.origin = q;
		t->bb.corner = p;
		break;
	case MACRO:
		t->bb.origin = add(t->origin,t->otherValues.list->bb.origin);
		t->bb.corner = add(t->origin,t->otherValues.list->bb.corner);
		break;
	}
}

Point computeArcOrigin(s,e) Point s,e;
{	Point t;
	if (e.x<s.x) { /*swap s and e */
		t=s; s=e; e=t;
	}
	return( div(add(add(e,Pt(s.y-e.y,e.x-s.x)),s),2) );
}

outOfSpace()
{
	jrectf(Rect(Xmin,YBOT,Xbut,YBOT+4*LW),F_CLR);
	jmoveto(Pt(Xtext,Ytext));
	jstring("Out of Storage - PUT and QUIT");
}

fontBlk *findFont(s,n) register int s, n;
{	register fontBlk *h, *f = {(fontBlk *)NULL}; register int i; 
	register char c, fn[50]; 
	if ((h=fonts)!=(fontBlk *)NULL) 
		do {
			if (h->ps == s && h->num == n) f = h;
			h = h->next;
		} while (f==(fontBlk *)NULL && h!=fonts);
	if (f==(fontBlk *)NULL) {
		cursswitch(&hourglass);
		f=(fontBlk *)alloc(sizeof(fontBlk));
		c = (n==ROMAN) ? 'R' : ((n==ITALIC) ? 'I' : 'B');
		i = s+1;
		do {
			sprintf(fn,"/usr/blit/font/%c.%d",c,--i);
		} while ((i>0) && (access(fn,4)!=0));
		f->f = (i==0) ? &defont : getfont(fn);
		f->ps = s;
		f->num = n;
		f->useCount = 0;
		fonts = insertFont(f);
		cursswitch((Word *)NULL);
	}
	f->useCount++;
	return(f);
}

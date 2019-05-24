#include "cip.h"

int errCount = 0;
char filename[100];
char *getFileName();
int Yoffset = YPIC, Xoffset = Xmin;
struct macro *macroList;
int nextMacroName;
Rectangle macroBB();
char buf[50];

extern int currentBrush, copyFlag, gridState, buttonState;
extern char c;
extern Rectangle brushes[];
extern Rectangle BBpic;
extern struct thing *readPic();

Texture prompt = {
	0x0000, 0x0000, 0x07E0, 0x7FFF, 0xFFFF, 0xFFFF, 0x8AE3, 0xBAEF,
	0x9AE7, 0xBAEF, 0xBA23, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000
};

Texture hourglass = {
	0x1FF0, 0x1FF0, 0x0820, 0x0820, 0x0820, 0x0C60, 0x06C0, 0x0100,
	0x0100, 0x06C0, 0x0C60, 0x0820, 0x0820, 0x0820, 0x1FF0, 0x1FF0
};
 

doPut(h)  struct thing *h;
{	register struct thing *t; register char *fn; register FILE *fp; 
	register struct macro *m; register int saveB; Point floc;
	floc = mouse.jxy;
	floc.x += 20;
	saveB = buttonState;
	changeButtons(BLANKbuttons);
	fn = getFileName(floc);
	if (fn[0]!='\0') {
		if ((fp = fopen(fn,"w")) != (FILE *) NULL) {
			fprintf(fp,".PS\nscale=100\n");
			cursswitch(&hourglass);
			findBBpic(h);
			nextMacroName = 0;
			for (m=macroList; m!=(struct macro *)NULL; m=m->next) {
			    if (m->useCount>0) {
				m->outName = nextMacroName++;
				fprintf(fp,"define m%d |\n",m->outName);
			    	fprintf(fp,"[ box invis ht %d wid %d with .sw at 0,0\n",		
					m->bb.corner.y, m->bb.corner.x);
			    	if ((t=m->parts) !=(struct thing *)NULL) do {
					writePIC(t,fp,Rpt(Pt(0,0),m->bb.corner));
					t=t->next;
				} while (t!=m->parts);
			    	fprintf(fp,"] |\n\n");
			    }
			}
			fprintf(fp,"box invis ht %d wid %d with .sw at 0,0\n",
				BBpic.corner.y - BBpic.origin.y,
				BBpic.corner.x - BBpic.origin.x);
			if ((t = h) != (struct thing *) NULL) 
				do {
					writePIC(t,fp,BBpic);
					t = t->next;
				} while (t!=h);
			fprintf(fp,".PE\n");
			fclose(fp);
			cursswitch((Word *)NULL);
		}
		else fileError("open",fn);
	}
	jrectf(Rect(Xtext,YBOT,Xbut,(YBOT+(4*LW))),F_CLR);
	changeButtons(saveB);
}

fileError(s,fn) register char *s, *fn;
{	
	beep();
	jrectf(Rect(Xtext,YBOT,Xbut,(YBOT+(4*LW))),F_CLR);
	jmoveto(Pt(Xtext,Ytext));
	jmoveto(jstring("cannot "));
	jmoveto(jstring(s));
	jmoveto(jstring(" file "));
	jstring(fn);
}

struct thing *doGet(h) register struct thing *h;
{	register char *fn; register FILE *fp; Point floc;
	floc = mouse.jxy;
	floc.x += 20;
	changeButtons(BLANKbuttons);
	fn = getFileName(floc);
	if (fn[0]!='\0') {
		sprintf(buf,"/usr/blit/bin/Jpic %s",fn);
		if (access(fn,4)!=0) fileError("access",fn);
		else if ((fp = popen(buf,"r")) == (FILE *) NULL) 
			fileError("open pipe for",fn);
		else {
			cursswitch(&hourglass);
			h = doClear(h);
			getChar(fp);
			h = readPic(fp,h);
			pclose(fp);
			cursswitch((Word *)NULL);
		}
	}
	jrectf(Rect(Xtext,YBOT,Xbut,(YBOT+(4*LW))),F_CLR);
	changeButtons(INITbuttons);
	return(h);
}

char *getFileName(p) Point p;
{	Point m; register int i;
	clearKeyboard();
	cursswitch(&prompt);
	m = p;
	for (i=0; filename[i]!='\0'; i++) {
		jmoveto(m);
		m = jchar(filename[i]);
	}
	wait(KBD);
	for( ; (c=kbdchar())!='\r'; wait(KBD)) {
		switch (c) {
		case '@':
		case 'U'-'@':
			jmoveto(p);
			jstring(filename);
			i=0;
			m=p;
			break;
		case 'W'-'@':
			jmoveto(p);
			jstring(filename);
			i = backspaceOneWord(filename,i);
			jmoveto(p);
			m = jstring(filename);
			break;
		case '\b':
			jmoveto(p);
			jstring(filename);
			filename[(i>0)? --i : 0] = '\0';
			jmoveto(p);
			m = jstring(filename);
			break;
		default:
			filename[i++] = c;
			filename[i] = '\0';
			jmoveto(m);
			m = jchar(c);
			break;
		}
	}
	cursswitch((Word *)0);
	jmoveto(p);
	jstring(filename);
	return(filename);
}

struct thing *doClear(h) struct thing *h;
{
	deleteAllThings(h);
	cursinhibit();
	jrectf(inset(brushes[PIC],LW), F_CLR);
	cursallow();
	copyFlag = 0;
	changeBrush(-1); 
	if (gridState==GRIDon) drawGrid();
	return((struct thing *) NULL);
}

struct thing *defineMacro(h) register struct thing *h;
{	Point p, q; register struct thing *s, *t, *l = { (struct thing *)NULL }; 
	Rectangle r; struct thing dummy;
	changeBrush(-1);
	changeButtons(MACRObuttons);
	for (; !button2(); nap(2)) ;
	p = sub(mouse.jxy,Pt(Xmin,YPIC));
	q = track(p,Pt(Xmin,YPIC),BOX,h);
	r.origin.x = min(p.x,q.x);
	r.origin.y = min(p.y,q.y);
	r.corner.x = max(p.x,q.x);
	r.corner.y = max(p.y,q.y);
	h = insert(&dummy,h);
	if ((t = h->next) != (struct thing *) NULL) 
		do {
			if (inside(r,t->bb)) {
				s = remove(t);
				l = insert(t,l);
				t = s;
			}
			else t = t->next;
		} while (t != &dummy);
	h = remove(&dummy);
	if ((t = l) != (struct thing *) NULL) {
		r = macroBB(l);
		do {
			makeRelative(t,r.origin);
			t = t->next;
		} while (t != l);
		p = r.origin;
		r.origin = sub(r.origin,p);
		r.corner = sub(r.corner,p);
		changeButtons(INITbuttons);
		return( insert(newMacro(p,recordMacro(l,r,NULL,NULL,NULL)),h));	
	}
	else {
		changeButtons(INITbuttons);
		return(h);
	}
}	

Rectangle macroBB(l) struct thing *l;
{	Point p, q, p1, p2;  register struct thing *t; Rectangle r;
	p.x = Xmax; p.y=YBOT; q.x=0; q.y=0;
	if ((t=l) != (struct thing *)NULL)
		do {
			if (t->type != LINE) {
				p1 = t->bb.origin;
				p2 = t->bb.corner;
			}
			else {
				p1.x = min(t->origin.x,t->otherValues.end.x);
				p1.y = min(t->origin.y,t->otherValues.end.y);
				p2.x = max(t->origin.x,t->otherValues.end.x);
				p2.y = max(t->origin.y,t->otherValues.end.y);
			}
			p.x = min(p.x,p1.x);
			p.y = min(p.y,p1.y);
			q.x = max(q.x,p2.x);
			q.y = max(q.y,p2.y);
			t=t->next; 
		} while (t != l);
	r.origin = p; r.corner = q;
	return(r);
}

struct macro *recordMacro(list,r,xro,yro,s) 
struct thing *list; Rectangle r; struct macro *xro, *yro; char *s;
{	register struct macro *m, *l, *n;
	if ((m = (struct macro *) alloc(sizeof(struct macro)))==(struct macro *)NULL) 
		outOfSpace();
	else {
		m->name = s;
		m->bb = r;
		m->useCount = 0;
		m->parts = list;
		m->xReflectionOf = xro;
		m->yReflectionOf = yro;
		l = (struct macro *)NULL;
		for (n=macroList; n!=(struct macro *)NULL; n=n->next) l=n;
		if (l == (struct macro *)NULL) macroList = m;
		else l->next = m;
	}
	return(m);
}

int inside(r,s) Rectangle r,s;
{
	return((r.origin.x <= s.origin.x) && (r.origin.y <= s.origin.y)
		&& (r.corner.x >= s.corner.x) && (r.corner.y >= s.corner.y));
}

struct thing *makeRelative(t,p) register struct thing *t; Point p;
{	register int i;
	t->origin = sub(t->origin,p);
	switch(t->type) {
	case CIRCLE:
	case ELLIPSE:
	case TEXT:
		break;
	case LINE:
		t->otherValues.end = sub(t->otherValues.end,p);	
		break;
	case BOX:
		t->otherValues.corner = sub(t->otherValues.corner,p);
		break;
	case ARC:
		t->otherValues.arc.start = sub(t->otherValues.arc.start,p);
		t->otherValues.arc.end = sub(t->otherValues.arc.end,p);
		break;
	case SPLINE:
		for (i=0; i<=t->otherValues.spline.used; i++) 
			t->otherValues.spline.plist[i] = 
				sub(t->otherValues.spline.plist[i],p);
		break;
	}
	BoundingBox(t);
	return(t);
}

int backspaceOneWord(s,i) register char *s; register int i;
{
	s[(i>0)? --i : 0] = '\0';
	for ( ; i>0 && (isdigit(s[i-1]) || isletter(s[i-1])); ) s[--i]='\0';
	return(i);
}


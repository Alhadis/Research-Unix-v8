#include "cip.h"

char c;

char *getText();
Point getPoint();
struct macro *findMacro();

extern struct macro *macroList;
extern int Xoffset;
extern int Yoffset;

struct thing *readPic(fp,h) register FILE *fp; register struct thing *h;
{	register struct thing *t, *l;
	struct macro *m;
	Rectangle r;
	register int num, i, a , b;
	Point *plist, p, q;
	register char *s, type;
	
	while ( c != EOF) {
		t = (struct thing *) NULL;
		type = c;
		getChar(fp);
		switch(type) {
		case EOF:
		case ' ':
		case '\n':
		case '\t':
			break;
		case 'm': 	/* moveto */
			p = getPoint(fp);
			break;
		case 'e':	/* ellipse */
			p = getPoint(fp);
			t = newEllipse(p);
			t->otherValues.ellipse.wid = (getInt(fp))<<1;
			t->otherValues.ellipse.ht = (getInt(fp))<<1;
			break;
		case 'c':	/* circle */
			p = getPoint(fp);
			t = newCircle(p);
			t->otherValues.radius = getInt(fp);
			break;
		case 'a': 	/* arc */
			p = getPoint(fp);
			q = getPoint(fp);
			t = newArc(q,getPoint(fp));
			t->origin = p;
			break;
		case 'l':	/* line */
			a=0;
			skipWhiteSpace(fp);
			if (c=='<') {
				a += startARROW;	
				getChar(fp);
			}
			if (c=='>') {
				a += endARROW;	
				getChar(fp);
			}
			if (c=='.' || c=='-') {
				b = (c=='.')? DOTTED : DASHED;
				getChar(fp);
				getInt(fp); /*size*/
			}
			else b = SOLID;
			num =  getInt(fp);
			q = getPoint(fp);
			for (i=1; i<num; i++) {
				p = getPoint(fp);
				t = newLine(q,p);
				t->border = b;
				t->arrow = a;
				BoundingBox(t);
				draw(t,Pt(Xmin,YPIC),F_STORE);
				h = insert(t,h);
				q = p;
			}
			break;
		case 'b':	/* box */
			if (c=='.' || c=='-') {
				a = (c=='.')? DOTTED : DASHED;
				getChar(fp);
				getInt(fp); /*size*/
			}
			else a = SOLID;
			p = getPoint(fp);
			q = getPoint(fp);
			t = newBox(Pt(min(p.x,q.x),min(p.y,q.y)),
				Pt(max(p.x,q.x),max(p.y,q.y)));
			t->border = a;
			break;
		case 't':   	/* text */
			if (c!='c') {
				p = getPoint(fp);
				skipWhiteSpace(fp);
				type = c;
				getChar(fp);
				s = getText(fp,'\n');
				t = newText(p,s);
				extractFontandPointSize(t);
				t->origin.y -= (t->otherValues.text.f->f->height>>1);
				BoundingBox(t);
				switch(type) {
				case 'C':
				case 'c':
					t->otherValues.text.just = CENTER;
					break;
				case 'L':
				case 'l':
					t->otherValues.text.just = LEFTJUST;
					break;
				case 'R':
				case 'r':
					t->otherValues.text.just = RIGHTJUST;
					break;
				}
			}
			break;
		case '~':
			a = 0;
			skipWhiteSpace(fp);
			if (c=='<') {
				a += startARROW;	
				getChar(fp);
			}
			if (c=='>') {
				a += endARROW;	
				getChar(fp);
			}
			num = getInt(fp);
			plist= (Point *) alloc((num+3)*sizeof(Point));
			if (plist==(Point *) NULL) outOfSpace();
			for (i=1; i<=num; i++) plist[i]=getPoint(fp);
			plist[num+1]=plist[num];
			t = newSpline(num+1,num,plist);
			t->arrow = a;
			break;
		case '.':
			switch (c) {
			case 'P':
				getChar(fp);
				getChar(fp);
				Yoffset=(YBOT-YPIC-getInt(fp))>>1;
				Xoffset=(Xmax-Xmin-getInt(fp))>>1;
				break;
			case 'U':
			case 'u':
				/* start of macro */
				getChar(fp);
				skipWhiteSpace(fp);
				s = getText(fp,' ');
				flushLine(fp);
				l = readPic(fp,(struct thing *) NULL);
				r = macroBB(l);
				if ((m=findMacro(s))==(struct macro *)NULL) {
					if ((t=l)!=(struct thing *)NULL) do {
						makeRelative(t,r.origin);
						t = t->next;
					} while (t != l);
					m=recordMacro(l,rsubp(r,r.origin),
						NULL,NULL,s);
				}
				t = newMacro(r.origin,m);
				break;
			case 'E':
			case 'e':
				/* end of macro */
				flushLine(fp);
				return(h);
				break;
			default:
				flushLine(fp);
				break;
			}
			break;
		default:
			flushLine(fp);
			break;
		}
		if ((t != (struct thing *) NULL) && (t->type != LINE)) {
			BoundingBox(t);
			draw(t,Pt(Xmin,YPIC),F_STORE);
			h = insert(t,h);
		}
	}
	return(h);
}

Point getPoint(f) FILE *f;
{	Point p;
	p.x = getInt(f) + Xoffset;
	p.y = getInt(f) + Yoffset;
	return(p);
}

int getInt(f) FILE *f;
{	register int i;
	skipWhiteSpace(f);
	i=0;
	while (c >= '0' && c<='9' ) {
		i = 10 * i + c - '0';
		getChar(f);
	} 
	return(i);
}

getChar(f) FILE *f;
{	
	c = getc(f);
}

skipWhiteSpace(f) FILE *f;
{
	while( c==' ' || c=='\t' ) getChar(f);
}

char *getText(f,term) FILE *f; register char term;
{	register char *s; register int i;
	if ((s = (char *) alloc(100))==NULL) outOfSpace();
	for (i=0; c != term; getChar(f)) s[i++]=c;
	s[i]=0;
	getChar(f);
	return(s);
}

flushLine(f) FILE *f;
{
	while (c != '\n') getChar(f);
	getChar(f);
}

struct macro *findMacro(s) register char *s;
{	struct macro *m;
	for (m=macroList; ((m!=(struct macro *)NULL) &&
		(compare(s,m->name)==0)); m=m->next) ;
	return(m);
}

int compare(s1,s2) register char *s1, *s2;
{
	while ( *s1!='\0' && *s2!='\0' && *s1==*s2) {
		s1++; s2++;
	}
	return( (*s1=='\0' && *s2=='\0')?1:0 );
}

extractFontansPointSize(t) register struct thing *t;
{	register int f, ps, i, j, len; register char *s;
	s = t->otherValues.text.s;
	len = strlen(s);
	if (compare(&s[len-6],"\\f1\\s0") && s[0]=='\\' && s[1]=='f') {
		switch(s[2]) {
		case '1':
		case 'R':
			f = ROMAN;
			break;	
		case '2':
		case 'I':
			f = ITALIC;
			break;	
		case '3':
		case 'B':
			f = BOLD;
			break;	
		default:
			f = ROMAN;
			break;
		}
		ps = s[5] - '0';
		if (isdigit(s[6])) {
			ps = ps*10 + s[6] - '0';
			i = 7;
		}
		else i = 6;
		for (j=0; i<len-6; ) s[j++] = s[i++];
		s[j] = '\0';
		t->otherValues.text.f = findFont(ps,f);
	}
}		

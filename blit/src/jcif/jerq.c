/*
 *	jerq.c - plotting routines for Jerq
 */

#include <stdio.h>
#include <sgtty.h>
#include "/usr/blit/include/jioctl.h"
#include "cif.h"
#define BLT 100		/* limited number of Bitmaps in Jerq */
#define BITBLT	21	/* Jerq protocol */
#define BALLOC	22
#define POP	23
#define SCROLL	24
#define FREE	25
#define CLEAR	26
#define PORT	27
#define EOP	28

int nbytes;
outchar(c)
char c;
{
	nbytes++;
	putchar(c);
}

typedef struct {
	int key;
	Rectangle mbb;
	int bitmap;	/* index for Bitmap table in Jerq */
} Instance;

int NOBLT,DEBUG,PARSE,FAT,XFLAG;
int scale,bltcnt;
Instance blt[BLT];
Rectangle viewport = {
	200,200,400,400};
Rectangle window;
int jerq;	/* file descriptor for Jerq replies */

balloc(r)
Rectangle r;
{
	short i;
	outchar(BALLOC);
	outrect(r);
	i = inshort();
	return(i);
}

freeblt()
{
	outchar(FREE);
	bltcnt = 0;
	fflush(stdout);
}

orientation(t)
Transform t;
{
	int i = 0;
	if (t.t11 == 0)	 /* assumes only orthogonal rotations */
		i |= 4;
	if (t.t11 + t.t12 < 0)
		i |= 2;
	if (t.t21 + t.t22 < 0)
		i |= 1;
	return(i);
}

inchar()
{
	int c;
	fflush(stdout);
	while (read(jerq,&c,1) == 0);
	return(c&0377);
}

inshort()
{
	short i;
	i = inchar();
	i |= inchar() << 8;
	return(i);
}

Point
inPoint()
{
	Point p;
	p.x = inshort();
	p.y = inshort();
	return(p);
}

Rectangle
inrect()
{
	Rectangle r;
	r.origin = inPoint();
	r.corner = inPoint();
	return(r);
}

Point
sub(p,q)
Point p,q;
{
	Point s;
	s.x = p.x - q.x;
	s.y = p.y - q.y;
	return(s);
}

Point
center(r)
Rectangle r;
{
	Point p;
	p.x = (r.origin.x + r.corner.x) >> 1;
	p.y = (r.origin.y + r.corner.y) >> 1;
	return(p);
}

Point
shiftpt(p,s)
Point p;
int s;
{
	return(newpt(p.x>>s,p.y>>s));
}

int hs,vs,dtx,dty;	/* horizontal and vertical scroll distance */

fixscroll()
{
	vs = ((window.corner.y - window.origin.y) >> 2) & ~3;
	hs = ((window.corner.x - window.origin.x) >> 2) & ~3;
	dtx = hs << scale;
	dty = vs << scale;
	hs = dtx >> scale;	/* to get the physical scrolling right */
	vs = dty >> scale;	/* we need to use the proper precision */
}

Transform
initscale(i)
{
	Point ts;	/* translation of Symbol */
	Rectangle MBB;		/* mbb of top level Symbol call */
	Transform t;
	t = newtrans(1,0,0,-1,0,0);
	MBB = transrect(t,syms[i].mbb);
	outchar(PORT);
	window = inrect();
	viewport = window;
	ts = shiftpt(sub(center(window),center(MBB)),-scale);
	t.tx = ts.x;
	t.ty = ts.y;
	fixscroll();
	return(t);
}

Transform
newscale(t,s)
Transform t;
int s;
{
	Point ts,ps;
	ts.x = t.tx;
	ts.y = t.ty;
	ps = sub(shiftpt(center(window),-scale),ts);
	ts = sub(shiftpt(center(window),-s),ps);
	t.tx = ts.x;
	t.ty = ts.y;
	scale = s;
	fixscroll();
	return(t);
}

plot(i,t)
int i;
Transform t;
{
	register c,j,n;
	Rectangle r;
	Transform t;
	if (DEBUG == 1) fprintf(stderr,"parsed, internal size: %d\n",bincnt);
	if (PARSE == 1) return;
	t = initscale(i);
	clear(window);
	call(i,t);
	outchar(EOP);
	n = 0;
	while ((c = inchar()) != 'q') {
/*
		fprintf(stderr,"nbytes = %d\n",nbytes);
		fflush(stderr);
 */
		nbytes = 0;
		if (c>='0' && c<='9') {
			n = n*10 + c-'0';
			outchar(EOP);
			continue;
		}
		else if (n == 0)
			n = 1;
		switch (c) {
		case ' ':
			t = initscale(i);	/* have funny mbb's */
			clear(window);
			call(i,t);
			break;
		case 'n':
			NOBLT = 1-NOBLT;
			break;
		case '<':
			for (j = 0; j < n; j++)
				t = newscale(t,scale+1);
			freeblt();
			clear(window);
			call(i,t);
			break;
		case '>':
			for (j = 0; j < n; j++)
				t = newscale(t,scale-1);
			freeblt();
			clear(window);
			call(i,t);
			break;
#define w	window
#define o	origin
#define c	corner
		case 'l':
		case 0xf3:
			if (n < 4) {
			scroll(w.o,w.c.x-n*hs,w.c.y,w.o.x+n*hs,w.o.y);
			viewport.c.x = w.o.x+n*hs;
			}
			clear(viewport);
			t.tx += n*dtx;
			call(i,t);
			break;
		case 'r':
		case 0xf4:
			if (n < 4) {
			scroll(w.o.x+n*hs,w.o.y,w.c,w.o);
			viewport.o.x = w.c.x-n*hs;
			}
			clear(viewport);
			t.tx -= n*dtx;
			call(i,t);
			break;
		case 'u':
		case 0xf1:
			if (n < 4) {
			scroll(w.o,w.c.x,w.c.y-n*vs,w.o.x,w.o.y+n*vs);
			viewport.c.y = w.o.y+n*vs;
			}
			clear(viewport);
			t.ty += n*dty;
			call(i,t);
			break;
		case 'd':
		case 0xf2:
			if (n < 4) {
			scroll(w.o.x,w.o.y+n*vs,w.c,w.o);
			viewport.o.y = w.c.y-n*vs;
			}
			clear(viewport);
			t.ty -= n*dty;
			call(i,t);
			break;
#undef w
#undef o
#undef c
		default:
			break;
		}
		n = 0;
		viewport = window;
		outchar(EOP);
	}
	freeblt();
}

Instance *
findinst(i,o,t,r)
int i,o;
Transform t;
Rectangle r;
{
	Rectangle sv;
	register Instance *p;
	Instance inst;
	register int k = (i<<8)+o;
	for (p = blt+bltcnt; --p >= blt;)
		if (p->key == k) {
			if (p->bitmap > 0)	/* did we fail before? */
				return(p);
			else
				return(NULL);
		}
	/* none seen yet, see if we can make one */
	if (bltcnt >= BLT)	/* do we have space? */
		return(NULL);
	p = &blt[bltcnt++];
	p->key = k;
	if (!rectltrect(r,window)) {	/* does mbb fit in viewport? */
		p->bitmap = 0;		/* cheap size criterion */
		return(NULL);
	}
	if ((p->bitmap = balloc(r)) == 0)	/* does Jerq have space? */
		return(NULL);
	/* all set, let's go! */
	sv = viewport;
	viewport = r;
	p->mbb = syms[i].mbb;	/* we need the original for later */
	draw(i,t);
	outchar(POP);		/* PUSH was implicit in balloc */
	viewport = sv;
	return(p);
}

call(i,t)
int i;
Transform t;
{
	register Instance *inst;
	register o;
	Rectangle r;
	r = transrect(t,syms[i].mbb);
	if (!rectXrect(r,viewport))
		return;
	if (syms[i].refcnt > 1 && NOBLT == 0) {	/* worth using bitblt? */
		o=orientation(t);
		if ((inst = findinst(i,o,t,r)) != NULL)/* already defined? */
			bitblt(inst,r.origin);
		else
			draw(i,t);
	}
	else
		draw(i,t);
}

Point
offset(p,o,mbbo)
Point p,o,mbbo;
{
	Point q;
	q.x = p.x - o.x + mbbo.x;
	q.y = p.y - o.y + mbbo.y;
	return(q);
}

bitblt(i,p)
register Instance *i;
Point p;
{
	if (DEBUG)
		fprintf(stderr,"bitblt from %d\n",i->bitmap);
	outchar(BITBLT);
	outchar(i->bitmap);
	outPoint(p);
}

scroll(r,p)
Rectangle r;
Point p;
{
	outchar(SCROLL);
	outrect(r);
	outPoint(p);
}

draw(i,t)
int i;
Transform t;
{
	int pc,s,c;
	char color;
	pc = syms[i].pc;
	for (;;) switch((c = bin[pc++])) {
	default:
		fprintf(stderr,"unrecognized object, pc = %d, code = %d\n",
		    pc-1,bin[pc-1]);
		break;
	case BOX:
		drawbox(transrect(t,rectatbin(pc)),color);
		pc += 4;
		break;
	case ERRBOX:
		drawbox(transrect(t,rectatbin(pc)),ERRBOX);
		pc += 4;
		break;
	case CALL:
		s = bin[pc++];
		call(s,transtrans(t,transatbin(pc)));
		pc += 6;
		break;
	case WIRE:
		pc=drawwire(pc,color,t);
		break;
	case POLYGON:
		pc = drawpoly(pc,color,t);
		break;
	case METAL:
	case DIFF:
	case POLY:
	case IMPLANT:
	case CUT:
	case POLYCON:
	case GLASS:
	case ERRS:
		color = c;
		break;
	case MZERO:
		return;
	}
}

drawbox(r,color)	/* r is already in screen coordinates */
Rectangle r;
register char color;
{
	if (color == ERRBOX) {
		r = inset(r,-4);
	}
	else if (color == ERRS)
		color = ERRBOX;
	if (r.origin.x == r.corner.x || r.origin.y == r.corner.y ||
	    !rectXrect(r,viewport))
		return;
	r.origin.x = max(r.origin.x,viewport.origin.x);
	r.origin.y = max(r.origin.y,viewport.origin.y);
	r.corner.x = min(r.corner.x,viewport.corner.x);
	r.corner.y = min(r.corner.y,viewport.corner.y);
	switch(color) {
	case ZZZZ:
		yyerror("unspecified layer");
		abort();
		color = -1;
		break;
	case METAL:
		outchar(METAL);
		break;
	case DIFF:
		outchar(DIFF);
		break;
	case POLY:
		outchar(POLY);
		break;
	case CUT:
		outchar(CUT);
		break;
	case POLYCON:
		outchar(POLYCON);
		break;
	case ERRBOX:		/* stingingly flickering boxes */
		outchar(ERRBOX);	/* currently disabled */
		break;
	case GLASS:
		outchar(GLASS);
		break;
	case IMPLANT:
		outchar(IMPLANT);
		break;
	default:
		color = -1;
		return;
	}
	outrect(r);

}

Point ptbuf[20];

drawpoly(pc,color,t)
int pc;
char color;
Transform t;
{
	int i,n;
	register Point *p = ptbuf;
	Rectangle r;
	r.origin = viewport.corner;
	r.corner = viewport.origin;
	for (n = 0; bin[pc] != MZERO; n++, pc += 2, p++) {
		*p = transpt(t,ptatbin(pc));
		r = mbbpt(r,*p);
	}
	if (!rectXrect(r,viewport))
		return(pc+1);
	outchar(POLYGON);
	outchar(n);
	outchar(color);
	p = ptbuf;
	do
		outPoint(*p++);
	while (--n > 0);
	return(pc + 1);
}

int
drawwire(pc,layer,t)
int pc;
char layer;
Transform t;
{
	int w,i,n;
	register Point *p = ptbuf;
	Rectangle r;
	w = bin[pc++] >> scale;
	r.origin = viewport.corner;
	r.corner = viewport.origin;
	for (n = 0; bin[pc] != MZERO; n++, pc += 2, p++) {
		*p = transpt(t,ptatbin(pc));
		r = mbbpt(r,*p);
	}
	if (!rectXrect(r,viewport))
		return(pc+1);
	outchar(WIRE);
	outchar(n);
	outchar(layer);
	outchar(w);
	p = ptbuf;
	do
		outPoint(*p++);
	while (--n > 0);
	return(pc + 1);
}

clear(r)
Rectangle r;
{
	outchar(CLEAR);
	outrect(r);
}

outPoint(p)
Point p;
{
	outchar(p.x&0377);
	outchar(p.y&0377);
	outchar(((p.y&0xf00)>>4) | ((p.x&0xf00)>>8));
}

outrect(r)
Rectangle r;
{
	outPoint(r.origin);
	outPoint(r.corner);
}

char obuf[BUFSIZ];
struct sgttyb modes,savetty;

main(argc,argv)
int argc;
char *argv[];
{
	char c,*p;
	int errj;
	while (argc > 1 && *(p = argv[1]) == '-') {
		argc--;
		argv++;
		while ((c = *++p) != '\0') switch(c) {
		case 'd':
			DEBUG=1;
			break;
		case 'p':
			PARSE=1;
			break;
		case 'n':
			NOBLT = 1;
			break;
		case 'f':	/* fat lines for Brian */
			FAT=1;
			break;
		case 'i':
			break;
		case 'x':
			XFLAG++;
			break;
		}
	}
	if (argc > 1) {
		if (freopen(argv[1],"r",stdin) == NULL) {
			fprintf(stderr,"Can't find %s\n",argv[1]);
			argv++;
			argc--;
		}
	}
	jerq = open("/dev/tty",0);	/* return cookies from Jerq */
	setbuf(stdout,obuf);	/* assumes stdout is /dev/tty */
	freopen(".jciferr","w",stderr);
	if (XFLAG)
		errj = system("68ld jcif.m > /dev/tty < /dev/tty");
	else
		errj = system("68ld /usr/blit/mbin/jcif.m > /dev/tty < /dev/tty");
	if (errj != 0) {
		fprintf(stderr,"Your working environment is not powerful");
		fprintf(stderr," enough to run jcif!\n");
		fflush(stderr);
	}
	else {
	ioctl(jerq, TIOCGETP, &modes);
	savetty = modes;
	modes.sg_flags|=RAW;
	modes.sg_flags&=~ECHO;
	ioctl(jerq, TIOCSETP, &modes);
	yyparse();
	ioctl(jerq, TIOCSETP, &savetty);
	ioctl(jerq, JTERM, 0);
	}
	system("cat .jciferr");
}

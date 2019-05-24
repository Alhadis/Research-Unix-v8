/*
 *  rect.c - output program for cif plotter
 */

#include <jerq.h>
#include <font.h>
typedef struct {
	int n;
	Point *p;
} Polygon;
#define WIRE	2
#define BITBLT	21
#define BALLOC	22
#define POP	23
#define SCROLL	24
#define FREE	25
#define CLEAR	26
#define PORT	27
#define GIMME	28
#define METAL	5
#define DIFF	6
#define POLY	7
#define IMPLANT	8
#define CUT	9
#define GLASS	10
#define POLYCON	13
#define ERRBOX	14
#define BLT	100
#define POLYGON	11

Texture metal = {
	0x1111, 0x2222, 0x4444, 0x8888,
	0x1111, 0x2222, 0x4444, 0x8888,
	0x1111, 0x2222, 0x4444, 0x8888,
	0x1111, 0x2222, 0x4444, 0x8888,
};
Texture diff = {
	0x6666, 0x0000, 0x0000, 0x6666,
	0x6666, 0x0000, 0x0000, 0x6666,
	0x6666, 0x0000, 0x0000, 0x6666,
	0x6666, 0x0000, 0x0000, 0x6666,
};
Texture poly = {
	0xaaaa, 0x5555, 0xaaaa, 0x5555,
	0xaaaa, 0x5555, 0xaaaa, 0x5555,
	0xaaaa, 0x5555, 0xaaaa, 0x5555,
	0xaaaa, 0x5555, 0xaaaa, 0x5555,
};
Texture implant = {
	0x0000, 0x1111, 0x0000, 0x4444,
	0x0000, 0x1111, 0x0000, 0x4444,
	0x0000, 0x1111, 0x0000, 0x4444,
	0x0000, 0x1111, 0x0000, 0x4444,
};
Texture cut = {
	0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff,
};
Texture glass = {
	0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff,
};
Texture polycon = {
	0x5555, 0xaaaa, 0x5555, 0xaaaa,
	0x5555, 0xaaaa, 0x5555, 0xaaaa,
	0x5555, 0xaaaa, 0x5555, 0xaaaa,
	0x5555, 0xaaaa, 0x5555, 0xaaaa,
};
Texture errbox = {
	0xffff, 0x0000, 0xffff, 0x0000,
	0xffff, 0x0000, 0xffff, 0x0000,
	0xffff, 0x0000, 0xffff, 0x0000,
	0xffff, 0x0000, 0xffff, 0x0000,
};

Texture *color[] = {
	0, 0, 0, 0, 0,
	&metal,		/* 5 */
	&diff,		/* 6 */
	&poly,		/* 7 */
	&implant,	/* 8 */
	&cut,		/* 9 */
	&glass,		/* 10 */
	0, 0,
	&polycon,	/* 13 */
	&errbox,		/* 14 */
};

char s[100];

sendrect(r)
Rectangle r;
{
	sendpoint(r.origin);
	sendpoint(r.corner);
}

sendpoint(p)
Point p;
{
	sendshort(p.x);
	sendshort(p.y);
}

sendshort(i)
short i;
{
	char c;
	c = i&0377;
	sendnchars(1,&c);
	c = (i>>8)&0377;
	sendnchars(1,&c);
}

inchar()
{
	register c;
	while ((c = rcvchar()) == -1)
		wait(RCV);
	return(c);
}

Point
inpoint()	/* encode a point as two 12 bit numbers in 3 bytes */
{
	register short i,x,y;
	Point p;
	x = inchar()<<4;	/* low 8 bits of x and y */
	y = inchar()<<4;
	i = inchar()<<8;	/* high 4 bits of y,x */
	p.x = ((i<<4) | x) >> 4;
	p.y = ((0xf000 & i) | y) >> 4;
	return(p);
}

Rectangle
inrect()
{
	Rectangle r;
	r.origin = inpoint();
	r.corner = inpoint();
	return(r);
}
/*
char buf[80];
int YS = 20;

mess(s)
char *s;
{
	string(&defont,s,&display,add(Drect.origin,Pt(0,YS)),F_XOR);
	if ((YS += 16) > Drect.corner.y-16)
		YS = Drect.origin.y;
}
*/
Bitmap *blt[BLT],*bltstack[20],**curblt;
Point pts[20];

roundiv(a,b)
register a,b;
{
	register c = b << 1;
	a <<= 1;
	return((a > 0) ? ((a+b)/c) : ((a-b)/c));
}

main(argc,argv)
int argc;
char *argv[];
{
	char c;
	int i,j,n,w;
	Point p,q,*pp,p1,p2;
	Rectangle r;
	Polygon poly;
	Bitmap *b;
	Texture *t;

	request(SEND|RCV|KBD);
	bltstack[0] = blt[0] = &display;
	curblt = bltstack;
	n = 1;
	while (1 == 1) {
		i = inchar();
		switch (i) {
		case BALLOC:
			if (n >= BLT) {
				sendshort(0);
				break;
			}
			b = balloc(inrect());
			if (b != (Bitmap *) NULL) {
				rectf(b,b->rect,F_CLR);
				curblt++;
				*curblt = b;
				blt[n] = b;
/*
sprintf(s,"n: %d, wid: %d, xx: %d %d",n,b->width,b->rect.origin.x,b->rect.corner.x);
mess(s);
 */
				sendshort(n);
				n++;
			}
			else
				sendshort(0);
			break;
		case GIMME:		/* send a char from the keyboard */
			wait(KBD);
			sendchar(kbdchar());
			blt[0] = &display;
			bltstack[0] = blt[0];
			curblt = bltstack;	/* in case of reshape */
			break;
		case POP:
			curblt--;
			break;
		case FREE:
			curblt = bltstack;
			for (i = 1; i < n; i++)
				if (blt[i] != (Bitmap *) NULL) {
					bfree(blt[i]);
					blt[i] = 0;
				}
			n = 1;
			break;
		case BITBLT:
			j = inchar();
			p = inpoint();
			r = blt[j]->rect;
			bitblt(blt[j], r, *curblt, p, F_OR);
			break;
		case SCROLL:
			r = inrect();
			p = inpoint();
			bitblt(&display,r,&display,p,F_STORE);
			break;
		case CLEAR:
			r = inrect();
			rectf(&display,r,F_CLR);
			break;
		case PORT:
			sendrect(Drect);
			break;
		case WIRE:
			j = inchar();
			t = color[inchar()];
			w = inchar()/2;		/* radius, not width */
			if (w == 0)
				w = 1;
			p1 = inpoint();
			discture(*curblt,p1,w,t,F_OR);
			while (--j > 0) {
				p2 = p1;
				p1 = inpoint();
				if (p1.x == p2.x)
					texture(*curblt,Rect(p1.x-w,min(p1.y,p2.y),p2.x+w,max(p1.y,p2.y)),t,F_OR);
				else if (p1.y == p2.y)
					texture(*curblt,Rect(min(p1.x,p2.x),p1.y-w,max(p1.x,p2.x),p2.y+w),t,F_OR);
				else {	/* make a polygon */
					p.x = p1.y - p2.y;
					p.y = p2.x - p1.x;
					i = (norm(mul(p,2),0)+1)/2;
					p.x = roundiv(p.x*w,i);
					p.y = roundiv(p.y*w,i);
					pp = pts;
					*pp++ = add(p1,p);
					*pp++ = add(p2,p);
					*pp++ = sub(p2,p);
					*pp++ = sub(p1,p);
					poly.n = 4;
					poly.p = pts;
					polygon(*curblt,poly,t,F_OR);
				}
				discture(*curblt,p1,w,t,F_OR);
			}
			break;
		case POLYGON:
			j = inchar();
			t = color[inchar()];
			for (i = 0, pp = pts; i < j; i++)
				*pp++ = inpoint();
			poly.n = j;
			poly.p = pts;
			polygon(*curblt,poly,t,F_OR);
			break;
		case METAL:
		case DIFF:
		case POLY:
		case IMPLANT:
		case CUT:
		case POLYCON:
		case GLASS:
			r = inrect();
			texture(*curblt,r,color[i],F_OR);
			break;
		case ERRBOX:
			r = inrect();
			for (j = 0; j < 8; j++) {
				texture(*curblt,r,&cut,F_XOR);
				sleep(10);
			}
			drerr("design rule error");
			break;
		default:
			break;
		}
	}
}

drerr(msg)
char *msg;
{
}

#include <jerq.h>

#define BLITBLT

#define SHORTSIZE	16
#define SHORT		short
#define SCAST(Wpt)	((SHORT *)Wpt)

#define sendword(w)	(sendch(w&255),sendch((w>>8)&255))

static Word buffer[XMAX/WORDSIZE], raster[XMAX/WORDSIZE];

static Bitmap bbuffer={ buffer, (sizeof buffer)/(sizeof(Word)), 0, 0, XMAX, 1};

static int ctype, count; static SHORT *p1, *endraster;

sendbitmap(bp,r,filnam)
Bitmap *bp; Rectangle r; char *filnam;
{
	register i; int nrasters, rastwid, nrword; Rectangle rrast;

	do sendch(*filnam); while (*filnam++ != 0); flushch();
	if (recvch()) return 1;

	nrasters = r.corner.y-r.origin.y;
	i        = r.corner.x-r.origin.x;
	rastwid  =(i+SHORTSIZE-1)/SHORTSIZE;
	nrword   =(i+WORDSIZE-1)/WORDSIZE;
	endraster= SCAST(raster)+rastwid-1;
	/* sendword(nrasters); i |= 0x8000; sendword(i); */
	sendword(0);
	sendword(r.origin.x); sendword(r.origin.y);
	sendword(r.corner.x); sendword(r.corner.y);

	rectf(&bbuffer,bbuffer.rect,F_CLR);
	for (i=0; i<nrword; i++) raster[i] = 0;
	rrast=r;
#ifdef BLITBLT
	rectf(bp,r,F_XOR);
#endif

	for (; rrast.origin.y<r.corner.y; rrast.origin.y++) {
		rrast.corner.y = rrast.origin.y+1;
#ifdef BLITBLT
		rectf(bp,rrast,F_XOR);
#endif
		bitblt(bp,rrast,&bbuffer,Pt(0,0),F_STORE);
		for (i=0; i<nrword; i++) raster[i] ^= buffer[i];
		sendrast();
		for (i=0; i<nrword; i++) raster[i]  = buffer[i];
	}

	flushch(); sendctl(0);
	return recvch();
}

static sendrast()
{
	SHORT *p2;

	p1=p2=SCAST(raster);
	do {
		if (p1 >= p2) {
			p2=p1+1; count=2;
			ctype=(*p1 == *p2);

		} else if ((*p2 == *(p2+1)) == ctype) {
			if (++count >= 127) {
				sendbits();
				p1=p2+2;
			} else p2++;

		} else if (ctype) {
			sendbits();
			p1=p2+1;
			ctype=0;

		} else {
			count--; sendbits();
			p1=p2;
			ctype=1;
		}
	} while (p2<endraster);

	if (p1 > endraster) return;
	if (p2 > endraster) count--;
	sendbits();
}

static sendbits()
{
	int c;
	c=count; if (ctype) { c += 128; count=1; }
	sendch(c);
	sendnch(sizeof(SHORT)*count,(char *)p1);
}

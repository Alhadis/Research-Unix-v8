#include <stdio.h>

extern FILE *infile;

extern void idjusttext();
extern void idstart();
extern void idmaxx();
extern void idmaxy();
extern void idminx();
extern void idminy();
extern void idendbound();
extern void idwidth();
extern void idheight();
extern void idcolwid();
extern void idline();
extern void idcircle();
extern void idarc();
extern void idleft();
extern void idcenter();
extern void idright();
extern void idendE();
extern void idendF();
extern void idspline();
extern void idknot();
extern void idendspline();
extern void idyeserase();
extern void idnoerase();

#define round(x)	((int) ((x) + 0.5))
#define abs(x)		((x)>0?(x):-(x))

#define boolean		int
#define TRUE		1
#define FALSE		0

extern boolean wanterase;

extern float maxx, maxy;
extern float minx, miny;
extern float xscale, yscale;
extern float width, height, colwid;

extern boolean boundset;
extern boolean widset, heightset, colset;
extern boolean minxset, maxxset;
extern boolean minyset, maxyset;

extern boolean wantquality;
extern boolean banzai;

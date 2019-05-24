#include "alph.h"
/*
 * kal.c
 *
 * Sun Kaleidoscope by Jeffrey Mogul
 *
 * stolen from:
// Kal.bcpl -- Alto Kaleidoscope by (in order of appearance):
//	Martin Newell
//	Joe Maleson
//	Dana Scott
//	David Boggs
// Copyright Xerox Corporation 1979
// Last modified June 11, 1979  1:41 PM by Boggs

// Kal uses three-parameter (a, b, c) sequence generators to
//   create and erase points with 8 or 12 way symmetry.  Two
//   other parameters, (period and persistence) are global
//   to all sequence generators.  On each cycle, each generator
//   does a _ G(a, b) and the 'a's are used to create 8 or 12 new points
//   and erase 8 or 12 old ones.  Every 'period' cycles each generator
//   does b _ G(b, c).  Generators come in pairs - one to create
//   points and the other to erase them.  The erasing generator
//   runs 'persistence' cycles behind the creating one.  Thus
//   'persistence' determines the life of each point, and hence
//   the number of points on the screen at any one time.
//
 */


#define HLFSCRN 256
#define SCRNMAX 511
#define	INTERACTIVE

struct GenParam {	/* generator parameter block */
	unsigned short	Xa;
	unsigned short	Xb;
	unsigned short	Xc;
	unsigned short	Ya;
	unsigned short	Yb;
	unsigned short	Yc;
	unsigned short	period;
	unsigned short	count;	/* initialized to period and decremented to 0 */
	unsigned short	symmetry;
};

struct GenParam Show;	/* one param block for showing generator */
struct GenParam Erase;	/* one param block for erasing generator */
struct GenParam Init = {	/* initial value param block */
	3, 10, 10,
	5, 3, 10,
	496, 0, 12};

unsigned short	persistance = 800;

#define	SHOWPOINT	F_OR
#define	ERASEPOINT	F_CLR
nkal()
{
	register unsigned short i;
	Init.count = Init.period;	/* initialize count */
	clearscreen();
#ifdef INTERACTIVE
	Inquire(&Init);
#endif INTERACTIVE
	WonB();
	clearscreen();
	Show=Init;
	Erase=Init;
	for (i = 0; i < persistance; i++) {
		/* start Show generator first by persistance cycles */
		Generate(&Show, SHOWPOINT);
	}
	while(!keyhit()){	/* generate next point in sequences */
		Generate(&Show, SHOWPOINT);
		Generate(&Erase, ERASEPOINT);
	}
	BonW();
}
/*#define	POINT(a, b)	point(&display, Pt(a, b), mode)*/
#define	DY	((YMAX-(SCRNMAX+1))/2)
#define	DX	((XMAX-(SCRNMAX+1))>>5)
#define	POINT(x, y)	(mode==SHOWPOINT?\
			*(display.base+((y+DY)*(unsigned)50)+((x)>>4)+DX)|=1<<(15-((x)&15)) :\
			*(display.base+((y+DY)*(unsigned)50)+((x)>>4)+DX)&=~(1<<(15-((x)&15))))
Generate(gp,mode)
	register struct GenParam *gp;	/* generator parameters */
	Code	mode;	/* either SHOWPOINT or ERASEPOINT */
{
	register short	x0,y0,x1,y1;
	gp->Xa = (gp->Xa + gp->Xb) ^ gp->Xb;
	gp->Ya = (gp->Ya + gp->Yb) ^ gp->Yb;
	if (--gp->count == 0) {
		gp->Xb = (gp->Xb + gp->Xc) ^ gp->Xc;
		gp->Yb = (gp->Yb + gp->Yc) ^ gp->Yc;
		gp->count = gp->period;
	}

	if (gp->symmetry == 8) {
		x0 = gp->Xa >> 8;
		y0 = gp->Ya >> 8;

		if (x0 <= y0) {
			x1 = (SCRNMAX)-x0;
			y1 = (SCRNMAX)-y0;

			POINT(x0, y0);
			POINT(x0, y1);
			POINT(y0, x0);
			POINT(y0, x1);
			POINT(x1, y0);
			POINT(x1, y1);
			POINT(y1, x1);
			POINT(y1, x0);
		}
	}
	else {	/* symmetry == 12 */
		x0 = gp->Xa >> 9;
		y0 = gp->Ya >> 9;

		if (x0 <= y0) {
			x1 = x0 << 1;
			y1 = y0 << 1;

			/* we might want to double scan lines here? */

			POINT((HLFSCRN)+x0+y0, (HLFSCRN)-x1+y1);
			POINT((HLFSCRN)+x0+y0, (HLFSCRN)+x1-y1);
			POINT((HLFSCRN)-x0-y0, (HLFSCRN)-x1+y1);
			POINT((HLFSCRN)-x0-y0, (HLFSCRN)+x1-y1);
			POINT((HLFSCRN)+x0-y1, (HLFSCRN)-x1);
			POINT((HLFSCRN)+x0-y1, (HLFSCRN)+x1);
			POINT((HLFSCRN)-x0+y1, (HLFSCRN)-x1);
			POINT((HLFSCRN)-x0+y1, (HLFSCRN)+x1);
			POINT((HLFSCRN)+x1-y0, (HLFSCRN)-y1);
			POINT((HLFSCRN)+x1-y0, (HLFSCRN)+y1);
			POINT((HLFSCRN)-x1+y0, (HLFSCRN)-y1);
			POINT((HLFSCRN)-x1+y0, (HLFSCRN)+y1);
		}
	}
}

#ifdef INTERACTIVE

char	response[100];
char *
gets(as)
	char *as;
{
	register char *s=as;
	char buf[2];
	buf[1]=0;
	do{
		wait(KBD);
		*s=kbdchar();
		buf[0]= *s;
		printf(buf);
	}while(*s++!='\r');
	s[-1]=0;
}
char *
ntos2(n, s)
    char *s;
{
    int i;
    i = n%10;
    n /= 10;
    *s='0'+i;
    return(n? ntos2(n, s-1) : s);
}

char *
ntos(n)
{
    static char buf[10];	/* Don't call ntos twice without saving! */
    return(ntos2(n, buf+8));
}

Inquire(gp)
struct GenParam *gp;
{
	unsigned int stemp;

	startprintf();
	printf("Enter parameters; <cr> means: use default");
	printf("\n");
	getparam(&(gp->Xa),"Xa");
	getparam(&(gp->Xb),"Xb");
	getparam(&(gp->Xc),"Xc");
	getparam(&(gp->Ya),"Ya");
	getparam(&(gp->Yb),"Yb");
	getparam(&(gp->Yc),"Yc");
	getparam(&(gp->period),"Period");

	getparam(&persistance,"Persistance (sic)");

retry_sym:
	printf("Symmetry [8 or 12] (");
	printf(ntos(gp->symmetry));
	printf(") => ");
	gets(response);
	if (*response) {
		stemp=atoi(response);
		if ((stemp == 8) || (stemp == 12))
			gp->symmetry = stemp;
		else {
			printf("not a possible symmetry\n");
			goto retry_sym;
		}
	}

#ifdef DEBUG
	printgp(gp);
#endif DEBUG
}
Point printfpt;
startprintf(){
	printfpt.x=32;
	printfpt.y=32;
}
printf(s)
	char *s;
{
	jmoveto(printfpt);
	printfpt=jstring(s);
	if(*s=='\r' || *s=='\n'){	/* only one char for newline, please */
		printfpt.x=32;
		printfpt.y+=16;
	}
}
getparam(param,name)
unsigned short *param;
char *name;
{
	int stemp;

	printf(name);
	printf(" (");
	printf(ntos(*param));
	printf(") => ");
	gets(response);
	if (*response)
		*param = atoi(response);
}

#ifdef DEBUG
printgp(g)
struct GenParam *g;
{

	printf("Xa %u b %u c %u Ya %u b %u c %u\n",
	    g->Xa,g->Xb,g->Xc,g->Ya,g->Yb,g->Yc);
	printf("period %u symmetry %u\n",g->period,g->symmetry);
}
#endif DEBUG

#endif INTERACTIVE

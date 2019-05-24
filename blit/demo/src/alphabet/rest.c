#include "alph.h"
#ifdef GEEWHIZ
/*
 * rotor.c
 *
 * Rotating Sun Spiral by Jeffrey Mogul
 *
 */


char response[100];


rotor()
{
	int x1,y1;
	int x2,y2;
	int delay;
	register int r;
	int count;

/*
	printf("Enter r ");
	gets(response);
	r = atoi(response);

	printf("Enter delay ");
	gets(response);
	delay = atoi(response);

	printf("r = %d, delay = %d\n",r,delay);
*/
r=20; d=20;

	clearscreen();

	x1 = 100;
	y1 = 0;

	x2 = x1;
	y2 = y1;

	if (r < 0) {
	    r = -r;
	    for (count = 0; count < delay;count++) {
		spiralclp(x1,y1,r,384,512,SHOWPOINT,512);
		step(&x1,&y1,r);
	    }
	    while (!keyhit()) {
		spiralclp(x1,y1,r,384,512,SHOWPOINT,512);
		step(&x1,&y1,r);
		spiralclp(x2,y2,r,384,512,ERASEPOINT,512);
		step(&x2,&y2,r);
	    }
	}
	else {
	    for (count = 0; count < delay;count++) {
		spiral(x1,y1,r,384,512,SHOWPOINT,350);
		step(&x1,&y1,r);
	    }
	    while (!keyhit()) {
		spiral(x1,y1,r,384,512,SHOWPOINT,350);
		step(&x1,&y1,r);
		spiral(x2,y2,r,384,512,ERASEPOINT,350);
		step(&x2,&y2,r);
	    }
	}
}
step(x,y,r)
register int *x;
register int *y;
register int r;
{
	*x += (*y>>r);
	*y -= (*x>>r);
}

/*
 * cyl.c
 *
 * Sun Cylinders by Jeffrey Mogul
 *
 */

ncyl()
{
	register unsigned int i;
	int xinit, rad;
	int delay;
	register int x,x1;

	clearscreen();

	rad = 100;

	x = 150;
	x1 = x-10;

	while (!keyhit()) {
		x++; 
		x1++;
		if (x>650) x = 150;
		if (x1>650) x1 = 150;
		circ(rad,4,x,x,SHOWPOINT);
		circ(rad,4,x1,x1,ERASEPOINT);
#ifdef TWOFER
		circ(rad,4,x,(800-x),SHOWPOINT);
		circ(rad,4,x1,(800-x1),ERASEPOINT);
#endif TWOFER
#ifdef FOURFER
		circ(rad,4,(800-x),x,SHOWPOINT);
		circ(rad,4,(800-x1),x1,ERASEPOINT);
		circ(rad,4,(800-x),(800-x),SHOWPOINT);
		circ(rad,4,(800-x1),(800-x1),ERASEPOINT);
#endif FOURFER
	}
}

/*
 * burst.c
 *
 * Sun Burst by Jeffrey Mogul
 *
 */

burst() {
	register unsigned int i;
	int xinit, rad;
	int delay;
	register int x,x1;

	clearscreen();

	while (!keyhit()) {
		for (rad = 50; rad < 350; rad++) {
			circ(rad,4,384,384,SHOWPOINT);
		}

		for (; rad > 50 ; rad--) {
			circ(rad,4,384,384,ERASEPOINT);
		}


		for (rad = 50; rad < 350; rad++) {
			circ(rad,4,384,384,SHOWPOINT);
		}

		for (rad = 50; rad < 350; rad++) {
			circ(rad,4,384,384,ERASEPOINT);
		}


		for (; rad > 50 ; rad--) {
			circ(rad,4,384,384,SHOWPOINT);
		}

		for (rad = 50; rad < 350; rad++) {
			circ(rad,4,384,384,ERASEPOINT);
		}


		for (; rad > 50 ; rad--) {
			circ(rad,4,384,384,SHOWPOINT);
		}

		for (rad = 350; rad > 50; rad--) {
			circ(rad,4,384,384,ERASEPOINT);
		}

	}
}

/*
 * circ.c
 *
 * Sun Circles by Jeffrey Mogul
 *
 * stolen from:
 * Hackmem Minksy?
 */

/*
 * draw a ci
 = 0; count < delay;count++) {
(x1,y1,r,384,512,SHOWPOINT,512);

			step(&x1,&y1,r);
tep(&x1,&y1,r);
		}
		while (!keyhit()) {
piralclp(x1,y1,r,384,512,SHOWPOINT,512);
y1,r,384,512,SHOWPOINT,512);
12);
&y1,r);
			spiralclp(x2,y2,r,384,512,ERASEPOINT,512);
,384,512,ERASEPOINT,512);
INT,512);
			step(&x2,&y2,r);
r);
		for (count = 0; count < delay;count++) {
< delay;count++) {
lay;count++) {
piral(x1,y1,r,384,512,SHOWPOINT,350);
l(x1,y1,r,384,512,SHOWPOINT,350);
2,SHOWPOINT,350);
INT,350);
350);
		}
		while (!keyhit()) {
l(x1,y1,r,384,512,SHOWPOINT,350);
OWPOINT,350);
			step(;
#endif WRONG
		x += (y>>r);

#ifdef WRONG
		y -= (x0>>r);
#else
		y -= (x>>r);
#endif WRONG
		}

}	

/*
 * spiral.c
 *
 * Sun Spirals by Jeffrey Mogul
 *
 * stolen from:
 * Hackmem Minksy?
 */

/*
 * spiral starts with a circle going through x0,y0 centered
 * at cx,cy
 * mode is either SHOWPOINT or ERASEPOINT
 * r should be something like 4
 * endx is limit on x value.
 */
spiral(x0,y0,r,cx,cy,mode,endx)
int x0;
int y0;
register int r,cx,cy;
unsigned int mode;
int endx;
{ /*	*/
	register int x,y;
	register int xtemp;

	SETGXFUNC(mode);

	x = x0;
	y = y0;

	while (x < endx) {
		/*	while ((x < endx) && (y < endx) && (x > -endx) && (y > -endx)) {*/
		POINT(cx+x,cy+y);
		xtemp = x;
		x += (y>>r);
		y -= (xtemp>>r);
	}

}

/*
 * spiralclp is like spiral but clips instead of scissors.
 */
spiralclp(x0,y0,r,cx,cy,mode,endx)
int x0;
int y0;
register int r,cx,cy;
unsigned int mode;
int endx;
{ /*	*/
	register int x,y;
	register int dx,dy;
	int xtemp;


	SETGXFUNC(mode);

	x = x0;
	y = y0;

	/*	while ( ((x < endx) && (x > -endx)) || ((y < endx) && (y > -endx)) ){*/
	while (x < endx) {
		dx = cx+x;
		dy = cy+y;
		if ((dx > 0) && (dx < GXBITMAPSIZE) &&
		    (dy > 0) && (dy < GXBITMAPSIZE))
			POINT(cx+x,cy+y);
		xtemp = x;
		x += (y>>r);
		y -= (xtemp>>r);
	}

}

/*
 * rect.c - a simple frame buffer demo program
 *
 * Original author: Who knows? Andy or Vaughan??
 *
 * Updated by Bill Nowicki April 27, 1981
 *	- Used include file
 *	- set map on SUN1
 * Updated by Jeffrey Mogul @ Stanford		2 July 1981
 *	- for version 1 sun board
 */

frect()
{
	register int x1, x2, y1, y2, y;
	int x;
	register int ffff = 0xffff;
	register short *ystartp;
	register short *yendp;
	register short *xstartp;
	register short *xendp;
	short pass, rand;

	pass = 0;

	/* erase display */
	clearscreen();

	/* create overlaping rectangles */

	SETGXWIDTH(16);

	while (!keyhit()) {
		rand = rand*4 + rand + 17623;
		x1 = rand & 0x3F0;
		rand = rand*4 + rand + 17624;
		x2 = rand & 0x3F0;
		x = (x1 > x2) ? x2 : x1;
		x2 = (x1 > x2) ? x1 : x2;
		rand = rand*4 + rand + 17623;
		y1 = rand & 1023;
		rand = rand*4 + rand + 17623;
		y2 = rand & 1023;
		y = (y1 > y2) ? y2 : y1;	/* sort so that y1 <= y2 */
		y2 = (y1 > y2) ? y1 : y2;

		pass++;

		if ((pass&7) == 0)
			SETGXFUNC(GXset)
		    else
			SETGXFUNC(GXinvert)

			    xstartp = (short *)(GXUnit0Base|GXselectX|(x1<<1));
		xendp = (short *)(GXUnit0Base|GXselectX|(x2<<1));

		y <<= 1;
		y2 <<= 1;

		while (xstartp < xendp) {
			*xstartp = 1;
			xstartp += 16;

			ystartp = 
			    (short *)(GXUnit0Base|GXupdate|GXsource|GXselectY|y);
			yendp = 
			    (short *)(GXUnit0Base|GXupdate|GXsource|GXselectY|y2);

#ifdef SLOWER
			while (ystartp <= yendp) {
				*ystartp++ = ffff;
			}
#else
#ifdef SLOW
			/* y1 <= y2 by "sort" above, so this is faster */
loop:	
			*ystartp++ = ffff;
			if (ystartp <= yendp) goto loop;
#else
loop:	
			asm(" moveml #/ffff,a4@-");
			if (ystartp <= yendp) goto loop;
#endif SLOW
#endif SLOWER
		}

	}
}

#endif
burst(){}
frect(){}
ncyl(){}
rotor(){}

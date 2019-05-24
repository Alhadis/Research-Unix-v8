/*
 * graphmacs.h
 *
 * Graphics Macros for Sun Frame Buffer
 *
 * Jeffrey Mogul @ Stanford 	30-June-1981
 */

#include "graphics.h"	/* should be framebuf.h! */

#define	GXBITMAPSIZE	(1024)	/* length of side of bitmap */

#ifndef XBIAS
#define XBIAS	(150)	/* x-offset from edge of screen to (0,0) */
#endif XBIAS

#ifndef YBIAS
#define YBIAS	(256)	/* y-offset from edge of screen to (0,0) */
#endif YBIAS

#define SHOWPOINT	GXset
#define	ERASEPOINT	GXclear

/*
 * POINT(x,y) sets the given point to whatever the prevailing
 * function is.
 */
#define	POINT(x,y) {\
    GXsetX((x)+XBIAS);\
    *(short*)(GXUnit0Base|GXupdate|GXsource|GXselectY|(((y)+YBIAS)<<1))=\
    						0xFFFF;\
    }

/*
 * SETGXFUNC(f) sets the function register to f
 */
#define SETGXFUNC(f)	GXfunction = (f);

/*
 * SETGXWIDTH(w) sets operation width to w
 */
#define	SETGXWIDTH(w)	GXwidth = (w);

/*
 * SETPOINT(x,y) turns on the point at (x,y)
 */
#define SETPOINT(x,y) {\
    SETGXFUNC(SHOWPOINT);\
    POINT(x,y);\
    }

/*
 * CLEARPOINT(x,y) turns off the point at (x,y)
 */
#define CLEARPOINT(x,y) {\
    SETGXFUNC(ERASEPOINT);\
    POINT(x,y);\
    }

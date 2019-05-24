#include "alph.h"
#define	SIZE	300
#define	XMIN	(800/2-SIZE/2)
#define	YMIN	(1024/2-SIZE/2)
#undef	XMAX
#undef	YMAX
#define	XMAX	(XMIN+SIZE)
#define	YMAX	(YMIN+SIZE)
lineslide() {
	int x, y, dx, dy, x1, y1, dx1, dy1;
	clearscreen();
	x=r(XMIN, XMAX-1);
	y=r(YMAX-(SIZE/4), YMAX-1);
	x1=r(XMIN, XMAX-1);
	y1=r(YMAX-(SIZE/4), YMAX-1);
	dx=r(1, 3);
	dy=r(1, 3);
	dx1=r(-3, -1);
	dy1=r(-3, -1);
	while (!keyhit()) {
		x=x+dx;
		if ((x<XMIN) || (XMAX-1<x)) {
			x=x-2*dx;
			dx= -dx;
		}
		y=y+dy;
		if ((y<YMAX-(SIZE/4)) || (YMAX-1<y)) {
			y=y-2*dy;
			dy= -dy;
		}
		x1=x1+dx1;
		if ((x1<XMIN) || (XMAX-1<x1)) {
			x1=x1-2*dx1;
			dx1= -dx1;
		}
		y1=y1+dy1;
		if ((y1<YMAX-(SIZE/4)) || (YMAX-1<y1)) {
			y1=y1-2*dy1;
			dy1= -dy1;
		}
		segment(&display, Pt(x, y), Pt(x1, y1), F_XOR);
		bitblt(&display, Rect(XMIN, YMIN+1, XMAX, YMAX),
			&display, Pt(XMIN, YMIN), F_STORE);
		rectf(&display, Rect(XMIN, YMAX-1, XMAX, YMAX), F_CLR);
	}
}


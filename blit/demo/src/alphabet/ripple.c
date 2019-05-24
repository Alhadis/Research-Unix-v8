#include "alph.h"
ripple() {
	int XSIZE = 383, YSIZE = 512;
	int x, y, xinc, yinc;
	WonB();
	xinc=1;
	yinc=1;
	clearscreen();
	x=0;
	y=0;
	GXfunction = NOT;
	while (!keyhit()) {
		if ((x!=0) && (y!=0))
			rasterop1 (x*2, y*2, 384-x, 512-y);
		x=x+xinc;
		if ((x==XSIZE) || (x==0)) xinc= -xinc;
		y=y+yinc;
		if ((y==YSIZE) || (y==0)) yinc= -yinc;
	}
	BonW();
}


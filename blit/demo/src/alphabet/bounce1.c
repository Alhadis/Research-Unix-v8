#include "alph.h"
bounce1() {
	int XSIZE = 641, YSIZE = 897;
	int x, y, xinc, yinc, i;
	WonB();
	xinc=1;
	yinc=1;
	clearscreen();
	x=0;
	y=0;
	GXfunction = NOT;
	while (!keyhit()) {
		rasterop1 (129, 129, x, y);
		x=x+xinc;
		if ((x==XSIZE) || (x==0)) xinc= -xinc;
		y=y+yinc;
		if ((y==YSIZE) || (y==0)) yinc= -yinc;
	}
	BonW();
}

#include "alph.h"
circle1() {
	int x, y;

	clearscreen();
	GXfunction = NOT;
	for (x=0; x<=180; x++) {
		y=0;
		while ((x*x+y*y)<(180*180)) y=y+1;
		if ((x!=0) && (y!=0))
			rasterop1 (2*x, 2*y, 400-x, 512-y);
	}
	while (!keyhit());
}

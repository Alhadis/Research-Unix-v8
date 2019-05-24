#include "alph.h"

circle2() {
	int x, y, z;

	while (!keyhit()) {
		clearscreen();
		GXfunction = NOT;
		for (x=1; x<=20; x++) {
			y = random() % 179;
			if (y<=0) y=y+179;
			z=1;
			while ((y*y+z*z)<(180*180)) z=z+1;
			rasterop1 (4*z,4*y,400-2*z,512-2*y);
			z = random() % 179;
			if (z<=0) z=z+179;
			y=1;
			while ((y*y+z*z)<(180*180)) y=y+1;
			rasterop1 (4*z,4*y,400-2*z,512-2*y);
		}
		for (x=1; x<=32766; x++)
			for (y=1; y<=2; y++);
	}
}

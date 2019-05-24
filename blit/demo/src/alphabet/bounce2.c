#include "alph.h"
bounce2() {
	int x, y, vx, vy, z;

	clearscreen();
	x=0;
	y=0;
	vx=4;
	vy=0;
	while (!keyhit()) {
		rectf(&display, Rect(x, y, x+20, y+20), F_XOR);
		x=x+vx;
		if (x>(XMAX-1-20)) {
			x=2*(XMAX-1-20)-x;
			vx= -vx;
		}
		else if (x<0) {
			x= -x;
			vx= -vx;
		}
		vy=vy+1;
		y=y+vy;
		if (y>=(YMAX-1-20)) {
			y=YMAX-1-20;
			if (vy<20) vy=1-vy;
			else vy=vy / 20 - vy;
			if (vy==0) {
				x=0;
				y=0;
				vx=4;
				vy=0;
			}
		}
		nap(1);
	}
}

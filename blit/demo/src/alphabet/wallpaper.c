#include "alph.h"
#define	SIZE 100
wallpaper() {
	int x0, y0, x1, y1, t, vx0, vy0, vx1, vy1, i, j;
	clearscreen();
	x0=0;
	y0=0;
	x1=0;
	y1=0;
	vx0=0;
	vy0=0;
	vx1=0;
	vy1=0;
	while (!keyhit()) {
		if (r(1, 10)!=1) GXfunction=NOT; 
		else GXfunction=SET;
		for (i= -1; i<=7; i++)
			for (j= -1; j<=9; j++)
				combox(x0+i*SIZE, y0+j*SIZE, x1-x0, y1-y0);
		vx0=vx0+r(-1, 1);
		if (vx0<-5) vx0= -5; 
		else if (5<vx0) vx0=5;
		x0=x0+vx0;
		if (x0<0) {
			x0= -x0;
			vx0= -vx0;
		} else if (SIZE<x0) {
			x0=2*SIZE-x0;
			vx0= -vx0;
		}
		vx1=vx1+r(-1, 1);
		if (vx1<-5) vx1= -5; 
		else if (5<vx1) vx1=5;
		x1=x1+vx1;
		if (x1<0) {
			x1= -x1;
			vx1= -vx1;
		} else if (SIZE<x1) {
			x1=2*SIZE-x1;
			vx1= -vx1;
		}
		if (x0>x1) {
			t=x0;
			x0=x1;
			x1=t;
			t=vx0;
			vx0=vx1;
			vx1=t;
		}
		vy0=vy0+r(-1, 1);
		if (vy0<-5) vy0= -5; 
		else if (5<vy0) vy0=5;
		y0=y0+vy0;
		if (y0<0) {
			y0= -y0;
			vy0= -vy0;
		} else if (SIZE<y0) {
			y0=2*SIZE-y0;
			vy0= -vy0;
		}
		vy1=vy1+r(-1, 1);
		if (vy1<-5) vy1= -5; 
		else if (5<vy1) vy1=5;
		y1=y1+vy1;
		if (y1<0) {
			y1= -y1;
			vy1= -vy1;
		} else if (SIZE<y1) {
			y1=2*SIZE-y1;
			vy1= -vy1;
		}
		if (y0>y1) {
			t=y0;
			y0=y1;
			y1=t;
			t=vy0;
			vy0=vy1;
			vy1=t;
		}
	}
}

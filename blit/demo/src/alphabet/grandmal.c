#include "alph.h"
#define SIZE 128

combox(x, y, xs, ys)
int x, y, xs, ys;
{

	if (x<0) { 
		xs=xs+x; 
		x=0; 
	}
	if (y<0) { 
		ys=ys+y; 
		y=0; 
	}
	if ((x<XMAX) && (y<YMAX)) {
		if (x+xs>XMAX-1) xs=XMAX-1-x;
		if (y+ys>YMAX) ys=YMAX-y;
		if ((xs>0) && (ys>0))
			rasterop1 (xs, ys, x, y);
	}
}

int flag;

grandmal() {
	int x0, y0, x1, y1, t, vx0, vy0, vx1, vy1, snooze;

	clearscreen();
	x0=XMAX/2;
	y0=YMAX/2;
	x1=XMAX/2;
	y1=YMAX/2;
	vx0=0;
	vy0=0;
	vx1=0;
	vy1=0;
	flag=NOT;
	while (!keyhit()) {
		GXfunction = flag;
		if ((x0!=x1) && (y0!=y1))
			combox (x0, y0, (x1-x0), (y1-y0));
		if (r(1, 10)!=1) flag=NOT; 
		else flag=SET;
		vx0=vx0+r(-1, 1);
		if (vx0<-5) vx0= -5; 
		else if (5<vx0) vx0=5;
		x0=x0+vx0;
		if (x0<0) {
			x0= -x0;
			vx0= -vx0;
		} else if (XMAX-1<x0) {
			x0=2*(XMAX-1)-x0;
			vx0= -vx0;
		}
		vx1=vx1+r(-1, 1);
		if (vx1<-5) vx1= -5; 
		else if (5<vx1) vx1=5;
		x1=x1+vx1;
		if (x1<0) {
			x1= -x1;
			vx1= -vx1;
		} else if (XMAX-1<x1) {
			x1=2*(XMAX-1)-x1;
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
		} else if (YMAX-1<y0) {
			y0=2*(YMAX-1)-y0;
			vy0= -vy0;
		}
		vy1=vy1+r(-1, 1);
		if (vy1<-5) vy1= -5; 
		else if (5<vy1) vy1=5;
		y1=y1+vy1;
		if (y1<0) {
			y1= -y1;
			vy1= -vy1;
		} else if ((YMAX-1)<y1) {
			y1=2*(YMAX-1)-y1;
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

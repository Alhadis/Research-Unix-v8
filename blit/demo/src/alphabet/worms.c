#include "alph.h"
#define N 20
#define	R 10
#define	centerdraw(b, x, y, f)	bitblt(b, b->rect, &display, Pt(x-R, y-R), F_XOR)
worms() {
	register i, j, x0, y0, t;
	int x[N], y[N], vx[N], vy[N];
	register Bitmap *b=balloc(Rect(-R, -R, R+1, R+1));

	if(b==0)
		return;
	clearscreen();
	rectf(b, b->rect, F_CLR);
	disc(b, Pt(0, 0), R, F_OR);
	for (i=0; i<N; i++) {
		x[i]=r(R, XMAX-1-R);
		y[i]=r(R, YMAX-1-R);
		vx[i]=r(-6, 6);
		vy[i]=r(-6, 6);
		centerdraw(b, x[i], y[i], F_XOR);
	}
	while (!keyhit()) {
		for (i=0; i<N; i++) {
			rectf(&display, Rect(x[i]-R ,y[i]-R, x[i]+R+1, y[i]+R+1),
				F_OR);
			x[i]=x[i]+vx[i];
			if (x[i]<R) {
				x[i]=2*R-x[i];
				vx[i]= -vx[i];
			} else if (x[i]>XMAX-1-R) {
				x[i]=2*(XMAX-1-R)-x[i];
				vx[i]= -vx[i];
			}
			y[i]=y[i]+vy[i];
			if (y[i]<R) {
				y[i]=2*R-y[i];
				vy[i]= -vy[i];
			} else if (y[i]>YMAX-1-R) {
				y[i]=2*(YMAX-1-R)-y[i];
				vy[i]= -vy[i];
			}
			centerdraw(b, x[i], y[i], F_XOR);
		}
		for (i=0; i<=N; i++) {
			for (j=i+1; j<=N; j++) {
				x0=x[i]-x[j];
				y0=y[i]-y[j];
				if ((long)x0*x0+(long)y0*y0<(long)2*R*2*R) {
					if (y0<0) {
						y0= -y0;
						x0= -x0;
					}
					if ((10*abs(x0))>(10*y0)) {
						vx[i]= -vx[i];
						vx[j]= -vx[j];
					} else if ((10*y0)>(12*abs(x0))) {
						vy[i]= -vy[i];
						vy[j]= -vy[j];
					} else if (y0>0) {
						t=vx[i];
						vx[i]= -vy[i];
						vy[i]= -t;
						t=vx[j];
						vx[j]= -vy[j];
						vy[j]= -t;
					} else {
						t=vx[i];
						vx[i]= -vy[i];
						vy[i]=t;
						t=vx[j];
						vx[j]= -vy[j];
						vy[j]=t;
					}
				}
			}
		}
	}
	bfree(b);
}

#include "alph.h"
#define MAXN	2
#define	NVERT	4
tetra() {
	int dx[MAXN][NVERT], dy[MAXN][NVERT];
	int x[MAXN][NVERT], y[MAXN][NVERT];
	register i, j, k, n;
	register first;

	if(param==0)
		n=6; 
	else
		n=param;
	if(n<1)
		n=1;
	clearscreen();
	for (i=0; i<NVERT; i++) {
		x[1][i]=x[0][i]=r(0, XMAX-1);
		y[1][i]=y[0][i]=r(0, YMAX-1);
		dx[0][i]=dx[1][i]=r(5, 9);
		dy[0][i]=dy[1][i]=r(5, 9);
	}
	first=n;
	while(!keyhit()){
		for (k=0; (first>=0)? (k<1) : (k<2); k++) {
			for (i=0; i<NVERT; i++) {
				x[k][i]+=dx[k][i];
				if(x[k][i]<0 || XMAX<=x[k][i]){
					x[k][i]-=2*dx[k][i];
					dx[k][i]= -dx[k][i];
				}
				y[k][i]+=dy[k][i];
				if(y[k][i]<0 || YMAX<=y[k][i]){
					y[k][i]-=2*dy[k][i];
					dy[k][i]= -dy[k][i];
				}
				for(j=0; j<i; j++)
					segment(&display, Pt(x[k][i], y[k][i]),
					    Pt(x[k][j], y[k][j]), F_XOR);
			}
		}
		if(first>=0)
			--first;
	}
}

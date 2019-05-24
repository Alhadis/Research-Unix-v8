#include "alph.h"
extern done;
#define NX 48
#define NY -36
#define NZ 80
long
lsqroot(a)
	register long a;
{
	register long x, y;
	if(a<=0)
		return 0;
	for(y=a,x=1;y!=0;y>>=2,x<<=1)
		;
	while((y=(a/x+x)>>1)<x)
		x=y;
	return x;
}
sphere (x0, y0, rad)
int x0, y0, rad;
{
	register x, y;
	register long zsq, i;

	for (x= -rad; x<=rad; x++) {
		if (!done) {
			done=keyhit();
			for (y= -rad; y<=rad; y++) {
				zsq=(long)rad*rad-(long)x*x-(long)y*y;
				if (zsq>=0) {
					i=NX*(long)x+NY*(long)y+NZ*lsqroot(zsq);
/*			if (dith[x0&DITHMASK][y0&DITHMASK] > (h0+h1+h2+h3)/64+32)*/
					if (r(0, rad*100)>i)
						*(display.base+((y+y0)*(unsigned)50)+((x+x0)>>4))|=1<<(15-((x+x0)&15));
					else
						*(display.base+((y+y0)*(unsigned)50)+((x+x0)>>4))&=~(1<<(15-((x+x0)&15)));
				}
			}
		}
	}
}
spheres() {
	clearscreen();
	done=false;
	do
		sphere(r(180, XMAX-1-180), r(180, YMAX-1-180), r(50, 180));
	while (!done);
}

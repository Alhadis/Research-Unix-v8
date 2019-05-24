#include "alph.h"
#define	POINT(x, y)	*(display.base+((y)*50)+((x)>>4))^=1<<(15-((x)&15))
munch() {
	register x, t, dt;
	register unsigned y;
	clearscreen();
	if (param==0)
		dt=1; 
	else
		dt=param;
	t=0;
	while (!keyhit()) {
		for (x=0; x<XMAX; x++){
			y=((YMAX-1) & (x ^ t));
			POINT(x, y);
		}
		t+=dt;
	}
}

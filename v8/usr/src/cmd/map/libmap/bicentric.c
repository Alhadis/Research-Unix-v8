#include "map.h"

static struct coord center;

Xbicentric(p,x,y)
struct place *p;
float *x, *y;
{
	if(p->wlon.c<=.01||p->nlat.c<=.01)
		return(-1);
	*x = -center.c*p->wlon.s/p->wlon.c;
	*y = p->nlat.s/(p->nlat.c*p->wlon.c);
	return(*x**x+*y**y<=9);
}

int (*bicentric(l))()
float l;
{
	l = fabs(l);
	if(l>89)
		return(0);
	deg2rad(l,&center);
	return(Xbicentric);
}

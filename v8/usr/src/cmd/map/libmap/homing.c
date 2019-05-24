#include "map.h"

struct coord stdpar;

Xmecca(place, x, y)
struct place *place;
float *x, *y;
{
	*x = - place->wlon.l;
	*y = -(place->nlat.c*stdpar.s -
		place->nlat.s*stdpar.c*place->wlon.c)/stdpar.c;
	if(fabs(place->wlon.l) > 0.01)
		*y *= *x/sin(*x);
	if(fabs(*y)>2.0)
		return(0);
	if(place->nlat.s*stdpar.s +
	   place->nlat.c*stdpar.c*place->wlon.c <0)
		return(-1);
	return(1);
}

int (*mecca(par))()
float par;
{
	if(fabs(par)>80.)
		return(0);
	deg2rad(par,&stdpar);
	return(Xmecca);
}

Xhoming(place, x, y)
struct place *place;
float *x, *y;
{
	int i = Xmecca(place, x, y);
	float r, h;
	if(i != 1)
		return(i);
	r = acos(stdpar.s*place->nlat.s +
		stdpar.c*place->nlat.c*place->wlon.c);
	h = hypot(*x, *y);
	r = h<.001? r: r/h;
	*x *= r;
	*y *= r;
	return(1);
}

int (*homing(par))()
float par;
{
	if(mecca(par)==0)
		return(0);
	return(Xhoming);
}

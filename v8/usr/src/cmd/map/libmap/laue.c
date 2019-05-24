#include "map.h"


Xlaue(place, x, y)
struct place *place;
float *x, *y;
{
	float r;
	if(place->nlat.l<PI/4+FUZZ)
		return(-1);
	r = tan(PI-2*place->nlat.l);
	if(r>3)
		return(-1);
	*x = - r * place->wlon.s;
	*y = - r * place->wlon.c;
	return(1);
}

int (*laue())()
{
	return(Xlaue);
}

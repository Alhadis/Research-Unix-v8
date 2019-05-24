#include "map.h"

struct coord stdp0;

Xcylequalarea(place, x, y)
struct place *place;
float *x, *y;
{
	*x = - place->wlon.l * stdp0.c;
	*y = place->nlat.s;
	return(1);
}

int (*cylequalarea(par))()
float par;
{
	if(par > 89.0)
		return(0);
	deg2rad(par, &stdp0);
	return(Xcylequalarea);
}

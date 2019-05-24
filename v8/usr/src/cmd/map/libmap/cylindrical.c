#include "map.h"

Xcylindrical(place, x, y)
struct place *place;
float *x, *y;
{
	if(fabs(place->nlat.l) > 80.*RAD)
		return(-1);
	*x = - place->wlon.l;
	*y = place->nlat.s / place->nlat.c;
	return(1);
}

int (*cylindrical())()
{
	return(Xcylindrical);
}

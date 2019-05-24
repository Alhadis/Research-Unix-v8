#include "map.h"

Xrectangular(place, x, y)
struct place *place;
float *x, *y;
{
	*x = -place->wlon.l;
	*y = place->nlat.l;
	return(1);
}

int (*rectangular())()
{
	return(Xrectangular);
}

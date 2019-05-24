#include "map.h"

Xorthographic(place, x, y)
struct place *place;
float *x, *y;
{
	*x = - place->nlat.c * place->wlon.s;
	*y = - place->nlat.c * place->wlon.c;
	return(place->nlat.l<0.? 0 : 1);
}

int (*orthographic())()
{
	return(Xorthographic);
}

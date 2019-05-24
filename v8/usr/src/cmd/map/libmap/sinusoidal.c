#include "map.h"

Xsinusoidal(place, x, y)
struct place *place;
float *x, *y;
{
	*x = - place->wlon.l * place->nlat.c;
	*y = place->nlat.l;
	return(1);
}

int (*sinusoidal())()
{
	return(Xsinusoidal);
}

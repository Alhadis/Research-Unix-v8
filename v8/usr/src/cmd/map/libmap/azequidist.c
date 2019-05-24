#include "map.h"

Xazequidistant(place, x, y)
struct place *place;
float *x, *y;
{
	float colat;
	colat = PI/2 - place->nlat.l;
	*x = -colat * place->wlon.s;
	*y = -colat * place->wlon.c;
	return(1);
}

int (*azequidistant())() 
{
		return(Xazequidistant);
}

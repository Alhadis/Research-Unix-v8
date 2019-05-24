#include "map.h"

Xazequalarea(place, x, y)
struct place *place;
float *x, *y;
{
	double r;
	r = sqrt(1. - place->nlat.s);
	*x = - r * place->wlon.s;
	*y = - r * place->wlon.c;
	return(1);
}

int (*azequalarea())()
{
	return(Xazequalarea);
}

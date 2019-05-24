#include "map.h"
#include <math.h>

static
Xmercator(place,x,y)
struct place *place;
float *x, *y;
{
	if(fabs(place->nlat.l) > 80.*RAD)
		return(-1);
	*x = -place->wlon.l;
	*y = 0.5*log((1+place->nlat.s)/(1-place->nlat.s));
	return(1);
}

int (*mercator())()
{
	return(Xmercator);
}
static float ecc = ECC;

static
Xspmercator(place,x,y)
struct place *place;
float *x, *y;
{
	if(Xmercator(place,x,y) < 0)
		return(-1);
	*y += 0.5*ecc*log((1-ecc*place->nlat.s)/(1+ecc*place->nlat.s));
	return(1);
}

int (*sp_mercator())()
{
	return(Xspmercator);
}

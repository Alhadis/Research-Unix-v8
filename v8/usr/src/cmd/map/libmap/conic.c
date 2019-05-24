#include "map.h"

struct coord stdpar;

Xconic(place, x, y)
struct place *place;
float *x, *y;
{
	float r;
	if(fabs(place->nlat.l-stdpar.l) > 80.*RAD)
		return(-1);
	r = stdpar.c/stdpar.s - tan(place->nlat.l - stdpar.l);
	*x = - r*sin(place->wlon.l * stdpar.s);
	*y = - r*cos(place->wlon.l * stdpar.s);
	if(r>3) return(0);
	return(1);
}

int (*conic(par))()
float par;
{
	extern Xcylindrical();
	if(fabs(par) <.1)
		return(Xcylindrical);
	deg2rad(par, &stdpar);
	return(Xconic);
}

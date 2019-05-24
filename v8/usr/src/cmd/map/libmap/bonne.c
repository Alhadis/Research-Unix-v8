#include "map.h"

struct coord stdpar;
float r0;

Xbonne(place, x, y)
struct place *place;
float *x, *y;
{
	double r, alpha;
	r = r0 - place->nlat.l;
	alpha = place->wlon.l * place->nlat.c / r;
	*x = - r*sin(alpha);
	*y = - r*cos(alpha);
	return(1);
}

int (*bonne(par))()
float par;
{
	extern Xsinusoidal();
	if(fabs(par*RAD) < .01)
		return(Xsinusoidal);
	deg2rad(par, &stdpar);
	r0 = stdpar.c/stdpar.s + stdpar.l;
	return(Xbonne);
}

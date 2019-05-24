#include "map.h"

#define Xaitwist Xaitpole.nlat
struct place Xaitpole;

Xaitoff(place, x, y)
struct place *place;
float *x, *y;
{
	struct place p;
	copyplace(place,&p);
	p.wlon.l /= 2.;
	sincos(&p.wlon);
	norm(&p,&Xaitpole,&Xaitwist);
	Xazequalarea(&p,x,y);
	*x *= 2.;
	return(1);
}

int (*aitoff())()
{
	latlon(0.,0.,&Xaitpole);
	return(Xaitoff);
}

#include "map.h"

float viewpt;

Xperspective(p, x, y)
struct place *p;
float *x, *y;
{
	float r;
	if(viewpt<=1. && p->nlat.s<=viewpt+.01)
		return(-1);
	r = p->nlat.c*(viewpt - 1.)/(viewpt - p->nlat.s);
	*x = - r*p->wlon.s;
	*y = - r*p->wlon.c;
	if(r>4.)
		return(0);
	if(fabs(viewpt)>1. && p->nlat.s<=1./viewpt)
		return(0);
	return(1);
}

Xstereographic(p, x, y)
struct place *p;
float *x, *y;
{
	viewpt = -1;
	return(Xperspective(p, x, y));
}

int (*perspective(radius))()
float radius;
{
	extern Xorthographic();
	viewpt = radius;
	if(radius>=1000.)
		return(Xorthographic);
	if(fabs(radius-1.)<.0001)
		exit(1);
	return(Xperspective);
}

int (*stereographic())()
{
	viewpt = -1.;
	return(Xperspective);
}

int (*gnomonic())()
{
	viewpt = 0.;
	return(Xperspective);
}

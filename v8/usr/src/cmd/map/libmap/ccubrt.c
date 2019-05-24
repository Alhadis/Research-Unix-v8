#include "map.h"
double cubrt();

ccubrt(zr,zi,wr,wi)
double zr,zi;
double *wr,*wi;
{
	double r, theta;
	theta = atan2(zi,zr);
	r = cubrt(hypot(zr,zi));
	*wr = r*cos(theta/3);
	*wi = r*sin(theta/3);
}

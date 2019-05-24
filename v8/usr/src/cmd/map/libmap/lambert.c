#include "map.h"

struct coord stdp0, stdp1;
float k;

Xlambert(place, x, y)
struct place *place;
float *x, *y;
{
	float r;
	if(place->nlat.l < -80.*RAD||place->nlat.l > 89.*RAD)
		return(-1);
	r = stdp0.c*exp(0.5*k*log(
	   (1+stdp0.s)*(1-place->nlat.s)/((1-stdp0.s)*(1+place->nlat.s))));
	if(stdp1.l<0.)
		r = -r;
	*x = - r*sin(k * place->wlon.l);
	*y = - r*cos(k * place->wlon.l);
	return(1);
}

int (*lambert(par0, par1))()
float par0, par1;
{
	extern (*mercator())(), (*perspective())();
	float temp;
	if(fabs(par0)>fabs(par1)){
		temp = par0;
		par0 = par1;
		par1 = temp;
	}
	deg2rad(par0, &stdp0);
	deg2rad(par1, &stdp1);
	if(fabs(par1+par0)<.1) 
		return(mercator());
	if(fabs(par1-par0)<.1)
		return(perspective(-1.));
	if(fabs(par0)>89.5||fabs(par1)>89.5)
		return(0);
	k = 2*log(stdp1.c/stdp0.c)/log(
		(1+stdp0.s)*(1-stdp1.s)/((1-stdp0.s)*(1+stdp1.s)));
	return(Xlambert);
}

#include "map.h"

Xgilbert(place,x,y)
struct place *place;
float *x,*y;
{
	float z1,z2;
	double w1,w2,t1,t2;
	struct place p;
	copyplace(place,&p);
	if(place->nlat.l<0) {
		p.nlat.l = -p.nlat.l;
		p.nlat.s = -p.nlat.s;
	}
	Xstereographic(&p,&z1,&z2);
	csqrt(-z2/2,z1/2,&w1,&w2);
	cdiv(w1-1,w2,w1+1,w2,&t1,&t2);
	*y = -t1;
	*x = t2;
	if(place->nlat.l<0)
		*y = -*y;
	return(1);
}

int (*gilbert())()
{
	return(Xgilbert);
}

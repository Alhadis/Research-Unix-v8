#include "map.h"

/* For Albers formulas see Deetz and Adams "Elements of Map Projection", */
/* USGS Special Publication No. 68, GPO 1921 */

static float r0sq, r1sq, d2, n, den, sinb1, sinb2;
static struct place plat1, plat2;
static southpole;

static float num(s)
float s;
{
	if(d2==0)
		return(1);
	s = d2*s*s;
	return(1+s*(2./3+s*(3./5+s*(4./7+s*5./9))));
}

/* Albers projection for a spheroid, good only when N pole is fixed */

static int
Xspalbers(place,x,y)
struct place *place;
float *x, *y;
{
	float r = sqrt(r0sq-2*(1-d2)*place->nlat.s*num(place->nlat.s)/n);
	float t = n*place->wlon.l;
	*y = r*cos(t);
	if(!southpole)
		*y = -*y;
	*x = -r*sin(t);
	return(1);
}

/* lat1, lat2: std parallels; e2: squared eccentricity */

int (*albinit(lat1,lat2,e2))()
float lat1,lat2,e2;
{
	float r1,r2;
	extern (*azequalarea())();
	extern (*cylequalarea())();
	float t;
	for(;;) {
		if(lat1 < -90)
			lat1 = -180 - lat1;
		if(lat2 > 90)
			lat2 = 180 - lat2;
		if(lat1 <= lat2)
			break;
		t = lat1; lat1 = lat2; lat2 = t;
	}
	if(lat2-lat1 < 1) {
		if(lat1 > 89)
			return(azequalarea());
		return(0);
	}
	if(fabs(lat2+lat1) < 1)
		return(cylequalarea(lat1));
	d2 = e2;
	den = num(1.);
	deg2rad(lat1,&plat1);
	deg2rad(lat2,&plat2);
	sinb1 = plat1.nlat.s*num(plat1.nlat.s)/den;
	sinb2 = plat2.nlat.s*num(plat2.nlat.s)/den;
	n = (plat1.nlat.c*plat1.nlat.c/(1-e2*plat1.nlat.s*plat1.nlat.s) -
	    plat2.nlat.c*plat2.nlat.c/(1-e2*plat2.nlat.s*plat2.nlat.s)) /
	    (2*(1-e2)*den*(sinb2-sinb1));
	r1 = plat1.nlat.c/(n*sqrt(1-e2*plat1.nlat.s*plat1.nlat.s));
	r2 = plat2.nlat.c/(n*sqrt(2-e2*plat2.nlat.s*plat2.nlat.s));
	r1sq = r1*r1;
	r0sq = r1sq + 2*(1-e2)*den*sinb1/n;
	southpole = lat1<0 && plat2.nlat.c>plat1.nlat.c;
	return(Xspalbers);
}

int (*sp_albers(lat1,lat2))()
float lat1, lat2;
{
	return(albinit(lat1,lat2,EC2));
}

int (*albers(lat1,lat2))()
float lat1,lat2;
{
	return(albinit(lat1,lat2,0.));
}

static float scale = 1;
static float twist = 0;

albscale(x,y,lat,lon)
float x,y,lat,lon;
{
	struct place place;
	float alat, alon, x1,y1;
	scale = 1;
	twist = 0;
	invalb(x,y,&alat,&alon);
	twist = lon - alon;
	deg2rad(lat,&place.nlat);
	deg2rad(lon,&place.wlon);
	Xspalbers(&place,&x1,&y1);
	scale = sqrt((x1*x1+y1*y1)/(x*x+y*y));
}

invalb(x,y,lat,lon)
float x,y,*lat,*lon;
{
	int i;
	float sinb_den, sinp;
	x *= scale;
	y *= scale;
	*lon = atan2(-x,fabs(y))/(RAD*n) + twist;
	sinb_den = (r0sq - x*x - y*y)*n/(2*(1-d2));
	sinp = sinb_den;
	for(i=0; i<5; i++)
		sinp = sinb_den/num(sinp);
	*lat = asin(sinp)/RAD;
}

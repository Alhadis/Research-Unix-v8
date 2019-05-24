#include "map.h"

struct place pole;	/* map pole is tilted to here */
struct coord twist;	/* then twisted this much */
struct place ipole;	/* inverse transfrom */
struct coord itwist;

double cirmod();

orient(lat, lon, theta)
float lat, lon, theta;
{
	lat = cirmod(lat);
	if(lat>90.) {
		lat = 180. - lat;
		lon -= 180.;
		theta -= 180.;
	} else if(lat < -90.) {
		lat = -180. - lat;
		lon -= 180.;
		theta -= 180;
	}
	latlon(lat,lon,&pole);
	deg2rad(theta, &twist);
	latlon(lat,180.-theta,&ipole);
	deg2rad(180.-lon, &itwist);
}

latlon(lat,lon,p)
float lat,lon;
struct place *p;
{
	lat = cirmod(lat);
	if(lat>90.) {
		lat = 180. - lat;
		lon -= 180.;
	} else if(lat < -90.) {
		lat = -180. - lat;
		lon -= 180.;
	}
	deg2rad(lat,&p->nlat);
	deg2rad(lon,&p->wlon);
}

deg2rad(theta, coord)
float theta;
struct coord *coord;
{
	theta = cirmod(theta);
	coord->l = theta*RAD;
	if(theta==90) {
		coord->s = 1;
		coord->c = 0;
	} else if(theta== -90) {
		coord->s = -1;
		coord->c = 0;
	} else
		sincos(coord);
}

double cirmod(theta)
double theta;
{
	while(theta >= 180.)
		theta -= 360;
	while(theta<-180.)
		theta += 360.;
	return(theta);
}

sincos(coord)
struct coord *coord;
{
	coord->s = sin(coord->l);
	coord->c = cos(coord->l);
}

normalize(gg)
struct place *gg;
{
	norm(gg,&pole,&twist);
}

invert(g)
struct place *g;
{
	norm(g,&ipole,&itwist);
}

norm(gg,pp,tw)
struct place *gg, *pp;
struct coord *tw;
{
	register struct place *g;	/*geographic coords */
	register struct place *p;	/* new pole in old coords*/
	struct place m;			/* standard map coords*/
	g = gg;
	p = pp;
	if(p->nlat.s == 1.) {
		if(p->wlon.l+tw->l == 0.)
			return;
		g->wlon.l -= p->wlon.l+tw->l;
	} else {
		if(p->wlon.l != 0) {
			g->wlon.l -= p->wlon.l;
			sincos(&g->wlon);
		}
		m.nlat.s = p->nlat.s * g->nlat.s
			+ p->nlat.c * g->nlat.c * g->wlon.c;
		m.nlat.c = sqrt(1. - m.nlat.s * m.nlat.s);
		m.nlat.l = atan2(m.nlat.s, m.nlat.c);
		m.wlon.s = g->nlat.c * g->wlon.s;
		m.wlon.c = p->nlat.c * g->nlat.s
			- p->nlat.s * g->nlat.c * g->wlon.c;
		m.wlon.l = atan2(m.wlon.s, - m.wlon.c)
			- tw->l;
		*g = m;
	}
	sincos(&g->wlon);
	if(g->wlon.l>PI)
		g->wlon.l -= 2*PI;
	else if(g->wlon.l<-PI)
		g->wlon.l += 2*PI;
}

double tan(x)
double x;
{
	return(sin(x)/cos(x));
}
printp(g)
struct place *g;
{
printf("%.3f %.3f %.3f %.3f %.3f %.3f\n",
g->nlat.l,g->nlat.s,g->nlat.c,g->wlon.l,g->wlon.s,g->wlon.c);
}

copyplace(g1,g2)
struct place *g1,*g2;
{
	*g2 = *g1;
}

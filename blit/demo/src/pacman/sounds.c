/*----------------------------------------------------------------------*/
/*									*/
/*	PACMAN for BBN BitGraphs					*/
/*									*/
/*	          File:							*/
/*	      Contents:							*/
/*	        Author: Bob Brown (rlb)					*/
/*			Purdue CS					*/
/*		  Date: May, 1982					*/
/*	   Description:							*/
/*									*/
/*									*/
/*									*/
/*----------------------------------------------------------------------*/

#include "style.h"
#include "pacman.h"

#define IOADDR(n)	((char *) (384*1024L+(n<<6)+056))
char *rport = IOADDR(2);
char *reg = IOADDR(0);
char *wport = IOADDR(1);

int tonegen(), snd_dot();

int
sounddot()
{
	if ( Silent ) return;
	snd_dot(NULL,0);
}
int
snd_dot(junk,part)
char *junk;
int part;
{
	switch(part){
	case 0:
		tonegen(1,1200,12);
		eladd(SNDDOTDELAY,snd_dot,NULL,1);
		break;
	case 1:
		tonegen(1,1000,15);
		eladd(SNDDOTDELAY,snd_dot,NULL,2);
		break;
	case 2:
		tonegen(1,0);
	}
}
/*
** This is the sound when a monster is killed
*/
#define KILLFROM 200
#define KILLTO	50

soundkill()
{
	tonegen(1,KILLFROM,13);
	psgsweep(0,KILLFROM,KILLTO,5);
	tonegen(1,0,0);
}
psgsweep(t,f1,f2,v)
{
	int i,f;
	i = (f1 > f2) ? -1 : 1;
	for (f = f1; f != f2; f += i) {
		tonegen(t+1,f,v);
		sleep(1);
	}
}
int
tonegen(t,p,v)
register t,p,v;
{
	char enable = 077;
	if ( p > 0 ) {
		enable &= ~(1<<(t-1));
		psgwrite(p&0xff,(t-1)*2);
		psgwrite((p>>8),t*2-1);
		psgwrite(v,7+t);
		psgwrite(enable,7);
	} else
		psgwrite(0,7+t);
}
psgwrite(i,n)
{
	*reg = n;
	*wport = i;
}

psgread(n)
{
	register i;
	*reg = n;
	i = *rport;
	return (i&0377);
}
soundoff()
{
	psgwrite(0,8);
	psgwrite(0,9);
	psgwrite(0,10);
}

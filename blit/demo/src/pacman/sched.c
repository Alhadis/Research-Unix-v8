/*----------------------------------------------------------------------*/
/*									*/
/*	PACMAN for BBN BitGraphs					*/
/*									*/
/*	          File:	sched.c68					*/
/*	      Contents:	passive real-time-clock-based scheduler		*/
/*	        Author: Bob Brown (rlb)					*/
/*			Purdue CS					*/
/*		  Date: May, 1982					*/
/*	   Description:	An event list manager for the scheduling 	*/
/*			of C procedures after given time delays		*/
/*									*/
/*----------------------------------------------------------------------*/

#include "style.h"
#include "pacman.h"

/*
** Beacuse the scheduler calls routines that re-enqueue themselves,
** The following constant is needed to prevent elpoll() from starving
** the keyboard polling routine when the monsters are running flat-out.
*/

#define MAXLOOPS 5

/*
** elinit() - initialize the event list to empty.
** eladd() - add an event to the event list.
** elpoll() - called at in polling loop to run scheduled events.
*/

/*
**  MAXEVENTS ...
**
**	1 for pacman
**	4 for monsters
**	4 for power pill effects
**	1 for bonus fruits
**	3 for tone generators
*/

#define MAXEVENTS	13
#define ENULL		((event *) 0)

typedef struct ev {
	struct ev *next;
	int time;
	int (*routine)();	/* procedure to call */
	char *arg0;		/* optional first argument */
	int arg1;		/* optional second argument */
} event;

event *Eventfree;
event *Eventhead;
event Event[MAXEVENTS];

extern int clock;

elinit()
{
	register event *ep;
	Eventfree = ENULL;
	for ( ep=Event ; ep<&Event[MAXEVENTS] ; ep++ ) {
		ep->next = Eventfree;
		Eventfree = ep;
	}
	Eventhead = ENULL;
	clock = 0;
}
/*
** eladd - add an event to the event list
*/
/* VARARGS 2 */
eladd(delta,routine,arg0,arg1)
int delta;
int (*routine)();
char *arg0;
int arg1;
{
	int time;
	register event *ep, *pp, *cp;

#ifdef BLIT
	delta = (delta+8)/16;
#endif
#ifdef V1_25
	delta = (delta+5)/10;
#endif

	if ( Eventfree == ENULL ) {
#ifndef BLIT
		error("Event list overflow");
#endif
		return;
	}
	cp = Eventfree;
	Eventfree = Eventfree->next;

	time = delta+clock;
	cp->time = time;
	cp->routine = routine;
	cp->arg0 = arg0;
	cp->arg1 = arg1;

	ep = Eventhead;
	pp = ENULL;
	while ( ep!=ENULL && time>ep->time ) {
		pp = ep;
		ep = ep->next;
	}
	cp->next = ep;
	if ( pp == ENULL )
		Eventhead = cp;
	else
		pp->next = cp;
}
/*
** elpoll - check if anything to schedule
*/
elpoll()
{
	register int maxloops;
	register event *ep;
	maxloops = MAXLOOPS;
	while ( (ep=Eventhead)!=ENULL && ep->time < clock && --maxloops > 0) {
		Eventhead = ep->next;
		(*ep->routine)(ep->arg0,ep->arg1);
		ep->next = Eventfree;
		Eventfree = ep;
	}
}
/*
** eladjust - adjust the scheduler based on processor delay
*/
eladjust(ms)
register int ms;
{
#ifdef BLIT
	ms = (ms+8)/16;
#endif
#ifdef V1_25
	ms = (ms+5)/10;
#endif
	clock -= ms;
}

#include "../chunix/chsys.h"
#include "../chunix/chconf.h"
#include "chch.h"
#include "../chaos/chaos.h"
#include "../h/systm.h"
/*
 * This file contains initializations of configuration dependent data
 * structures and device dependent initialization functions.
 */

/*
 * We must identify ourselves
 */
short Chmyaddr = -1;
char Chmyname[CHSTATNAME] = "Uninitialized";
short chhosts[] = {0};
int Chhz = 60;		/* This is set correctly at auto-conf time but needs
			 * a non-zero initial value at boot time.
			 */

/*
 * Reset all devices
 */
chrreset()
{
	register struct chroute *r;

	for (r = Chroutetab; r < &Chroutetab[CHNSUBNET]; r++)
		if (r->rt_cost == 0)
			r->rt_cost = CHHCOST;
#if NCHDR > 0
	chdrinit();
#endif NCHDR
#if NCHCH > 0
	chchinit();
#endif NCHCH
#if NCHIL > 0
	chilinit();
#endif NCHIL

	/*
	 * This is necessary to preserve the modularity of the
	 * NCP.
	 */
	Chhz = hz;
}
/*
 * Check for interface timeouts
 */
chxtime()
{
#ifdef NDR11C
	chdrxtime();
#endif NDR11C
}

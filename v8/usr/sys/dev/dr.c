/*
 *  DR-11C interface
 *  The routines in this driver are not called through the normal
 *  device interface.  Instead, they are available for other device
 *  drivers to use to send arbitrary information out on a DR-11C.
 */

#include "dr.h"
#if NDR > 0
#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/conf.h"
#include "../h/inode.h"
#include "../h/file.h"
#include "../h/systm.h"
#include "../h/pte.h"
#include "../h/buf.h"
#include "../h/ubareg.h"
#include "../h/ubavar.h"

/* number of output bits available in a DR -- must be a power of 2 */
#define DRWIDTH 16

/* DRWIDTH is 2**DRSHIFT */
#define DRSHIFT 4

struct drreg {
	u_short drcsr, drout, drin;
};

int	drattach(), drprobe();
struct	uba_device *drinfo[NDR];
u_short	drstd[] = { 0 };
struct	uba_driver drdriver =
	{ drprobe, 0, drattach, 0, drstd, "dr", drinfo };

drprobe(reg)
caddr_t reg;
{
	register int br, cvec;
	register struct drreg *draddr = (struct drreg *) reg;

	draddr->drcsr = 0;
	br = 0x15;
	cvec = 0700;		/* hope this will not be used */
	return 1;
}

drattach()
{
}

/* set bit n of the dr-11 complex to v */
drsetbit (n, v)
	register int n;
	register int v;
{
	if (drinfo[n>>DRSHIFT]->ui_alive == 0)
		return;
	if (n >= 0 && n < (NDR << DRSHIFT)) {
		register u_short bit;
		register struct drreg *drptr;
		drptr = (struct drreg *) drinfo[n>>DRSHIFT] -> ui_addr;
		bit = 1 << (n & (DRWIDTH - 1));
		if (v)
			drptr->drout |= bit;
		else
			drptr->drout &= ~bit;
	}
}

drrint()
{
}

drxint()
{
}
#endif

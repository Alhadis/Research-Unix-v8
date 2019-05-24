/*	sys.c	4.3	81/03/08	*/

/*
 * Indirect driver for controlling tty.
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/file.h"
#include "../h/mx.h"

/*ARGSUSED*/
syopen(dev, flag)
{

	if(u.u_ttyp == NULL || (u.u_procp->p_flag&SDETACH)) {
		u.u_error = ENXIO;
		return;
	}
	if (u.u_ttychan)	/* mpx channel; do nothing */
		return;
	(*cdevsw[major(u.u_ttyd)].d_open)(u.u_ttyd, flag);
}

/*ARGSUSED*/
syread(dev)
{
	register struct chan *cp;

	if (u.u_procp->p_flag&SDETACH) {
		u.u_error = ENXIO;
		return;
	}
	if (cp=u.u_ttychan)
		msread(FMPY, cp);
	else
		(*cdevsw[major(u.u_ttyd)].d_read)(u.u_ttyd);
}

/*ARGSUSED*/
sywrite(dev)
{
	register struct chan *cp;

	if (u.u_procp->p_flag&SDETACH) {
		u.u_error = ENXIO;
		return;
	}
	if (cp=u.u_ttychan)
		mswrite(FMPY, cp);
	else
		(*cdevsw[major(u.u_ttyd)].d_write)(u.u_ttyd);
}

/*ARGSUSED*/
syioctl(dev, cmd, addr, flag)
caddr_t addr;
{

	register struct chan *cp;

	if (u.u_procp->p_flag&SDETACH) {
		u.u_error = ENXIO;
		return;
	}
	if (cp=u.u_ttychan) {
		mxsioctl(cp, cmd, addr, flag);
		return;
	}
	(*cdevsw[major(u.u_ttyd)].d_ioctl)(u.u_ttyd, cmd, addr, flag);
}

/*	%W%	*/
#include "iti.h"
#include "../h/pte.h"
#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/systm.h"
#include "../h/ioctl.h"
#include "../h/ubareg.h"
#include "../h/buf.h"
#include "../h/ubavar.h"
#include "../h/itireg.h"
int	itiprobe(), itiattach();
struct	uba_device *itidinfo[NITI];
u_short	itistd[]={ 0160100, 0 };
struct uba_driver itidriver={
	itiprobe, 0, itiattach, 0, itistd, "iti", itidinfo, 0, 0
};
struct iti{
	char open;
}iti[NITI];
itiprobe(reg)
caddr_t reg;
{
	register int br=0x15, cvec=0710;		/* value-result */
	return(1);
}
/*ARGSUSED*/
itiattach(ui)
struct uba_device *ui;
{
}
itiopen(dev)
dev_t dev;
{
	register struct iti *p=&iti[minor(dev)];
	register struct uba_device *ui=itidinfo[minor(dev)];
	if(minor(dev)>=NITI || p->open || !ui->ui_alive)
		u.u_error=ENXIO;
	else{
		p->open=1;
		maptouser(ui->ui_addr);
	}
}
iticlose(dev)
dev_t dev;
{
	iti[minor(dev)].open=0;
	unmaptouser(itidinfo[minor(dev)]->ui_addr);
}
/*ARGSUSED*/
itiioctl(dev, cmd, data, flag)
dev_t dev;
int cmd;
register caddr_t data;
int flag;
{
	register struct uba_device *ui = itidinfo[minor(dev)];
	switch (cmd) {
	case ITIADDR:
		copyout((caddr_t)&ui->ui_addr, (caddr_t)data, sizeof ui->ui_addr);
		break;
	default:
		return (ENOTTY);
	}
	return (0);
}
/*ARGSUSED*/
itiintr(dev)
dev_t dev;
{}
/*
 * Map a virtual address into users address space. Actually all we
 * do is turn on the user mode write protection bits for the particular
 * page of memory involved.
 */
maptouser(vaddress)
caddr_t vaddress;
{
	Sysmap[((unsigned)vaddress-0x80000000)>>9].pg_prot=PG_UW>>27;
}
unmaptouser(vaddress)
caddr_t vaddress;
{
	Sysmap[((unsigned)vaddress-0x80000000)>>9].pg_prot=PG_KW>>27;
}

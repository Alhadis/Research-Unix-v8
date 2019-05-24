/*
 * DN-11 ACU interface
 *  Minor devices < <128 are standard 801's
 *  Minor devices >= 128 are Parker shared
 *  units in which the 1st digit selects a line.
 *  The only interface difference is that DLO
 *  can appear some time after the line is selected
 *  to indicate that the line is busy or out of service.
 *
 *  If the system has an accessible DR-11C, allow some of
 *  its bits to be fiddled.
 */

#include "dn.h"
#if NDN > 0
#include "dr.h"
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

struct device {
	u_short	dn_reg[4];
};

#define	PWI	0100000
#define	ACR	040000
#define	DLO	010000
#define	DONE	0200
#define	IENABLE	0100
#define	DSS	040
#define	PND	020
#define	MAINT	010
#define	MENABLE	04
#define	DPR	02
#define	CRQ	01

#define	DNPRI	(PZERO+5)

int	dnattach(), dnprobe();
struct	uba_device *dninfo[NDN];
u_short	dnstd[] = { 0 };
struct	uba_driver dndriver =
	{ dnprobe, 0, dnattach, 0, dnstd, "dn", dninfo };

dnprobe(reg)
caddr_t reg;
{
	register int br, cvec;
	register struct device *dnaddr = (struct device *)reg;

	dnaddr->dn_reg[0] = MENABLE|IENABLE|MAINT;
	dnaddr->dn_reg[0] |= CRQ;
	dnaddr->dn_reg[0] |= 1<<8;	/* digit 0 loop to PND */
	DELAY(10000);
	dnaddr->dn_reg[0] = MENABLE;
	return(1);
}

dnattach()
{
}

dnopen(dev)
register dev;
{
	register struct device *dp;
	register unit;

	dev = minor(dev) & 0177;
	unit = dev>>2;
	dp = (struct device *)dninfo[unit]->ui_addr;
	if (unit >= NDN ||
	   dninfo[unit]->ui_alive == 0 ||
	   dp->dn_reg[dev&03]&(PWI|DLO|CRQ))
		u.u_error = ENXIO;
	else {
		dp->dn_reg[0] |= MENABLE;
		dp->dn_reg[dev&03] = IENABLE|MENABLE|CRQ;
	}
}

dnclose(dev)
{
	dev = minor(dev) & 0177;
	((struct device *)dninfo[dev>>2]->ui_addr)->dn_reg[dev&03] = MENABLE;
}

dnwrite(dev)
{
	register c;
	register u_short *dp;
	register first;
	extern lbolt;
	int ldlo;
	register int setspeed = -1;
	register k;

	ldlo = 0;
	if (dev & 0200)
		ldlo = DLO;
	dev = minor(dev) & 0177;
	dp = & ((struct device *)dninfo[dev>>2]->ui_addr)->dn_reg[dev&03];
	if (*dp & DLO) {
		u.u_error = EIO;
		return;
	}
	*dp |= CRQ;
	first = 0;
	while ((*dp & (PWI|ACR|DSS|ldlo)) == 0) {
		spl4();
		if ((*dp&PND) == 0 || u.u_count == 0 || (c=cpass()) < 0)
			sleep((caddr_t)dp, DNPRI);
		else if (c == '-') {
			sleep((caddr_t)&lbolt, DNPRI);
			sleep((caddr_t)&lbolt, DNPRI);
#if NDR > 0
		} else if ((k = dnindex (c, "abcdefghABCDEFGH")) >= 0) {
			setspeed = k;
#endif
		} else {
			*dp = (c<<8)|IENABLE|MENABLE|DPR|CRQ;
			if (first == 0) {
				while (first <  800)
					first++;
				if (*dp&ldlo) {
					*dp = MENABLE;
					break;
				}
				ldlo = 0;
			}
			sleep((caddr_t)dp, DNPRI);
		}
		spl0();
	}
	if (*dp&(PWI|ACR|ldlo))
		u.u_error = EIO;
	else if (setspeed >= 0)
		drsetbit (setspeed % 8, setspeed >= 8);
}

/*
 * interrupt-- "dev" applies to
 * system unit number, not minor device
 */
dnint(dev)
{
	register u_short *ep;
	register struct device *dp;

	dp = (struct device *)dninfo[dev]->ui_addr;
	dp->dn_reg[0] &= ~MENABLE;
	for (ep = &dp->dn_reg[0]; ep < &dp->dn_reg[4]; ep++)
		if (*ep&DONE) {
			*ep &= ~DONE;
			wakeup((caddr_t)ep);
		}
	dp->dn_reg[0] |= MENABLE;
}

static int
dnindex (c, s)
	register char c, *s;
{
	register char *p;

	p = s;
	while (*p) {
		if (c == *p)
			return p - s;
		p++;
	}
	return -1;
}
#endif

/*	dr.c	4.10	82/05/18	*/

#include "rtk.h"
#if NRTK > 0
/*
 * UNIBUS DR11-B driver for various graphical systems,.
 * TS
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/cpu.h"
#include "../h/nexus.h"
#include "../h/dk.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/map.h"
#include "../h/pte.h"
#include "../h/mtpr.h"
#include "../h/vm.h"
#include "../h/ubavar.h"
#include "../h/ubareg.h"
#include "../h/cmap.h"

#include "../h/drreg.h"

#define	DR_GO	01
#define	DR_FNCT1	02
#define	DR_GIE	010
#define	DR_IE	0100
#define	DR_ATTN	0200
#define DR_CYCLE	0400
#define	DR_NEX	00400000
#define	DR_GRY	04000
#define	DR_GIR	020000
#define	DR_ERROR	01000000

char label[] = "Change the size";
int	rtkprobe(),rtkattach(),rtkdgo(),rtkintr();
int	drstrategy(),drstart();
struct	uba_ctlr *rtkminfo[NRTK];
struct	uba_device *rtkdinfo[NRTK];
struct	uba_ctlr drctlr[NRTK];	/* rtkminfo points to this */
u_short rtkstd[]	= { 0172466, 0};	/* used to be
u_short	rtkstd[]	= { 0172410,0172430,0172450,0172470,0 }; */
struct	uba_driver rtkdriver =
	{ rtkprobe,0,rtkattach,rtkdgo,rtkstd,"rtk",rtkdinfo,"rtk",rtkminfo };

#define ui_open	ui_type
struct	buf	drbuf[NRTK];
struct	buf	drutab[NRTK];

/*ARGSUSED*/
rtkprobe(reg)
caddr_t reg;
{
	register int br,cvec;

#ifdef LINT
	br = 0; cvec = br; br = cvec;
#endif
printf("rtkprobe entered.\n");

/* There seems to be no way to make this vile animal
   interrupt, so we cheat...				*/

	br = 0x15;
	cvec = 0210;
}

rtkattach(ui)
register struct uba_device *ui;
{
	register struct uba_ctlr *um;
	register int unit;

	unit = ui->ui_unit;

	um = &drctlr[unit];
	rtkminfo[unit] = um;
	ui->ui_ctlr = unit;
	ui->ui_mi = um;
	um->um_driver = ui->ui_driver;
	um->um_ctlr = unit;
	um->um_ubanum = ui->ui_ubanum;
	um->um_alive = 1;
	um->um_intr = ui->ui_intr;
	um->um_addr = ui->ui_addr;
	um->um_hd = ui->ui_hd;
}

rtkopen(dev)
dev_t dev;
{
	register int unit;

	unit = minor(dev);
	if((unit >= NRTK) || (rtkdinfo[unit]->ui_open)) {
		u.u_error = ENXIO;
		return;
	}
	rtkdinfo[unit]->ui_open++;
}

rtkclose(dev)
dev_t dev;
{
	register int unit;

	rtkdinfo[minor(dev)]->ui_open = 0;
}

rtkread(dev)
dev_t dev;
{
	register int unit = minor(dev);

	physio(drstrategy,&drbuf[unit],dev,B_READ,minphys);
}

rtkwrite(dev)
dev_t dev;
{
	register int unit = minor(dev);

	physio(drstrategy,&drbuf[unit],dev,B_WRITE,minphys);
}

/*
 * Due to the fact the drstrategy routine is called only by rtkread
 * and rtkwrite via physio, there will only be one transaction in each
 * DR11-B's queue at any time.  Therefore, one can just tack the given
 * buffer header pointer on at the end of the queue, and call drstart.
 */
drstrategy(bp)
register struct buf *bp;
{
	register struct uba_device *ui;
	register struct uba_ctlr *um;
	register struct buf *dp;
	dev_t unit;

	spl5();
	unit = minor(bp->b_dev);	/* chose a DR11-B */
	ui = rtkdinfo[unit];
	um = ui->ui_mi;		/* get ctlr ptr */
	dp = &drutab[unit];
	dp->b_actf = bp;
	dp->b_actl = bp;
	bp->av_forw = NULL;
	um->um_tab.b_actf = dp;
	um->um_tab.b_actl = dp;

	drstart(um);
	spl0();
}

drstart(um)
register struct uba_ctlr *um;
{
	register struct buf *bp,*dp;
	register struct drdevice *draddr;
	int cmd;

	dp = um->um_tab.b_actf;
	bp = dp->b_actf;

	um->um_tab.b_active++;
	draddr = (struct drdevice *)um->um_addr;
	draddr->drwc = -bp->b_bcount / sizeof (short);
	if(bp->b_flags & B_READ)
		cmd = DR_IE|DR_FNCT1;
	else
		cmd = DR_IE;
	um->um_cmd = cmd|DR_GO;
	draddr->drcs = cmd;
	DELAY(10);
	(void) ubago(rtkdinfo[minor(bp->b_dev)]);
}

rtkioctl() {}
drreset() {}
rtkdgo(um)
struct uba_ctlr *um;
{
	register struct drdevice *draddr = (struct drdevice *)um->um_addr;

	draddr->drba = um->um_ubinfo;
	draddr->drcs = um->um_cmd|((um->um_ubinfo>>12)&0x30);
}
rtkintr(dr11)
register dr11;
{
	register struct buf *bp,*dp;
	register struct drdevice *draddr;
	register struct uba_ctlr *um;

	um = rtkminfo[dr11];

	if(um->um_tab.b_active == 0)
		return;

	dp = um->um_tab.b_actf;
	bp = dp->b_actf;
	draddr = (struct drdevice *)um->um_addr;

	if((draddr->drcs&0100000) && draddr->drwc && (draddr->drba == 0)) {
		draddr->drcs = um->um_cmd|(((um->um_ubinfo>>12)+1)&0x30);
		return;
	}

	um->um_tab.b_active = 0;
	um->um_tab.b_errcnt = 0;
	um->um_tab.b_actf = dp->b_forw;
	dp->b_errcnt = 0;
	dp->b_active = 0;
	bp->b_resid = (-draddr->drwc * sizeof(short));
	ubadone(um);
	iodone(bp);
}
#endif

/*	dr.c	4.10	82/05/18	*/

#include "ekx.h"
#if NEKX > 0
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

#include "../h/ekcmd.h"

struct ekdevice{
	u_short ekgc;
	u_short ekdc;
	u_short ekgs;
	u_short ekds;
	short ekwc;
	u_short ekba;
	u_short ekcs;
	u_short ekdata;
};
struct ekctl {
	u_short mode;
	u_short dma_device;
	u_short device;
	u_short state;
} ek[NEKX];
#define	EK_GO	01
#define EK_FCTN		016
#define EK_12BIT	0
#define EK_12BITST	04
#define EK_8BIT		02
#define EK_8BITST	06
#define EK_DARK		0
#define EK_GAIN		02
#define EK_HTST		010
#define EK_GTST		012
#define EK_X1617	060
#define	EK_IE	0100
#define EK_READY	0200
#define EK_DIR	0400
#define EK_ODEV		01000
#define EK_OVER		02000
#define EK_ATTN	020000
#define	EK_NEX	040000
#define	EK_ERROR	0100000

int	ekxprobe(),ekxattach(),ekxdgo(),ekxintr();
int	ekstrategy(),ekstart();
struct	uba_ctlr *ekxminfo[NEKX];
struct	uba_device *ekxdinfo[NEKX];
struct	uba_ctlr ekctlr[NEKX];	/* ekxminfo points to this */
u_short ekxstd[]	= { 0163700, 0};	
struct	uba_driver ekxdriver =
	{ ekxprobe,0,ekxattach,ekxdgo,ekxstd,"ekx",ekxdinfo,"ekx",ekxminfo };

#define ui_open	ui_type
struct	buf	ekbuf[NEKX];
struct	buf	ekutab[NEKX];
int ct1, ct2, ct3, ct4, ct5, ct6, ct7, ct8, ct9, ct10, ct11;
short sekgs, sekds,  sekcs;
unsigned short sekwc;

/*ARGSUSED*/
ekxprobe(reg)
caddr_t reg;
{
	register int br,cvec;

#ifdef LINT
	br = 0; cvec = br; br = cvec;
#endif

/* There seems to be no way to make this vile animal
   interrupt, so we cheat...				*/

	br = 0x15;
	cvec = 0270;
}

ekxattach(ui)
register struct uba_device *ui;
{
	register struct uba_ctlr *um;
	register int unit;

	unit = ui->ui_unit;

	um = &ekctlr[unit];
	ekxminfo[unit] = um;
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

ekxopen(dev)
dev_t dev;
{
	register int unit;

	ct1 = ct2 = ct3 = ct4 = ct5 = 0;
	ct6 = ct7 = ct8 = ct9 = 0;
	ct10 = ct11 = 0;
	unit = minor(dev);
	if((unit >= NEKX) || (ekxdinfo[unit]->ui_open)) {
		u.u_error = ENXIO;
		return;
	}
	ekxdinfo[unit]->ui_open++;
}

ekxclose(dev)
dev_t dev;
{
	register int unit;

	ekxdinfo[minor(dev)]->ui_open = 0;
}

ekxread(dev)
dev_t dev;
{
	register int unit = minor(dev);

	ct1++;
	physio(ekstrategy,&ekbuf[unit],dev,B_READ,minphys);
	ct2++;
}

ekxwrite(dev)
dev_t dev;
{
	register int unit = minor(dev);

	physio(ekstrategy,&ekbuf[unit],dev,B_WRITE,minphys);
}

/*
 * Due to the fact the ekstrategy routine is called only by ekxread
 * and ekxwrite via physio, there will only be one transaction in each
 * DR11-B's queue at any time.  Therefore, one can just tack the given
 * buffer header pointer on at the end of the queue, and call ekstart.
 */
ekstrategy(bp)
register struct buf *bp;
{
	register struct uba_device *ui;
	register struct uba_ctlr *um;
	register struct buf *dp;
	register int s;
	struct ekdevice *draddr;
	dev_t unit;

	ct3++;
	unit = minor(bp->b_dev);	/* chose a DR11-B */
	ui = ekxdinfo[unit];
	um = ui->ui_mi;		/* get ctlr ptr */
	dp = &ekutab[unit];
	s = spl5();
	dp->b_actf = bp;
	dp->b_actl = bp;
	bp->av_forw = NULL;
	um->um_tab.b_actf = dp;
	um->um_tab.b_actl = dp;

	ekstart(um);
	splx(s);
	switch(tsleep((caddr_t)bp, PRIBIO+1, 20)) {
	case TS_OK:
		break;
	case TS_SIG:	/* not supposed to happen*/
	case TS_TIME:
		draddr = (struct ekdevice *)um->um_addr;
		sekgs = draddr->ekgs;
		sekds = draddr->ekds;
		sekwc = draddr->ekwc;
		sekcs = draddr->ekcs;
		printf("gs %o ds %o wc %o cs %o\n",sekgs, sekds, sekwc, sekcs);
		printf("ct1 %o ct2 %o ct3 %o ct4 %o ct5 %o ct6 %o\n",ct1,ct2,ct3,ct4,ct5,ct6);
		printf("ct7 %o ct8 %o ct9 %o ct10 %o ct11 %o\n",ct7,ct8,ct9,ct10,ct11);
		bp->b_flags |= B_DONE | B_ERROR;
		s = spl6();
		ekxintr(unit);
		splx(s);
	}
}

ekstart(um)
register struct uba_ctlr *um;
{
	register struct buf *bp,*dp;
	register struct ekdevice *draddr;
	register struct ekctl *ekp;
	int cmd;

	dp = um->um_tab.b_actf;
	bp = dp->b_actf;
	ekp = &ek[minor(bp->b_dev)];

	um->um_tab.b_active++;
	draddr = (struct ekdevice *)um->um_addr;
	draddr->ekwc = -bp->b_bcount / sizeof (short);
	if(bp->b_flags & B_READ)
		cmd = EK_IE|EK_DIR|ekp->mode;	
	else
		cmd = EK_IE|ekp->dma_device;
	um->um_cmd = cmd|EK_GO;
	ct4++;
	draddr->ekcs = cmd;
	ct5++;
	DELAY(10);
	if( ubago(ekxdinfo[minor(bp->b_dev)])== 0)
		printf("ubago returned 0\n");
}

ekxioctl(dev, cmd, arg)
dev_t dev;
int cmd;
register caddr_t arg;
{
	register struct ekctl *ekp = &ek[minor(dev)];
	register struct uba_device *ui = ekxdinfo[minor(dev)];
	register struct ekdevice *draddr = (struct ekdevice *)ui->ui_addr;
	u_short realcmd;

	if( (cmd != EKGS) && (cmd != EKDMA)){
		if(copyin(arg, (caddr_t)&realcmd, sizeof(realcmd))){
			u.u_error = EFAULT;
			return;
		}
	}
	switch(cmd){
	case EKGC:
		draddr->ekgc = realcmd;
		return;
	case EKDC:
		draddr->ekdc = realcmd;
		return;
	case EKGS:
		realcmd = draddr->ekgs;
		break;
	case EKDS:
		draddr->ekdc = EKDC_READ|realcmd;
		realcmd = draddr->ekds;
		break;
	case EKMOD:
		ek->mode = realcmd & EK_FCTN;
		return;
	case EKDEV:
		ek->dma_device = realcmd & EK_FCTN;
		return;
	default:
		u.u_error = ENXIO;
		return;
	}
	if(copyout((caddr_t)&realcmd, arg, sizeof(realcmd)))
		u.u_error = EFAULT;
}
ekreset() {}
ekxdgo(um)
struct uba_ctlr *um;
{
	register struct ekdevice *draddr = (struct ekdevice *)um->um_addr;

	ct6++;
	draddr->ekba = um->um_ubinfo;
	draddr->ekcs = um->um_cmd|((um->um_ubinfo>>12)&EK_X1617);
	ct7++;
}
ekxintr(dr11)
register dr11;
{
	register struct buf *bp,*dp;
	register struct ekdevice *draddr;
	register struct uba_ctlr *um;
	register int stat;

	ct8++;
	um = ekxminfo[dr11];

	if(um->um_tab.b_active == 0)
		return;

	if(ekxdinfo[dr11]->ui_open == 0)
		return;
	dp = um->um_tab.b_actf;
	bp = dp->b_actf;
	draddr = (struct ekdevice *)um->um_addr;
	stat = draddr->ekcs;

	if(stat & EK_ODEV){
		ct11++;
	}

			/*should check stat&EK_OVER for retry*/
	if((stat&EK_ERROR) && draddr->ekwc && (draddr->ekba == 0)) {
		draddr->ekcs = um->um_cmd|(((um->um_ubinfo>>12)+1)&EK_X1617);
		ct10++;
	}

	um->um_tab.b_active = 0;
	um->um_tab.b_errcnt = 0;
	um->um_tab.b_actf = dp->b_forw;
	dp->b_errcnt = 0;
	dp->b_active = 0;
	bp->b_resid = (-draddr->ekwc * sizeof(short));
	ubadone(um);
	iodone(bp);
	ct9++;
}
#endif

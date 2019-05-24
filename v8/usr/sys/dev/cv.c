#include "cv.h"
#if NCV > 0
/*
 * Raster Technologies Color Video Interface -- Rob Pike
 *	Model One 20 with Option Card, on DR-11/W
 */

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/systm.h"
#include "../h/pte.h"
#include "../h/map.h"
#include "../h/ioctl.h"
#include "../h/ubareg.h"
#include "../h/ubavar.h"

struct cvdevice {
	short	wcr;		/* Unibus word count reg */
	u_short	bar;		/* Unibus address register */
	u_short	csr;		/* Unibus status/command reg */
	u_short	idr;		/* Input Data Register */
};
#define	eir	csr		/* Error and Information Register */
#define	odr	idr		/* Output Data Register */

/*
 * Unibus status/command register bits
 */

#define GO		0000001
#define IENABLE		0000100
#define READY		0000200
#define	MAINT		0010000
#define	ERROR		0100000

/*
 * Function codes
 */
#define	WR_IMAGE	(1<<1)
#define	WR_CMD		(4<<1)
#define	WR_RLE		(5<<1)	
#define RD_CMD		(7<<1)
#define RD_IMAGE	(3<<1)
#define	FUNCMASK	(7<<1)
#define	TOGGLE		(8<<1)	/* toggle between X and WR_CMD, WR_CMD first */

/*
 * IOCTL
 */
#define	CVSETDMA	(('c'<<8)|1)

#define BUSY 01
#define DMAPRI (PZERO-1)
#define WAITPRI (PZERO+1)

int	cvprobe(), cvattach(), cvintr();
struct	uba_device *cvdinfo[NCV];
u_short	cvstd[] = { 0772410, 0000000, 0 };
struct	uba_driver cvdriver = {
	cvprobe, 0, cvattach, 0, cvstd, "cv", cvdinfo, 0, 0
};

struct cvsoftc {
	char	open;
	short	uid;
	short	state;
	int	ubinfo;
	int	count;
	struct	buf *bp;
	int	bufp;
	int	icnt;
	short	dmamode;
	short	func;
} cvsoftc[NCV];

int	cvstrategy();
u_int	cvminphys();
struct	buf rcvbuf[NCV];

#define UNIT(dev) (minor(dev))

cvprobe(reg)
	caddr_t reg;
{
	register int br, cvec;		/* value-result */
	register struct cvdevice *cvaddr = (struct cvdevice *) reg;

#ifdef lint
	br = 0; cvec = br; br = cvec;
	cvintr(0);
#endif
	cvaddr->csr = IENABLE;
	cvaddr->odr = 0x0707;	/* clear screen, force interrupt */
	DELAY(10000);
	cvaddr->csr = 0;
	/* KLUDGE */
	br=0x15;
	cvec=0124;
	return (sizeof (struct cvdevice));
}

/*ARGSUSED*/
cvattach(ui)
	struct uba_device *ui;
{

}

cvopen(dev)
	dev_t dev;
{
	register struct cvsoftc *cvp;
	register struct uba_device *ui;

	if (UNIT(dev) >= NCV || (cvp = &cvsoftc[minor(dev)])->open ||
	    (ui = cvdinfo[UNIT(dev)]) == 0 || ui->ui_alive == 0)
		u.u_error = EBUSY;
	else {
		cvp->open = 1;
		cvp->icnt = 0;
		cvp->state = 0;
		cvp->uid = u.u_uid;
		cvp->dmamode = -1;
	}
}

cvclose(dev)
	dev_t dev;
{

	cvsoftc[minor(dev)].open = 0;
	cvsoftc[minor(dev)].state = 0;
}

cvread(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register int unit = UNIT(dev);

	if (unit >= NCV)
		return (ENXIO);
	return (physio(cvstrategy, &rcvbuf[unit], dev, B_READ, cvminphys, uio));
}

cvwrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register int unit = UNIT(dev);

	if (unit >= NCV)
		return (ENXIO);
	return (physio(cvstrategy, &rcvbuf[unit], dev, B_WRITE, cvminphys, uio));
}

u_int
cvminphys(bp)
	register struct buf *bp;
{

	if (bp->b_bcount > 65536)	/* may be able to do twice as much */
		bp->b_bcount = 65536;
}

cvstrategy(bp)
	register struct buf *bp;
{
	register struct cvsoftc *cvp = &cvsoftc[UNIT(bp->b_dev)];
	register struct uba_device *ui;

	if (UNIT(bp->b_dev) >= NCV || (ui = cvdinfo[UNIT(bp->b_dev)]) == 0
				|| ui->ui_alive == 0)
		goto bad;
	(void) spl5();
	while (cvp->state & BUSY)
		sleep((caddr_t)cvp, DMAPRI+1);
	cvp->state |= BUSY;
	cvp->bp = bp;
	if(bp->b_bcount<=0 || (bp->b_bcount&1) || 
	    ((int)bp->b_un.b_addr&1) || cvp->dmamode<0){
		u.u_error = EINVAL;
		goto bad;
	}
	cvp->ubinfo = ubasetup(ui->ui_ubanum, bp, UBA_NEEDBDP);
	cvp->bufp = cvp->ubinfo & 0x3ffff;
	cvp->count = -(bp->b_bcount>>1);	/* it's a word count */
	cvstart(ui);
	if(tsleep((caddr_t)cvp, DMAPRI+1, 15) != TS_OK){
		register struct cvdevice *cvaddr = (struct cvdevice *) ui->ui_addr;
		bp->b_flags |= B_ERROR;
		/* stop the dma */
		cvslam(cvaddr);
		cvaddr->wcr=0xFFFF;	/* gently; -1 first... */
		cvaddr->wcr=0x0000;	/* then zero */
		/* reset device */
		cvslam(cvaddr);
		/* fake an interrupt to clear software status */
		cvintr(UNIT(bp->b_dev));
	}
	cvp->count = 0;
	cvp->bufp = 0;
	(void) spl0();
	ubarelse(ui->ui_ubanum, &cvp->ubinfo);
	cvp->bp = 0;
	iodone(bp);
	wakeup((caddr_t)cvp);
	return;
   bad:
	cvp->state &= ~BUSY;
	bp->b_flags |= B_ERROR;
	iodone(bp);
	return;
}

cvslam(cvaddr)
	register struct cvdevice *cvaddr;
{
	register i;
	cvaddr->csr=MAINT;
	for(i=0; i<10; i++)
		;
	cvaddr->csr=0;
	for(i=0; i<10; i++)
		;
}
cvstart(ui)
	register struct uba_device *ui;
{
	register int csr;
	register struct cvdevice *cvaddr = (struct cvdevice *) ui->ui_addr;
	register struct cvsoftc *cvp = &cvsoftc[UNIT(ui->ui_unit)];

	if((cvp->dmamode&TOGGLE) && cvp->func!=WR_CMD)
		cvp->func = WR_CMD;
	else
		cvp->func = cvp->dmamode&~TOGGLE;
	csr = IENABLE|((cvp->bufp>>12)&060) | cvp->func;
	cvaddr->wcr =  cvp->count;
	cvaddr->bar = cvp->bufp;
	cvaddr->csr = csr;	/* No GO bit; let function codes settle */
	cvaddr->csr = csr|GO;
}

/*ARGSUSED*/
cvioctl(dev, cmd, data)
	dev_t dev;
	int cmd;
	register caddr_t data;
{
	register struct uba_device *ui = cvdinfo[UNIT(dev)];
	register struct cvsoftc *cvp = &cvsoftc[UNIT(dev)];
	int fmt;

	switch (cmd) {
	case CVSETDMA:	/* set DMA mode */
		if (copyin(data, (caddr_t)&fmt, sizeof(fmt))) {
			u.u_error = EFAULT;
			goto out;
		}
		switch (fmt&~TOGGLE) {
		case WR_IMAGE:
		case WR_CMD:
		case WR_RLE:
		case RD_CMD:
		case RD_IMAGE:
			cvp->dmamode = cvp->func = fmt;
			cvp->func &= ~TOGGLE;
			break;
		default:
			u.u_error = EINVAL;
		}
		goto out;

	default:
		return (ENOTTY);
	}
   out:
	return u.u_error;
}

/*ARGSUSED*/
cvintr(dev)
	dev_t dev;
{
	register struct cvdevice *cvaddr =
			(struct cvdevice *) cvdinfo[UNIT(dev)]->ui_addr;
	register struct cvsoftc *cvp = &cvsoftc[UNIT(dev)];
	register csr;

	cvp->icnt++;
	if (cvp->state&BUSY) {
		csr = cvaddr->csr;
		if(csr & ERROR){
			cvaddr->eir |= ERROR;
			printf("cv: CSR %x EIR %x\n", csr, cvaddr->eir);
		}
		cvaddr->csr = csr&~FUNCMASK;
		cvp->state &= ~BUSY;
		wakeup((caddr_t)cvp);
	}
}

cvreset(uban)
	int uban;
{
	register int i;
	register struct uba_device *ui;
	register struct cvsoftc *cvp = cvsoftc;
#ifdef untried
	for (i = 0; i < NCV; i++, cvp++) {
		if ((ui = cvdinfo[i]) == 0 || ui->ui_alive == 0 ||
		    ui->ui_ubanum != uban || cvp->open == 0)
			continue;
		printf(" cv%d", i);
		if ((cvp->state&CVBUSY) == 0)
			continue;
		cvp->ubinfo =
		    ubasetup(ui->ui_ubanum, cvp->bp, UBA_NEEDBDP);
		cvp->count = -(cvp->bp->b_bcount/2);
		cvstart(ui);
	}
#endif
}
#endif

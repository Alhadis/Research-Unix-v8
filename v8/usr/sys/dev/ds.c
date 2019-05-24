/* 5-29-84  dsrelease checks for zombie state or even death */
/* dsclose() shuts down device */
/* DSWAIT added */
/* 5-25-84  process pointer saved in dsopen for dsintr() */
/*	vsunlock must then be duplicated using correct process structure */
/* the interrelation of dsstart-sleeping, dsintr, and dscleanup
	needs to be cleaned up 
	Probably dscleanup should not call wakeup */


#include "ds.h"
# if NDS > 0

/*
 * DSC System 200 driver
 * via DSC dma11.
 * vax 4.1bsd version
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/mount.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/pte.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../h/ubareg.h"
#include "../h/ubavar.h"
#include "../h/conf.h"
#include "../h/proc.h"
#include "../h/file.h"
#include "../h/vmmac.h"

#include "../h/dsreg.h"
#include "../h/dsc.h"

/*
 * THESE ARE SITE-SPECIFIC
 *
 * starting base for the
 * d/a and a/d converters,
 * for setting up sequence ram.
 */

/*
 * reset device
 */
# define RESETDEV	bit(4)
# define dsseq(sc, reg, conv, dir) \
	(sc->c_ascseq[reg] = conv | ((dir & 01) << 6))

/*
 * ds flags
 */
# define DS_CLOSED	0
# define DS_OPEN	bit(0)
# define DS_IDLE	bit(1)
# define DS_NDSK	bit(2)
# define DS_MON		bit(3)
# define DS_BRD		bit(4) 
# define DS_READ	bit(5)
# define DS_WRITE	bit(6)


# define DSPRI		(PZERO+1)
/*
 * Redundant book-keeping
 * to help avoid page-wise overlap of buffers
 *	remember that this device is both asynchronous and
 *	uses user-supplied buffers
 */
struct dspageinfo {
	int firstunit,
		lastunit;	/* in units of CLSIZE*NBPG, i.e. 1024 */
};
/*
 * relevant information
 * about the dma and asc
 */
struct ds_softc {
	short		c_dmacsr;	/* copy of dma csr */
	short		c_asccsr;	/* copy of asc csr */
	short		c_ascflags;	/* initial asc flags */
	short		c_ascsrt;	/* sampling rate */
	short		c_ascseq[8];
	int		c_flags;	/* internal flags */
	int		c_errs;		/* errors, returned via ioctl */
	int		c_bufno;	/* dsubinfo/buffer */
	int		c_outbufno;
	int		c_uid;		/* user id */
	short		c_pid;		/* process id */
	struct proc	*c_procp;	/* process structure pointer */
	int		c_ubinfo[NDSB];	/* uba info */
	struct dspageinfo	c_pageinfo[NDSB];
	struct buf	c_dsb[NDSB];	/* buffer list */
	struct buf	*c_ibp, *c_obp;	/* buffer list pointers */
	int		c_nbytes;	/* total # of bytes xferred since reset */
	int		c_to_idle;	/* total # times idle flag set */
	int		c_to_active;	/* total # times idle flag cleared */
} ds_softc[NDS];

int dsprobe(), dsattach(), dsintr();

struct uba_device *dsdinfo[NDS];
struct uba_ctlr dsctlr[NDS], *dscinfo[NDS];	/* dsattach() sets dscinfo to point to this */

u_short dsstd[] = {
	0165400, 0
};

struct uba_driver dsdriver = {
	dsprobe, 0, dsattach, 0, dsstd, "ds", dsdinfo, "ds", dscinfo
};
int	dsdebug;

/*
 * all this just to generate
 * an interrupt; rather
 * involved isn't it?
 */
dsprobe(reg)
caddr_t reg;
{
	register int br, cvec;		/* value-result */
	register struct dsdevice *dsaddr;
	int dummy;

# ifdef lint	
	br = 0; cvec = br; br = cvec;
# endif lint

	dsaddr = (struct dsdevice *) reg;

	dsaddr->dmacsr = 0;
	dsaddr->dmacls = 0;
	dsaddr->ascseq[0] = DABASE+0 | ((DA & 01) << 6);
	dsaddr->ascseq[0] |= LAST_SEQ;	/* last sequence register */
	dsaddr->ascsrt = 0100;
	dsaddr->dmacsr = DMA_IE;
	dsaddr->asccsr = ASC_RUN | ASC_IE;

	DELAY(40000);
	/*
	 * now shut everything down.
	 * too bad we have to duplicate
	 * the code from dsinit but to
	 * call dsinit we need to give
	 * it a unit.
	 */

	dsaddr->dmacls = 0;
	dummy = dsaddr->dmasar;			/* clears sar flag */
	dsaddr->dmaclr = 0;

	dummy = dsaddr->ascrst;
	dsaddr->ascrst = 0; 

	return(1);
}

dsattach(ui)
struct uba_device *ui;
{
	register struct uba_ctlr *um;
	register int unit;
	register int i;

	unit = ui->ui_unit;

	um = &dsctlr[unit];
	dscinfo[unit] = um;
	ui->ui_ctlr = unit;
	ui->ui_mi = um;
	um->um_driver = ui->ui_driver;
	um->um_ctlr = unit;
	um->um_ubanum = ui->ui_ubanum;
	um->um_alive = 1;
	um->um_intr = ui->ui_intr;
	um->um_addr = ui->ui_addr;
	um->um_hd = ui->ui_hd;
	ds_softc[unit].c_flags = DS_IDLE;
	for(i=0;i<NDSB;i++)
		ds_softc[unit].c_dsb[i].b_flags &= ~B_BUSY;
}

/* ARGSUSED */
dsopen(dev, flag)
dev_t dev;
{
	register struct dsdevice *dsaddr;
	register struct uba_device *ui;
	register struct ds_softc *sc;
	register int unit, i;
	register struct buf *bp;
	int dummy;

	if ((unit = (minor(dev) & ~RESETDEV)) >= NDS)
		goto bad;

	if ((ui = dsdinfo[unit]) == NULL)
		goto bad;

	if (ui->ui_alive == 0)
		goto bad;

	sc = &ds_softc[ui->ui_unit];
	dsaddr = (struct dsdevice *)ui->ui_addr;

	if(dsdebug)printf("dsopen: unit %d, flag %x\n",unit,flag);
	if (dsaddr->dmacsr & DMA_OFL) {
		u.u_error = ENXIO;
		return;
	}

	/*
	 * if this is the reset device
	 * then just do a reset and return.
	 */
	if (minor(dev) & RESETDEV) {
		/*
		 * if the converters are in use then
		 * only the current user or root can
		 * do a reset.
		 */
		if (sc->c_flags & DS_OPEN) {
			if ((sc->c_uid != u.u_ruid) && (u.u_uid != 0)) {
				u.u_error = ENXIO;
				return;
			}
		}
		ds_softc[unit].c_flags = DS_CLOSED | DS_IDLE;
		dscleanup(unit);
		printf("ds%d: reset\n",unit);
		return;
	}

	/*
	 * only one person can use it at a time
	 * and it can't be opened for both reading and writing
	 */
	if (sc->c_flags & DS_OPEN || (flag & (FREAD|FWRITE)) == (FREAD|FWRITE)) {
bad:		u.u_error = ENXIO;
		return;
	}

	/*
	 * initialize
	 */


	/* set defaults and initial conditions in ds_softc */
	sc->c_flags = DS_IDLE;
	sc->c_errs = 0;

	sc->c_nbytes = sc->c_to_idle = sc->c_to_active = 0;

	sc->c_ascflags = (flag & FWRITE ? ASC_PLAY : ASC_RECORD) | ASC_HZ04;
	sc->c_ascseq[0] = (
		(flag & FWRITE) ?
			(DABASE+0 | (DA << 6)) :  (ADBASE+0 | (AD << 6))
						) | LAST_SEQ;
	sc->c_ascsrt = 399;


	sc->c_uid = u.u_ruid;
	sc->c_pid = u.u_procp->p_pid;
	sc->c_procp = u.u_procp;
	if(dsdebug)printf("pid %d %d, flag %x stat %x\n",
		sc->c_pid,sc->c_procp->p_pid,sc->c_procp->p_stat);
	sc->c_flags |= DS_OPEN | (flag & FREAD ? DS_READ : DS_WRITE);
	u.u_procp->p_flag |= SPHYSIO;
	dsaddr->dmacsr = 0;
	dummy = dsaddr->dmasar;
}

/* ARGSUSED */
dsclose(dev, flag) {
	register int unit;
	register struct dsdevice *dsaddr;
	register struct uba_device *ui;
	register struct ds_softc *sc;

	unit = minor(dev) & ~RESETDEV;
	sc = &ds_softc[unit];
	ui = dsdinfo[unit];
	dsaddr = (struct dsdevice *)ui->ui_addr;
	if(dsdebug)printf("X");
	if (sc->c_outbufno != sc->c_bufno && 
		sc->c_errs == 0 && !(sc->c_flags & DS_IDLE))
		tsleep((caddr_t)&ds_softc[unit], DSPRI,0);
	dsaddr->asccsr = 0;
	ds_softc[unit].c_flags = DS_CLOSED | DS_IDLE;
	dscleanup(unit);
	u.u_procp->p_flag &= ~ SPHYSIO;
	if(dsdebug){
		printf("to_active %d, to_idle %d, c_nbytes %d\n",
		sc->c_to_active,
		sc->c_to_idle,
		sc->c_nbytes);
		printf("c_ascseq[0] %x\n",sc->c_ascseq[0]);
		dsdb(dsaddr);
	}
}

/*
 *
 * Using u.u_count, u.u_base, each buffer header is set up.
 * The converters only need the base address of the buffer and the word
 * count.
 *
 */
dsstart(dev, rw)
register dev_t dev;
{
	register struct dsdevice *dsaddr;
	register struct uba_device *ui;
	register struct ds_softc *sc;
	register struct buf *bp;
	register int	unit, c, bufno, ps;
	int dummy,i, count;
	caddr_t base;
	int bits;

	if(dsdebug)printf("S");

	unit = minor(dev) & ~RESETDEV;
	sc = &ds_softc[unit];
	ui = dsdinfo[unit];
	dsaddr = (struct dsdevice *)ui->ui_addr;

	/*
	 * check user access rights to buffer
	 */
	if (useracc(u.u_base, u.u_count, rw==B_READ?B_WRITE:B_READ) == NULL) {
		u.u_error = EFAULT;
		return;
	}

	if (sc->c_errs) {
		u.u_error = EIO;
		return;
	}


	/*
	 * Get a buffer for each 64K block
	 * point each device buffer somewhere into
	 * the user's buffer
	 */
	/* NOTE that in order to get reasonable performance,
	 * buffer lengths (initial u.u_count) must be either
	 * less than 64K or sufficiently greater than a multiple
	 * of 64K.
	 */
	base = u.u_base;
	count = u.u_count;
	while(count > 0) {

		if(dsdebug)printf("LT ");

		/*
		 * no hardware buffers left, sleep till interrupt
		 * wakes us up
		 */

		
		ps = spl6();
		if ((sc->c_dmacsr = dsaddr->dmacsr) & DMA_SFL) {
			register	ts;
			if(dsdebug)printf("s");
			ts = tsleep((caddr_t) &ds_softc[unit], DSPRI, 0);
			if(dsdebug)printf("w");
			switch(ts) {
				case TS_SIG:
					u.u_error = EINTR;
					/* stop dsc, if active */
					dsaddr->asccsr = dummy = 0;
					dscleanup(unit);
					return;
				case TS_OK:
					if ((sc->c_dmacsr = dsaddr->dmacsr) & DMA_SFL) {
						u.u_error = EIO;
						printf("ds%d  still full after tsleep\n",unit);
						dscleanup(unit);
						return; 
					}
			/* I don't think this allows for a clean dsclose()
				in all cases */
					if((sc->c_flags & DS_CLOSED) ||
					  !(sc->c_flags & DS_OPEN)){
						if(dsdebug)
					  printf("ds: wakeup unopen\n");
						u.u_error = EIO;
						dscleanup(unit);
						return;
					}
					break;
				default:
					printf("ds: %x from tsleep\n",ts);
					dscleanup(unit);
					u.u_error = EIO;
					return;
			}
		}

		splx(ps);


		ps = spl6();

	/*
	 *	If device is IDLE (initial read/write, or datalate condition
	 *	  has been detected in dsintr()), (re)initialize dsc
	 */

	if (sc->c_flags & DS_IDLE) {

		if(dsdebug)
			printf("I1 ");
		dsaddr->asccsr = sc->c_asccsr = sc->c_ascflags;
		dummy = dsaddr->ascrst;
		dsaddr->asccsr = (sc->c_asccsr |= ASC_IE);
		dsaddr->ascsrt = sc->c_ascsrt;
		{register	j, i = 0;
			do {
				dsaddr->ascseq[i] = sc->c_ascseq[i];
				j = i++;
			} while(!(sc->c_ascseq[j] & LAST_SEQ) && i<8);
		}
		dsaddr->dmacsr = sc->c_dmacsr = 0;
		dsaddr->dmacls = 0;
		dummy = dsaddr->dmasar;			/* clears sfl flag */
		dsaddr->dmawc = -1;
		dsaddr->dmacsr = sc->c_dmacsr = (DMA_IE | DMA_CHN
				| (rw == B_READ ? DMA_W2M : 0) );

		sc->c_bufno = sc->c_outbufno = 0;
		sc->c_ibp = sc->c_obp = &sc->c_dsb[0];	/* correct?  */

	}


		/* get next device buffer and set it up */
		bp = sc->c_ibp++;
		if (sc->c_ibp > &sc->c_dsb[NDSB-1])
			sc->c_ibp = &sc->c_dsb[0];

		/* hopefully  redundant check against going to fast */
		if(bp->b_flags & B_BUSY){
			printf("dsstart: about to set up BUSY buffer!\n");
			u.u_error = EIO;
			return;
		}


		c = MIN(count, 1024*64);
		bp->b_un.b_addr = base;
		bp->b_error = 0;
		bp->b_proc = u.u_procp;
		bp->b_dev = dev;
		bp->b_blkno = sc->c_bufno;
		bp->b_bcount = c;
		bufno = sc->c_bufno % NDSB;

		/* the page computations should be relative to param.h defs */

		sc->c_pageinfo[bufno].firstunit = (int)base >> 10;
		sc->c_pageinfo[bufno].lastunit=(((int)base+c+1023)>>10)-1;

		for(i=0;i<NDSB;i++)
			if((sc->c_dsb[i].b_flags & B_BUSY)
				&&
			(!(sc->c_pageinfo[bufno].firstunit >
				sc->c_pageinfo[i].lastunit)
				&&
			 !(sc->c_pageinfo[bufno].lastunit <
				sc->c_pageinfo[i].firstunit))) {
				printf("ds: buf %d overlaps %d\n",i,bufno);
				u.u_error=EIO;
				return;
			}

		/* lock the buffer and send it off */

		bp->b_flags = B_BUSY | B_PHYS  | rw;
		if(dsdebug)printf("lk%d %x %d ",bufno,base,c);
		dslock(base, (int) c);	/* fasten seat belts */

		sc->c_ubinfo[bufno] = ubasetup(ui->ui_ubanum, bp, UBA_WANTBDP);

		dsaddr->dmablr = ~ (c >> 1);
		dsaddr->dmasax = (sc->c_ubinfo[bufno] >> 16) & 03;
		dsaddr->dmasar = sc->c_ubinfo[bufno];
			
		sc->c_bufno++;

		/*
		 * start the dsc, if necessary
		 */
		if(sc->c_flags & DS_IDLE){
			if(dsdebug)printf("I2 ");
			sc->c_flags &= ~DS_IDLE;
			++sc->c_to_active;
			dsaddr->asccsr = (sc->c_asccsr |= ASC_RUN);
		}

		splx(ps);

		base += c;
		count -= c;
		if(dsdebug)printf("LB%x",u.u_error);
	}
		
	u.u_count = count;
}
dsdb(dsaddr)
register struct dsdevice *dsaddr;
{
	printf("dmacsr %b\n", dsaddr->dmacsr, DMA_BITS);
	printf("asccsr %b\n", dsaddr->asccsr, ASC_BITS);
	printf("dmawc %o\t", dsaddr->dmawc);
	printf("dmablr %o\n", dsaddr->dmablr);
	printf("dmaac %o\t", dsaddr->dmaac);
	printf("dmasar %o\n", dsaddr->dmasar);
	printf("dmaacx %o\t", dsaddr->dmaacx);
	printf("dmasax %o\n", dsaddr->dmasax);
	printf("ascsrt(decimal) %d\n", dsaddr->ascsrt);
	printf("ascseq[0] %o\n", dsaddr->ascseq[0]);
}
/*
 * this is where the real work is done. we copy any device registers that
 * we will be looking at to decrease the amount of traffic on the ds 200
 * bus.
 *
 */
dsintr(dev)
dev_t dev;
{
	register struct dsdevice *dsaddr;
	register struct uba_device *ui;
	register struct ds_softc *sc;
	register struct buf *bp;
	register int bufno;
	register int i;
	int unit, dummy;

	unit = minor(dev) & ~RESETDEV;
	sc = &ds_softc[unit];
	ui = dsdinfo[unit];

	dsaddr = (struct dsdevice *) dsdinfo[unit]->ui_addr;
	
	sc->c_dmacsr = dsaddr->dmacsr;

	dsaddr->dmacls = 0;
	if(dsdebug)printf("i");
	if (sc->c_flags & DS_IDLE) {
		printf("ds%d: interrupt while idle\n", unit);
		return;
	}

	if (sc->c_dmacsr & DMA_ERR) {
		if(dsdebug)printf("e");
		sc->c_asccsr = (sc->c_dmacsr & DMA_XIN ? dsaddr->asccsr : 0);
		dsaddr->asccsr = dummy = 0;

		dsaddr->dmacsr = (sc->c_dmacsr &= ~DMA_CHN);
		if ((sc->c_dmacsr & (DMA_XER|DMA_UBA|DMA_AMPE|DMA_XBA))
				|| (sc->c_asccsr & (ASC_BA|ASC_DCN|ASC_DNP))) {
			++sc->c_errs;
			printf("ds%d error: asccsr=%b dmacsr=%b\n",
				unit, sc->c_asccsr, ASC_BITS,
				sc->c_dmacsr, DMA_BITS);
		} else
			sc->c_nbytes += sc->c_obp->b_bcount;
	/* the above may be wrong, since DLT interrupt seems to occur
	 *	separately from (and later than) DMA completion
	 *	interrupt	*/
		dscleanup(unit);
		return;
	}
		
	/*
	 * get current buffer and release it
	 */

	bp = sc->c_obp++;
	if (sc->c_obp > &sc->c_dsb[NDSB-1])
		sc->c_obp = &sc->c_dsb[0];

	sc->c_outbufno++;
	if(dsdebug)printf("b ");
	dsrelease(sc, ui, bp);

	/*
	 * update byte count.
	 */

	sc->c_nbytes += bp->b_bcount;

	wakeup((caddr_t) &ds_softc[unit]);
}

/*
 * release resources, if any, associated with a buffer
 */
dsrelease(sc, ui, bp)
register struct ds_softc *sc;
struct uba_device *ui;
register struct buf *bp;
{
	register int	ps;
	register struct pte *pte;
	register int npf;
	int	bufno;

	bufno = bp->b_blkno % NDSB;
	if(dsdebug)printf("R%d ",bufno);
	/*
	 * release uba resources
	 * and memory resources if process seems healthy
	 */
	ps = spl6();

	if (bp->b_flags & B_BUSY) {
		if(dsdebug)printf("%x %d ",bp->b_un.b_addr,bp->b_bcount);
		ubarelse(ui->ui_ubanum, &sc->c_ubinfo[bufno]);

		/* we can't call vsunlock() because u.u_procp may be
			different */
	if( !(sc->c_pid == sc->c_procp->p_pid) ||
	    (sc->c_procp->p_flag & SWEXIT) ||
		(sc->c_procp->p_stat == SZOMB)){
			   /* process is dying or dead */
			/* exit() will release memory (does it unlock???) */
			/* but will not release uba resources */
		printf(" REL pid %d %d flag %x stat %x ",sc->c_pid,
				sc->c_procp->p_pid,sc->c_procp->p_flag,
					sc->c_procp->p_stat);
		sc->c_flags |= DS_CLOSED;
	}
	else {
		pte = vtopte(sc->c_procp,btop(bp->b_un.b_addr));
		npf = btoc(bp->b_bcount + ((int)(bp->b_un.b_addr)&CLOFSET));
		if(dsdebug)printf("U%x %d ",pte->pg_pfnum,npf);
		while(npf > 0) {
			munlock(pte->pg_pfnum);
			if(sc->c_ascflags & ASC_RECORD)
				pte->pg_m = 1;	/* memory written */
			pte += CLSIZE;
			npf -= CLSIZE;
		}
	}

		bp->b_flags = 0;  /* better than resetting B_BUSY */
	}

	splx(ps);
}

/*
 * release all buffers and do general cleanup
 */
dscleanup(unit)
int unit;
{
	register struct dsdevice *dsaddr;
	register struct uba_device *ui;
	register struct ds_softc *sc;
	register struct buf *bp;
	int	dummy;

	unit = (unit & ~RESETDEV);
	sc = &ds_softc[unit];
	ui = dsdinfo[unit];
	dsaddr = (struct dsdevice *) dsdinfo[unit]->ui_addr;
	
	if(dsdebug)printf("C ");
	if (!(sc->c_flags & DS_IDLE))
		++sc->c_to_idle;
	sc->c_flags |= DS_IDLE;

	dsaddr->asccsr = dummy = 0;
	dummy = dsaddr->ascrst;
	dsaddr->dmacsr = dummy = 0;
	dummy = dsaddr->dmasar;

	for (bp = &(sc->c_dsb[0]); bp < &(sc->c_dsb[NDSB]); ++bp)
		dsrelease(sc, ui, bp);


	/* if called from dsintr while dsstart is sleeping */

	wakeup((caddr_t) &ds_softc[unit]);
}

/*
 * a/d conversion
 */
dsread(dev)
dev_t dev;
{
	dsstart(dev, B_READ);		/* writes on disk */
}

/*
 * d/a conversion
 */
dswrite(dev)
dev_t dev;
{
	dsstart(dev, B_WRITE);		/* reads from disk */
}

/* ARGSUSED */
dsioctl(dev, cmd, addr, flag)
dev_t dev;
caddr_t addr;
{
	register struct dsdevice *dsaddr;
	register struct ds_softc *sc;
	register struct ds_seq *dq;
	register struct ds_err *de;
	register struct ds_trans *dt;
	struct uba_device *ui;
	struct ds_seq ds_seq;
	struct ds_err ds_err;
	struct ds_trans ds_trans;
	int unit;
	int flts;
	int i;
	int iarg[2];

	if (minor(dev) & RESETDEV) {
		u.u_error = EINVAL;
		return;
	}
	unit = minor(dev) & ~RESETDEV;

	sc = &ds_softc[unit];
	ui = dsdinfo[unit];
	dsaddr = (struct dsdevice *) ui->ui_addr;

	switch (cmd) {
		/* set sample rate */
		case DSRATE:
			
			if (copyin(addr, (caddr_t) iarg, (unsigned) sizeof(int))) {
				u.u_error = EFAULT;
				return;
			}
			sc->c_ascsrt = 4000000/iarg[0] - 1;
			break;

		case DS08KHZ:
			sc->c_ascflags &= ~ ASC_HZMSK;
			sc->c_ascflags |= ASC_HZ08;	/* set 8kHz filter */
			break;

		case DS04KHZ:
			sc->c_ascflags &= ~ ASC_HZMSK;
			sc->c_ascflags |= ASC_HZ04;	/* set 4kHz filter */
			break;



		case DSBYPAS:
			sc->c_ascflags &= ~ ASC_HZMSK;
			sc->c_ascflags |= ASC_BYPASS;	/* set bypass */
			break;

		/* fetch errors */
		case DSERRS:
			de = &ds_err;
			de->dma_csr = sc->c_dmacsr;
			de->asc_csr = sc->c_asccsr;
			de->errors = sc->c_errs;
			if (copyout((caddr_t) de, addr, (unsigned) sizeof(struct ds_err))) {
				u.u_error = EFAULT;
				return;
			}
			break;

		/* fetch transition counts */
		case DSTRANS:
			dt = &ds_trans;
			dt->to_idle = sc->c_to_idle;
			dt->to_active = sc->c_to_active;
			if (copyout((caddr_t) dt, addr, sizeof(struct ds_trans))) {
				u.u_error = EFAULT;
				return;
			}
			break;

		/* how many samples actually converted */
		case DSDONE:
			if (copyout((caddr_t) sc->c_nbytes, addr, (unsigned) sizeof(int))) {
				u.u_error = EFAULT;
				return;
			}
			break;

		case DSSTEREO:
			while(!sc->c_flags & DS_IDLE);
			if (sc->c_flags & DS_READ) {
				dsseq(sc, 0, ADBASE, AD);
				dsseq(sc, 1, ADBASE+1 | LAST_SEQ, AD);
			} else {
				dsseq(sc, 0, DABASE, DA);
				dsseq(sc, 1, DABASE+1 | LAST_SEQ, DA);
			}
			break;

		case DSMONO:
			while(!sc->c_flags & DS_IDLE);
			if (sc->c_flags & DS_READ)
				dsseq(sc, 0, ADBASE | LAST_SEQ, AD);
			else
				dsseq(sc, 0, DABASE | LAST_SEQ, DA);
			break;

		case DSRESET:
				/* not adequate */
			sc->c_flags = DS_CLOSED | DS_IDLE;
			dscleanup(unit);
			break;

		case DSDEBUG:
 			dsdebug = 1 - dsdebug;
			break;
		case DSWAIT:
			while(sc->c_outbufno != sc->c_bufno
				&& !(sc->c_flags & DS_IDLE))
				tsleep((caddr_t)&ds_softc[unit],DSPRI,0);
			break;

		default:
			u.u_error = ENOTTY;
			break;
	}
}


/*
 * zero uba vector.
 * shut off converters.
 * set error bit.
 */
dsreset(uban) {
	register struct uba_device *ui;
	register struct ds_softc *sc;
	register int ds;

	for (ds = 0; ds < NDS; ds++) {
		if ((ui = dsdinfo[ds]) == NULL)
			continue;
		if (ui->ui_alive == 0)
			continue;
		if (ui->ui_ubanum != uban)
			continue;

		printf(" ds%d", ds);

		sc = &ds_softc[ds];

		/*
		 * release unibus resources
		 */
		sc->c_flags = DS_CLOSED | DS_IDLE;
		dscleanup(ds);
	}
}
/*
 equivalent to vslock() from vmmem.c
  inserted differently here for debugging convenience only.
 */
dslock(base, count)
	caddr_t base;
{
	register unsigned v;
	register int npf;
	register struct pte *pte;

	u.u_procp->p_flag |= SDLYU;
	v = btop(base);
	pte = vtopte(u.u_procp, v);
	npf = btoc(count + ((int)base & CLOFSET));
	if(dsdebug)printf("Lk %x %d ",pte->pg_pfnum,npf);
	while (npf > 0) {
		if (pte->pg_v) 
			mlock(pte->pg_pfnum);
		else
			if (fubyte((caddr_t)ctob(v)) < 0)
				panic("vslock");
		pte += CLSIZE;
		v += CLSIZE;
		npf -= CLSIZE;
	}
	u.u_procp->p_flag &= ~SDLYU;
}
# endif

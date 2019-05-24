/*	hp.c	4.47	82/02/18	*/

#ifdef HPDEBUG
int	hpdebug;
#endif
#ifdef HPBDEBUG
int	hpbdebug;
#endif

#include "hp.h"
#if NHP > 0
/*
 * HP disk driver for RP0x+RMxx+ML11
 *
 * TODO:
 *	check RM80 skip sector handling when ECC's occur later
 *	check offset recovery handling
 *	see if DCLR and/or RELEASE set attention status
 *	print bits of mr && mr2 symbolically
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dk.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/map.h"
#include "../h/pte.h"
#include "../h/mbareg.h"
#include "../h/mbavar.h"
#include "../h/mtpr.h"
#include "../h/vm.h"
#include "../h/cmap.h"
#include "../h/dkbad.h"
#include "../h/dkio.h"

#include "../h/hpreg.h"

/* THIS SHOULD BE READ OFF THE PACK, PER DRIVE */
struct	size {
	daddr_t	nblocks;
	int	cyloff;
} hp6_sizes[8] = {
	15884,	0,		/* A=cyl 0 thru 37 */
	33440,	38,		/* B=cyl 38 thru 117 */
	340670,	0,		/* C=cyl 0 thru 814 */
	0,	0,
	0,	0,
	0,	0,
#ifndef NOBADSECT
	291280,	118,		/* G=cyl 118 thru 814 */
#else
	291346,	118,
#endif
	0,	0,
}, rm3_sizes[8] = {
	15884,	0,		/* A=cyl 0 thru 99 */
	33440,	100,		/* B=cyl 100 thru 309 */
	131680,	0,		/* C=cyl 0 thru 822 */
	0,	0,
	0,	0,
	0,	0,
#ifndef NOBADSECT
	81984,	310,		/* G=cyl 310 thru 822 */
#else
	82080,	310,
#endif
	0,	0,
}, rm5_sizes[8] = {
	15884,	0,		/* A=cyl 0 thru 26 */
	33440,	27,		/* B=cyl 27 thru 81 */
	500384,	0,		/* C=cyl 0 thru 822 */
	15884,	562,		/* D=cyl 562 thru 588 */
	55936,	589,		/* E=cyl 589 thru 680 */
#ifndef NOBADSECT
	86240,	681,		/* F=cyl 681 thru 822 */
	158592,	562,		/* G=cyl 562 thru 822 */
#else
	86636,	681,
	158688,	562,
#endif
	291346,	82,		/* H=cyl 82 thru 561 */
}, rm80_sizes[8] = {
	15884,	0,		/* A=cyl 0 thru 36 */
	33440,	37,		/* B=cyl 37 thru 114 */
	242606,	0,		/* C=cyl 0 thru 558 */
	0,	0,
	0,	0,
	192223,	115,		/* F=cyl 115 thru 558 */
	82080,	115,		/* G=cyl 115 thru 304 */
	110143,	305,		/* H=cyl 305 thru 558 */
}, mfj_sizes[8] = {
	10240,	0,		/* A=cyl 0 thru 31 */
	20480,	32,		/* B=cyl 32 thru 95 */
	232640,	96,		/* C=cyl 96 thru 822 */
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0,
}, eagle_sizes[8] ={
	27520,	0,		/* A=cyl 0 thru 31 */
	27520,	32,		/* B=cyl 32 thru 63 */
	667360,	64,		/* C=cyl 64 thru 839 */
	232640,	64,		/* D=cyl 64 thru 340 (partial) */
	232640,	341,		/* E=cyl 341 thru 617 (partial) */
	0,	0,
	0,	0,
	0,	0,
/* 48 sector Emulex Eagle */
}, eag48_sizes[8] = {	/* cyl 841 used for bad sectors + info */

	45*960,   0,	/* A = cyl 0 thru 44		for /tmp */
	64*960,  45,	/* B = cyl 45 thru 108		for swap */
	842*960,  0,	/* C = all cyl 0 thru 841	for testing */
	244*960,109,	/* D = cyl 109 thru 352		nearly 1/3 */
	244*960,353,	/* E = cyl 353 thru 596		nearly 1/3 */
	244*960,597,	/* F = cyl 597 thru 840		nearly 1/3 */
	0,	0,	/* G = cyl 0 thru 402		nearly 1/2 */
	0, 	0,	/* H = cyl 438 thru 840		nearly 1/2 */
}, hp7_sizes[8] = {
	15884,	0,		/* A=cyl 0 thru 9 */
	64000,	10,		/* B=cyl 10 thru 49 */
	1008000,0,		/* C=cyl 0 thru 629 */
	504000, 0,		/* D=cyl 0 thru 314 */
	504000, 315,		/* E=cyl 315 thru 629 */
	928000,	50,		/* F=cyl 50 thru 629 */
	0,	0,
	0,	0,
};
/* END OF STUFF WHICH SHOULD BE READ IN PER DISK */

#ifndef JONES
#define	_hpSDIST	2
#define	_hpRDIST	3
#else JONES
/*
	these constants were originally 2 and 3 respectively.  they
	have been changed to improve the performance of the eagle
	drives with the emulex controller.  to date, 8,9 is better than
	2,3.  12,13 did not do any better than 8,9.  changing these
	constants changes the lead for all drives using the hp driver
	including the rm80.  that's a mistake, but empirically,
	changing these to 8,9 did not measurably change rm80
	performance.
*/
#define	_hpSDIST	8
#define	_hpRDIST	9
#endif JONES

int	hpSDIST = _hpSDIST;
int	hpRDIST = _hpRDIST;

short	hptypes[] =
	{ MBDT_RM03, MBDT_RM05, MBDT_RP06, MBDT_RM80, MBDT_RP05, MBDT_RP07,
	  MBDT_FUJI, MBDT_FUJI, MBDT_FUJI, MBDT_ML11A, MBDT_ML11B, 0 };
struct	mba_device *hpinfo[NHP];
int	hpattach(),hpustart(),hpstart(),hpdtint();
struct	mba_driver hpdriver =
	{ hpattach, 0, hpustart, hpstart, hpdtint, 0,
	  hptypes, "hp", 0, hpinfo };

struct hpst {
	short	nsect;
	short	ntrak;
	short	nspc;
	short	ncyl;
	struct	size *sizes;
} hpst[] = {
	32,	5,	32*5,	823,	rm3_sizes,	/* RM03 */
	32,	19,	32*19,	823,	rm5_sizes,	/* RM05 */
	22,	19,	22*19,	815,	hp6_sizes,	/* RP06 */
	31,	14, 	31*14,	559,	rm80_sizes,	/* RM80 */
	22,	19,	22*19,	411,	hp6_sizes,	/* RP05 */
	50,	32,	50*32,	630,	hp7_sizes,	/* RP07 */
	/* the following three entries must be contiguous */
	32,	10,	32*10,	823,	mfj_sizes,	/* little fujitsu */
	43,	20,	43*20,	842,	eagle_sizes,	/* eagle */
	48,	20,	48*20,	842,	eag48_sizes,	/* eagle 48 sectors */
	1,	1,	1,	1,	0,		/* ML11A */
	1,	1,	1,	1,	0,		/* ML11B */
};

u_char	hp_offset[16] = {
    HPOF_P400, HPOF_M400, HPOF_P400, HPOF_M400,
    HPOF_P800, HPOF_M800, HPOF_P800, HPOF_M800,
    HPOF_P1200, HPOF_M1200, HPOF_P1200, HPOF_M1200,
    0, 0, 0, 0,
};

struct	buf	rhpbuf[NHP];
#ifndef NOBADSECT
struct	buf	bhpbuf[NHP];
struct	dkbad	hpbad[NHP];
#endif
char	hpinit[NHP];
char	hprecal[NHP];
char	hphdr[NHP];
daddr_t	mlsize[NHP];

#define	b_cylin b_resid
 
/* #define ML11 0  to remove ML11 support */
#define ML11	0
/*#define	ML11	(hptypes[mi->mi_type] == MBDT_ML11A)*/
#define	RP06	(hptypes[mi->mi_type] <= MBDT_RP06)
#define	RM80	(hptypes[mi->mi_type] == MBDT_RM80)

#ifdef INTRLVE
daddr_t dkblock();
#endif
 
int	hpseek;

/*ARGSUSED*/
hpattach(mi, slave)
	struct mba_device *mi;
{
	register struct hpst *st = &hpst[mi->mi_type];

	if (hptypes[mi->mi_type] == MBDT_ML11B)
		mi->mi_type--;	/* A CHEAT - ML11B D.T. SHOULD == ML11A */
	if (ML11) {
		register struct hpdevice *hpaddr =
			(struct hpdevice *)mi->mi_drv;
		register int trt, sz;

		sz = hpaddr->hpmr & HPMR_SZ;
		if ((hpaddr->hpmr & HPMR_ARRTYP) == 0)
			sz >>= 2;
		mlsize[mi->mi_unit] = sz;
		if (mi->mi_dk >= 0) {
			trt = (hpaddr->hpmr & HPMR_TRT) >> 8;
			dk_mspw[mi->mi_dk] = 1.0 / (1<<(20-trt));
		}
	} else if (mi->mi_dk >= 0) {
		register struct hpdevice *hp;
		register int type;

		hp = (struct hpdevice *) mi->mi_drv;
		type = hp->hpdt & MBDT_TYPE;
		if (type == MBDT_FUJI) {
			short maxtrk, maxsect;
			/*
			 *	we have some kind of disk on an Emulex SC750.
			 *	We assume it's a little Fuji, and must now check
			 *	if it's an Eagle.
			 */
			hp->hphr = HPHR_MAXTRK;
			maxtrk = hp->hphr;
			if (maxtrk >= hpst[mi->mi_type].ntrak) {
				mi->mi_type++;
				/* it's an Eagle. now check # of sectors */
				hp->hphr = HPHR_MAXSECT;
				maxsect = hp->hphr;
				if (maxsect > hpst[mi->mi_type].nsect) {
					printf("%d sectors\n", maxsect);
					mi->mi_type++;
				}
			}
		}
		st = &hpst[mi->mi_type];
		dk_mspw[mi->mi_dk] = 1.0 / 60 / (st->nsect * 256);
	}
}

hpstrategy(bp)
	register struct buf *bp;
{
	register struct mba_device *mi;
	register struct hpst *st;
	register int unit;
	long sz, bn;
	int xunit = minor(bp->b_dev) & 07;
	int s;

	sz = bp->b_bcount;
	sz = (sz+511) >> 9;
	unit = dkunit(bp);
	if (unit >= NHP)
		goto bad;
	mi = hpinfo[unit];
	if (mi == 0 || mi->mi_alive == 0)
		goto bad;
	st = &hpst[mi->mi_type];
	if (ML11) {
		if (bp->b_blkno < 0 || dkblock(bp)+sz > mlsize[mi->mi_unit])
			goto bad;
		bp->b_cylin = 0;
	} else {
		if (bp->b_blkno < 0 ||
			(bn = dkblock(bp))+sz > st->sizes[xunit].nblocks)
			goto bad;
		bp->b_cylin = bn/st->nspc + st->sizes[xunit].cyloff;
	}
	s = spl5();
	disksort(&mi->mi_tab, bp);
	if (mi->mi_tab.b_active == 0)
		mbustart(mi);
	splx(s);
	return;

bad:
	bp->b_flags |= B_ERROR;
	iodone(bp);
	return;
}

hpustart(mi)
	register struct mba_device *mi;
{
	register struct hpdevice *hpaddr = (struct hpdevice *)mi->mi_drv;
	register struct buf *bp = mi->mi_tab.b_actf;
	register struct hpst *st = &hpst[mi->mi_type];
	daddr_t bn;
	int sn, dist;

	hpaddr->hpcs1 = 0;
	if ((hpaddr->hpcs1&HP_DVA) == 0)
		return (MBU_BUSY);
	if ((hpaddr->hpds & HPDS_VV) == 0 || hpinit[mi->mi_unit] == 0) {
#ifndef NOBADSECT
		struct buf *bbp = &bhpbuf[mi->mi_unit];
#endif

		hpinit[mi->mi_unit] = 1;
		hpaddr->hpcs1 = HP_DCLR|HP_GO;
		if (mi->mi_mba->mba_drv[0].mbd_as & (1<<mi->mi_drive))
			printf("DCLR attn\n");
		hpaddr->hpcs1 = HP_PRESET|HP_GO;
		if (!ML11)
			hpaddr->hpof = HPOF_FMT22;
		mbclrattn(mi);
#ifndef NOBADSECT
		if (!ML11) {
			bbp->b_flags = B_READ|B_BUSY;
			bbp->b_dev = bp->b_dev;
			bbp->b_bcount = 512;
			bbp->b_un.b_addr = (caddr_t)&hpbad[mi->mi_unit];
			bbp->b_blkno = st->ncyl*st->nspc - st->nsect;
			bbp->b_cylin = st->ncyl - 1;
			mi->mi_tab.b_actf = bbp;
			bbp->av_forw = bp;
			bp = bbp;
		}
#endif
	}
	if (mi->mi_tab.b_active || mi->mi_hd->mh_ndrive == 1)
		return (MBU_DODATA);
	if (ML11)
		return (MBU_DODATA);
	if ((hpaddr->hpds & HPDS_DREADY) != HPDS_DREADY)
		return (MBU_DODATA);
	bn = dkblock(bp);
	sn = bn%st->nspc;
	sn = (sn+st->nsect-hpSDIST)%st->nsect;
	if (bp->b_cylin == (hpaddr->hpdc & 0xffff)) {
		if (hpseek)
			return (MBU_DODATA);
		dist = ((hpaddr->hpla & 0xffff)>>6) - st->nsect + 1;
		if (dist < 0)
			dist += st->nsect;
		if (dist > st->nsect - hpRDIST)
			return (MBU_DODATA);
	} else
		hpaddr->hpdc = bp->b_cylin;
	if (hpseek)
		hpaddr->hpcs1 = HP_SEEK|HP_GO;
	else {
		hpaddr->hpda = sn;
		hpaddr->hpcs1 = HP_SEARCH|HP_GO;
	}
	return (MBU_STARTED);
}

hpstart(mi)
	register struct mba_device *mi;
{
	register struct hpdevice *hpaddr = (struct hpdevice *)mi->mi_drv;
	register struct buf *bp = mi->mi_tab.b_actf;
	register struct hpst *st = &hpst[mi->mi_type];
	daddr_t bn;
	int sn, tn;

	bn = dkblock(bp);
	if (ML11)
		hpaddr->hpda = bn;
	else {
		sn = bn%st->nspc;
		tn = sn/st->nsect;
		sn %= st->nsect;
		hpaddr->hpdc = bp->b_cylin;
		hpaddr->hpda = (tn << 8) + sn;
	}
	if (hphdr[mi->mi_unit]) {
		if (bp->b_flags & B_READ)
			return (HP_RHDR|HP_GO);
		else
			return (HP_WHDR|HP_GO);
	}
	return (0);
}

hpdtint(mi, mbsr)
	register struct mba_device *mi;
	int mbsr;
{
	register struct hpdevice *hpaddr = (struct hpdevice *)mi->mi_drv;
	register struct buf *bp = mi->mi_tab.b_actf;
	register int er1, er2;
	int retry = 0;

#ifndef NOBADSECT
	if (bp->b_flags&B_BAD) {
		if (hpecc(mi, CONT))
			return(MBD_RESTARTED);
	}
#endif
	if (hpaddr->hpds&HPDS_ERR || mbsr&MBSR_EBITS) {
#ifdef HPDEBUG
		if (hpdebug) {
			int dc = hpaddr->hpdc, da = hpaddr->hpda;
			struct hpst *st = &hpst[mi->mi_type];

			printf("hperr: bp %x cyl %d blk %d as %o ",
				bp, bp->b_cylin, bp->b_blkno,
				hpaddr->hpas&0xff);
			printf("dc %x da %x\n",dc&0xffff, da&0xffff);
			printf("errcnt %d ", mi->mi_tab.b_errcnt);
			printf("mbsr=%b ", mbsr, mbsr_bits);
			printf("er1=%b er2=%b\n",
			    hpaddr->hper1, HPER1_BITS,
			    hpaddr->hper2, HPER2_BITS);
			DELAY(1000000);
		}
#endif
		er1 = hpaddr->hper1;
		er2 = hpaddr->hper2;
		if (er1 & HPER1_HCRC) {
			er1 &= ~(HPER1_HCE|HPER1_FER);
			er2 &= ~HPER2_BSE;
		}
		if (er1&HPER1_WLE) {
			printf("hp%d: write locked\n", dkunit(bp));
			bp->b_flags |= B_ERROR;
		} else if ((er1&0xffff) == HPER1_FER && RP06 &&
		    hphdr[mi->mi_unit] == 0) {
#ifndef NOBADSECT
			if (hpecc(mi, BSE))
			{
#ifdef HPBDEBUG
				if (hpdebug)
					printf("MBD_RESTARTED\n");
#endif
				return(MBD_RESTARTED);
			}
			else
#endif
				goto hard;
		} else if (++mi->mi_tab.b_errcnt > 27 ||
		    mbsr & MBSR_HARD ||
		    er1 & HPER1_HARD ||
		    hphdr[mi->mi_unit] ||
		    (!ML11 && (er2 & HPER2_HARD))) {
hard:
			harderr(bp, "hp");
			if (mbsr & (MBSR_EBITS &~ (MBSR_DTABT|MBSR_MBEXC)))
				printf("mbsr=%b ", mbsr, mbsr_bits);
			printf("er1=%b er2=%b",
			    hpaddr->hper1, HPER1_BITS,
			    hpaddr->hper2, HPER2_BITS);
			if (hpaddr->hpmr)
				printf(" mr=%o", hpaddr->hpmr&0xffff);
			if (hpaddr->hpmr2)
				printf(" mr2=%o", hpaddr->hpmr2&0xffff);
			printf("\n");
			bp->b_flags |= B_ERROR;
			hprecal[mi->mi_unit] = 0;
		} else if ((er2 & HPER2_BSE) && !ML11) {
#ifndef NOBADSECT
			if (hpecc(mi, BSE))
				return(MBD_RESTARTED);
			else
#endif
				goto hard;
		} else if (RM80 && er2&HPER2_SSE) {
			(void) hpecc(mi, SSE);
			return (MBD_RESTARTED);
		} else if ((er1&(HPER1_DCK|HPER1_ECH))==HPER1_DCK) {
			if (hpecc(mi, ECC))
				return (MBD_RESTARTED);
			/* else done */
		} else
			retry = 1;
		hpaddr->hpcs1 = HP_DCLR|HP_GO;
		if (ML11) {
			if (mi->mi_tab.b_errcnt >= 16)
				goto hard;
		} else if ((mi->mi_tab.b_errcnt&07) == 4) {
			hpaddr->hpcs1 = HP_RECAL|HP_GO;
			hprecal[mi->mi_unit] = 1;
			return(MBD_RESTARTED);
		}
		if (retry)
			return (MBD_RETRY);
	}
#ifdef HPDEBUG
	else
		if (hpdebug && hprecal[mi->mi_unit]) {
			printf("recal %d ", hprecal[mi->mi_unit]);
			printf("errcnt %d\n", mi->mi_tab.b_errcnt);
			printf("mbsr=%b ", mbsr, mbsr_bits);
			printf("er1=%b er2=%b\n",
			    hpaddr->hper1, HPER1_BITS,
			    hpaddr->hper2, HPER2_BITS);
		}
#endif
	switch (hprecal[mi->mi_unit]) {

	case 1:
		hpaddr->hpdc = bp->b_cylin;
		hpaddr->hpcs1 = HP_SEEK|HP_GO;
		hprecal[mi->mi_unit]++;
		return (MBD_RESTARTED);
	case 2:
		if (mi->mi_tab.b_errcnt < 16 ||
		    (bp->b_flags & B_READ) == 0)
			goto donerecal;
		hpaddr->hpof = hp_offset[mi->mi_tab.b_errcnt & 017]|HPOF_FMT22;
		hpaddr->hpcs1 = HP_OFFSET|HP_GO;
		hprecal[mi->mi_unit]++;
		return (MBD_RESTARTED);
	donerecal:
	case 3:
		hprecal[mi->mi_unit] = 0;
		return (MBD_RETRY);
	}
	hphdr[mi->mi_unit] = 0;
	bp->b_resid = -(mi->mi_mba->mba_bcr) & 0xffff;
	if (mi->mi_tab.b_errcnt >= 16) {
		/*
		 * This is fast and occurs rarely; we don't
		 * bother with interrupts.
		 */
		hpaddr->hpcs1 = HP_RTC|HP_GO;
		while (hpaddr->hpds & HPDS_PIP)
			;
		mbclrattn(mi);
	}
	if (!ML11) {
		hpaddr->hpof = HPOF_FMT22;
		hpaddr->hpcs1 = HP_RELEASE|HP_GO;
	}
	return (MBD_DONE);
}

hpread(dev)
	dev_t dev;
{
	register int unit = minor(dev) >> 3;

	if (unit >= NHP)
		u.u_error = ENXIO;
	else
		physio(hpstrategy, &rhpbuf[unit], dev, B_READ, minphys);
}

hpwrite(dev)
	dev_t dev;
{
	register int unit = minor(dev) >> 3;

	if (unit >= NHP)
		u.u_error = ENXIO;
	else
		physio(hpstrategy, &rhpbuf[unit], dev, B_WRITE, minphys);
}

/*ARGSUSED*/
hpioctl(dev, cmd, addr, flag)
	dev_t dev;
	int cmd;
	caddr_t addr;
	int flag;
{

	switch (cmd) {
	case DKIOCHDR:	/* do header read/write */
		if (suser())
			hphdr[minor(dev)>>3] = 1;
		return;

	default:
		u.u_error = ENXIO;
	}
}

hpecc(mi, flag)
	register struct mba_device *mi;
	int flag;
{
	register struct mba_regs *mbp = mi->mi_mba;
	register struct hpdevice *rp = (struct hpdevice *)mi->mi_drv;
	register struct buf *bp = mi->mi_tab.b_actf;
	register struct hpst *st = &hpst[mi->mi_type];
	int npf, o;
	int bn, cn, tn, sn;
	int bcr;
	static nerr;

	bcr = mbp->mba_bcr & 0xffff;
	if (bcr)
		bcr |= 0xffff0000;		/* sxt */
#ifndef NOBADSECT
	if (flag == CONT)
		npf = bp->b_error;
	else
#endif
		npf = btop(bcr + bp->b_bcount);
	o = (int)bp->b_un.b_addr & PGOFSET;
	bn = dkblock(bp);
#ifdef HPBDEBUG
	if (hpbdebug)	
	printf("hpecc: mbp->mba_bcr %x bcr %x bp->b_bcount %x bp->b_error %x bn %x npf %x\n",
		mbp->mba_bcr, bcr, bp->b_bcount, bp->b_error, bn, npf);
	if (hpbdebug)
	printf("ber npf: %x\n", ((rp->hpdc & 0xffff) * (st->nspc))
					  + ((((rp->hpda) >> 8) & 0xff) * st->nsect)
				      + ((rp->hpda) & 0xff)
					  - (((st->sizes[minor(bp->b_dev)&07].cyloff) * st->nspc)
					   + 1 + bn));
#endif
	cn = bp->b_cylin;
	sn = bn%(st->nspc) + npf;
	tn = sn/st->nsect;
	sn %= st->nsect;
	cn += tn/st->ntrak;
	tn %= st->ntrak;
	switch (flag) {
	case ECC:
		{
		register int i;
		caddr_t addr;
		struct pte mpte;
		int bit, byte, mask;

		npf--;		/* because block in error is previous block */
		printf("hp%d%c: soft ecc sn%d\n", dkunit(bp),
		    'a'+(minor(bp->b_dev)&07), bp->b_blkno + npf);
		nerr++;
		if ((nerr%100)==0)
			printf("%d soft errors\n", nerr);
		mask = rp->hpec2&0xffff;
		i = (rp->hpec1&0xffff) - 1;		/* -1 makes 0 origin */
		bit = i&07;
		i = (i&~07)>>3;
		byte = i + o;
		while (i < 512 && (int)ptob(npf)+i < bp->b_bcount && bit > -11) {
			mpte = mbp->mba_map[npf+btop(byte)];
			addr = ptob(mpte.pg_pfnum) + (byte & PGOFSET);
			putmemc(addr, getmemc(addr)^(mask<<bit));
			byte++;
			i++;
			bit -= 8;
		}
		if (bcr == 0)
			return (0);
		npf++;
		break;
		}

	case SSE:
#ifdef HPBDEBUG
		if (hpbdebug)
		printf("hpeccSSE\n");
#endif
		rp->hpof |= HPOF_SSEI;
		mbp->mba_bcr = -(bp->b_bcount - (int)ptob(npf));
		break;

#ifndef NOBADSECT
	case BSE:
#ifdef HPBDEBUG
		if (hpbdebug)
		printf("hpeccBSE: bn %d cn %d tn %d sn %d\n", bn, cn, tn, sn);
#endif
		if ((bn = isbad(&hpbad[mi->mi_unit], cn, tn, sn)) < 0)
			return(0);
		bp->b_flags |= B_BAD;
		bp->b_error = npf + 1;
		bn = st->ncyl*st->nspc - st->nsect - 1 - bn;
		cn = bn/st->nspc;
		sn = bn%st->nspc;
		tn = sn/st->nsect;
		sn %= st->nsect;
		mbp->mba_bcr = -512;
#ifdef HPBDEBUG
		if (hpbdebug)
		printf("revector to cn %d tn %d sn %d\n", cn, tn, sn);
#endif
		break;

	case CONT:
#ifdef HPBDEBUG
		if (hpbdebug)
		printf("hpecc, CONT: bn %d cn %d tn %d sn %d\n", bn,cn,tn,sn);
#endif
		npf = bp->b_error;
		bp->b_flags &= ~B_BAD;
		mbp->mba_bcr = -(bp->b_bcount - (int)ptob(npf));
		if ((mbp->mba_bcr & 0xffff) == 0)
			return(0);
		break;
#endif
	}
#ifdef HPBDEBUG
	if (hpbdebug)
	printf("hpecc, endcase: bn %d\n",bn);
#endif
	rp->hpcs1 = HP_DCLR|HP_GO;
	if (rp->hpof&HPOF_SSEI)
		sn++;
	rp->hpdc = cn;
	rp->hpda = (tn<<8) + sn;
	mbp->mba_sr = -1;
	mbp->mba_var = (int)ptob(npf) + o;
	rp->hpcs1 = bp->b_flags&B_READ ? HP_RCOM|HP_GO : HP_WCOM|HP_GO;
	mi->mi_tab.b_errcnt = 0;	/* error has been corrected */
	return (1);
}

#define	DBSIZE	20

hpdump(dev)
	dev_t dev;
{
	register struct mba_device *mi;
	register struct mba_regs *mba;
	struct hpdevice *hpaddr;
	char *start;
	int num, unit;
	register struct hpst *st;

	num = maxfree;
	start = 0;
	unit = minor(dev) >> 3;
	if (unit >= NHP)
		return (ENXIO);
#define	phys(a,b)	((b)((int)(a)&0x7fffffff))
	mi = phys(hpinfo[unit],struct mba_device *);
	if (mi == 0 || mi->mi_alive == 0)
		return (ENXIO);
	mba = phys(mi->mi_hd, struct mba_hd *)->mh_physmba;
	mba->mba_cr = MBCR_INIT;
	hpaddr = (struct hpdevice *)&mba->mba_drv[mi->mi_drive];
	if ((hpaddr->hpds & HPDS_VV) == 0) {
		hpaddr->hpcs1 = HP_DCLR|HP_GO;
		hpaddr->hpcs1 = HP_PRESET|HP_GO;
		hpaddr->hpof = HPOF_FMT22;
	}
	st = &hpst[mi->mi_type];
	if (dumplo < 0 || dumplo + num >= st->sizes[minor(dev)&07].nblocks)
		return (EINVAL);
	while (num > 0) {
		register struct pte *hpte = mba->mba_map;
		register int i;
		int blk, cn, sn, tn;
		daddr_t bn;

		blk = num > DBSIZE ? DBSIZE : num;
		bn = dumplo + btop(start);
		cn = bn/st->nspc + st->sizes[minor(dev)&07].cyloff;
		sn = bn%st->nspc;
		tn = sn/st->nsect;
		sn = sn%st->nsect;
		hpaddr->hpdc = cn;
		hpaddr->hpda = (tn << 8) + sn;
		for (i = 0; i < blk; i++)
			*(int *)hpte++ = (btop(start)+i) | PG_V;
		mba->mba_sr = -1;
		mba->mba_bcr = -(blk*NBPG);
		mba->mba_var = 0;
		hpaddr->hpcs1 = HP_WCOM | HP_GO;
		while ((hpaddr->hpds & HPDS_DRY) == 0)
			;
		if (hpaddr->hpds&HPDS_ERR)
			return (EIO);
		start += blk*NBPG;
		num -= blk;
	}
	return (0);
}
#endif

/*	uba.c	4.42	82/03/31	*/

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/cpu.h"
#include "../h/map.h"
#include "../h/pte.h"
#include "../h/buf.h"
#include "../h/vm.h"
#include "../h/ubareg.h"
#include "../h/ubavar.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/conf.h"
#include "../h/mtpr.h"
#include "../h/nexus.h"
#include "../h/dk.h"
#include "../h/trace.h"

#if VAX780
char	ubasr_bits[] = UBASR_BITS;
#endif

/*
 * Do transfer on device argument.  The controller
 * and uba involved are implied by the device.
 * We queue for resource wait in the uba code if necessary.
 * We return 1 if the transfer was started, 0 if it was not.
 * If you call this routine with the head of the queue for a
 * UBA, it will automatically remove the device from the UBA
 * queue before it returns.  If some other device is given
 * as argument, it will be added to the request queue if the
 * request cannot be started immediately.  This means that
 * passing a device which is on the queue but not at the head
 * of the request queue is likely to be a disaster.
 */
ubago(ui)
	register struct uba_device *ui;
{
	register struct uba_ctlr *um = ui->ui_mi;
	register struct uba_hd *uh;
	register int s, unit;

	uh = &uba_hd[um->um_ubanum];
	s = spl6();
	if (um->um_driver->ud_xclu && uh->uh_users > 0 || uh->uh_xclu)
		goto rwait;
	/* if(uh->uh_users < 0)
		printf ("(1) uh_users = %d\n", uh->uh_users); */
	if(um->um_tab.b_actf == NULL) {
		panic("null ptr in ubago\n");
		splx(s);
		return;
	}
	um->um_ubinfo = ubasetup(um->um_ubanum, um->um_tab.b_actf->b_actf,
	    UBA_NEEDBDP|UBA_CANTWAIT);
	if (um->um_ubinfo == 0)
		goto rwait;
	uh->uh_users++;
	trace(TR_BDPOFF, -1, uh->uh_users);	/* overloaded lazy */
	if (um->um_driver->ud_xclu)
		uh->uh_xclu = 1;
	splx(s);
	if (ui->ui_dk >= 0) {
		unit = ui->ui_dk;
		dk_busy |= 1<<unit;
		dk_xfer[unit]++;
		dk_wds[unit] += um->um_tab.b_actf->b_actf->b_bcount>>6;
	}
	if (uh->uh_actf == ui)
		uh->uh_actf = ui->ui_forw;
	/* if(uh->uh_users <= 0)
		printf ("(2) uh_users = %d\n", uh->uh_users); */
	(*um->um_driver->ud_dgo)(um);
	return (1);
rwait:
	trace(TR_BDPOFF, -3, uh->uh_users);
	if (uh->uh_actf != ui) {
		ui->ui_forw = NULL;
		if (uh->uh_actf == NULL)
			uh->uh_actf = ui;
		else
			uh->uh_actl->ui_forw = ui;
		uh->uh_actl = ui;
	}
	splx(s);
	return (0);
}

ubadone(um)
	register struct uba_ctlr *um;
{
	register struct uba_hd *uh = &uba_hd[um->um_ubanum];

	if (um->um_driver->ud_xclu)
		uh->uh_xclu = 0;
	uh->uh_users--;
	trace(TR_BDPOFF, -2, uh->uh_users);	/* overloaded lazy */
	/* if(uh->uh_users < 0)
		printf ("(3) uh_users = %d\n", uh->uh_users); */
	ubarelse(um->um_ubanum, &um->um_ubinfo);
}

/*
 * Allocate and setup UBA map registers, and bdp's
 * Flags says whether bdp is needed, whether the caller can't
 * wait (e.g. if the caller is at interrupt level).
 *
 * Return value:
 *	Bits 0-8	Byte offset
 *	Bits 9-17	Start map reg. no.
 *	Bits 18-27	No. mapping reg's
 *	Bits 28-31	BDP no.
 */
ubasetup(uban, bp, flags)
	struct buf *bp;
{
	register struct uba_hd *uh = &uba_hd[uban];
	register int temp, i;
	int npf, reg, bdp;
	unsigned v;
	register struct pte *pte, *io;
	struct proc *rp;
	int a, o, ubinfo;

#if VAX7ZZ
	if (cpu == VAX_7ZZ)
		flags &= ~UBA_NEEDBDP;
#endif
	v = btop(bp->b_un.b_addr);
	o = (int)bp->b_un.b_addr & PGOFSET;
	npf = btoc(bp->b_bcount + o) + 1;
	a = spl6();
	while ((reg = rmalloc(uh->uh_map, npf)) == 0) {
		if (flags & UBA_CANTWAIT) {
			splx(a);
			return (0);
		}
		uh->uh_mrwant++;
		sleep((caddr_t)uh->uh_map, PSWP);
	}
	bdp = 0;
	if (flags & UBA_NEEDBDP) {
		while ((bdp = ffs(uh->uh_bdpfree)) == 0) {
			if (flags & UBA_CANTWAIT) {
				rmfree(uh->uh_map, npf, reg);
				splx(a);
				return (0);
			}
			uh->uh_bdpwant++;
			sleep((caddr_t)uh->uh_map, PSWP);
		}
		uh->uh_bdpfree &= ~(1 << (bdp-1));
	} else if (flags & UBA_HAVEBDP)
		bdp = (flags >> 28) & 0xf;
	else if(flags & UBA_WANTBDP) {
		bdp = ffs(uh->uh_bdpfree);
		if(bdp)
			uh->uh_bdpfree &= ~(1 << (bdp-1));
	}
	trace(TR_BDPON, uh->uh_bdpfree, flags);
	splx(a);
	reg--;
	ubinfo = (bdp << 28) | (npf << 18) | (reg << 9) | o;
	temp = (bdp << 21) | UBAMR_MRV;
	if (bdp && (o & 01))
		temp |= UBAMR_BO;
	rp = bp->b_flags&B_DIRTY ? &proc[2] : bp->b_proc;
	if ((bp->b_flags & B_PHYS) == 0)
		pte = &Sysmap[btop(((int)bp->b_un.b_addr)&0x7fffffff)];
	else if (bp->b_flags & B_UAREA)
		pte = &rp->p_addr[v];
	else if (bp->b_flags & B_PAGET)
		pte = &Usrptmap[btokmx((struct pte *)bp->b_un.b_addr)];
	else
		pte = vtopte(rp, v);
	io = &uh->uh_uba->uba_map[reg];
	while (--npf != 0) {
		if (pte->pg_pfnum == 0)
			panic("uba zero uentry");
		*(int *)io++ = pte++->pg_pfnum | temp;
	}
	*(int *)io++ = 0;
	return (ubinfo);
}

/*
 * Non buffer setup interface... set up a buffer and call ubasetup.
 */
uballoc(uban, addr, bcnt, flags)
	int uban;
	caddr_t addr;
	int bcnt, flags;
{
	struct buf ubabuf;

	ubabuf.b_un.b_addr = addr;
	ubabuf.b_flags = B_BUSY;
	ubabuf.b_bcount = bcnt;
	/* that's all the fields ubasetup() needs */
	return (ubasetup(uban, &ubabuf, flags));
}
 
/*
 * Release resources on uba uban, and then unblock resource waiters.
 * The map register parameter is by value since we need to block
 * against uba resets on 11/780's.
 */
ubarelse(uban, amr)
	int *amr;
{
	register struct uba_hd *uh = &uba_hd[uban];
	register int bdp, reg, npf, s;
	int mr;
 
	/*
	 * Carefully see if we should release the space, since
	 * it may be released asynchronously at uba reset time.
	 */
	s = spl6();
	mr = *amr;
	if (mr == 0) {
		/*
		 * A ubareset() occurred before we got around
		 * to releasing the space... no need to bother.
		 */
		splx(s);
		return;
	}
	*amr = 0;
	splx(s);		/* let interrupts in, we're safe for a while */
	bdp = (mr >> 28) & 0x0f;
	if (bdp) {
		switch (cpu) {
#if VAX780
		case VAX_780:
			uh->uh_uba->uba_dpr[bdp] |= UBADPR_BNE;
			break;
#endif
#if VAX750
		case VAX_750:
			uh->uh_uba->uba_dpr[bdp] |=
			    UBADPR_PURGE|UBADPR_NXM|UBADPR_UCE;
			break;
#endif
		}
		uh->uh_bdpfree |= 1 << (bdp-1);		/* atomic */
		trace(TR_BDPOFF, uh->uh_bdpfree, 0);
		if (uh->uh_bdpwant) {
			uh->uh_bdpwant = 0;
			wakeup((caddr_t)uh->uh_map);
		}
	}
	/*
	 * Put back the registers in the resource map.
	 * The map code must not be reentered, so we do this
	 * at high ipl.
	 */
	npf = (mr >> 18) & 0x3ff;
	reg = ((mr >> 9) & 0x1ff) + 1;
	s = spl6();
	rmfree(uh->uh_map, npf, reg);
	splx(s);

	/*
	 * Wakeup sleepers for map registers,
	 * and also, if there are processes blocked in dgo(),
	 * give them a chance at the UNIBUS.
	 */
	if (uh->uh_mrwant) {
		uh->uh_mrwant = 0;
		wakeup((caddr_t)uh->uh_map);
	}
	while (uh->uh_actf && ubago(uh->uh_actf))
		;
}

#define	PRGTIM	10	/* time to spin for bdp flush on comet */

ubapurge(um)
	register struct uba_ctlr *um;
{
	register struct uba_hd *uh = um->um_hd;
	register int bdp = (um->um_ubinfo >> 28) & 0x0f;
	register int i;
	register int *reg;

	switch (cpu) {
#if VAX780
	case VAX_780:
		uh->uh_uba->uba_dpr[bdp] |= UBADPR_BNE;
		break;
#endif
#if VAX750
	case VAX_750:
		reg = &uh->uh_uba->uba_dpr[bdp];
		*reg |= UBADPR_PURGE|UBADPR_NXM|UBADPR_UCE;
		for (i = PRGTIM; i > 0; i--)
			if ((*reg & UBADPR_PURGE) == 0)
				break;
		if (i <= 0)
			printf("uba%d bdp%d stuck\n", um->um_ubanum, bdp);
		if (*reg & UBADPR_ERROR)
			printf("uba%d bdp%d error %x\n", um->um_ubanum,
				bdp, *reg);
		break;
#endif
	}
}

/*
 * Generate a reset on uba number uban.  Then
 * call each device in the character device table,
 * giving it a chance to clean up so as to be able to continue.
 */
ubareset(uban)
	int uban;
{
	register struct cdevsw *cdp;
	register struct uba_hd *uh = &uba_hd[uban];
	int s;

	s = spl6();
	uh->uh_users = 0;
	uh->uh_zvcnt = 0;
	uh->uh_xclu = 0;
	uh->uh_hangcnt = 0;
	uh->uh_actf = uh->uh_actl = 0;
	uh->uh_bdpwant = 0;
	uh->uh_mrwant = 0;
	wakeup((caddr_t)&uh->uh_bdpwant);
	wakeup((caddr_t)&uh->uh_mrwant);
	printf("uba%d: reset", uban);
	ubainit(uh->uh_uba);
	for (cdp = cdevsw; cdp->d_open; cdp++)
		(*cdp->d_reset)(uban);
#ifdef INET
	ifubareset(uban);
#endif
	printf("\n");
	splx(s);
}

/*
 * Init a uba.  This is called with a pointer
 * rather than a virtual address since it is called
 * by code which runs with memory mapping disabled.
 * In these cases we really don't need the interrupts
 * enabled, but since we run with ipl high, we don't care
 * if they are, they will never happen anyways.
 */
ubainit(uba)
	register struct uba_regs *uba;
{

	switch (cpu) {
#if VAX780
	case VAX_780:
		uba->uba_cr = UBACR_ADINIT;
		uba->uba_cr = UBACR_IFS|UBACR_BRIE|UBACR_USEFIE|UBACR_SUEFIE;
		while ((uba->uba_cnfgr & UBACNFGR_UBIC) == 0)
			;
		break;
#endif
#if VAX750
	case VAX_750:
#endif
#if VAX7ZZ
	case VAX_7ZZ:
#endif
#if defined(VAX750) || defined(VAX7ZZ)
		mtpr(IUR, 0);
		/* give devices time to recover from power fail */
/* THIS IS PROBABLY UNNECESSARY */
		DELAY(500000);
/* END PROBABLY UNNECESSARY */
		break;
#endif
	}
}

#if VAX780
/*
 * Check to make sure the UNIBUS adaptor is not hung,
 * with an interrupt in the register to be presented,
 * but not presenting it for an extended period (5 seconds).
 */
unhang()
{
	register int uban;

	for (uban = 0; uban < numuba; uban++) {
		register struct uba_hd *uh = &uba_hd[uban];
		register struct uba_regs *up = uh->uh_uba;

		if (up->uba_sr == 0)
			return;
		up->uba_sr = UBASR_CRD|UBASR_LEB;
		uh->uh_hangcnt++;
		if (uh->uh_hangcnt > 5*hz) {
			uh->uh_hangcnt = 0;
			printf("uba%d: hung\n", uban);
			ubareset(uban);
		}
	}
}

/*
 * This is a timeout routine which decrements the ``i forgot to
 * interrupt'' counts, on an 11/780.  This prevents slowly growing
 * counts from causing a UBA reset since we are interested only
 * in hang situations.
 */
ubawatch()
{
	register struct uba_hd *uh;
	register int uban;

	if (panicstr)
		return;
	for (uban = 0; uban < numuba; uban++) {
		uh = &uba_hd[uban];
		if (uh->uh_hangcnt)
			uh->uh_hangcnt--;
	}
}

int	ubawedgecnt = 10;
int	ubacrazy = 500;
/*
 * This routine is called by the locore code to
 * process a UBA error on an 11/780.  The arguments are passed
 * on the stack, and value-result (through some trickery).
 * In particular, the uvec argument is used for further
 * uba processing so the result aspect of it is very important.
 * It must not be declared register.
 */
/*ARGSUSED*/
ubaerror(uban, uh, xx, uvec, uba)
	register int uban;
	register struct uba_hd *uh;
	int uvec;
	register struct uba_regs *uba;
{
	register sr, s;

	if (uvec == 0) {
		uh->uh_zvcnt++;
		if (uh->uh_zvcnt > 250000) {
			printf("uba%d: too many zero vectors\n");
			ubareset(uban);
		}
		uvec = 0;
		return;
	}
	if (uba->uba_cnfgr & NEX_CFGFLT) {
		printf("uba%d: sbi fault sr=%b cnfgr=%b\n",
		    uban, uba->uba_sr, ubasr_bits,
		    uba->uba_cnfgr, NEXFLT_BITS);
		ubareset(uban);
		uvec = 0;
		return;
	}
	sr = uba->uba_sr;
	s = spl7();
	printf("uba%d: uba error sr=%b fmer=%x fubar=%o\n",
	    uban, uba->uba_sr, ubasr_bits, uba->uba_fmer, 4*uba->uba_fubar);
	splx(s);
	uba->uba_sr = sr;
	uvec &= UBABRRVR_DIV;
	if (++uh->uh_errcnt % ubawedgecnt == 0) {
		if (uh->uh_errcnt > ubacrazy)
			panic("uba crazy");
		printf("ERROR LIMIT ");
		ubareset(uban);
		uvec = 0;
		return;
	}
	return;
}
#endif

#if	defined(CHAOS)
/*
 * This routine allows remapping of previously
 * allocated UNIBUS bdp and map resources
 * onto different memory addresses.
 * It should only be used by routines which need
 * small fixed length mappings for long periods of time
 * (like the ARPANET ACC IMP interface).
 * It only maps kernel addresses.
 */
ubaremap(uban, ubinfo, addr)
	int uban;
	register unsigned ubinfo;
	caddr_t addr;
{
	register struct uba_hd *uh = &uba_hd[uban];
	register struct pte *pte, *io;
	register int temp, bdp;
	int npf, o;

	o = (int)addr & PGOFSET;
	bdp = (ubinfo >> 28) & 0xf;
	npf = (ubinfo >> 18) & 0x3ff;
	io = &uh->uh_uba->uba_map[(ubinfo >> 9) & 0x1ff];
	temp = (bdp << 21) | UBAMR_MRV;

	/*
	 * If using buffered data path initiate purge
	 * of old data and set byte offset bit if next
	 * transfer will be from odd address.
	 */
	if (bdp) {
		switch (cpu) {
#if VAX780
		case VAX_780:
			uh->uh_uba->uba_dpr[bdp] |= UBADPR_BNE;
			break;
#endif
#if VAX750
		case VAX_750:
			uh->uh_uba->uba_dpr[bdp] |=
			    UBADPR_PURGE|UBADPR_NXM|UBADPR_UCE;
			break;
#endif
		}
		if (o & 1)
			temp |= UBAMR_BO;
	}

	/*
	 * Set up the map registers, leaving an invalid reg
	 * at the end to guard against wild unibus transfers.
	 */
	pte = &Sysmap[btop(((int)addr)&0x7fffffff)];
	while (--npf != 0)
		*(int *)io++ = pte++->pg_pfnum | temp;
	*(int *)io = 0;

	/*
	 * Return effective UNIBUS address.
	 */
	return ((ubinfo & ~PGOFSET) | o);
}
#endif	defined(CHAOS)

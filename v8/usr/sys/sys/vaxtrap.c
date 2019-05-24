/*
 * Routines that deal closely with VAX-specific traps:
 * machine checks, memory errors, and the like
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/reg.h"
#include "../h/mtpr.h"
#include "../h/proc.h"
#include "../h/psl.h"
#include "../h/conf.h"
#include "../h/mem.h"
#include "../h/cpu.h"

/*
 * Memenable enables the memory controller corrected data reporting.
 * This runs at regular intervals, turning on the interrupt.
 * The interrupt is turned off, per memory controller, when error
 * reporting occurs.  Thus we report at most once per memintvl.
 */
int	memintvl = MEMINTVL;

memenable()
{
	register struct mcr *mcr;
	register int m;

	for (m = 0; m < nmcr; m++) {
		mcr = mcraddr[m];
		switch (cpu) {
#if VAX780
		case VAX_780:
			M780_ENA(mcr);
			break;
#endif
#if VAX750
		case VAX_750:
			M750_ENA(mcr);
			break;
#endif
#if VAX7ZZ
		case VAX_7ZZ:
			M7ZZ_ENA(mcr);
			break;
#endif
		}
	}
	if (memintvl > 0)
		timeout(memenable, (caddr_t)0, memintvl);
}

/*
 * Memerr is the interrupt routine for corrected read data
 * interrupts.  It looks to see which memory controllers have
 * unreported errors, reports them, and disables further
 * reporting for a time on those controller.
 */
memerr()
{
	register struct mcr *mcr;
	register int m;

	for (m = 0; m < nmcr; m++) {
		mcr = mcraddr[m];
		switch (cpu) {
#if VAX780
		case VAX_780:
			if (M780_ERR(mcr)) {
				printf("mcr%d: soft ecc addr %x syn %x\n",
				    m, M780_ADDR(mcr), M780_SYN(mcr));
				M780_INH(mcr);
			}
			break;
#endif
#if VAX750
		case VAX_750:
			if (M750_ERR(mcr)) {
				printf("mcr%d: soft ecc addr %x syn %x\n",
				    m, M750_ADDR(mcr), M750_SYN(mcr));
				M750_INH(mcr);
			}
			break;
#endif
#if VAX7ZZ
		case VAX_7ZZ:
			if (M7ZZ_ERR(mcr)) {
				struct mcr amcr;
				amcr.mc_reg[0] = mcr->mc_reg[0];
				printf("mcr%d: soft ecc addr %x syn %x\n",
				    m, M7ZZ_ADDR(&amcr), M7ZZ_SYN(&amcr));
				M7ZZ_INH(mcr);
			}
			break;
#endif
		}
	}
}

/*
 * write timeout trap
 * panic if in kernel mode
 * trap if user mode
 */

wtimeout(ps, pc)
long ps, pc;
{

	if (USERMODE(ps)) {
		/*
		 * code stolen from setrun
		 */
		runrun++;
		aston();
		psignal(u.u_procp, SIGBUS);
		return;
	}
	printf("wtmo pc %x\n", pc);
	panic("wtmo");
}

/*
 * Machine check error recovery code.
 * Print the machine check frame and reset things.
 * If possible, recover and return;
 * if not but in user mode, send a signal;
 * if not and in kernel mode, panic.
 */

machinecheck(ps, f)
long ps;
caddr_t f;
{
	int ok;

	switch (cpu) {
#if defined(VAX780)
	case VAX_780:
		ok = mc780(f);
		break;
#endif

#if defined(VAX750)
	case VAX_750:
		ok = mc750(f);
		break;
#endif

#if defined(VAX7ZZ)
	case VAX_7ZZ:
		ok = mc7ZZ(f);
		break;
#endif

	default:
		panic("unknown cpu in mchk");
	}
	machreset();
	if (ok)
		return;
	if (USERMODE(ps)) {
		/*
		 * code stolen from setrun
		 */
		runrun++;
		aston();
		psignal(u.u_procp, SIGBUS);
		return;
	}
	panic("mchk");
}

/*
 * VAX-11/780 machine checks
 */

#if VAX780
char *mc780name[] = {
	"rd timeout",	"ctrl str par",	"tbuf par",	"cache par",
	"??",		"rd data sub",	"ucode lost",	"??",
	"??",		"??",		"ib tbuf par",	"??",
	"ib rd data sub", "ib rd timeout", "??",	"ib cache par"
};

#define	M8TMOUT	0
#define	M8CSPAR	1
#define	M8TBPAR	2
#define	M8CPAR	3
#define	M8RDS	5
#define	M8UCLOST 6
#define	M8IBTB	10
#define	M8IBRDS	12
#define	M8IBTMO	13
#define	M8IBCP	15

struct mc780frame {
	int	mc8_bcnt;		/* byte count == 0x28 */
	int	mc8_summary;		/* summary parameter (as above) */
	int	mc8_cpues;		/* cpu error status */
	int	mc8_upc;		/* micro pc */
	int	mc8_vaviba;		/* va/viba register */
	int	mc8_dreg;		/* d register */
	int	mc8_tber0;		/* tbuf error reg 0 */
	int	mc8_tber1;		/* tbuf error reg 1 */
	int	mc8_timo;		/* timeout address divided by 4 */
	int	mc8_parity;		/* parity */
	int	mc8_sbier;		/* sbi error register */
	int	mc8_pc;			/* trapped pc */
	int	mc8_psl;		/* trapped psl */
};

mc780(f)
register struct mc780frame *f;
{
	register int sbifs;

	sbifs = mfpr(SBIFS);
	printf("machine check type x%x: %s%s\n", f->mc8_summary,
	    mc780name[f->mc8_summary&0xf],
	    (f->mc8_summary&0xf0) ? " abort" : " fault"); 
	printf("\tcpues %x upc %x va/viba %x dreg %x tber %x %x\n",
	   f->mc8_cpues, f->mc8_upc, f->mc8_vaviba,
	   f->mc8_dreg, f->mc8_tber0, f->mc8_tber1);
	printf("\ttimo %x parity %x sbier %x pc %x psl %x sbifs %x\n",
	   f->mc8_timo*4, f->mc8_parity, f->mc8_sbier,
	   f->mc8_pc, f->mc8_psl, sbifs);
	switch (f->mc8_summary) {
	case M8RDS:
	case M8IBRDS:
		memerr();
		return (1);
	}
	return (0);
}

#endif

/*
 * VAX 11/750 machine checks
 */

#if VAX750

#define	NMC750	8
char *mc750name[NMC750] = {
	"??",		"cs par",	"tbuf/bus/cache",	"??",
	"??",		"??",		"ucode lost",		"unused ird slot",
};

#define	M5BUS	2	/* the only one we care about */

/*
 * bits in various error registers
 */

#define	M5UCD	0x4	/* mcsr: uncorrectable read data error */
#define	M5NXM	0x8	/* mcsr: non-existent memory */
#define	M5TBPAR	0x4	/* mcesr: tb parity error */
#define	M5CDATA	0x4	/* cacherr: cache data error */
#define	M5CTAG	0x8	/* cacherr: cache tag error */

struct mc750frame {
	int	mc5_bcnt;		/* byte count == 0x28 */
	int	mc5_summary;		/* summary parameter (as above) */
	int	mc5_va;			/* virtual address register */
	int	mc5_errpc;		/* error pc */
	int	mc5_mdr;
	int	mc5_svmode;		/* saved mode register */
	int	mc5_rdtimo;		/* read lock timeout */
	int	mc5_tbgpar;		/* tb group parity error register */
	int	mc5_cacherr;		/* cache error register */
	int	mc5_buserr;		/* bus error register */
	int	mc5_mcesr;		/* machine check status register */
	int	mc5_pc;			/* trapped pc */
	int	mc5_psl;		/* trapped psl */
};

/* use 780 names for now, even though they're wrong */

mc750(f)
register struct mc750frame *f;
{

	printf("machine check type x%x:", f->mc5_summary);
	if (0 <= f->mc5_summary && f->mc5_summary < NMC750)
		printf(" %s\n", mc750name[f->mc5_summary]);
	else
		printf("\n");
	printf("\tva %x errpc %x mdr %x smr %x rdtimo %x tbgpar %x cacherr %x\n",
	    f->mc5_va, f->mc5_errpc, f->mc5_mdr, f->mc5_svmode,
	    f->mc5_rdtimo, f->mc5_tbgpar, f->mc5_cacherr);
	printf("\tbuserr %x mcesr %x pc %x psl %x mcsr %x\n",
	    f->mc5_buserr, f->mc5_mcesr, f->mc5_pc, f->mc5_psl,
	    mfpr(MCSR));
	mtpr(TBIA, 0);
	if (f->mc5_summary != M5BUS)
		return (0);
	if (f->mc5_mcesr & M5TBPAR) {
		printf("ignore tbuf par err\n");
		return (1);
	}
	else if (f->mc5_cacherr & (M5CDATA | M5CTAG))
		printf("cache err\n");
	else if (f->mc5_buserr & M5UCD) {
		printf("hard mem err\n");
		memerr();
		return (0);	/* can't recover */
	}
	else if (f->mc5_buserr & M5NXM) {
		printf("nxm\n");
		return (0);	/* can't recover */
	}
	else {
		printf("unknown problem\n");
		return (0);
	}
	/* could perhaps recover here */
	return (0);
}

#endif

/*
 * VAX-11/730 machine checks
 */

#if VAX7ZZ

#define	NMC7ZZ	12
char *mc7ZZname[] = {
	"tb par",	"bad retry",	"bad intr id",	"cant write ptem",
	"unkn mcr err",	"iib rd err",	"nxm ref",	"cp rds",
	"unalgn ioref",	"nonlw ioref",	"bad ioaddr",	"unalgn ubaddr",
};

struct mc7ZZframe {
	int	mc3_bcnt;		/* byte count == 0xc */
	int	mc3_summary;		/* summary parameter */
	int	mc3_parm[2];		/* parameter 1 and 2 */
	int	mc3_pc;			/* trapped pc */
	int	mc3_psl;		/* trapped psl */
};

mc7ZZ(f)
register struct mc7ZZframe *f;
{

	printf("machine check type x%x: ", f->mc3_summary);
	if (type < NMC7ZZ)
		printf("%s", mc7ZZname[type]);
	printf("\n");
	printf("params %x,%x pc %x psl %x mcesr %x\n",
	    mcf->mc3_parm[0], mcf->mc3_parm[1],
	    mcf->mc3_pc, mcf->mc3_psl, mfpr(MCESR));
}

#endif

/*
 * reset processor error registers
 * call if we get a machine check that's really ok,
 * or perhaps when the system is started
 */

machreset()
{

	switch (cpu) {
#if VAX780
	case VAX_780:
		/* THE FUNNY BITS IN THE FOLLOWING ARE FROM THE ``BLACK */
		/* BOOK AND SHOULD BE PUT IN AN ``sbi.h'' */
		mtpr(SBIFS, mfpr(SBIFS) &~ 0x2000000);
		mtpr(SBIER, mfpr(SBIER) | 0x70c0);
		break;
#endif

#if VAX750
	case VAX_750:
		mtpr(MCESR, 0xf);
		break;
#endif

#if VAX7ZZ
	case VAX_7ZZ:
		mtpr(MCESR, 0xf);
		break;
#endif
	}
}

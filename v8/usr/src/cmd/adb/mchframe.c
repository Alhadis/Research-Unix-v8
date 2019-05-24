/*
 * machine-dependent code for
 * looking in stack frames
 * vax version
 */

#include "defs.h"
#include <sys/param.h>
#include "regs.h"
#include <a.out.h>
#include <stab.h>

MSG NOCFN;
MSG BADVAR;

/*
 * VAX stack frame
 */

#define	F_PSW	4
#define	F_FLAGS	6
#define	F_AP	8	/* saved ap */
#define	F_FP	12	/* saved fp */
#define	F_PC	16	/* return pc */
#define	F_REGS	20	/* saved regs, if any */

/*
 * flags
 */

#define	FFREGS	0x1fff	/* saved register flags; 01 is r0 */
#define	FFCALLS	0x2000	/* called by calls, not callg */
#define	FFOFF	0xc000	/* offset added to align stack */
#define	SALIGN(f)	(((f)>>14) & 03)

/*
 * is the pc probably in signal trampoline code?
 * == it's in the user block
 */

#define	sigtramp(pc)	(0x80000000 > (pc) && (pc) > 0x80000000 - ctob(UPAGES))

#define	F_HACK	64	/* where to find the saved pc in a trampoline frame */
/*
 * return an address for a local variable
 * no register vars, unfortunately, as we can't provide an address
 * gn is the procedure; ln the local name
 *
 * this is vax dependent because symbol tables vary
 */

localaddr(gn, ln)
char *gn, *ln;
{
	WORD fp, ap;
	extern WORD expv;
	extern int expsp;
	ADDR laddr();

	if (gn) {
		if (findrtn(gn) == 0)
			error(NOCFN);
	}
	else {
		findsym((WORD)atow(rget(PC)), INSTSP);
		if (cursym == NULL)
			error(NOCFN);
	}
	if (findframe(&fp, &ap) == 0)
		error(NOCFN);
	if (ln == NULL) {
		expsp = 0;
		expv = fp;
		return;
	}
	while (localsym()) {
		if (strcmp(ln, cursym->n_un.n_name) != 0)
			continue;
		expv = laddr(cursym, fp, ap);
		if (cursym->n_type == N_RSYM)
			expsp = REGSP;
		else
			expsp = NOSP;
		return;
	}
	error(BADVAR);
	/* NOTREACHED */
}

/*
 * print a stack traceback
 * give locals if possible
 */

ctrace(modif)
char modif;
{
	register ADDR fp, ap, callpc;
	register int narg;
	ADDR oap;
	register int fl;
	int tramp;

	if (adrflg) {
		fp = adrval;
		fl = stow(sget(fp + F_FLAGS, CORF|DATASP));
		if ((fl & FFCALLS) == 0)
			ap = fp;	/* callg, can't figure out ap */
		else {
			ap = adrval + F_REGS + SALIGN(fl);
			fl &= FFREGS;
			while (fl) {
				if (fl & 1)
					ap += sizeof(TREG);
				fl >>= 1;
			}
		}
		callpc = atow(aget(fp + F_PC, CORF|DATASP));
	} else {
		ap = (ADDR)rtow(rget(AP));
		fp = (ADDR)rtow(rget(FP));
		callpc = (ADDR)rtow(rget(PC));
	}
	clrraddr();
	while (cntval--) {
		chkerr();
		tramp = 0;
		if (sigtramp(callpc)) {
			printf("sigtramp(");
			tramp++;
		} else {
			findsym(callpc, INSTSP);
			if (cursym == NULL)
				printf("?(");
			else if (strcmp("start", cursym->n_un.n_name) == 0)
				break;
			else
				printf("%s(", cursym->n_un.n_name);
		}
		narg = ctow(cget(ap, CORF|DATASP));
		oap = ap;
		while (--narg >= 0) {
			printf("%R", ltow(lget(ap += sizeof(TREG), CORF|DATASP)));
			if (narg != 0)
				printc(',');
		}
		printf(") from %R\n", callpc);
		if (modif == 'C')
			locals(fp, oap);
		if (tramp)	/* hack */
			callpc = atow(aget(fp + F_HACK, CORF|DATASP));
		else
			callpc = atow(aget(fp + F_PC, CORF|DATASP));
		fl = stow(sget(fp + F_FLAGS, CORF|DATASP));
		setraddr(fl, fp);
		ap = atow(aget(fp + F_AP, CORF|DATASP));
		fp = atow(aget(fp + F_FP + SALIGN(fl), CORF|DATASP));
		if (fp == 0)
			break;
	}
	clrraddr();
}

static
locals(fp, ap)
ADDR fp, ap;
{
	WORD val;
	register int sp;
	ADDR laddr();

	while (localsym()) {
		sp = CORF | DATASP;
		if (cursym->n_type == N_RSYM)
			sp = CORF | REGSP;
		val = ltow(lget(laddr(cursym, fp, ap), sp));
		if (errflg == 0)
			printf("%8t%s/%10t%R\n", cursym->n_un.n_name, val);
		else {
			printf("%8t%s/%10t?\n", cursym->n_un.n_name);
			errflg = 0;
		}
	}
}

ADDR
laddr(sp, fp, ap)
struct nlist *sp;
ADDR fp, ap;
{

	switch (sp->n_type) {
	case N_STSYM:
		return (sp->n_value);

	case N_LSYM:
		return (fp - sp->n_value);

	case N_PSYM:
		return (ap + sp->n_value);

	case N_RSYM:
		return (sp->n_value * sizeof(TREG));
	}
	error("bad local symbol");
	/* NOTREACHED */
}

int
findframe(fpp, app)
ADDR *fpp, *app;
{
	register ADDR fp, ap, pc;
	register int fl;
	struct nlist *svcur;

	svcur = cursym;
	fp = rtow(rget(FP));
	ap = rtow(rget(AP));
	pc = rtow(rget(PC));
	if (errflg)
		return (0);
	clrraddr();
	for (;;) {
		findsym(pc, INSTSP);
		if (cursym == svcur)
			break;
		if (cursym && strcmp(cursym->n_un.n_name, "start") == 0) {
			clrraddr();
			return (0);
		}
		fl = stow(sget((ADDR)fp + F_FLAGS, CORF|DATASP));
		setraddr(fl, fp);
		pc = atow(aget(fp + F_PC, CORF|DATASP));
		ap = atow(aget(fp + F_AP, CORF|DATASP));
		fp = atow(aget(fp + F_FP + SALIGN(fl), CORF|DATASP));
		/* sigtramp? */
		if (errflg) {
			clrraddr();
			return (0);
		}
	}
	*fpp = fp;
	*app = ap;
	return (1);
}

/*
 * set addresses for saved registers for this frame
 */

setraddr(mask, fp)
register int mask;
register ADDR fp;
{
	register int r;
	register int i;
	extern ADDR raddr[];

	mask &= FFREGS;
	for (r = 0, i = 0; mask; r++)
		if (mask & (1 << r)) {
			if (MINREG <= r && r <= MAXREG)
				raddr[r - MINREG] = fp + F_REGS +
					i * sizeof(TREG);
			i++;
			mask &=~ (1 << r);
		}
}

clrraddr()
{
	register int i;
	extern ADDR raddr[];

	for (i = 0; i <= MAXREG - MINREG; i++)
		raddr[i] = 0;
}

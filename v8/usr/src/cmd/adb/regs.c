/*
 * code to keep track of registers
 */

#include "defs.h"
#include "regs.h"
#include <sys/param.h>

struct reglist {
	char	*rname;
	short	roffs;
	short	rsys;
	TREG	rval;
};

struct reglist reglist[] = {
	{"p1lr", P1LR, 1},
	{"p1br", P1BR, 1},
	{"p0lr", P0LR, 1},
	{"p0br", P0BR, 1},
	{"ksp",	KSP, 1},
	{"esp",	ESP, 1},
	{"ssp",	SSP, 1},
#define	FW	7	/* first register we may write */
	{"psl",	PSL, 0},
	{"pc",	PC, 0},
	{"sp",	USP, 0},
	{"fp",	FP, 0},
	{"ap",	AP, 0},
	{"r11",	R11, 0},
	{"r10",	R10, 0},
	{"r9",	R9, 0},
	{"r8",	R8, 0},
	{"r7",	R7, 0},
	{"r6",	R6, 0},
	{"r5",	R5, 0},
	{"r4",	R4, 0},
	{"r3",	R3, 0},
	{"r2",	R2, 0},
	{"r1",	R1, 0},
	{"r0",	R0, 0},
	{NULL}
};

/*
 * the following are needed only to
 * make registers `addressable'
 * which is needed only so we can
 * examine register variables
 */

ADDR raddr[MAXREG - MINREG + 1];
int roffs[MAXREG - MINREG + 1] = {
	R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11
};

/*
 * get/put registers
 * in our saved copies
 */

TREG
rget(r)
{
	register struct reglist *rp;

	for (rp = reglist; rp->rname; rp++)
		if (rp->roffs == r)
			return (rp->rval);
	error("panic: rget");
	/* NOTREACHED */
}

rput(r, v)
TREG v;
{
	register struct reglist *rp;

	for (rp = reglist; rp->rname; rp++)
		if (rp->roffs == r) {
			rp->rval = v;
			return;
		}
	error("panic: rput");
	/* NOTREACHED */
}

/*
 * grab registers into saved copy
 * should be called before looking at the process
 */

rsnarf()
{
	register struct reglist *rp;

	for (rp = reglist; rp->rname; rp++)
		fget((ADDR)rp->roffs, CORF|UBLKSP, (char *)&rp->rval, sizeof(rp->rval));
}

/*
 * put registers back
 */

rrest()
{
	register struct reglist *rp;

	if (pid == 0)
		return;
	for (rp = &reglist[FW]; rp->rname; rp++)
		fput((ADDR)rp->roffs, CORF|UBLKSP, (char *)&rp->rval, sizeof(rp->rval));
}

/*
 * print the registers
 */

printregs(c)
char c;
{
	register struct reglist *rp;

	for (rp = reglist; rp->rname; rp++) {
		if (rp->rsys == 1 && c != 'R')
			continue;
		printf("%s%6t%R %16t", rp->rname, rtow(rp->rval));
		valpr(rtow(rp->rval), (rp->roffs == PC) ? INSTSP : DATASP);
		printc(EOR);
	}
	printpc();
}

/*
 * translate a name to a magic register offset
 * the latter useful in rget/rput
 */

int
rname(n)
char *n;
{
	register struct reglist *rp;

	for (rp = reglist; rp->rname; rp++)
		if (strcmp(n, rp->rname) == 0)
			return (rp->roffs);
	return (BADREG);
}

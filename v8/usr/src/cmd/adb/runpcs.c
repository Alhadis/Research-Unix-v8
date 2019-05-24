#
/*
 *
 *	UNIX debugger
 *
 */

#include "defs.h"
#include <signal.h>
#include "regs.h"

MSG	ENDPCS;

BKPT *bkpthead;

BOOL bpin;

int pid;
int sigcode, signo;

/* service routines for sub process control */

runpcs(runmode, keepsig)
{
	register int rc;
	register BKPT *bkpt;

	if (adrflg)
		;	/* something = dot */
	printf("%s: running\n", symfil);
	while (--loopcnt >= 0) {
		rrest();
		if (runmode == SINGLE)
			runstep(keepsig);
		else {
			if ((bkpt = scanbkpt((ADDR)rtow(rget(PC)))) != NULL) {
				execbkpt(bkpt, keepsig);
				keepsig = 0;
			}
			setbp();
			runrun(keepsig);
		}
		bpwait();
		chkerr();
		keepsig = 0;
		delbp();
		if (signo != 0 || (bkpt = scanbkpt((ADDR)rtow(rget(PC)))) == NULL) {
			keepsig = 1;
			rc = 0;
			continue;
		}
		/* breakpoint */
		dot = bkpt->loc;
		if (bkpt->flag == BKPTSKIP) {
			execbkpt(bkpt, keepsig);
			keepsig = 0;
			loopcnt++;	/* we didn't really stop */
			continue;
		}
		bkpt->flag = BKPTSKIP;
		if (bkpt->comm[0] != EOR
		&&  command(bkpt->comm, ':') == 0
		&&  --bkpt->count == 0) {
			execbkpt(bkpt, keepsig);
			keepsig = 0;
			loopcnt++;
			continue;
		}
		bkpt->count = bkpt->initcnt;
		rc = 1;
	}
	return(rc);
}

/*
 * finish the process off;
 * kill if still running
 */

endpcs()
{
	register BKPT *bk;

	if (pid) {
		killpcs();
		pid=0;
		for (bk=bkpthead; bk; bk = bk->nxtbkpt)
			if (bk->flag != BKPTCLR)
				bk->flag = BKPTSET;
	}
	bpin = FALSE;
}

/*
 * start up the program to be debugged in a child
 */

setup()
{

	startpcs();
	if (errflg) {
		printf("%s: cannot execute\n", symfil);
		endpcs();
		error(0);
	}
	bpin = FALSE;
}

/*
 * skip over a breakpoint:
 * remove breakpoints, then single step
 * so we can put it back
 */
execbkpt(bk, keepsig)
BKPT *bk;
{
	runstep(keepsig);
	bk->flag = BKPTSET;
	bpwait();
	chkerr();
}

/*
 * find the breakpoint at adr, if any
 */

BKPT *
scanbkpt(adr)
ADDR adr;
{
	register BKPT *bk;

	for (bk = bkpthead; bk; bk = bk->nxtbkpt)
		if (bk->flag != BKPTCLR && bk->loc == adr)
			break;
	return(bk);
}

/*
 * remove all breakpoints from the process' address space
 */

delbp()
{
	register BKPT *bk;

	if (bpin == FALSE || pid == 0)
		return;
	for (bk = bkpthead; bk; bk = bk->nxtbkpt)
		if (bk->flag != BKPTCLR)
			bkput(bk, 0);
	bpin = FALSE;
}

/*
 * install all the breakpoints
 */

setbp()
{
	register BKPT *bk;

	if (bpin == TRUE || pid == 0)
		return;
	for (bk = bkpthead; bk; bk = bk->nxtbkpt)
		if (bk->flag != BKPTCLR)
			bkput(bk, 1);
	bpin = TRUE;
}

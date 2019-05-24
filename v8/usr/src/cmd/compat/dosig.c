#define FBSD
/*	Handle signal trapping from version 6 or version 7 compatability
 *	mode programs.
 *	Art Wetzel	November 1979
 */
#ifdef TRACE
#include <stdio.h>
#endif
#include <signal.h>
#include "defs.h"
unsigned int  sigvals[NSIG+1];
/* actual catch point for all signals */
#ifdef FBSD
sigcatch(signum,faultcode,myaddr,stpc,stps) int signum;
#else
sigcatch(signum) int signum;
#endif
{
	unsigned short *pcptr;
	extern getregs();
	if(incompat) {
		/* hurry up and get the registers before they are destroyed */
		getregs();
	} else {
		/* we were in native mode simulating a sys call */
		/* set it up with the old values */
		dosig(signum, pc);
		/* go back where we were doing the sys call */
		return(0);
	}
	/* figure out the pc */
#ifdef FBSD
	pcptr = (unsigned short *) &stpc;
#else
	pcptr = (unsigned short *)((char *)&pcptr + 20);
#endif
	pc = (unsigned short *) *pcptr;
	/* get the psl with condition codes */
	/* requires UNIX-32V patch to not clear out condition codes */
#ifdef FBSD
	psl = 0x83c00000 | (stps & 017);
#else
	psl = 0x83c00000 | (*(pcptr - 6) & 017);
#endif
	/* actually do the thing */
	if(sigvals[signum] != (unsigned int)SIG_DFL && (sigvals[signum] & (unsigned int)SIG_IGN) == 0)
		dosig(signum, pc);
	/* go back to compatability mode and the signal routine there */
	incompat++;
	compat();
}
/* routine to set up pdp11 space for return from a signal */
dosig(signum, from) {
	unsigned short *sp;
#ifdef TRACE
	fprintf(stderr,"Caught sig %d from 0%o -> 0%o\n",signum,(pc-1),*(pc-1));
#endif
	/* where is the stack */
	sp = (unsigned short *)regs[6];
	/* stack up psw condition codes so rti works */
	*(--sp) = psl & 017;
	/* stack pc */
	*(--sp) = (unsigned short)(int)pc;
	/* reset stack pointer */
	regs[6] = (unsigned short)(int)sp;
	/* reset pc to signal catching routine */
	pc = (unsigned short *)sigvals[signum];
}

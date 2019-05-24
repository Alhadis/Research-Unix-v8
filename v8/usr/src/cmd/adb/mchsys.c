/*
 * system-mode mapping
 * vax version
 */

#include "defs.h"
#include <sys/param.h>
#include <sys/pte.h>
#include <sys/pcb.h>
#include "machine.h"

static struct pte *sbr;
static struct pcb pcb;
#define	HWPAGE	512

printkm()
{

	if (sbr)
		printf("sbr %R\n", (WORD)sbr);
	if (pcb.pcb_p0br)
		printf("p0 %R %R; p1 %R %R\n",
			(WORD)pcb.pcb_p0br, (WORD)pcb.pcb_p0lr,
			(WORD)pcb.pcb_p1br, (WORD)pcb.pcb_p1lr);
}

kmsys()
{

	if (adrflg)
		sbr = (struct pte *)adrval;
	else
		printkm();
}

kmproc()
{
	struct pte pte;

	if (adrflg == 0) {
		printkm();
		return;
	}
	if (adrval == -1) {
		pcb.pcb_p0br = 0;
		return;
	}
	if (sbr == 0)
		error("map the kernel first");
	if (INKERNEL(adrval)) {
		*(long *)&pte = lget((ADDR)adrval, CORF|DATASP);
		if (pte.pg_v == 0)
			error("pcb address not found");
		adrval = pte.pg_pfnum * HWPAGE;
	}
	if (fget((ADDR)adrval, CORF|DATASP|RAWADDR, (char *)&pcb, sizeof(pcb)) == 0) {
		pcb.pcb_p0br = 0;
		chkerr();
	}
}

/*
 * map a kernel address to a physical address
 * arg is a pointer to be filled in
 * returns nonzero if address is valid
 */

int
kmap(paddr, sp)
ADDR *paddr;
{
	long a;
	long pte;
#define	Ppte	(*(struct pte *)&pte)
	register int off, pfn;

	if (sbr == 0)
		return (1);
	sp &=~ RAWADDR;
	a = (long)*paddr & ~0xc0000000;
	off = a % HWPAGE;
	pfn = (a / HWPAGE);
	switch ((ADDR)*paddr & 0xc0000000) {
	case 0xc0000000:		/* really illegal, but who cares? */
	case 0x80000000:		/* system space */
		pte = (long)lget((ADDR)(sbr + pfn), sp | RAWADDR);
		break;

	case 0x40000000:		/* p1 */
		if (pcb.pcb_p0br == 0)	/* sic */
			return (1);
		if (pfn <= pcb.pcb_p1lr)
			return (0);
		pte = lget((ADDR)(pcb.pcb_p1br + pfn), sp);
		if (errflg)
			return (0);
		break;

	case 0x00000000:		/* p0 */
		if (pcb.pcb_p0br == 0)
			return (1);
		if (pfn > pcb.pcb_p0lr)
			return (0);
		pte = lget((ADDR)(pcb.pcb_p0br + pfn), sp);
		if (errflg)
			return (0);
		break;
	}
	if (Ppte.pg_v == 0)
		return (0);
	*paddr = (ADDR)(Ppte.pg_pfnum * HWPAGE + off);
	return (1);
}

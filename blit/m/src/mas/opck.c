/*
 * Argument syntax types
 */
#define	AREG	1
#define	AIMM	2
#define	AEXP	3
#define	AIREG	4
#define	AINC	5
#define	ADEC	6
#define	AOFF	7
#define	ANDX	8
#define	AGEN	9

/*
 * Modifiers to AREG in op table
 */
#define	A	0100
#define	D	0200
#define	C	0400
#define	SR	01000
#define	P	02000
#define	U	04000
#define	SP	010000

/*
 * Modifiers to AGEN
 */
#define	AM	0100	/* alterable memory */

/*
 * Modifiers to AIMM
 */
#define	O	0100	/* 1 */
#define	Q	0200	/* 1 to 8 */
#define	M	0400	/* -128 to 127 */
#define	N	01000	/* -8 to -1 */
#define	V	02000	/* 0 to 15 */
#define	H	04000	/* -32K to 32K */

/*
 * Dispositions of address forms
 */
#define	DIG	0	/* ignore this address */
#define	DEA	1	/* E.A. to low order 6 bits */
#define	DRG	2	/* register to low order 3 bits */
#define	DRGL	3	/* register to bits 11-9 */
#define	DBR	4	/* branch offset (short) */
#define	DMQ	5	/* move-quick 8-bit value */
#define	DAQ	6	/* add-quick 3-bit value in 11-9 */
#define	DIM	7	/* Immediate value, according to size */
#define	DEAM	8	/* E.A. to bits 11-6 as in move */
#define	DBCC	9	/* branch address as in DCNT */
#define	DIMH	10	/* Immediate forced to be 1 word */

/*
 * Size codes
 */
#define	B	1	/* byte */
#define	W	2	/* word */
#define	L	4	/* long */
#define	WL	6	/* word or long */
#define	BWL	7	/* any type */

/*
 * Size dispositions
 */
#define	SIG	0	/* Ignore the size */
#define	SD	1	/* Standard coding in bits 7-6 */
#define	SMD	2	/* size code for mov instruction, bits 13-12 */

struct optab {
	unsigned short opcode;
	char	*opname;
	short	size;
	short	sdisp;
	short	addr1;
	short	a1disp;
	short	addr2;
	short	a2disp;
};

struct optab	optab[] = {
#include "ops.c"
0,	0,	0,	0,	0,	0,	0,	0
};

main()
{
	register struct optab *op, *op1;
	int mask1, mask2, m1, m2, m3;
	int mask3, mask4;

	for (op=optab; op->opname; op++) {
		m1 = genamask(op->a1disp);
		m2 = genamask(op->a2disp);
		m3 = gensmask(op->sdisp);
		if (m1&m2 || m1&m3 || m2&m3)
			printf("I conflict %s, %.6o\n", op->opname, op->opcode);
		mask1 = m1|m2|m3;
		mask3 = genAMmask(op->addr1, op->a1disp) | genAMmask(op->addr2, op->a2disp);
		for (op1=op+1; op1->opname; op1++) {
			mask2 = genamask(op1->a1disp) | genamask(op1->a2disp)
			   | gensmask(op1->sdisp);
			mask4 = genAMmask(op1->addr1, op1->a1disp) | genAMmask(op1->addr2, op1->a2disp);
			if ((op->opcode&~mask1) == (op1->opcode&~mask2))
			if ((op->opcode|mask3) == (op1->opcode|mask4))
				printf("D confl %s %.6o, %s %.6o\n",
					op->opname, op->opcode, op1->opname, op1->opcode);
		}
	}
}

genamask(d)
{
	switch (d) {

	case DIG:
	case DIMH:
	case DIM:
		return(0);

	case DEA:
		return(077);

	case DRG:
		return(07);

	case DRGL:
	case DAQ:
		return(07000);

	case DBR:
	case DMQ:
		return(0377);

	case DEAM:
		return(07700);

	case DBCC:
		return(0);

	default:
		printf("Unknown adisp\n");
	}
}

gensmask(d)
{
	switch(d) {

	case SIG:
		return(0);

	case SD:
		return(0300);

	case SMD:
		return(030000);

	default:
		printf("Unknown sdisp\n");
	}
}

genAMmask(a, d)
{
	if (a&AM) {
		if (d==DEA)
			return(060);
		if (d==DEAM)
			return(0600);
	}
	return(0);
}

/*
 *  Header file for m68000 assembler
 */

#include <a.out.h>

/* redefine some symtab fields */
#define	tag	n_dtype
#define	index	n_desc

#define	NSYM	2000
#define	NHASH	(NSYM+1)
#define	NLOC	4		/* number of location ctrs */
#define	NCPS	8

/*
 * Extra flags and masks for symbol types
 */
#define	XTXRN	0xA	/* used in combine */
#define	XTYPE	016
#define	XFORW	0x20	/* Was forward-referenced when undefined */

#define	ERR	(-1)
#define	NBPW	32
#define	TMPC	9


/*
 * Alignment requirements-- word boundary only
 */
#define	HW	01
#define	FW	01
#define	DW	01

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
#define	APIC	10
#define	API2	11

#define	AMASK	017

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
#define	CT	0200	/* Control */
#define	AL	0400	/* alterable */
#define	DA	01000	/* data (not address register) */

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
#define	DBCC	9	/* branch address as in "dbcc" */
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

#define	DREGS	0	/* data registers */
#define	AREGS	8	/* address registers */
#define	PCREG	16
#define	SPREG	17
#define	CCREG	18
#define	SRREG	19

#define	SNULL	(struct nlist *)NULL

struct instab {
	unsigned short opcode;
	char	*opname;
	short	size;
	short	sdisp;
	short	addr1;
	short	a1disp;
	short	addr2;
	short	a2disp;
	short	iflag;
};

/* special iflags */
#define	ISL	01		/* shrinkable long immediate */
#define	ISH	02		/* shift instruction */

#define	SEGRND	0x1L

#define	round(x,y)	(((x)+(y)) & ~(y))

struct	arg {
	char	atype;
	char	areg1;
	char	areg2;
	char	asize;
	struct	exp *xp;
};

struct	exp {
	char	xtype;
	char	xloc;
	long	xvalue;
	struct	nlist *xname;
};

typedef union {
	int	yint;
	long	*ynumber;
	struct	nlist	*yname;
	struct	exp	*yexp;
	struct	arg	*yarg;
	char	*ystring;
} YYSTYPE;

struct	nlist	symtab[NSYM];

struct	nlist	*hshtab[NHASH];
extern	YYSTYPE	yylval;
extern	int	lineno;
extern	char	*filename;
extern	int	hshused;
extern	int	usrname;
extern	struct	nlist *nextsym;
extern	struct	nlist *firstsym;
extern	struct	exp	usedot[NLOC+NLOC];
extern	FILE	*usefile[NLOC+NLOC];
extern	FILE	*rusefile[NLOC+NLOC];
extern	char	*tmpn2;
extern	char	*tmpn3;
extern	struct	exp	*dotp;
extern	int	loctr;
extern	long	tsize;
extern	long	dsize;
extern	long	datbase;
extern	int	bitoff;
extern	long	bitfield;
extern	char	yytext[NCPS+2];
extern	char	yystring[NCPS];
extern	FILE	*txtfil;
extern	FILE	*tmpfil;
extern	FILE	*relfil;
extern	FILE	*stabfil;
extern	int	passno;
extern	int	nstabs;
extern	int	anyerrs;
extern	int	*brptr;
struct	instab	instab[];
int	curlen;
int	debugflag;
int	gindex;
extern	struct arg *ap;
extern	struct exp *xp;
int	orgwarn;
struct	nlist	**lookup();
char	*mktemp();
char	*strcpy();
long	ckabs();
struct	exp	*combine();

/* m68000 on vax */
struct	exec {	/* a.out header */
	long	a_magic;	/* magic number */
	long	a_text; 	/* size of text segment */
	long	a_data; 	/* size of initialized data */
	long	a_bss;  	/* size of unitialized data */
	long	a_syms; 	/* size of symbol table */
	long	a_entry; 	/* entry point */
	long	a_unused;	/* not used */
	long	a_flag; 	/* relocation info stripped */
};

struct	nlist {	/* symbol table entry */
	char	n_name[8];	/* symbol name */
	char	n_type;    	/* type flag */
	char	n_other;
	unsigned short n_desc;	/* C type code */
	long	n_value;	/* value */
};

/* values for type flag */
#define	N_UNDF	0	/* undefined */
#define	N_ABS	02	/* absolute */
#define	N_TEXT	04	/* text symbol */
#define	N_DATA	06	/* data symbol */
#define	N_BSS	010	/* bss symbol */
#define	N_EXT	01	/* external bit, or'ed in */
#define N_TYPE	036

#define	FORMAT	"%08x"	/* to print a value */
#define NAMESZ	8	/* 8 is largest string */

/*
 * Quantities for relocation words
 */
struct reloc {
	unsigned short rtype:4;
	unsigned short r2wds:1;
	unsigned short rpc:1;
	unsigned short runused:1;
	unsigned short rnum:9;
};	/* why is the ext bit wasting a bit in this? */
/* modifiers applying to relocation bits */
#define	X2WDS	020	/* long (2-word) quantity */
#define	XPCREL	040	/* PC-relative */

/* Instruction types, for loader-optimizer */
/* Apply only if type is N_ABS */
#define	TMASK	03600	/* Mask for type */
#define	TBR0	00200	/* Short (8-bit) branch */
#define	TBR1	00400	/* medium (16-bit) branch */
#define	TEA0	00600	/* Standard effective addr. follows immediately */
#define	TEA1	01000	/* effective addr. follows after 1 word */
#define	TEA2	01200	/* effective addr follows after 2 words */
#define TIM0	01400	/* shortenable long immediate follows */

/* Possible second E.A. for mov instruction */
#define	TMASK1	074000
#define	TSHFT1	4

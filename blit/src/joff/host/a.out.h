struct	exec {	/* a.out header */
	long	a_magic;	/* magic number */
	long	a_text; 	/* size of text segment */
	long	a_data; 	/* size of initialized data */
	long	a_bss;  	/* size of unitialized data */
	long	a_syms; 	/* size of symbol table */
	long	a_entry; 	/* entry point */
	long	a_textorg;	/* -b textorg */
	long	a_flag; 	/* relocation info stripped */
};

struct	nlist {	/* symbol table entry */
	char	n_name[8];	/* symbol name */
	char	n_type;    	/* type flag */
	char	n_other;
	unsigned short n_desc;	/* C type code */
	long	n_value;	/* value */
};

#define	N_UNDF	0
#define	N_ABS	02
#define	N_TEXT	04
#define	N_DATA	06
#define	N_BSS	010
#define N_CTEXT	012
#define N_RDATA	014
#define N_COMM	016
#define	N_EXT	01	/* external bit, or'ed in */
#define N_TYPE	036
#define N_FN	036
#define N_MORE	040
#define N_REG	0100	/* place holder of some sort */
#define	X2WDS	020	/* long (2-word) quantity */

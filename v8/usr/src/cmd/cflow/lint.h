/*	the key:
	LDI	defined and initialized: storage set aside
	LIB	defined on a library
	LDC	defined as a common region on UNIX
	LDX	defined by an extern: if ! pflag, same as LDI
	LRV	function returns a value
	LUV	function used in a value context
	LUE	function used in effects context
	LUM	mentioned somewhere other than at the declaration
	*/
# define LDI 01
# define LIB 02
# define LDC 04
# define LDX 010
# define LRV 020
# define LUV 040
# define LUE 0100
# define LUM 0200

# define LFN 0400  /* filename record */

# ifdef FMTARGS
# define LPF	02000	/* printf-like function */
# define LSF	04000	/* scanf-like function */
# endif

	/* number of chars in NAME, and filename */
#ifndef FLEXNAMES
# define LCHNM 8
# define LFNM 14
#endif

typedef struct ty {
	TWORD aty;
	short extra;
#ifdef FMTARGS
	long straddr;		/* string file offset */
#endif
	} ATYPE;

typedef struct line {
	short decflag;
#ifndef FLEXNAMES
	char name[LCHNM];
#else
	char *name;
#endif
	short nargs;
	short fline;
	ATYPE type;
	} LINE;

union rec {
	struct line l;
	struct {
		short decflag;
#ifndef FLEXNAMES
		char fn[LFNM];
#else
		char *fn;
#endif
		} f;
	};

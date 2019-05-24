/*
 * adb - vax string table version; common definitions
 */

#include <ctype.h>

#include "mtype.h"

#define	DBNAME	"adb\n"

typedef	char	BOOL;
typedef	char	MSG[];

#define	NVARS	36
#define MAXOFF	255
#define MAXPOS	80
#define MAXLIN	128
#define	ARB	512
#define MAXCOM	64
#define MAXARG	32
#define LINSIZ	512
#define	MAXSYM	255

#define EOR	'\n'
#define SP	' '
#define TB	'\t'
#define QUOTE	0200

#define STRIP	0177
#define LOBYTE	0377
#define EVEN	(~1)

#define	STDIN	0
#define	STDOUT	1

#define	NULL	0

#define	TRUE	(-1)
#define	FALSE	0

/*
 * `space' tags: qualify addresses
 */

#define	SPTYPE	077

#define	NOSP	00	/* nowhere (= command) */
#define	DATASP	01	/* data space */
#define	INSTSP	02	/* instruction space */
#define	UBLKSP	03	/* user block (saved registers) */
#define	REGSP	04	/* registers */

#define	CORF	0	/* in core image */
#define	SYMF	0100	/* in symbol file */

#define	RAWADDR	0200	/* ignore kernel mapping for this address */

/*
 * breakpoints
 */

#define	BKPTCLR	0	/* not a real breakpoint */
#define BKPTSET	1	/* real, ready to trap */
#define BKPTSKIP 2	/* real, skip over it next time */

struct bkpt {
	ADDR	loc;
	WORD	ins;
	int	count;
	int	initcnt;
	int	flag;
	char	comm[MAXCOM];
	struct bkpt *nxtbkpt;
};
typedef struct bkpt	BKPT;

/*
 * run modes
 */

#define	SINGLE	1
#define	CONTIN	2

/*
 * file address maps
 * each open file has one per segment
 * if b <= address <= e, address is valid in space type sp
 * and may be found at address + f in the file
 */

#define	NMAP	5	/* text data stack u-area endmarker */

struct map {
	ADDR	b;		/* base */
	ADDR	e;		/* end */
	ADDR	f;		/* offset within file */
	short	sp;		/* type of space */
	short	flag;
};
typedef	struct map	MAP;
#define	MPINUSE	01

/*
 * common globals
 */

extern	WORD	expv, adrval;
extern	int	expsp;
extern	int	adrflg;
extern	WORD	cntval;
extern	int	cntflg;
extern	WORD	loopcnt;
extern	ADDR	maxoff;
extern	ADDR	localval;
extern	ADDR	maxfile;
extern	ADDR	maxstor;

extern	ADDR	dot;
extern	int	dotsp;
extern	WORD	dotinc;

extern	WORD	var[];

extern	int	xargc;

extern	BOOL	wtflag;
extern	char	*corfil, *symfil;
extern	int	fcor, fsym;
extern	MAP	cormap[], symmap[];
extern	BOOL	mkfault;

extern	int	pid;
extern	int	signo;
extern	int	sigcode;

extern	char	*errflg;

/* result type declarations */

struct	nlist *symtab, *esymtab;
struct	nlist *cursym;
struct	nlist *lookup();

ADDR	inkdot();
char	*exform();
WORD	round();
BKPT	*scanbkpt();
WORD	findsym();

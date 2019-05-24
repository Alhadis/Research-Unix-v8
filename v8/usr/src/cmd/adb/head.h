/*
 * common globals
 */

extern	WORD	expv, adrval;
extern	int	adrflg;
extern	WORD	cntval;
extern	int	cntflg;
extern	WORD	loopcnt;
extern	ADDR	maxoff;
extern	ADDR	localval;
extern	ADDR	maxfile;
extern	ADDR	maxstor;

extern	ADDR	dot;
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


struct	nlist *symtab, *esymtab;
struct	nlist *cursym;
struct	nlist *lookup();

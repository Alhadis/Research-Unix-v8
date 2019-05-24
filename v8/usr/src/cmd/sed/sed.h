#
/*
 * sed -- stream  editor
 *
 *
 */

#define CBRA	1
#define	CCHR	2
#define	CDOT	4
#define	CCL	6
#define	CNL	8
#define	CDOL	10
#define	CEOF	11
#define CKET	12
#define CNULL	13
#define CLNUM	14
#define CEND	16
#define CDONT	17
#define	CBACK	18

#define	STAR	01

#define NLINES	256
#define	DEPTH	20
#define PTRSIZE	512
#define RESIZE	10000
#define	ABUFSIZE	20
#define	LBSIZE	4000
#define	LABSIZE	50
#define NBRA	9

FILE	*fin;
union reptr	*abuf[ABUFSIZE];
union reptr **aptr;
char	*lastre;
char	ibuf[512];
char	*cbp;
char	*ebp;
char	genbuf[LBSIZE];
char	*loc1;
char	*loc2;
char	*locs;
char	seof;
char	*reend;
char	*lbend;
char	*hend;
char	*lcomend;
union reptr	*ptrend;
int	eflag;
int	dolflag;
int	sflag;
int	jflag;
int	numbra;
int	delflag;
long	lnum;
char	linebuf[LBSIZE+1];
char	holdsp[LBSIZE+1];
char	*spend;
char	*hspend;
int	nflag;
int	gflag;
char	*braelist[NBRA];
char	*braslist[NBRA];
long	tlno[NLINES];
int	nlno;
char	fname[20][40];
FILE	*fcode[20];
int	nfiles;

#define ACOM	01
#define BCOM	020
#define CCOM	02
#define	CDCOM	025
#define	CNCOM	022
#define COCOM	017
#define	CPCOM	023
#define DCOM	03
#define ECOM	015
#define EQCOM	013
#define FCOM	016
#define GCOM	027
#define CGCOM	030
#define HCOM	031
#define CHCOM	032
#define ICOM	04
#define LCOM	05
#define NCOM	012
#define PCOM	010
#define QCOM	011
#define RCOM	06
#define SCOM	07
#define TCOM	021
#define WCOM	014
#define	CWCOM	024
#define	YCOM	026
#define XCOM	033

char	*cp;
char	*reend;
char	*lbend;

union	reptr {
	struct reptr1 {
		char	*ad1;
		char	*ad2;
		char	*re1;
		char	*rhs;
		FILE	*fcode;
		char	command;
		char	gfl;
		char	pfl;
		char	inar;
		char	negfl;
	} r1;
	struct reptr2 {
		char	*ad1;
		char	*ad2;
		union reptr	*lb1;
		char	*rhs;
		FILE	*fcode;
		char	command;
		char	gfl;
		char	pfl;
		char	inar;
		char	negfl;
	} r2;
} ptrspace[PTRSIZE], *rep;


char	respace[RESIZE];

struct label {
	char	asc[9];
	union reptr	*chain;
	union reptr	*address;
} ltab[LABSIZE];

struct label	*lab;
struct label	*labend;

int	f;
int	depth;

int	eargc;
char	**eargv;

extern	char	bittab[];

union reptr	**cmpend[DEPTH];
int	depth;
union reptr	*pending;
char	*badp;
char	bad;
char	*compile();
char	*ycomp();
char	*address();
char	*text();
char	*compsub();
struct label	*search();
char	*gline();
char	*place();
char	compfl;

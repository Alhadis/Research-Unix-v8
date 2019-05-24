 struct ELM {
	short transf;		/* next state function */
	short valtrans;		/* value transferred   */
 };

 struct IND {
	short nrpils;
	struct ELM *one;
 };

 struct VARPARS {
	short nrms,    *ms;		/* message parameters */
	short nrvs,    *vs;		/* pvar parameters    */
	short nrlvars, *lvarvals;	/* local variables    */
 };

 struct TBLPARS {
	short nrms;
	short nrvs;			/* max available space for params */	
	short nrlvars;			/* max available space for locvars */
 };

 struct LOCVARS {
	short nrlvars, *lvarvals;
 };

 struct CPARS {
	short callwhat;
	short nrms,    *ms;		/* message parameters */
	short nrvs,    *vs;		/* pvar parameters    */
 };

 struct LBT {
	int *mapcol, *orgcol;		/* mapped versions of colmap and colorg */
 };

/*
 *	endrow:		proper endstates of the process table
 *	deadrow:	set if deadend (eg return state)
 *	badrow:		row with at least one output option
 *	labrow:		labeled row (potential start of a loop)
 */

 struct TBL {
	int nrrows, nrcols;
	int *endrow, *deadrow, *badrow, *labrow;
	int *colmap, *colorg, *coltyp;

	struct IND **ptr;
	struct CPARS *calls;
 };

 struct MBOX {
	char limit;		/* length of queue */
	char owner;		/* the process reading from this queue */
	char qname[PLENTY];	/* user-level name for the queue */
 };

 struct MNAME {
	char mname[PLENTY];
 };

 struct BLTIN {
	short target;
	short type;
	short value;
 };

 struct REVPOL {
	char toktyp;
	short tokval;
 };

 struct PROCSTACK {
	short uptable;			/* table where we come from         */
	short uptransf;			/* pending transition in that table */

	struct VARPARS  *varparsaved;	/* saves parameters and locals */
	struct PROCSTACK *follow;	/* next stackframe */
 };

#define USED	32768
#define PASSED	16384

 struct QUEUE {
	short mesg;			/* on rcv PASSED is OR'ed in */
	unsigned short cargo;		/* when set USED is OR'ed in */
	struct QUEUE *next;
	struct QUEUE *last;
	struct QUEUE *s_forw;
	struct QUEUE *s_back;
 };

 struct CONTS {
	short mesg;
	unsigned short cargo;
 };

 struct TUPLE {
	short prev;
	short pmap;
 };

 struct STATE {
	short hash;
	int nrvisits;		/* nr of returns to this state (diff q's) */

	short *g_vars;			/* current globals */
	struct TUPLE *trip;		/* old states, and maps */
	struct LOCVARS   **l_vars;	/* current local vars */
	struct PROCSTACK **traceback;	/* traceback of complete stack */

	struct VISIT *next;	/* for loop analysis along a single path  */
 };

 struct STUFF {
	struct QUEUE *s;	/* pntr to last message sent  */
 };

 struct VISIT {
	char analyzed;		/* set when done with tree below this node */
	char howmany;		/* how many queues are nonempty when analyzed */
	struct {
		struct STUFF *h;	/* when searching, points into history */
		struct CONTS **c;	/* when analyzed, points to qcontents  */
	} prop;
	struct VISIT *next;
 };

/*
 *	normal ppushes (via forward()) save the parameters in the procstack
 *	so that the corresponding ppop (via backup()) can reset them properly
 *
 *	series of ppushes and ppops, performed in a single freeze() call
 *	(via the retable() and dotable() subroutines) are different:
 *
 *	additional ppushes are no problem, since they save the
 *	proper state information via the normal route
 *
 *	additional ppops however would lose the procstacked info
 *	that info is stored in the CUBE structure so that when
 *	a ppop from freeze() is undone in unfreeze()
 *	the proper state information can be restored onto the procstack
 */

 struct CUBE {
	short poporpush;
	short which;
	short procsaved;
	short transfsaved;

	struct VARPARS  *varparsaved;	/* parameter map and local var map */

	struct CUBE *pntr;
	struct CUBE *rtnp;
 };

 struct FREEZE {
	short lastsav;
	short *statsaved;		/* save states of tables */
	short *varsaved;		/* save global variables */

	short whichvar;			/* set if a var changed */
	short oldvalue;			/* of that loc var      */

	struct CUBE *cube;		/* place to save stackframe */
 };

#define NSPOKES	10

 struct Spoke {			/* spokes and wheels are used in trace8.c */
	struct STATE *st;
	struct VISIT *vi;
 };
 struct Wheel {
	struct Spoke spoke[NSPOKES];
	short n;
 };

 struct Swiffle {
	struct STATE *st;
	struct VISIT *vi;
	struct Swiffle *next;
	struct Swiffle *last;
 };

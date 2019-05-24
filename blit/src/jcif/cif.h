/*
 *  cif.h - header file for cif plotters
 */

#define BOX 1
#define WIRE 2
#define CALL 3
#define TRANS 4
#define METAL 5
#define DIFF 6
#define POLY 7
#define IMPLANT 8
#define CUT 9
#define GLASS 10
#define POLYGON 11
#define ZZZZ 12
#define POLYCON 13
#define ERRBOX	14	/* for DR to make constant size boxes */
#define ERRS	15	/* a layer, for a slightly different purpose */
#define MZERO 0100000   /* minus zero, used to delimit paths */
#define max(x,y) (x > y ? x : y)
#define min(x,y) (x < y ? x : y)

typedef struct {
    int x,y;
} Point;

typedef struct {
    Point origin,corner;
} Rectangle;

typedef struct {
    int t11,t12,t21,t22;
    int tx,ty;
} Transform;

typedef struct {
    Rectangle mbb;
    int pc;
    int refcnt;
} Symbol;

extern int scale,*bin,bincnt,symcnt,linecnt;
extern Symbol syms[];
extern Point thispt;
extern Rectangle thisrect;
extern Transform thistrans,nulltrans;
extern Point newpt(),transpt(),ptatbin(),minpt(),maxpt();
extern Rectangle newrect(),pttobox(),boxtorect(),transrect(),sortfrompt();
extern Rectangle rectatbin(),mbbpt(),mbbrect(),segtorect(),inset();
extern Transform transtrans(),newtrans(),transatbin();

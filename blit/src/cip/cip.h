#include <jerq.h>
#include <jerqio.h>
#include <font.h>

#define max(A, B)  ((A) > (B) ? (A) : (B))
#define min(A, B)  ((A) < (B) ? (A) : (B))
#define abs(A)  ((A)<0 ? -(A) : (A))
#define isletter(C) ((C>='a' && (C)<='z') || (C>='A' && (C)<='Z'))
#define isdigit(C) (C>='0' && (C)<='9')
#define ZERO Pt(0,0)

#define LW 5			/*  line width for frame boxes */
#define Xmin 10			/*  left edge of frame */
#define Xmax 790		/*  right edge of frame */
#define Ymin 10			/*  top of frame */
#define Ymax 1024		/*  bottom of frame */
#define YPIC 100		/*  top of drawing frame */
#define YBOT 950		/*  bottom of drawing frame */
#define YBR (YPIC-LW-LW)	/*  bottom of brush frame */
#define YCEN ((YBR+Ymin)>>1) 	/* center line of brush area */
#define Xbut (((Xmax-Xmin)<<1)/3)
#define XeditD ((Xmax-Xmin)/3)
#define Xtext Xmin
#define Ytext YBOT+(3*LW)-10
#define butHt min(((Ymax-YBOT-(LW<<2))/3), ((Xmax-Xbut-(3*LW))/18))

#define CIRCLE 0
#define BOX 1
#define ELLIPSE 2
#define LINE 3
#define ARC 4
#define SPLINE 5
#define TEXT 6
#define MACRO 7
#define NUMBR 7
#define DXBR ((Xmax-Xmin)/NUMBR)

#define PIC NUMBR
#define ED  PIC+1
#define BRUSH PIC+1
#define GROWCIRCLE BRUSH+1
#define MOVE BRUSH+2
#define GROWEWID BRUSH+3
#define GROWEHT BRUSH+4

#define RADdefault ((Xmax-Xmin)/24)
#define WIDdefault ((Xmax-Xmin)/8)
#define HTdefault ((Xmax-Xmin)/12)
#define nearlyStraight 3
#define nearEDGE 5
#define nearPT 8

#define SOLID 0
#define DASHED 1
#define DOTTED 2
#define startARROW 1
#define endARROW 2
#define doubleARROW 3

#define ROMAN 1
#define ITALIC 2
#define BOLD 3
#define CENTER 0
#define LEFTJUST 1
#define RIGHTJUST 2
#define POINTSIZE 10

#define GRIDon 1
#define GRIDoff 0

#define WHITEfield 1
#define BLACKfield 0

#define INITbuttons 0
#define DRAWbuttons 1
#define EDITbuttons 2
#define SPLINEbuttons 3
#define BLANKbuttons 4
#define MACRObuttons 5
#define COPYbuttons 6
#define MOVEbuttons 7
#define numButtonStates 8

#define fontBlk struct FONTBlk
struct FONTBlk {
	int ps, num, useCount;
	Font *f;
	fontBlk *next, *last;
};

typedef struct {
	Point start, end;
} pointPair;

typedef struct {
	int ht, wid;
} intPair;

typedef struct {
	int used, size;
	Point *plist
} pointStream;

typedef struct {
	int just;
	char *s;
	fontBlk *f;
} textString;

struct macro {
	char *name;
	int outName;
	int useCount;
	Rectangle bb;
	struct thing *parts;
	struct macro *next;
	struct macro *xReflectionOf, *yReflectionOf;
};

struct thing {
	short type;
	Point origin;	
	Rectangle bb;
	union {
		int brush;
		int radius;
		Point corner;
		Point end;
		pointPair arc;
		intPair	ellipse;
		textString text;
		pointStream spline;
		struct macro *list;
	} otherValues;
	int arrow, border;
	struct thing *next, *last, *sticky;
};

extern fontBlk *fonts;

extern Rectangle *select();
extern Rectangle moveBox();
extern Rectangle macroBB();

extern struct macro *recordMacro();

extern struct thing *newCircle();
extern struct thing *newBox();
extern struct thing *newEllipse();
extern struct thing *newLine();
extern struct thing *newArc();
extern struct thing *newText();
extern struct thing *newSpline();
extern struct thing *newMacro();
extern struct thing *selectThing();
extern struct thing *copyThing();
extern struct thing *deleteThing();
extern struct thing *deleteAllThings();
extern struct thing *insert();
extern struct thing *remove();
extern struct thing *doMouseButtons();
extern struct thing *place();
extern struct thing *displayCommandMenu();
extern struct thing *displayThingMenu();
extern struct thing *doGet();
extern struct thing *doClear();
extern struct thing *defineMacro();
extern struct thing *makeRelative();
extern struct thing *reflect();

extern Point track();
extern Point track2();
extern Point computeArcOrigin();
extern Point near();
extern Point jchar();

extern char *getString();

extern FILE *popen();

extern fontBlk *findFont();
extern fontBlk *insertFont();

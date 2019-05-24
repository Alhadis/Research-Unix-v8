#include <jerq.h>
#include <font.h>
#define	MAXLINES	(YMAX/14)	/* 14==newlnsz */

typedef struct String{
	char *s;	/* pointer to string */
	short n;	/* number used, s[n] == 0 */
	short size;	/* size of allocated area */
} String;

typedef unsigned char Nchar;	/* number of chars on a line */
typedef struct Textframe{
	Rectangle rect;		/* Screen area of frame, exact #lines high */
	Rectangle scrollrect;	/* Screen area of scrollbar */
	Rectangle totalrect;	/* Screen area covered by entire frame */
	String 	*str;		/* What's in the frame */
	int	s1, s2;		/* Indexes of ends of selected text */
	int	scrolly;	/* last argument to tellseek, for redrawing */
	char	file;		/* Which file associated with frame */
	char	obscured;	/* Whether another frame has covered part of this */
	int	selecthuge;	/* selection is bigger than visible */
	int	nlines;		/* Number of screen lines of text */
	Nchar	cpl[MAXLINES];	/* Number of characters per line */
} Textframe;

#define	NFRAME	20	/* 19 files plus diagnostic frame */
#define	NMAGICSTRING	2
#define	NSTRING	(NFRAME + NMAGICSTRING)
extern Textframe frame[NFRAME];
#define	TEXT(i)	(&_string[i])
#define	BUF	(TEXT(NFRAME+0))
#define	TYPEIN	(TEXT(NFRAME+1))
String _string[NSTRING];
#define	SCROLLWIDTH	10	/* width of scroll bar */
#define	M	2	/* margin inside frame border */

extern Textframe *current,*newframe();
extern Rectangle canon();
extern Point nullpoint,toscreen(),ptofchar(), startline();
extern void oprectf();
extern short newlnsz;
extern Point endpoint;	/* last position drawn during a frameop() */
extern complete;	/* did frameop do all it was supposed to? */
extern inscomplete;	/* is full insertion visible on screen? */
extern F_rectf;		/* function code for oprectf */
extern void opnull();	/* do nothing routine for frameop side effects */
extern void opdraw();	/* standard routine to draw the text */

#define	D	(&display)
#define	TRUE	1
#define	FALSE	0
#define	cwidth(c)	defont.info[c].width

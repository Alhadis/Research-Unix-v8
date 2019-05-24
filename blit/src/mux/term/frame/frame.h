#define	MAXLINES	(YMAX/16)	/* 16==newlnsz */
#define	MINCHARS 64	/* for efficiency; min size of a string we'll alloc */

typedef struct String{
	char *s;	/* pointer to string */
	short n;	/* number used, s[n] == 0 */
	short size;	/* size of allocated area */
} String;

typedef unsigned char Nchar;	/* number of chars on a line */
typedef struct Frame{
	Rectangle rect;		/* Screen area of frame, exact #lines high */
	Rectangle scrollrect;	/* Screen area of scrollbar */
	Rectangle totalrect;	/* Screen area covered by entire frame */
	String 	*str;		/* What's in the frame */
	int	s1, s2;		/* Indexes of ends of selected text */
	Point	scroll;		/* Scroll bar, 0<=(x=top),(y=bot)<=4096 */
	int	nlines;		/* Number of screen lines of text */
	Nchar	cpl[MAXLINES];	/* Number of characters per line */
} Frame;

#define	SCROLLWIDTH	10	/* width of scroll bar */
#define	SCROLLRANGE	4096	/* range of scrolling parameter */
#define	M		2	/* margin inside frame border */

#define	D		(P->layer)
#define	TRUE		1
#define	FALSE		0
#define	cwidth(c)	defont.info[c].width

extern Frame 		*newframe();
extern Rectangle	canon();
extern Point		nullpoint;
extern Point		toscreen(),ptofchar(), startline();
extern void		oprectf();
extern void		opclear();
extern short		newlnsz;
extern Point		endpoint;	/* last position drawn during a frameop() */
extern int		complete;	/* did frameop do all it was supposed to? */
extern int		inscomplete;	/* is full insertion visible on screen? */
extern int		F_rectf;	/* function code for oprectf */
extern void		opnull();	/* nop routine for frameop side effects */
extern void		opdraw();	/* standard routine to draw the text */
char	*Ualloc();

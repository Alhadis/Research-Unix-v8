/*	jf.h	*/

#include <jerq.h>
#include <jerqio.h>
#include <font.h>

#define UP	0	/* button state */
#define DOWN	1	/* button state */

#define MFDISP	5	/* maximum number of open fonts */
#define MEDISP	10	/* maximum number of open characters */

#define WBORD	2	/* size of border around character (fontdisp) */
#define WMARG	1	/* size of margin inside border (fontdisp) */

#define EDSIZE	8	/* size of edit pixel */

#define CNULL	(char *) 0
#define TNULL	(Texture *) 0
#define FNULL	(Font *) 0
#define FDNULL	(Fontdisp *) 0
#define EDNULL	(Editdisp *) 0

#define drstore(r)	rectf(&display,r,F_STORE)
#define drflip(r)	rectf(&display,r,F_XOR)
#define drclr(r)	rectf(&display,r,F_CLR)
#define drstring(s,p)	string(&defont,s,&display,p,F_XOR)

#define Fontdisp	struct _Fontdisp
#define Editdisp	struct _Editdisp

struct _Fontdisp {
	Rectangle r;	/* rectangle containing display */
	Font *fp;	/* pointer to displayed font */
	Editdisp *edp;	/* pointer to first edit field (if any) */
	int mwidth;	/* maximum character width */
	int cbx, cby;	/* size of small box around each character */
	int ncpl;	/* number of characters per display line */
	char *filnam;	/* source and/or destination file */
};

struct _Editdisp {
	Rectangle r;	/* rectangle containing display */
	Fontdisp *fdp;	/* pointer to originating font */
	Editdisp *edp;	/* pointer to next edit field (if any) */
	int size;	/* expansion factor */
	int c;		/* character code */
};

struct _Mousetrack {
	Fontdisp *fdp;	/* font on which mouse is sitting */
	int c;		/* character on which mouse is sitting */
	Editdisp *edp;	/* mouse's edit display area */
	Point pxl;	/* mouse's pixel */
};

extern Texture menu3, deadmouse, target, skull;

#ifdef MAIN

Fontdisp fdisp[MFDISP] = { 0 };
Editdisp edisp[MEDISP] = { 0 };
int nedisp=0;

struct _Mousetrack mtk;

Rectangle rkbd;
Point pkbd;
FILE *filep;

Texture lrarrow = {
	0xC007, 0x4001, 0x4007, 0x4004, 0xE007, 0x0000, 0x0810, 0x1818,
	0x300C, 0x6006, 0xFFFF, 0xFFFF, 0x6006, 0x300C, 0x1818, 0x0810,
};

Texture udarrow = {
	0xC180, 0x43C0, 0x47E0, 0x4DB0, 0xE990, 0x0180, 0x0180, 0x0180,
	0x0180, 0x0180, 0x0180, 0x0997, 0x0DB1, 0x07E7, 0x03C4, 0x0187,
};

Texture flarrow = {
	0x0018, 0x003C, 0x007E, 0x00DB, 0x1018, 0x3018, 0x6018, 0xFF18,
	0xFF00, 0x6007, 0x3001, 0x1607, 0x0204, 0x0207, 0x0200, 0x0700,
};

Texture trarrow = {
	0xC0FF, 0x403F, 0x400F, 0x401F, 0xE03B, 0x0073, 0x00E1, 0x01C1,
	0x8380, 0x8700, 0xCE00, 0xDC07, 0xF801, 0xF007, 0xFC04, 0xFF07,
};

Texture *arrows[] = { &lrarrow, &udarrow, &flarrow, &trarrow };

Texture widthmark = {
	0x0180, 0x0180, 0x0180, 0x019C, 0x0184, 0x019C, 0x8190, 0x819C,
	0x8180, 0x8180, 0x8000, 0x9800, 0x8800, 0x8800, 0x8800, 0x9C00,
};

#else

extern Fontdisp fdisp[];
extern Editdisp edisp[];
extern int nedisp;

extern struct _Mousetrack mtk;

extern Rectangle rkbd;
extern Point pkbd;
extern FILE *filep;

extern Texture lrarrow, udarrow, flarrow, trarrow, *arrows[], widthmark;

#endif

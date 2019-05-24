#include "frame/frame.h"
#include "msgs.h"
#include "menu.h"
extern Rectangle diagrect;
extern int diagclr;
extern short newlnsz;
extern Textframe *current;	/* which guy we are typing at (can be DIAG) */
extern Textframe *workframe;	/* which guy we are working on (never DIAG) */
extern int diagdone;
extern int diagnewline;
extern int snarfhuge;

#define	DIAG	(&frame[0])	/* must be the zero'th one */

#define	PRIME		0
#define	STARDOT		1

Textframe *frameoffile(), *pttoframe();
char *GCalloc();

Rectangle screenrect;
#define	UP	0
#define	DOWN	1

/*
 * jerq
 */
typedef struct Point{
	short x;
	short y;
}Point;
typedef struct Rectangle{
	Point origin;
	Point corner;
}Rectangle;
typedef short Word;
typedef unsigned short UWord;
typedef struct Bitmap{
	Word	*base;		/* pointer to start of data */
	unsigned width;		/* width in words of total data area */
	Rectangle rect;		/* rectangle in data area, local coords */
	char *_null;		/* unused, always zero */
}Bitmap;
typedef struct Menu{
	char **item;		/* string array, ending with 0 */
	short n;		/* number of items, deduced by menuhit */
	short lasty;
	Bitmap *b;		/* storage for menu and overlay */
} Menu;
typedef struct Texture{
	Word bits[16];
}Texture;
typedef int Code;
#define	WORDSHIFT	4
#define	WORDSIZE	16
#define	WORDMASK	(WORDSIZE-1)
#define	ONES		0xFFFF
#define	FIRSTBIT	((unsigned)0x8000)
#define	LASTBIT		((unsigned)0x1)
#define	XMAX	800
#define	YMAX	1024
#define	muldiv(a,b,c)	((short)((a)*((long)b)/(c)))
extern struct Mouse{
	Point	xy,jxy;
	short	buttons;
}mouse;
#define button(i)	(mouse.buttons & (8 >> i))
#define button1()	(mouse.buttons&4)
#define button2()	(mouse.buttons&2)
#define button3()	(mouse.buttons&1)
#define button12()	(mouse.buttons&6)
#define button13()	(mouse.buttons&5)
#define button23()	(mouse.buttons&3)
#define button123()	(mouse.buttons&07)

#define	DADDR	((short *)(384*1024L+030))	/* display base address/4 */
#define	DSTAT	((short *)(384*1024L+040))
#define	YMOUSE	((unsigned *)(384*1024L+000))
#define	XMOUSE	((unsigned *)(384*1024L+002))
extern short topbits[], botbits[];
extern Rectangle Jrect;
extern Bitmap display;
/*
 * Function Codes
 */
#define	F_STORE	((Code) 0)	/* target = source */
#define	F_OR	((Code) 1)	/* target |= source */
#define	F_CLR	((Code) 2)	/* target &= ~source */
#define	F_XOR	((Code) 3)	/* target ^= source */

Point add(), sub(), mul(), div(), jstring(), string();
Rectangle rsubp(), raddp(), inset();
Word *addr();
#define	Pt(x, y)	(short)(x), (short)(y)
#define	Rpt(x, y)	x, y
#define	Rect(a, b, c, d)	(short)(a), (short)(b), (short)(c), (short)(d)
char *alloc(), *gcalloc();
Bitmap *balloc();
Texture *cursswitch();
#define	NULL	((char *)0)
#define	KBD	1
#define	SEND	2
#define	MOUSE	4
#define	RCV	8
#define	CPU	16
#define ALARM	32

#ifdef	MPX
#include <mpx.h>
Rectangle Drect;
#else
#ifndef	MPXTERM
#define	sw(n)
#define	request(n)
#define sleep(n) nap(n)
#define	exit()	reboot()
#define	transform(p)	(p)
#define	rtransform(r)	(r)
#endif
#define	Drect	Jrect
#define	Jcursinhibit()	cursinhibit()
#define	Jcursallow()	cursallow()
#endif
#ifdef	lint
#undef	Rect
#undef	Rpt
#undef	Pt
#undef	sleep
#undef	request
#undef	sw
#undef	unblock
extern	Point Pt();
extern	Rectangle Rpt(), Rect();
#endif

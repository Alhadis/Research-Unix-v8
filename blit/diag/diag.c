#include <jerq.h>
#include <font.h>
#include "/usr/jerq/include/acia.h"
#define	kbd_data	((char *)(384*1024L+062))
#define	kbd_stat	((char *)(384*1024L+060))
#define sio_data	((char *)(384*1024L+013))
#define sio_stat	((char *)(384*1024L+011))
#define	stripe(x)	bit=(unsigned long)0x80000000>>x;\
			p=(long *)0x6000;\
			i=25L*1024L;\
			do *p++=bit; while(--i);
#define	clear()		p=(long *)0x6000;\
			i=25L*1024L;\
			do *p++=0; while(--i);
#define	bstripe(x)	bit=0x80>>(x&7);\
			cp=(char *)(0x6000 + (x>>3));\
			i=25L*1024L;\
			do {*cp=bit; cp+=4;} while(--i);
#define	pause()		i=200000; do; while(--i);
Bitmap display = {
	(Word * )0x6000, XMAX >> WORDSHIFT, 0, 0, XMAX, YMAX
};


/*
 *	Because we're testing it, main() can't use
 *	memory until it's known to be OK.  Therefore...
 */
main()
{
	register long	i, *p;
	register x, y, j, dj;
	register unsigned long	bit;
	register char	*cp;

	STARTOFMAIN();
Loop:
	*DSTAT = 0;		/* initialize display */
	*DADDR = (0x6000) / 4;
	*kbd_stat = A_RESET;		/* intialize keyboard */
	*kbd_stat = A_S2NB8 | A_CDIV16 | A_RSBLTD;
	clear();
	/* Munching squares, 'cause it's traditional */
	p = (long *)0x6000;

	for (dj = 1; ; ) {
		j = 0;
		do {
			if ((*kbd_stat & A_RDRF) && (*kbd_data == '\r'))
				goto Proceed0;
			for (x = 0; x < XMAX; x++) {
				y = (x ^ j) & (YMAX - 1);
				cp = (char *)(p + (y * 25)) + (x >> 3);
				*cp ^= (0x80 >> (x & 7));
			}
			j += dj;
		} while (j != dj * YMAX);
prime:
		dj++;
		for (j = 2; j < dj; j++)
			if (j*(dj/j) == dj)
				goto prime;
	}
Proceed0:
	clear();
	*DSTAT = 1;
	/* Chip test; long word writes */
	for (; ; )
		for (j = 0; j < 32; j++) {
			if ((*kbd_stat & A_RDRF) && (*kbd_data == '\r'))
				goto Proceed1;
			stripe(j);
			pause();
			clear();
		}
Proceed1:
	clear();
	/* Byte write test */
	for (; ; )
		for (j = 0; j < 32; j++) {
			if ((*kbd_stat & A_RDRF) && (*kbd_data == '\r'))
				goto Proceed2;
			bstripe(j);
			pause();
			clear();
		}
Proceed2:
	clear();
	/*
	 * Memory's got the OK; subroutines are allowed.
	 * Note that all writable memory hangs off local (stack) storage.
	 */
asm("	mov.l	&256*1024-4, %sp	# set up stack");
	/* Do the rest using local variables */
	autotest();
asm("	mov.w	&0x2700, %sr");	/* spl7(); */
	goto Loop;
}


Clear()
{
	register long	*p;
	register i;
	clear();
	pause();
}


#define	KBDC	(*((char *)(256+4)))	/* A global, eh? */
#define	INTERRUPT	(*((int *)(256+6)))	/* A global, eh? */
/*
 * Each remaining phase is terminated by a carriage return
 */
autotest()
{
	int	i, j;
	int	kbdchar(), intkbdchar(), always(), sixtyhz();
	/* send some cookies to the kbd_data */
	putkbd(2);
	for (i = 0; i < 128; i++) {
		putkbd((i << 1) | 1);
	}
	putkbd(1);
	putkbd(4);
	herald("keyboard");
	kbdtest(kbdchar, Pt(5, 30));	/* No interrupts */
	/* Set up vector */
	intkbdinit();
	herald("int-keyboard");
	kbdtest(intkbdchar, Pt(5, 30));	/* Interrupts */
	herald("mouse");
	mousetest(always, 'A');	/* No interrupts */
	Clear();
	intmouseinit();
	herald("int-mouse");
	mousetest(sixtyhz, 'B');	/* Interrupts */
	Clear();
	bounce1();
	Clear();
}


herald(s)
char	*s;
{
	string(&defont, s, &display, Pt(5, 10), F_XOR);
}


putkbd(c)
{
	int	i;
	while ((*kbd_stat & A_TDRE) == 0)
		;
	*kbd_data = c;
}


bounce1() 
{
#define SIZE	47
	int	XSIZE = Drect.corner.x-SIZE, YSIZE = Drect.corner.y-SIZE;
	int	x, y, xinc, yinc, i;
	xinc = 1;
	yinc = 1;
	rectf(&display, Drect, F_XOR);
	x = Drect.origin.x;
	y = Drect.origin.y;
	for (KBDC = 0; KBDC != '\r'; ) {
		rectf(&display, Rect(x, y, x + SIZE, y + SIZE), F_XOR);
		x = x + xinc;
		if ((x == XSIZE) || (x == Drect.origin.x)) 
			xinc = -xinc;
		y = y + yinc;
		if ((y == YSIZE) || (y == Drect.origin.y)) 
			yinc = -yinc;
	}
}


kbdtest(get, curpos)
int	(*get)();
Point curpos;
{
	register c;
	char	s[2];
	s[1] = 0;
	/* Keyboard test: put line of text on screen until CR */
	while ((c = (*get)()) != '\r') {
		s[0] = c;
		drawchar(c, curpos);
		curpos.x += strwidth(&defont, s);
	}
	Clear();
}


kbdchar()
{
	do;
	while ((*kbd_stat & A_RDRF) == 0)
		;
	return * kbd_data & 0xFF;
}


intkbdinit()
{
asm("	mov.l	&Auto2, 0x68");	/* Vector 2 */
	*kbd_stat = A_RESET;
	*kbd_stat = A_S1NB8 | A_CDIV16 | A_RE;
	KBDC = 0;
	binit();	/* Gotta do this before dropping to spl1() */
asm("	mov.w	&0x2100, %sr");	/* spl1() */
}


auto2()
{
	KBDC = *kbd_data;
}


intkbdchar()
{
	register c;
	do;
	while (KBDC == 0)
		;
	c = KBDC;
	KBDC = 0;
	return c;
}


always()
{
	register j = 20000;
	do;
	while (j--)
		;	/* kill time */
	return;
}


sixtyhz()
{
	INTERRUPT = 0;
	do;
	while (!INTERRUPT)
		;
	return;
}


intmouseinit()
{
asm("	mov.l	&Auto1, 0x64");	/* Vector 1 */
asm("	mov.w	&0x2000, %sr");	/* spl0() */
}


auto1()
{
asm("	mov.w	&0, 384*1024+070");	/* Turn off interrupt */
	INTERRUPT = 1;
}


mousetest(wait, c)
int	(*wait)();
{
	register oldx = 10, oldy = 10, xm, ym;
	drawchar(c, Pt(oldx, oldy));
	for (KBDC = 0; KBDC != '\r'; (*wait)()) {
		xm = *XMOUSE & 1023;
		ym = *YMOUSE & 1023;
		if (xm < 0 || ym < 0)	/* bad data */
			continue;
		drawchar(c, Pt(oldx, oldy));
		if (xm < 10)
			xm = 10;
		if (ym < 10)
			ym = 10;
		if (xm > XMAX - 10)
			xm = XMAX - 10;
		if (ym > YMAX - 10)
			ym = YMAX - 10;
		xm = XMAX - xm;
		ym = YMAX - ym;
		drawchar(c, Pt(xm, ym));
		oldx = xm;
		oldy = ym;
	}
	drawchar(c, Pt(oldx, oldy));
}


drawchar(c, p)
int	c;
Point p;
{
	char	s[2];
	s[0] = c;
	s[1] = 0;
	string(&defont, s, &display, p, F_XOR);
}


#define	BUTADDR	(384*1024L+021)
binit()
{
	char	c;
asm("	mov.l	&Auto4, 0x70");
}


auto4()
{
	register buttons;
	buttons = *((char *) BUTADDR) & 0xFF;
	rectf(&display, Rect(0, 30, 40, 50), F_CLR);
	if (buttons & 1)
		drawchar('3', Pt(5, 30));
	else if (buttons & 2)
		drawchar('2', Pt(5, 30));
	else if (buttons & 4)
		drawchar('1', Pt(5, 30));
}


asm("Auto1:");
asm("	movm.l	&0xC0C0, -(%sp)");
asm("	jsr	auto1");
asm("	movm.l	(%sp)+, &0x0303");
asm("	rte");
asm("Auto2:");
asm("	movm.l	&0xC0C0, -(%sp)");
asm("	jsr	auto2");
asm("	movm.l	(%sp)+, &0x0303");
asm("	rte");
asm("Auto4:");
asm("	movm.l	&0xC0C0, -(%sp)");
asm("	jsr	auto4");
asm("	movm.l	(%sp)+, &0x0303");
asm("	rte");
STARTOFMAIN()
{
}		/* Used by the code generator */



#include "style.h"
#include "pacman.h"

/*	PIECES COMMENTED OUT TO SAVE SPACE */
#ifdef BLIT
#include <font.h>

Code textmode;
Point curloc;

showchar(c)
char c;
{
	static char str[] = "x";

	str[0] = c;
	curloc = string(&defont,str,&display,curloc,textmode);
}

movexy(x,y)
int	x,y;
{
	curloc.x = x*9+40;
	curloc.y = y*16+128;
}
#endif

/*VARARGS1*/
printf(fmt, x1)
char *fmt;
unsigned x1;
{
    extern int showchar();

    prf(fmt, &x1, showchar);
}


/*
 * Scaled down version of C Library printf.
 */

prf(fmt, adx, pfunc)
register char *fmt;
register int  *adx;
int          (*pfunc)();
{
	register int b, c, i, len;
	char *s;
	int zfill;
	int any;

loop:
	while ((c = *fmt++) != '%') {
		if(c == '\0')
			return;
		(*pfunc)(c);
	}
	len = 0;
	zfill = 0;

again:
	c = *fmt++;

	switch (c) {

/*
	case 'x': case 'X':
		b = 16;
		goto number;
	case 'o': case 'O':
		b = 8;
		goto number;
	case 'D':
	case 'u':
*/
	case 'd':
		b = 10;
number:
		printn((long)*adx, b, len, zfill, pfunc);
		break;
	case 'l':
		b = 10;
		printn(*((long *)(adx++)), b, len, zfill, pfunc);
		break;
/*
	case 'c':
		b = *adx;
		for (i = 24; i >= 0; i -= 8)
			if (c = (b >> i) & 0x7f)
				(*pfunc)(c);
		break;
	case 'b':
		b = *adx++;
		s = (char *)*adx;
		printn((long)b, *s++,0,0, pfunc);
		any = 0;
		if (b) {
			(*pfunc)('<');
			while (i = *s++) {
				if (b & (1 << (i-1))) {
					if (any)
						(*pfunc)(',');
					any = 1;
					for (; (c = *s) > 32; s++)
						(*pfunc)(c);
				} else
					for (; *s > 32; s++)
						;
			}
			if (any)
				(*pfunc)('>');
		}
		break;
*/
	case 's':
		s = (char *)*adx;
		while (c = *s++)
			(*pfunc)(c);
		break;

	case '0':
		if ( len==0 ) zfill = 1;
	case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		len = len*10 + (c-'0');
		goto again;




	case '%':
		(*pfunc)('%');
		break;
	}
	adx++;
	goto loop;
}

/*
 * Printn prints a number n in base b.
 * We don't use recursion to avoid deep kernel stacks.
 */
printn(n, b, len, zfill, pfunc)
register long n;
register int len;
int (*pfunc)();
{
	char prbuf[11];
	register char *cp;

	if (b == 10 && n < 0) {
		(*pfunc)('-');
		n = (unsigned)(-n);
	}
	cp = prbuf;
	do {
		*cp++ = "0123456789abcdef"[n%b];
		n /= b;
	} while (n);
	while ( len-- > cp-prbuf )
		if ( zfill )
			(*pfunc)('0');
		else
			(*pfunc)(' ');

	do
		(*pfunc)(*--cp);
	while (cp > prbuf);
}



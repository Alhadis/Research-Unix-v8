#include <jerq.h>
#include <font.h>
/*
 * viable terminal
 * layer|vitty:\
 * 	:am:cl=\014:ce=\EK:cm=\EY%r%+ %+ :co#80:li#72:pt:bs:al=\EI:dl=\ED:\
 * 	:vb=\E^G:up=\013:
 */
/* Font properties */
#define	CW	9	/* width of a character */
#define	NS	16	/* newline size; height of a character */
#define	CURSOR	"\1"	/* By convention */

#define	XMARGIN	3	/* inset from border of layer */
#define	YMARGIN	3

int	x, y;	/* character positions */
int	xmax, ymax;
Point	  pt();

main()
{
	char buf[2];

	buf[1] = 0;
	request(RCV);
	xmax = (Drect.corner.x-Drect.origin.x-2*XMARGIN)/CW-1;
	ymax = (Drect.corner.y-Drect.origin.y-2*YMARGIN)/NS-1;
	x = 0;
	y = 0;
	curse();
	for(;;) {
		wait(RCV);
		curse();
	Nocurse:
		buf[0] = rcvchar();
		switch(buf[0]) {

		case '\007':		/* bell */
			*((char *)(384*1024L+062)) = 2;
			break;

		case '\013':		/* up */
			if (y > 0)
				y--;
			break;

		case '\t':		/* tab modulo 8 */
			x = (x|7)+1;
			break;

		case '\033':
			switch(getchar()) {

			case '\007':	/* visible bell */
				rectf(&display,Drect,F_XOR);
				rectf(&display,Drect,F_XOR);
				break;

			case 'Y':	/* position cursor */
				x = getchar()-' ';
				y = getchar()-' ';
				if (x<0)
					x = 0;
				if (x > xmax)
					x = xmax;
				if (y < 0)
					y = 0;
				if (y > ymax)
					y = ymax;
				break;

			case 'I':	/* insert blank line */
				scroll(y, ymax, y+1, y);
				break;

			case 'D':	/* delete line */
				scroll(y+1, ymax+1, y, ymax);
				break;

			case 'K':	/* clear to EOL */
				stipple(Rpt(pt(x, y), pt(xmax+1, y+1)));
				break;
			}
			break;

		case '\b':		/* backspace */
			if(x > 0)
				--x;
			break;

		case '\n':		/* linefeed */
			newline();
			break;

		case '\r':		/* carriage return */
			x = 0;
			break;

		case '\014':		/* clear screen */
			stipple(Drect);
			x = 0;
			y = 0;
			break;

		default:		/* ordinary char */
			/* chars are drawn at baseline; hence y+1 */
			string(&defont, buf, &display, pt(x, y), F_STORE);
			x++;
			break;
		}
		if(x > xmax) {
			x = 0;
			newline();
		}
		if(own()&RCV)
			goto Nocurse;
		curse();
	}
}

newline()
{
	if(y >= ymax) {
		scroll(1, ymax+1, 0, ymax);
		y = ymax;
	} else
		y++;
}

curse()
{
	string(&defont, CURSOR, &display, pt(x, y), F_XOR);
}

getchar()
{
	wait(RCV);
	return rcvchar();
}

Point
pt(x, y)
register x, y;
{
	return add(Drect.origin, Pt(x*CW+XMARGIN, y*NS+YMARGIN));
}

scroll(sy, ly, dy, cy)	/* source, limit, dest, which line to clear */
{
	bitblt(&display, Rpt(pt(0, sy), pt(xmax+1, ly)),
	    &display, pt(0, dy), F_STORE);
	stipple(Rpt(pt(0, cy), pt(xmax+1, cy+1)));
}

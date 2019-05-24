#include <jerq.h>
#include <font.h>

#define SIGNED(ch)	((ch) & 0x80 ? (ch) | 0xffffff00 : (ch))

#define echo(ich,p)	bitblt(defont.bits,\
				Rect(ich->x,0,(ich+1)->x,defont.height),\
				&display,Pt(p.x+SIGNED(ich->left),p.y),F_XOR)

#define kbdcurs(p)	rectf(&display,Rect(p.x,p.y,p.x+1,p.y+defont.height),F_XOR)

kbdstring(str,nchmax,p)	/* read string from keyboard with echo at p */
register char *str; int nchmax; Point p;
{
	register int kbd, nchars = 0; register Fontchar *ich;

	*str = 0; kbdcurs(p);
	for (;;) {
		wait(KBD);
		kbdcurs(p);
		switch (kbd = kbdchar()) {
		case 0:
			break;
		case '\r':
		case '\n':
			return nchars;
		case '\b':
			if (nchars <= 0) break;
			kbd = *--str; *str = 0; nchars--;
			ich = defont.info + kbd;
			p.x -= ich->width;
			echo(ich, p);
			break;
		default:
			if (nchars >= nchmax) break;
			*str++ = kbd; *str = 0; nchars++;
			ich = defont.info + kbd;
			echo(ich, p);
			p.x += ich->width;
		}
		kbdcurs(p);
	}
}

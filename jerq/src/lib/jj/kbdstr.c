#include	<jerq.h>
#include	<font.h>

static void
getstr(s, p, start)
	char *s, *start;
	Point p;
{
	char c;
	static char str[] = "x";

	for(;;)
	{
		wait(KBD);
		if(((c=kbdchar()) == '\r') || (c == '\n'))
		{
			*s = '\0';
			return;
		}
		if(c == '\b')
		{
			if(s>start)
			{
				str[0] = *(--s);
				string(&defont,str,&display,(p = sub(p,Pt(9,0))),F_XOR);
			}
		}
		else if(c == '@')
		{
			if(s>start)
			{
				string(&defont,start,&display,(p = sub(p,Pt(9*(s-start),0))),F_XOR);
				s = start;
			}
		}
		else if((c >= ' ') && (c <= '~'))
		{
			if(s-start<50)
			{
				*s++ = (str[0] = c);
				p = string(&defont,str,&display,p,F_XOR);
			}
		}
	}
}

kbdstr(s)
	char *s;
{
	Bitmap *b;
	Point p;
	Rectangle r;

	p = mouse.xy;
	p.x = min(p.x, Drect.corner.x-300);
	b = balloc(raddp(Rect(0, 0, 300, 20), p));
	bitblt(&display, b->rect, b, b->rect.origin, F_STORE);
	rectf(&display, b->rect, F_STORE);
	rectf(&display, r = inset(b->rect, 1), F_XOR);
	p = add(r.origin, Pt(1,1));
	p = string(&defont, s, &display, p, F_XOR);
	getstr(s+strlen(s), p, s);
	bitblt(b, b->rect, &display, b->rect.origin, F_STORE);
	bfree(b);
}

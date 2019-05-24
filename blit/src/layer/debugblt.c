#include	<jerq.h>
#include	<layer.h>
#ifdef	DEBUG
#include	<font.h>
static rdump();
#endif	DEBUG

/*
	lbitblt:

*/

#define		MAXAREA		50

static lessthan();
typedef struct
{
	Bitmap *bp;	/* bit map */
	Rectangle rect;	/* rectangle to be copied */
} Area;
typedef struct
{
	Area list[MAXAREA];
	Area *next;
} List;
static List Dest, Src;
static Point delta;

static
Pass(lp, r, sb, ap, op)
	Layer *lp;
	Rectangle r;
	Bitmap *sb;
	register struct
	{
		List *l;
	} *ap;
	Obscured *op;
{
	register List *lp;
#ifdef	DEBUG
	char buf[300];

	nerk("Pass Rect");
	rdump(buf, r); nerk(buf);
#endif	DEBUG
	lp = ap->l;
	if(lp->next != &(lp->list[MAXAREA]))
	{
		register Area *n;

		n = lp->next;
		n->bp = sb;
		n->rect = r;
		for(n = lp->list; n != lp->next; n++)
			if(! lessthan(n, lp->next))
				break;
		if(n == lp->next)
			(lp->next)++;
		else
		{
			register Area *m;

			for(m = (lp->next)++; m != n; m--)
				*m = *(m-1);
			n->bp = sb;
			n->rect = r;
		}
	}
}

void
lbitblt(sl, rect, dl, pt, f)
	Layer *sl, *dl;
	Rectangle rect;
	Point pt;
	Code f;
{
	register Area *d, *s;
	Rectangle r;
#ifdef	DEBUG
	char buf[256];
	int debug = 0;

	rectf(sl, rect, F_XOR);
	while(!button123()) sleep(2);
	debug = button1();
	rectf(sl, rect, F_XOR);
#endif	DEBUG

	/*
		check for degenerate cases
	 */
	if(sl->obs == (Obscured *)0)
	{
		extern bitblt(), lblt();

		(*(dl->obs? lblt:bitblt))(sl, rect, dl, pt, f);
		return;
	}
	/*
		build up the source rectangle lists and destination
		rectangle lists
	*/

	delta = sub(pt, rect.origin);
	Dest.next = &(Dest.list[0]);
	layerop(dl, Pass, raddp(rect, delta), &Dest);
	Src.next = &(Src.list[0]);
	layerop(sl, Pass, rect, &Src);

#ifdef	DEBUG
	if(debug)
	{
		nerk("Src");
		for(s = Src.list; s != Src.next; s++)
		{
			rdump(buf, s->rect);
			nerk(buf);
		}
		nerk("Dest");
		for(s = Dest.list; s != Dest.next; s++)
		{
			rdump(buf, s->rect);
			nerk(buf);
		}
	}
#endif	DEBUG

	for(s = Src.list; s != Src.next; s++)
	{
		for(d = Dest.list; d != Dest.next; d++)
		{
			r = raddp(s->rect, delta);
			if(rectclip(&r, d->rect))
			{
#ifdef	DEBUG
				rectf(s->bp, rsubp(r, delta), F_XOR); nerk("src");
				rectf(s->bp, rsubp(r, delta), F_XOR);
				rectf(d->bp, r, F_XOR); nerk("dest");
				rectf(d->bp, r, F_XOR);
#endif	DEBUG
				bitblt(s->bp, rsubp(r, delta), d->bp, r.origin, F_STORE);
			}
		}
	}
}

static
lessthan(a, b)
	register Area *a, *b;
{
	register n;

	if((a->rect.origin.y < b->rect.corner.y) && (b->rect.origin.y < a->rect.corner.y))
		return(((a->rect.origin.x - b->rect.origin.x)*(long)delta.x) >= 0);
	else
		return(((a->rect.origin.y - b->rect.origin.y)*(long)delta.y) >= 0);
}

#ifdef	DEBUG
nerk(s)
	char *s;
{
	static Rectangle rr = { XMAX-500, YMAX-100, XMAX, YMAX };

	rectf(&display, rr, F_CLR);
	string(&defont, s, &display, Pt(XMAX-490, YMAX-60), F_XOR);
	while(! button1()) sleep(2);
	while(button1()) sleep(2);
}

static char *
dec(n, p)
	register n;
	register char *p;
{
	char buf[30];
	register char *s = buf;

	if(n < 0)
	{
		*p++ = '-';
		n = -n;
	}
	do
	{
		*s++ = (n%10) + '0';
		n /= 10;
	} while(n);
	do
	{
		*p++ = *--s;
	} while(s != buf);
	return(p);
}

static
rdump(buf, r)
	char *buf;
	Rectangle r;
{
	*buf++ = '(';
	buf = dec(r.origin.x, buf);
	*buf++ = ',';
	buf = dec(r.origin.y, buf);
	*buf++ = ')';
	*buf++ = ' ';
	*buf++ = '(';
	buf = dec(r.corner.x, buf);
	*buf++ = ',';
	buf = dec(r.corner.y, buf);
	*buf++ = ')';
	*buf = 0;
}
#endif	DEBUG

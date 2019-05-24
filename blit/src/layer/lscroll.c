#include <jerq.h>
#include <layer.h>
#define NEWLINESIZE 16	/* this number critical for shading */
/*
 * scroll layer up by NEWLINESIZE rasters
 */
lscroll(l)
	register Layer *l;
{
	Rectangle r;

	r = inset(l->rect, 2);
	r.origin.y += NEWLINESIZE;
	lbitblt(l, r, l, Pt(r.origin.x, r.origin.y-NEWLINESIZE), F_STORE);
}

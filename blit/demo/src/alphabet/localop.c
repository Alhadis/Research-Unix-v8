#include "alph.h"
localop() {
	Point p;
	register Bitmap *b;
	register int x0, y0;
	register int mask;

	if(param==0)
		mask=341;
	else
		mask=param;
	clearscreen();
	p=div(add(Drect.origin, Drect.corner), 2);
	b=balloc(Rpt(sub(p, Pt(128, 128)), add(p, Pt(128, 128))));
	if(b==0)
		return;
	rectf(b, b->rect, F_CLR);
	rectf(b, sub(p, Pt(1, 1)), add(p, Pt(1, 1)), F_XOR);
	x0=b->rect.origin.x;
	y0=b->rect.origin.y;
	while (!keyhit()) {
		bitblt(b, b->rect, &display, b->rect.origin, F_STORE);
		rectf(b, b->rect, F_CLR);
		if(mask & 1)
			bitblt(&display, b->rect, b, Pt(x0-1, y0  ), F_XOR);
		if(mask & 2)
			bitblt(&display, b->rect, b, Pt(x0-1, y0-1), F_XOR);
		if(mask & 4)
			bitblt(&display, b->rect, b, Pt(x0  , y0-1), F_XOR);
		if(mask & 8)
			bitblt(&display, b->rect, b, Pt(x0+1, y0-1), F_XOR);
		if(mask & 16)
			bitblt(&display, b->rect, b, Pt(x0+1, y0  ), F_XOR);
		if(mask & 32)
			bitblt(&display, b->rect, b, Pt(x0+1, y0+1), F_XOR);
		if(mask & 64)
			bitblt(&display, b->rect, b, Pt(x0  , y0+1), F_XOR);
		if(mask & 128)
			bitblt(&display, b->rect, b, Pt(x0-1, y0+1), F_XOR);
		if(mask & 256)
			bitblt(&display, b->rect, b, Pt(x0  , y0  ), F_XOR);
	}
	bfree(b);
}

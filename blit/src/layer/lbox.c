#include <jerq.h>
#include <layer.h>
#define	INSET	2
Lbox(l)
	register Layer *l;
{
	cursinhibit();
	lrectf(l, l->rect, F_STORE);
	lrectf(l, inset(l->rect, INSET), F_CLR);
	cursallow();
}
Lgrey(r)
	Rectangle r;
{
	static Word greymap[16]={
		0x1111,	0x4444,	0x1111,	0x4444,	0x1111,	0x4444,	0x1111,	0x4444,
		0x1111,	0x4444,	0x1111,	0x4444,	0x1111,	0x4444,	0x1111,	0x4444,
	};
	cursinhibit();
	texture(&display, r, greymap, F_STORE);
	cursallow();
}

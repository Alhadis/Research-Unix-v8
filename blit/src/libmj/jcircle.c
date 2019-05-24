#include <jerq.h>
extern Point Jtransform();
jcircle(p,r,f)
	Point p;
{
	circle(&display,transform(p),
		muldiv(r, Drect.corner.x-Drect.origin.x, XMAX),f);
}


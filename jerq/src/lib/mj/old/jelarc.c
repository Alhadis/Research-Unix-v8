#include <jerq.h>
extern Point Jtransform();
jelarc(p0, a, b, p1, p2, f)
	Point p0, p1, p2;
	int a, b, f;
{
	elarc(&display, transform(p0),
		muldiv(a, Drect.corner.x-Drect.origin.x, XMAX),
		muldiv(b, Drect.corner.y-Drect.origin.y, YMAX),
		transform(p1), transform(p2), f);
}

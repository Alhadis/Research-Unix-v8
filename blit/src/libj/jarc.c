#include <jerq.h>
jarc(p0, p1, p2, f)
	Point p0, p1, p2;
	Code f;
{
	arc(&display, p0, p1, p2, f);
}

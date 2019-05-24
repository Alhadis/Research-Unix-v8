#include <jerq.h>
jtexture(r, map, f)
	Rectangle r;
	Texture *map;
{
	texture(&display, r, map, f);
}

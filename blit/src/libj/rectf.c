#include <jerq.h>

static Texture ones={
	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,
	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,
};

jrectf(rec, f)
	Rectangle rec;
{
	extern Bitmap display;
	jtexture(rec, &ones, f);
}

rectf(bp, rec, f)
	Bitmap *bp;
	Rectangle rec;
{
	texture(bp, rec, &ones, f);
}

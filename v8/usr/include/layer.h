#ifndef LAYER_H
#define	LAYER_H	LAYER_H
typedef struct obscured Obscured;
typedef struct layer Layer;
struct obscured{
	Rectangle rect;	/* obscured rectangle, screen coords */
	Bitmap *bmap;	/* where the obscured data resides */
	Layer *lobs;	/* layer currently in rect on screen */
	Obscured *next;	/* chaining */
	Obscured *prev;
};

struct layer{
	/* The first part looks like (is) a Bitmap */
	Word *base;
	unsigned width;
	Rectangle rect;	/* bounding box of layer */
	Obscured *obs;	/* linked list of obscured rectangles */
	Layer *front;	/* adjacent layer in front */
	Layer *back;	/* adjacent layer behind */
};

Bitmap *balloc();
char *alloc();
Rectangle rsubp();
extern Layer *lfront, *lback;
Layer *newlayer();
#endif

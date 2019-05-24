#include <jerq.h>
#include <layer.h>

lpoint(l, p, f)
	register Layer *l;
	Point p;
	register Code f;
{
	register Obscured *o;
	if(ptinrect(p, l->rect)){
		for(o=l->obs; o; o=o->next)
			if(ptinrect(p, o->rect)){
				point(o->bmap, p, f);
				return;
			}
		point(l, p, f);
	}
}

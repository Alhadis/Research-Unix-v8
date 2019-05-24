#include <jerq.h>
point(b, pt, f)
	register Bitmap *b;
	Point pt;
{
	register bit;
	register Word *p=addr(b, pt);
	if(ptinrect(pt, b->rect)==0)
		return;
	bit=1<<(WORDSIZE-1)-(pt.x&WORDMASK);
	switch(f){
	case F_XOR:
		*p^=bit;
		break;
	default:
	case F_OR:
	case F_STORE:
		*p|=bit;
		break;
	case F_CLR:
		*p&=~bit;
		break;
	}
}

#include <jerq.h>

texture(bp, rec, map, f)	
	register Bitmap *bp;
	register Texture map;
	Rectangle rec;
{
	short x=rec.origin.x, y=rec.origin.y;
	register dx=rec.corner.x-rec.origin.x, dy=rec.corner.y-rec.origin.y;
	register i;	/* i is the inner loop count */
	register Word mask1, mask2;
	register Word *screenp, *sp;
	register bits;
	int bitindex;
	short s1, s2, ii;

	if(dx<=0)
		return;
	s1=x&WORDMASK;
	s2=(x+dx)&WORDMASK;
	mask1=~topbits[s1];
	mask2=s2==0? ONES : topbits[s2];
	screenp=addr(bp,rec.origin);

	/*
	 * ii is -1 if all in one word, == 0 if two adjacent words.
	 */
 	ii=((x+dx-1)>>WORDSHIFT) - (x>>WORDSHIFT) - 1;
	if (ii < 0) {		/* diddle with masks for single words */
		mask1 &= mask2;
		mask2 = ~mask1;
	}
	bitindex=y&WORDMASK;
	switch(f){
	case F_STORE:
		while(dy-->0){
			i = ii;
			bits=map[bitindex];
			bitindex=(bitindex+1)&WORDMASK;
			if(i<0){
				*screenp=(bits&mask1)|(*screenp&mask2);
			}else{
				sp=screenp;
				*sp++=(bits&mask1)|(*sp&~mask1);
				if(i>0)
					do *sp++=bits; while(--i>0);
				*sp=(bits&mask2)|(*sp&~mask2);
			}
			screenp+=bp->width;
		}
		break;
	case F_OR:
		while(dy-->0){
			i = ii;
			bits=map[bitindex];
			bitindex=(bitindex+1)&WORDMASK;
			if(i<0){
				*screenp|=bits&mask1;
			}else{
				sp=screenp;
				*sp++|=bits&mask1;
				if(i>0)
					do *sp++|=bits; while(--i>0);
				*sp|=bits&mask2;
			}
			screenp+=bp->width;
		}
		break;
	case F_CLR:
		while(dy-->0){
			i = ii;
			bits=map[bitindex];
			bitindex=(bitindex+1)&WORDMASK;
			if(i<0){
				*screenp&=~(bits&mask1);
			}else{
				sp=screenp;
				*sp++&=~(bits&mask1);
				if(i>0)
					do *sp++= ~bits; while(--i>0);
				*sp&=~(bits&mask2);
			}
			screenp+=bp->width;
		}
	break;
	case F_XOR:
		while(dy-->0){
			i = ii;
			bits=map[bitindex];
			bitindex=(bitindex+1)&WORDMASK;
			if(i<0){
				*screenp^=bits&mask1;
			}else{
				sp=screenp;
				*sp++^=bits&mask1;
				if(i>0)
					do *sp++^=bits; while(--i>0);
				*sp^=bits&mask2;
			}
			screenp+=bp->width;
		}
	}
}

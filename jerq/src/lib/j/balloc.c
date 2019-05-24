#include <jerq.h>
/*
 * All Bitmaps come from here
 */
Bitmap *
realballoc(r, caller)
	Rectangle  r;
	char *caller;
{
	register rox=r.origin.x, roy=r.origin.y;
	register rcx=r.corner.x, rcy=r.corner.y;
	register left, right;
	register Bitmap *bp;
	char *realgcalloc(), *realalloc();

	if((bp=(Bitmap *)realalloc(sizeof(Bitmap), caller))==0)
		return 0;
	left=rox&~WORDMASK;
	if((right=rcx)&WORDMASK){
		right|=WORDMASK;
		right++;
	}
	bp->width=(right-left)>>WORDSHIFT;
	bp->rect.origin.x=rox; bp->rect.origin.y=roy;
	bp->rect.corner.x=rcx; bp->rect.corner.y=rcy;
	if(realgcalloc(((unsigned long)bp->width<<2)*(rcy-roy),
	   (long **)&bp->base, caller)==0){
		free((char *) bp);
		return 0;
	}
	bp->_null=0;
	return bp;
}


Bitmap *
balloc(r)
	Rectangle r;
{
	return realballoc(r, (char *)0);
}

bfree(bp)
	register Bitmap *bp;
{
	if(bp){
		gcfree((char *)bp->base);
		free((char *)bp);
	}
}

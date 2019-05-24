#include <jerq.h>
/*
 * All Bitmaps come from here
 */
/*VARARGS*/
Bitmap *
	/* A Rectangle......! */
realballoc(rox, roy, rcx, rcy, caller)
	register short rox, roy, rcx, rcy;
	char *caller;
{
	register short left, right;
	register Bitmap *bp;
	char *realgcalloc(), *realalloc();

	if((bp=(Bitmap *)realalloc(sizeof(Bitmap), caller)) == 0)
		return 0;
	left=rox&~WORDMASK;
	if((right=rcx)&WORDMASK){
		right|=WORDMASK;
		right++;
	}
	bp->width=(right-left)>>WORDSHIFT;
	bp->rect.origin.x=rox; bp->rect.origin.y=roy;
	bp->rect.corner.x=rcx; bp->rect.corner.y=rcy;
	if(realgcalloc(((unsigned long)bp->width<<1)*(rcy-roy),
		(long **)&bp->base, caller)==0){
		free((char *)bp);
		return 0;
	}
	bp->_null=0;
	rectf(bp, bp->rect, F_OR);
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

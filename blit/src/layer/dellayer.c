#include <jerq.h>
#include "layer.h"

extern Layer *lfront, *lback;

dellayer(l)
	register Layer *l;
{
	register Obscured *op;
	if(l==0)
		return 0;
	if(upfront(l)==0)
		return -1;
	Lgrey(l->rect);	/* Make rectangle into background */
	while(lback!=l){
		if(upfront(lback)==0)
			return -1;	/* even though things are wrong... */
	}
	/*
	 * Window to be deleted is now at rear; free the obscure's
	 */
	for(op=l->obs; op; op=op->next){
		bfree(op->bmap);
		free((char *)op);
	}
	/*
	 * Remove layer from list
	 */
	lback=l->front;
	if(lfront==l)
		lfront=0;
	if(l->front)
		l->front->back=0;
	free((char *)l);
	return 0;
}

#include <jerq.h>
#include <layer.h>
#include <queue.h>
#include "jerqproc.h"
#include "frame.h"

#define	MAXROOM	1024	/* maximum extra number of chars after insure() */

char *
GCalloc(n, p, pp)
	int n;
	char **p;
	struct Proc *pp;
{
	extern char *realgcalloc();
	while(realgcalloc((unsigned long)n, p, pp)==0){
		mesg("out of memory; clean up bitmaps and hit a button");
		do{
			sleep(30);
			Uwait(MOUSE);
		}while(!button123());
		mesg("trying again...                                 ");
	}
}
insure(p, n, pp)
	register String *p;
	register short n;
	struct Proc *pp;
{
	register i;
	char *old;
	if(p->size < n){	/* p needs to grow */
		for(i = 1; i < n; i <<= 1)
			;
		if(i > n+MAXROOM)
			i = n+MAXROOM;
		if(p->s==0)
			old=0;
		else{
			GCalloc(p->n, &old, pp);
			movstring(p->n, p->s, old);
			gcfree(p->s);
		}
		GCalloc(i, &p->s, pp);
		if(old){
			movstring(p->n, old, p->s);
			gcfree(old);
		}
		p->size = i;
	}
}
insstring(p, i, q)
	register String *p, *q;
	register short i;
{
	insure(p, p->n+q->n, P);
	movstring(i-p->n, p->s+p->n, p->s+p->n+q->n);
	movstring(q->n, q->s, p->s+i);
	p->n += q->n;
}
delstring(p, i, j)
	register String *p;
	register short i, j;
{
	register n = j-i;
	movstring(p->n-j, p->s+j, p->s+i);
	p->n -= n;
}
movstring(i, s, d)
	register short i;
	register char *s, *d;
{
	if (i > 0)
		do
			*d++ = *s++;
		while (--i > 0);
	else {
		*d = *s;		/* assumed to be null char */
		if (i++ < 0)		/* transfer -i chars in loop */
			do
				*--d = *--s;
			while (i++ < 0);
	}
}

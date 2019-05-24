#include "frame/frame.h"

#define	MAXROOM	1024	/* maximum extra number of chars after insure() */
#define	MINCHARS 63	/* for efficiency; min size of a string we'll alloc */

zerostring(p)
	String *p;
{
	if(p->s && p->size>MINCHARS+1){
		p->n=0;
		p->size=0;	/* forces insure to free the old stuff */
	}
	insure(p, MINCHARS);
	*(p->s) = '\0';
	p->n = 0;
}

addstring(p, c)
	register String *p;
	char c;
{
	insure(p, p->n+1);
	p->s[p->n++] = c;
	p->s[p->n] = '\0';
}
char *
GCalloc(n, p)
	int n;
	char **p;
{
	while(gcalloc((unsigned long)n, p)==0){
		mesg("out of memory; clean up bitmaps and hit a button\n", TRUE);
		for(;;){
			sleep(30);
			wait(MOUSE);
			if(button123())
				break;
		}
		mesg("trying again\n", TRUE);
	}
}
insure(p, n)
	register String *p;
	register short n;
{
	register i;
	char *old;
	if (p->size <= n) {	/* p needs to grow */
		for (i = 1; i <= n; i <<= 1)
			;
		if(i > n+MAXROOM)
			i = n+MAXROOM;
		if(p->s==0)
			old=0;
		else{
			GCalloc(p->n+1, &old);
			movstring(p->n+1, p->s, old);
			gcfree(p->s);
		}
		GCalloc(i, &p->s);
		if(old){
			movstring(p->n+1, old, p->s);
			gcfree(old);
		}
		p->size = i;
	}
}
insstring(p, i, q)
	register String *p, *q;
	register short i;
{
	insure(p, p->n+q->n);
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
	*(p->s+p->n) = '\0';
}
snarf(p, i, j, q)
	register String *p, *q;
	register short i, j;
{
	register n = j-i;
	insure(q, n);
	movstring(n, p->s+i, q->s);
	q->s[n] = '\0';
	q->n = n;
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

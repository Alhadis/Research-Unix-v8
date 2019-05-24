/*
 * String code for file support; basically the same as textframe strings
 * except that insstring() and delstring() take a regular
 * String argument and a char *.
 */
#include "file.h"

#define	NCHAR	80000
#define	NSTRING	(3*NFILE+2+2)	/* buffer+transmit+2 patterns */

static String string[NSTRING];

static char area[NCHAR], *endarea = &area[NCHAR], *strfree = area;

String *		/* just some information hiding here */
newstring()
{
	register String *p;
	for(p= &string[0]; p<&string[NSTRING]; p++)
		if(p->s==0){
			zerostring(p);
			return p;
		}
	error("can't allocate a string pointer", (char *)0);
	/*NOTREACHED*/
}

remstring(s)
	String *s;
{
	s->s=0;
	s->n=0;
	s->size=0;
}
String *
bldstring(s)
	register char *s;
{
	static String junk;
	junk.s = s;
	junk.n = strlen(s);
	return &junk;
}
dupstring(s, d)
	register String *s, *d;
{
	insure(d, s->n+1);
	movstring(s->n+1, s->s, d->s);
	d->n=s->n;
}
zerostring(p)
	String *p;
{
	insure(p, 31);	/* a smallish number so we aren't always calling insure */
	*(p->s) = '\0';
	p->n=0;
	p->size=32;	/* throw away the garbage */
}

addstring(p, c)
	register String *p;
	char c;
{
	insure(p, p->n+1);
	p->s[p->n++] = c;
	p->s[p->n] = '\0';
}

insure(p, n)
	register String *p;
	register short n;
{
	register int i;
	if (p->size <= n) {	/* p needs to grow */
		for (i = 1; i <= n; i <<= 1)
			;
		if (endarea - strfree < i)		/* whups, clean house */
			compact(i);
		if(p->s)
			movstring(p->n+1, p->s, strfree);
		p->s = strfree;
		strfree += i;
		p->size = i;
	}
}

compact(n)
	register int n;		/* amount of space required after compaction */
{
	register String *p, *q;
	register char *s, *t;
	s = area;
	for (;;) {
		t = endarea;
		for (q = string; q != &string[NSTRING]; q++)
			if (s <= q->s && q->s < t){
				t = q->s;
				p = q;
			}
		if (t == endarea)
			break;
		if(p->s != s)
			movstring(p->n+1, p->s, s);
		p->s = s;
		s += p->size;
	}
	strfree = s;
	if (s+n >= endarea){
		strfree = area;
		mesg("compact failed", (char *)0);
	}
}

insstring(p, i, s, n)
	register String *p;
	register i, n;
	register char *s;
{
	insure(p, p->n+n);
	movstring(i-p->n, p->s+p->n, p->s+p->n+n);
	movstring(n, s, p->s+i);
	p->n += n;
}

delstring(p, i, n)
	register String *p;
	register i, n;
{
	register j=i+n;
	movstring(p->n-j, p->s+j, p->s+i);
	p->n -= n;
	*(p->s+p->n) = '\0';
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
	

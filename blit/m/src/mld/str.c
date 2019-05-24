#include "stdio.h"
/* string routines */
extern char *malloc();
int errcnt;

char *
ncat(s, t, n)
char *s, *t;
{	int i;
	char *p, *x;
	for(i = n; i > 0 && t[i - 1] == ' '; i--)
		;
	p = malloc(i + strlen(s) + 3);
	for(x = p; *s; *x++ = *s++)
		;
	*x++ = '(';
	for(; i > 0; i--)
		*x++ = *t++;
	*x++ = ')';
	*x = 0;
	return(p);
}

char *
cat4(a, b, c, d)
char *a, *b, *c, *d;
{	char *x;
	x = malloc(strlen(a) + strlen(b) + strlen(c) + strlen(d) + 1);
	strcpy(x, a);
	strcat(x, b);
	strcat(x, c);
	strcat(x, d);
	return(x);
}

/*VARARGS1*/
fatal(s, a)
char *s;
{
	fprintf(stderr, "internal mld err: ");
	fprintf(stderr, s, a);
	abort(1);
}

/*VARARGS1*/
error(s, a)
char *s;
{
	fprintf(stderr, s, a);
	errcnt++;
}

/*VARARGS1*/
warn(s, a)
char *s;
{
	fprintf(stderr, s, a);
}

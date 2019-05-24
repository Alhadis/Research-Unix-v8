#include	<jerq.h>
#include	<jerqio.h>
#include	<font.h>
#define ISIZE(n)	((n+1)*sizeof(Fontchar))
#ifdef	MPX
#undef	alloc
#define		alloc(u)	Tpchar(4)(u)
#endif	MPX

/*
 * read a font from an input stream
 * 	<font header>
 *	<f->info>
 *	<f->bits>	no bitmap header!!
 *
 * WARNING! This code believes it knows what the Font structure looks like
 */

Font *
infont(inch)
	register char (*inch)();
{
	int n;
	register Font *f;
	register Bitmap *b;
	char *temp;

	temp = (char *)&n;
	if(ninch(2, &temp, inch))
		return((Font *)0);
	if((f = (Font *) alloc(sizeof(Font)+ISIZE(n))) == (Font *)0)
		return(f);
	f->n = n-1;
	temp = 2 + (char *)f;
	if(ninch(6, &temp, inch))	/* 6 == sizeof(height+ascent+unused) */
		goto err;
	temp = (char *)f->info;
	if(ninch(ISIZE(n), &temp, inch))
		goto err;
	if((b = balloc(Rect(0, 0, f->info[n].x, f->height))) == (Bitmap *)0)
		goto err;
	f->bits = b;
	if(ninch(2*f->height*b->width, (char **)&(b->base), inch))
		goto berr;
	return(f);

	berr:
		bfree(f->bits);
	err:
		free(f);
	return((Font *)0);
}

static
ninch(n, base, inch)
	register n;
	register char **base, (*inch)();
{
	register x, i;

	i = 0;
	do {
		x = (*inch)();
		(*base)[i++] = x;
		if(x == -1)
			return(1);
	} while (--n > 0);
	return(0);
}

ffree(f)
	register Font *f;
{
	if (f != (Font *) NULL) {
		bfree(f->bits);
		free(f);
	}
}

outfont(f,ouch)
	register Font *f;
	register (*ouch)();
{
	register Bitmap *b = f->bits;

	f->n++;
	if(
		nouch(8,f,ouch) ||
		nouch(ISIZE(f->n),f->info,ouch) ||
		nouch(2*f->height*b->width,b->base,ouch)
	)
	{
		f->n--;
		return(-1);
	}
	else
	{
		f->n--;
		return(0);
	}
}

static
nouch(n,s,ouch)
	register n,(*ouch)();
	register char *s;
{
	do {
		if((*ouch)(*s++) == -1)
			return(-1);
	} while (--n > 0);
	return(0);
}

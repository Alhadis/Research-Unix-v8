
#include <jerq.h>

#define TABLESIZE	3
static char *(byn[TABLESIZE]);

/* byn[m], for m+1 < TABLESIZE, is the table for expansion by m+1.  byn[TABLESIZE-1]
   is used for all larger sizes.  You may wish to initialize some of the values of
   byn to avoid initialization, at the cost of down-load speed.  Once initialized,
   the smaller values are retained permanently */

nullfunc() {
}

extern int magby2(), magby3(), magslow(), magbyeven(), magbyodd();

static int (*(magby[]))() = {magby2,magby3};
/* for those sizes for which special expansions are provided, special magnification
   routines are expected. */

static int magsize = 1;

setmag(n) {
	if ((n <= 1) || (n == magsize))
		return 1;
	if (n <= TABLESIZE) {
		if (byn[n-2])
			return 1;
		else
			return ngen(n,&byn[n-2]);
	}
	return ngen(n,&byn[TABLESIZE-1]);
}

freemag() {
	if (byn[TABLESIZE-1])
		gcfree(byn[TABLESIZE-1]);
	byn[TABLESIZE-1] = NULL;
	magsize = 1;
}

ngen(n,byn)
char **byn;
{
	register int i,j,k,acc;
	register char *bits;

	if (magsize != n)
		freemag();
	if ((*byn == NULL) && ((*byn = gcalloc((unsigned long)256*n, byn)) == NULL))
		return 0;
	magsize = n;
	bits = *byn;
	for (i = 0; i<256; i++)
		for (j = 8*(n-1); j >= 0; j -= 8) {
			acc = 0;
			for (k = 0; k < 8; k++)
				if (i & (1<<((k+j)/n)))
					acc |= (1<<k);
			*bits++ = acc;
		}
	return 1;
}

/*
 * Like bitblt, only magnifies.  fac = (xscale, yscale), expansion factors.
 * F_STORE into tb only.  Assumptions: b != tb != db; tb offscreen and trashable;
 * db is the ultimate destination (0 if none desired) bitmap, and dp is the
 * destination point in that bitmap; the ultimate bitblt is done in mode mode.  If
 * db is 0, the magnified image is found in tb, in a rectangle based at (0,0).
 * magn does the last bitblt for you so that it can freely 'reshape' the
 * bitmap, and noone will notice.  If tb is exactly the right size, this is
 * not important, so feel free to set db = (Bitmap *)0.  tb->rect.origin MUST
 * equal (0,0).  If xscale is not equal to the last argument to setmag, the
 * expansion will be ungodly slow, unless it is 1, 2, or 3.
 */
magn(b, r, tb, db, dp, fac, mode)
	register Bitmap *b, *tb, *db;
	Rectangle r;
	Point dp, fac;
	int mode;
{
	Bitmap temp;
	register i, shift;
	unsigned char *from;
	char *to;
	int tjump, fjump;
	int hcount,vcount;
	Point d,q,o;
	int w,wid,goodbytes;
	int (*func)();
	char *bits;

	if(fac.x<1 || fac.y<1)
		return;
	d=sub(q=r.corner, o=r.origin);
	if ((d.x <= 0) || (d.y <= 0))
		return;
	temp = *tb;
	if (db)
		tb->width = (d.x * fac.x + 15) / 16;
	tb->rect.corner.y = d.y;
	w = tb->width * 16;
	tb->width *= fac.y;
	tb->rect.corner.x = tb->width * 16;
	rectf(tb, tb->rect, F_CLR);
	wid = (d.x + 7) / 8;
	/* 1: move into place, and expand vertically, by cheating */
	if ((fac.x == 1) || (o.x & 0x7) || (d.x & 0x7) || (b->_null)) {
		from = ((unsigned char *)(tb->base))+wid;
		fjump = tb->width*2+wid;
		bitblt(b,r,tb,Pt(0,0),F_OR);
	} else {
		from = ((unsigned char *) addr(b,o))+wid+(o.x & 0x8?1:0);
		fjump = b->width*2+wid;
	}
	/* 2: expand horizontally */
	to = ((char *)tb->base)+wid*fac.x;
	tjump = tb->width*2+wid*fac.x;
	hcount = wid;
	vcount = d.y;
	goodbytes = (i = d.x & 0x8) ? ((i * fac.x + 7) / 8) : fac.x;
	bits = NULL;
	if (fac.x == 1)
		func = nullfunc;
	else if ((fac.x <= TABLESIZE) && (bits = byn[i = fac.x-2]))
		func = magby[i];
	else if ((fac.x == magsize) && (bits = byn[TABLESIZE-1])) {
		if (fac.x & 1)
			func = magbyodd;
		else
			func = magbyeven;
	} else
		func = magslow;
	(*func)(from,to,fjump,tjump,hcount,vcount,bits,goodbytes,fac.x);
	/* 3: smear 'vertically' */
	for(i=1; i<fac.y; i<<=1){
		shift=min(i, fac.y-i)*w;
		bitblt(tb, Rect(0, 0, shift, d.y),
		       tb, Pt(i*w, 0), F_OR);
	}
	if (db) {
		tb->rect.corner.y *= fac.y;
		tb->rect.corner.x = d.x * fac.x;
		tb->width /= fac.y;
		bitblt(tb, tb->rect, db, dp, mode);
	}  
	*tb = temp;	
}

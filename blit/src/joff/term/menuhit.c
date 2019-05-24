#include <jerq.h>
#include <font.h>
#undef menuhit

typedef struct NewMenu{
	char	**item;			/* string array, ending with 0 */
	char	*(*generator)();	/* used if item == 0 */
	int	prevhit;		/* retained from previous call */
	int	prevtop;		/* ditto */
} NewMenu;

#define scale( x, inmin, inmax, outmin, outmax )\
	( outmin + muldiv(x-inmin,outmax-outmin,inmax-inmin) )

#define bound( x, low, high ) ( min( high, max( low, x ) ) )

#define SPACING		14
#define DISPLAY		12
#define CHARWIDTH	9

#define DELTA		6
#define BARWIDTH	18

static Bitmap physical = { (Word *) (156*1024L), 50, 0, 0, XMAX, YMAX };

static char **table;
static char *tablegen(i)
{
	return table[i];
}

menuhit( m, but )
register NewMenu *m;
{
	register int width, i, j, top, newtop, hit, newhit, items, lines, length;
	Point p, q, savep;
	Rectangle sr, tr, mr;	/* scroll, text, menu */
	register Bitmap *b;
	register char *s, *(*generator)(), *from, *to, fill[64];

#define sro sr.origin
#define src sr.corner
#define tro tr.origin
#define trc tr.corner
#define mro mr.origin
#define mrc mr.corner

	generator = (table=m->item) ? tablegen : m->generator;
	p = mouse.xy;
	for( length = items = 0; s=(*generator)(items); ++items )
		length = max( length, strlen(s) );
	if( items == 0 ) return -1;
	width = length*CHARWIDTH+10;
	src.x = tro.x = mro.x = mro.y = 0;
	if( items <= DISPLAY ) lines = items;
	else {
		lines = DISPLAY;
		tro.x = src.x = BARWIDTH;
		sro.x = sro.y = 1;
	}
#define ASCEND 2
	tro.y = ASCEND;
	mrc = trc = add(tro, Pt(width,min(items,lines)*SPACING) );
	src.y = mrc.y-1;
	newtop = bound( m->prevtop, 0, items-lines );
	p.y -= bound(m->prevhit, 0, lines-1)*SPACING+SPACING/2;
	p.x = bound( p.x-(src.x+width/2), 0, XMAX-mrc.x );
	p.y = bound( p.y, 0, YMAX-mrc.y );
	sr = raddp(sr,p);
	tr = raddp( tr, p );
	mr = raddp( mr, p );
	b = balloc(mr);
	if( b ) bitblt( &physical, mr, b, mro, F_STORE );
	rectf( &physical, mr, F_STORE );
PaintMenu:
	rectf( &physical, inset(mr,1), F_CLR );
	top = newtop;
	if( items > DISPLAY ){
		p.y = scale( top, 0, items, sro.y, src.y );
		p.x = sr.origin.x;
		q.y = scale( top+DISPLAY, 0, items, sro.y, src.y );
		q.x = sr.corner.x;
		rectf( &physical, Rpt(p,q), F_XOR );
	}
	for( p = tro, i = top; i < min(top+lines,items); ++i ){
		q = p;
		from = generator(i);
		for( to = &fill[0]; *from; ++from ){
			if( *from != '\t' ) *to++ = *from;
			else for(j=length-(strlen(from+1)+(to-&fill[0])); j-->0; )
				*to++ = ' ';
		}
		*to = '\0';
		q.x += ( width-jstrwidth(fill) )/2;
		string( &defont, fill, &physical, q, F_XOR );
		p.y += SPACING;
	}
	savep = mouse.xy;
	for( newhit = hit = -1; button(but); nap(2) ){
		if( ptinrect( p = mouse.xy, sr ) ){
			newtop = scale( p.y, sro.y, src.y, 0, items );
			newtop = bound( newtop-DISPLAY/2, 0, items-DISPLAY );
/* >>>>>>> */		if( newtop != top ) goto PaintMenu;
		} else if( ptinrect(savep,sr) ){
			register dx, dy;
			if( abs(dx = p.x-savep.x) < DELTA ) dx = 0;
			if( abs(dy = p.y-savep.y) < DELTA ) dy = 0;
			if( abs(dy) >= abs(dx) ) dx = 0; else dy = 0;
			cursset( p = add( savep, Pt(dx,dy) ) );
		}
		savep = p;
		newhit = -1;
		if( ptinrect( p, tr ) ){
			newhit = bound( (p.y-tro.y)/SPACING, 0, lines-1 );
			if( newhit != hit && hit >= 0
			 && abs(tro.y+SPACING*newhit+SPACING/2-p.y) > SPACING/3 )
				newhit = hit;
		}
		if( newhit != hit ){
			flip( tr, hit );
			flip( tr, hit = newhit );
		}
	}
	if( b ){
		screenswap( b, b->rect, b->rect );
		bfree(b);
	}
	m->prevhit = hit;
	m->prevtop = top;
	return hit<0 ? -1 : hit+top;
}

static flip(r,n)
Rectangle r;
{
	if( n<0 ) return;
	++r.origin.x;
	r.corner.y = (r.origin.y += SPACING*n-1) + SPACING;
	--r.corner.x;
	rectf( &physical, r, F_XOR );
}
#include <jerq.h>
#include <font.h>
#include "../protocol.h"

#define SCROLL 32
#define BLANK (strwidth( &defont, " " ) )
#define STAT_HT (defont.height*2)
Rectangle scrolling;

char put( c )
register char c;
{
	register blank = BLANK, i;
	static int stateline = 0, index;
	static Point p, slp;
	static curpos = 0;
	static Rectangle drect, sld, d;
	static char statetext[128];
	char s[2];

	s[0] = c;
	s[1] = '\0';
	if( !eqrect( drect, Drect ) && !stateline ) {
		P->state &= ~(RESHAPED|MOVED);
		drect = Drect;
		put( '\f' );
		put( STATELINE );
		for( i = 0; statetext[i]; ) put( statetext[i++] );
		put( '\r' );
	}
	if( stateline ){
		if( c == '\r' ){
			stateline = 0;
			statetext[index] = 0;
			sleep( 20 );
			rectf( &display, sld, F_XOR );
		} else{
			statetext[index++] = c;
			if( slp.x+blank < Drect.corner.x )
				slp = string( &defont, s, &display, slp, F_XOR );
		}
	} else if( c >= 040  && c <= 0177 ) {
		p = string( &defont, s, &display, p, F_XOR );
		++curpos;
		if( p.x+blank > Drect.corner.x ) put( '\n' );
	} else
	switch( c ) {
	case STATELINE: stateline = 1;
			index = 0;
			slp = Drect.origin; slp.y += 4; slp.x += 4;
			sld = Drect;
			sld.corner.y = sld.origin.y+defont.height*3/2;
			stipple( sld );
			rectf( &display, sld, F_XOR );
			break;
	case '\b':	if( curpos ) { p.x -= blank; --curpos; } break;
	case '\f':	p = Drect.origin;
			scrolling = Drect;
			scrolling.origin.y += defont.height*3/2;
			stipple( scrolling );
			d = scrolling;
			scrolling.origin.y = d.corner.y = d.origin.y+4;
			rectf( &display, d, F_XOR );
			p.y += STAT_HT;
	case '\n':	p.y += defont.height;
	case '\r':	p.x = Drect.origin.x+4; curpos = 0;
			if( p.y < Drect.corner.y-defont.height ) break;
			scroll();
			p.y -= SCROLL;
			break;
	case '\t':	put( ' ' );
			while( curpos%8 ) put( ' ' );
			break;
	case '\v':	put( '\r' );
			stipple(Rect(p.x, p.y, Drect.corner.x, p.y+defont.height));
			/*while( p.x+BLANK < Drect.corner.x )
				p = string( &defont, " ", &display, p, F_STORE );
			put( '\r' );*/
			break;
	default:	put( ' ' );
	}
	return c;
}

scroll(){
	Rectangle s;
	Point d;

	s = Drect;
	s.origin.y += SCROLL+STAT_HT;
	d = s.origin;
	d.y -= SCROLL;
	bitblt( &display, s, &display, d, F_STORE );
	s = Drect; s.origin.y = s.corner.y-SCROLL;
	stipple( s );
}

lprintf( fmt, x )
char *fmt;
unsigned x;
{
	register c;
	char *s, *decimal();

	while( (c = *fmt++) ){
		if( c != '%') put(c);
		else {
			c = *fmt++;
			if (c=='D' ) lprintf( decimal( *(long *) &x ) );
			else if(c == 'c') put( x );
			else if(c == 's') {
				s = *(char **) &x;
				while( c = *s++ ) put(c);
			}
		}
	}
}

#include <jerq.h>
#include "../protocol.h"

Bitmap physical = { (Word *) (156*1024L), 50, 0, 0, XMAX, YMAX };

Texture xhairst = {
	 0x0100, 0x7FFC, 0x4104, 0x4104,
	 0x4104, 0x4104, 0x4104, 0xFEFE,
	 0x4104, 0x4104, 0x4104, 0x4104,
	 0x4104, 0x7FFC, 0x0100, 0x0000,
};
Bitmap xhairs = { (Word *) &xhairst, 1, 0, 0, 16, 16 };

Texture boxt = {
	 0x0100, 0x7FFC, 0x4104, 0x4004,
	 0x4004, 0x4004, 0x4004, 0xE00E,
	 0x4004, 0x4004, 0x4004, 0x4004,
	 0x4104, 0x7FFC, 0x0100, 0x0000,
};
Bitmap box = { (Word *) &boxt, 1, 0, 0, 16, 16 };

outline( r )
Rectangle r;
{
	segment( &physical, r.origin, Pt(r.origin.x,r.corner.y), F_XOR );
	segment( &physical, Pt(r.origin.x,r.corner.y), r.corner, F_XOR );
	segment( &physical, r.corner, Pt(r.corner.x,r.origin.y), F_XOR );
	segment( &physical, Pt(r.corner.x,r.origin.y), r.origin, F_XOR );
}
extern Rectangle scrolling;

#define  POINTNAP 30
graphop( c )
char c;
{
	long a, addrfromhost();
	Rectangle r, clipped;
	Bitmap *b;
	Point shift, p;
	int n, i;

	a = addrfromhost();
	wait( MOUSE );
	switch( c ){
	default: hosterr();
	case G_POINT:
		p = *(Point *) a;
		r = raddp( Rect(0,0,15,15), sub(p,Pt(7,7) ) );
		cursswitch( &boxt );
		while( !button123() ){
			bitblt( &physical, inset(r,3), &box, Pt(3,3), F_STORE );
			for( i = 1; i < 3; ++i ){
				screenswap( &xhairs, xhairs.rect, r );
				for( n = POINTNAP; n && !button123(); --n )
					if( i==1 ) nap( 1 ); else sleep( 1 );
			}
		}
		while( button123() ) wait(MOUSE);
		dmcursor();
		return;
	case G_TEXTURE:
		cursswitch( a );
		while( !button123() ) {};
		if( button23() ){
			r = raddp( Rect(0,0,16,16),
				Pt(mouse.xy.x&0xFFF0, mouse.xy.y&0xFFF0) );
			if( rectclip( &r, scrolling ) )
			    texture( &display, r, a, F_STORE );
		}
		while( button123() );
		dmcursor();
		return;
	case G_RECTANGLE:
		outline(*(Rectangle *)a);
		return;
	case G_BITMAP:
		b = (Bitmap *) a;
		cursswitch( 0L );
		while( !button123() ){
			shift = sub( mouse.xy,
				div(add(b->rect.corner,b->rect.origin),2) );
			r = raddp( b->rect, shift );
			outline(r);
			nap(2);
			outline(r);
		}
		if( button23() && rectclip( &r, scrolling ) ){
			clipped = rsubp( r, shift );
			bitblt(b, clipped, &display, r.origin, F_STORE);
		}
		while( button123() ) {};
		dmcursor();
		return;
	}
}

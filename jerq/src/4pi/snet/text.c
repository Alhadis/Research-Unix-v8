#include "term.h"

void PutText(c)
{
	PutScreen(c);
}

void PutTextf(fmt, x)
char *fmt;
long x;
{
	register char *s;
	register long c, *ap = &x;

	while( (c = *fmt++) ){
		if( c != '%') PutText((int)c);
		else {
			switch( c = *fmt++ ){
			case '%':
				PutText('%');
				break;
			case 'd':
			case 'D':
				PutTextf( dec(*ap++) );
				break;
			case 'X':
			case 'x':
				PutTextf( "0x" );
				PutTextf( hex(*ap++) );
				break;
			case  'c':
				PutText( (int)*ap++ );
				break;
			case  's':
				if( s = (char *) *ap++ ){
					while( c = *s++ ) PutText((int)c);
				break;
				}
			}
		}
	}
}

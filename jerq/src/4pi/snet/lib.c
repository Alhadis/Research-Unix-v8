#include "term.h"

long assertf(l)
long l;
{
	extern char *J_hist;
	register c;

	if( l ) return l;
#ifdef JOURNAL
	PutTextf( "pi.m: assertion failed near %s\n", J_hist );
#else
	PutTextf( "pi.m: assertion failed\n" );
#endif
	asm("jmp 0x20000C");
}

char *dec(n)
register long n;
{
	register char *pic = "-zzzzzzzzz9";
	char cip[10];
	register char *p = pic, *c;

	if( n < 0 ) *p++ = '-', n = -n;
	*(c=&cip[0]) = '\0';
	do {
		*++c = '0'+(n%10);
	} while( n = n/10 );
	while( *c ) *p++ = *c--;
	*p = '\0';
	return pic;
}

char *hex(n)
register long n;
{
	register char *p = "zzzzzzzx"+7;

	do {
		*p-- = "0123456789ABCDEF"[n&0xF];
		n >>= 4;
	} while( n );
	return p+1;
}

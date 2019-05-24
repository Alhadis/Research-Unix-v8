#include "common.h"

void jerqdo( r )
{
	assert( !oflag );
	putchar( ESCAPE );
	putchar( r ); flush();
}

char *jerqkbd( s )
char *s;
{
	char c;
	int i;

	assert( !oflag );
	jerqdo( DO_KBD );
	i = 0;
	for( c = getchar(); c != '\n' && c != '\r'; c = getchar() )
			if( c != '\b' ) s[i++] = c; else assert( i-- );
	s[i] = '\0';
	return s;
}

void addr_desc( a, s ) MLONG a;
{
	char buf[8];
	
	assert( !oflag );
	flush();
	buf[0] =  ESCAPE; buf[1] = s;
	buf[2] = a>>16; buf[3] = a>>8; buf[4] = a;
	write( 1, buf, 5 ); 
}

MLONG shiftfromjerq(i)
{
	long l = 0;

	assert( !oflag );
	while( i-- ) l = (l<<8) | (0xFF & getchar() );
	return l;
}

MLONG shifttojerq(x,n)
MLONG x;
{
	assert(!oflag);
	if( n > 3 ) putchar(x>>24);
	if( n > 2 ) putchar(x>>16);
	if( n > 1 ) putchar(x>>8);
	putchar(x);
}

MLONG longfromjerq()	{ return shiftfromjerq(4); }

MLONG wordfromjerq()	{ return shiftfromjerq(2); }

longtojerq(x)		{ shifttojerq(x,4); }

wordtojerq(x)		{ return shifttojerq(x,2); }

MLONG peekbyte( b )
MLONG b;
{
	if( oflag ) return ofetch( b );
	addr_desc( b, PEEK_BYTE );
	return 0xFF & (MLONG) getchar();
}

MLONG peekword( w )
MLONG w;
{
	if( oflag ) return (ofetch(w+1)<<8) | ofetch(w);
	addr_desc( w, PEEK_WORD );
	return wordfromjerq();
}

MLONG peeklong( l )
MLONG l;
{
	if( oflag ) return (ofetch(l)<<16) | ofetch(l+2);
	addr_desc( l, PEEK_LONG );
	return longfromjerq();
}

char *peeknstr( s, n )
MLONG s;
{
	int  i = 0;
	char *buf = talloc(n+1);

	addr_desc( s, PEEK_STR );
	assert( n < 256 );
	putchar( n ); flush();
	while( buf[i++] = getchar()){}
	return buf;
}

pokebwl( s, n, i, l )
MLONG n, i;
{

	assert( !oflag );
	addr_desc(n, s);
	shifttojerq( i, l );
}

void pokelong( a, i ) MLONG a, i; { pokebwl( POKE_LONG, a, i, 4 ); }
 	
void pokeword( a, i ) MLONG a, i; { pokebwl( POKE_WORD, a, i, 2 ); }

void pokebyte( a, i ) MLONG a, i; { pokebwl( POKE_BYTE, a, i, 1 ); }

float f68ktovax( l )
long l;
{
	union {
		unsigned long	f68k;
		float		fvax;
	} u;

	u.f68k = l;
	u.f68k = (u.f68k<<16) | (u.f68k>>16);
	return u.fvax;
}

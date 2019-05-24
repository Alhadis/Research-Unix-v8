#include "common.h"
#include "user.h"

char *fmtbyte( c )
MBYTE c;
{
	switch( c &= 0xFF ){
		case '\0' : return "\\0";	
		case '\b' : return "\\b";	
		case '\f' : return "\\f";	
		case '\n' : return "\\n";	
		case '\r' : return "\\r";	
		case '\t' : return "\\t";	
		case '\v' : return "\\v";	
		case ' ' : return " ";		/* ikeya!<ctype.h> is wrong */	
		case '\'': return "\\\'";
		case '\"': return "\\\"";
		case '\\': return "\\\\";
		default:
			if( isascii(c) && isprint(c) ) return fmt( "%c", c );
 			else return fmt( "\\%03o", c&0xFF );
	}
}

char *fmtchar( c )
MBYTE c;
{
	return fmt( "\'%s\'", fmtbyte( c ) );
}

char *fmtstring( p )
MLONG p;
{
	int   width, i = 0;
	char *buf, *s;

	if( !p ) return "0";
	width = dotstrlen ? dotstrlen+2 : 32, i = 0;
	if( width < 5 ) width = 5;
	buf = talloc( width+32 );
	strcpy( buf, "\"" );
	s = peeknstr( p, width );
	while( s[i] && strlen(buf) <= width ) strcat(buf, fmtbyte(s[i++]));
	strcat( buf, "\"" );
	if( strlen( buf ) > width ) strcpy( &buf[width-4], "...\"" );
	return fmt( "%s@%s", buf, doh(p) );
}

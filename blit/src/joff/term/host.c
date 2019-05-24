#include <jerq.h>
#include "../protocol.h"

charfromhost()
{	register int c;

	if( (c = rcvchar()) == -1 ) c = waitrcvchar();
	return c;
}

long shiftfromhost( bytes )
{
	long n = 0;

	while( bytes-- ) n = (n<<8) | (charfromhost()&0xFF);
	return n;
}

long addrfromhost() { return shiftfromhost(3); }

long longfromhost() { return shiftfromhost(4); }

wordfromhost() { return shiftfromhost(2); }

longtohost( l )
long l;
{
	char buf[4];

	buf[0] = (char) (l>>24)&0xFF;
	buf[1] = (char) (l>>16)&0xFF;
	buf[2] = (char) (l>>8)&0xFF;
	buf[3] = (char) l&0xFF;
	sendnchars( 4, buf );
}

wordtohost( w )
{
	char buf[2];

	buf[0] = (char) (w>>8)&0xFF;
	buf[1] = (char) w&0xFF;
	sendnchars( 2, buf );
}

strntohost( s, limit )
char *s;
{
	int i = 0;

	if( ((long)s) >= LO_ADDR && ((long)s)+limit <= HI_ADDR )
		for( i = 0; i < limit && s[i]; ++i ){}
	sendnchars( i, s );
	sendchar( 0 );
}

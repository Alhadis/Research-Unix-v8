#include "snet.h"
PutRemote(c)
{
	sendchar(c);
}

GetRemote()
{
	register int c;

	while( (c = rcvchar()) == -1 ){
		if( kbdchar() != -1 )
			asm("jmp 0x20000C");
	}
	return c;
}

PutScreen(c)
{
	typchar(c);
}

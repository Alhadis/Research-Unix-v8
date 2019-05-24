#include "common.h"
#define DADDR (384*1024L+030)

zdc( addr, duration )
MLONG addr;
{
	if( addr < 0 || addr >= 256 || duration < 0 || duration > 60 ){
		printf( "use zdc 0<=k<=255 0<=t<=60\n" );
		return;
	}
	pokeword( DADDR, addr*(1024/4) );
	flush();
	sleep( duration );
	pokeword( DADDR, 156*(1024/4) );
}

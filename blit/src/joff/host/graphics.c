#include "common.h"

strop( id )
char *id;
{
	if( idmatch( id, "Bitmap" ) ) return G_BITMAP;
	if( idmatch( id, "Layer" ) ) return G_BITMAP;
	if( idmatch( id, "Point" ) ) return G_POINT;
	if( idmatch( id, "Rectangle" ) ) return G_RECTANGLE;
	if( idmatch( id, "Texture" )  )return G_TEXTURE;
	return 0;
}

char *graphopnames( op )
{
	switch( op ){
	case G_BITMAP: return "%bitblt(~)";
	case G_POINT:  return "%point(~)";
	case G_TEXTURE:return "%texture(~)";
	case G_RECTANGLE: return "%outline(~)";
	default: return 0;
	}
}

g_addr_desc( a, s )
MLONG a;
{
	if( !inmemory(a) ){
		printf( "%s cannot be applied at this address: %s\n",
			graphopnames( s ), doh(a) );
		return;
	}
	addr_desc( a, s );
}

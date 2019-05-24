#include "common.h"

static struct list {
	char        *user;
	struct list *link;
};

static struct list *track = 0;

tfree()
{
	while( track ){
		free( track->user );
		free( track );
		track = track->link;
	}
}

char *talloc( n )
{
	struct list *p = (struct list *) calloc( 1, sizeof *p );

	assert( p && ( p->user = (char *) calloc( 1, n ) ) );
	p->link = track;
	track = p;
	return p->user;
}

char *fmt( f, x1, x2, x3, x4, x5, x6 )
{
	char s[1024];

	sprintf( s, f, x1, x2, x3, x4, x5, x6 );
	assert( strlen(s) < 1024 );
	return strcpy( talloc( strlen(s)+1 ), s );
}

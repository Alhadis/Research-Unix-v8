#include "common.h"
#include "user.h"
#include "frame.h"
#include "../traps.h"

cdc( dir )
char *dir;
{
	char *getenv();

	if( dir[0] == 0 ) dir = getenv( "HOME" );
	if( dir == 0 ){
		printf( "$HOME not found\n" );
		return;
	}
	if( chdir( dir ) == -1 ){
		printf( "cannot cd to %s\n", dir );
		return;
	}
	pwdc();
}

pwdc()
{
	system( "pwd" );
}

struct sdlist{
	char		*s_dir;
	struct sdlist	*s_next;
};

struct sdlist *sdl;

searchdirc( dir )
{
	struct sdlist *new = (struct sdlist *) calloc( 1, sizeof *new );	

	assert( new );
	assert( new->s_dir = calloc(128,1) );
	strcpy( new->s_dir, dir ); 
	new->s_next = sdl;
	sdl = new;
	hatebsmsg();
}

FILE *opensrc( file )
char *file;
{
	FILE *f;
	struct sdlist *try;
	char *srcfile;

	srcfile = file;
	if( f = fopen( srcfile, "r" ) ) return f;
	for( try = sdl; try; try = try->s_next ){
		srcfile = fmt( "%s/%s", try->s_dir, file );
		if( (f = fopen( srcfile, "r" ) ) ) return f;
	}
	return 0;
}

char *srcline( pc )
MLONG pc;
{
	char *file, c[2], *line = talloc( 128 );
	FILE *src;
	int lineno = 0, i;
	struct ramnl *p, *s = lookup(USER_MPX,S_SLINE,0,pc);;

	if( !s || s->n_value != pc ) return "";
	file = sofile( s );
	if( !(src = opensrc(file)) ) return "";
	for( c[0] = '\n'; c[0]>0 && lineno < s->n_desc ; c[0] = getc(src) ){
		if( c[0] == '\n' ) ++lineno;
	}
	c[1] = 0;
	for( i = 0; i < 80; ++i ){
		if( c[0] == '\n' || c[0] == 0 ) break;
		if( !isascii(c[0]) || !isprint(c[0]) ) c[0] = ' ';
		strcat( line, c );
		c[0] = getc(src);
	}
	fclose( src );		
	if( lineno != s->n_desc ) return "";
	return line;	
}

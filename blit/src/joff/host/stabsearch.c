#include "common.h"
#include "stab.h"

region( tab, a )
MLONG a;
{
	struct stable *t = stabtab[tab];
	struct exec *h = t ? &t->header : 0;

	if( t && t->rom && a >= t->rom && a < t->rom+24*1024 ) return N_ABS;
	if( !t || !t->base || a < t->text + h->a_textorg ) return 0;
	a -= t->text + h->a_textorg;
	if( a < h->a_text ) return N_TEXT;
	if( a < h->a_text + h->a_data ) return N_DATA;
	if( a < h->a_text + h->a_data + h->a_bss ) return N_BSS;
	return 0;
}
	
idmatch( a, b )
char *a, *b;
{
	return a[0] == '*' || !strncmp( a, b, 8 );
}

struct ramnl *lookup( tables, class, id, addr )
long tables, class;
char *id;
MLONG addr;
{
	struct ramnl *b = 0, *s;
	struct stable *t;
	long h, tab;

	for( ; t = stabtab[tab = tables & 0xFF]; tables >>= 8 ){
	    if( t->base && ( !addr || region( tab, addr ) ) )
		for( h = class; h; h >>= 8 ){
		    for( s = t->threader[h&0xFF]; s; s = s->n_thread ){
	   		if( !id && s->n_value <= addr
				&& ( !b || s->n_value > b->n_value ) ) b = s;
			else if( id && idmatch( id, s->n_name ) ) return s;
		    }
		}
	}
	return b;
}


char *basename( path )
char *path;
{
	char *rindex(), *lsi = rindex( path, '/' );
	return lsi ? lsi+1 : path; 
}

char *sofile( s )
struct ramnl *s;
{
	struct ramnl *base = stabtab[table(s)]->base;
	char *t = talloc(128);

	while( s > base && s->n_other != S_SO ) --s;
	while( s > base && (s-1)->n_other == S_SO ) --s;
	if( s->n_other != S_SO ) return "<src not found>";
	while( s->n_other == S_SO ) strncat( t, (s++)->n_name, 8 );
	return t;
}

char *soline( l )
MLONG l;
{
	struct ramnl *p, *s = lookup( USER_MPX, S_SLINE, 0, l );
	MLONG offset = s ? l - s->n_value : 0;
	int index = 1;

	if( !s ){
	    if( s = lookup( USER_MPX, S_BFUN, 0, l ) )
		return fmt( "%s:<pc=%s>", basename(sofile(s)), doh(l) );
	    return "";
	}
	for( p = s-1; p->n_other != S_SO; --p ) if( p->n_other == S_SLINE ){
		if( p->n_desc == s->n_desc ) ++index; 
		if( p->n_desc < s->n_desc ) break;
	}
	return fmt( "%s:%d%s%s", basename(sofile(s)), s->n_desc,
			index>1 ? fmt( ".%d", index ) : "",
				offset ?  fmt( "+%s", doh(offset)) : "" );
}

struct ramnl *visible( s, class, id, addr )	/* now also gives file statics */
struct ramnl *s;
long class;
char *id;
MLONG addr;
{
	int nest = 0;
	long h;

	if( !s ) return 0;
	for( ++s; s->n_other != S_SO; ){
	    switch( s->n_other ){
	    case S_BFUN:  nest = 1000000; break;		/* infinity */
	    case S_EFUN:  nest = 0; break;
	    case S_LBRAC: ++nest; break;
	    case S_RBRAC: if( nest ) --nest; break;
	    default:
	        if(!nest) for( h = class; h; h >>= 8 ) if(s->n_other == (h&0xFF))
			if( !id && s->n_value == addr ) return s;
			else if( id && idmatch( id, s->n_name ) ) return s;
	    }
	    if( table(++s) == USER_MPX ) break;		/* off end - not found */
	}
	return 0;
}

table( s )
struct ramnl *s;
{
	int i;

	if( !s ) return 0;
	for( i = 1; stabtab[i]; ++i )
		if( s >= stabtab[i]->base && s < stabtab[i]->base+stabtab[i]->size )
			return i;
	return USER_MPX;
}

unsigned char ofetch( a )
MLONG a;
{	
	static table;
	int t;
	unsigned char b;

	if( a >= INDEX_BASE && a <= LAST_INDEX ) return 0;
	for( t = 1; stabtab[t]; ++t ) if( region( t, a ) ){
		if( t != table ) printf( "fetching from %s\n", stabtab[t]->path );
		table = t;
		lseek( stabtab[t]->fd, a - stabtab[t]->text
			- stabtab[t]->header.a_textorg + sizeof(struct exec) , 0 );
		read( stabtab[t]->fd, &b, 1 );
		return b;
	}
	printf( "invalid address: %s\n", doh(a) );
	return 0;
}
	    




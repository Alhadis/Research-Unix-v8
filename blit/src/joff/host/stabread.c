#include "common.h"
#include "stab.h"

stabfree( stab )
struct stable *stab;
{
	int i;

	if( stab->base ) free( stab->base );
	if( stab->fd ) close( stab->fd );
	stab->base = 0;
	stab->fd = 0;
	stab->size = 0;
	for( i = 0; i < 0xFF; ++i ) stab->threader[i] = 0;
	stab->header.a_magic = 0; 
	stab->header.a_text = 0; 
	stab->header.a_data = 0; 
	stab->header.a_bss = 0; 
	stab->header.a_syms = 0; 
	stab->header.a_entry = 0; 
	stab->header.a_textorg = 0; 
	stab->header.a_flag = 0; 
	stab->text = 0;
	stab->st_mtime = 0;
}

time_t mtime( fd )
{
	struct stat buf;

	fstat( fd, &buf );
	return buf.st_mtime;
}

stabread( stab, mode )
struct stable *stab;
{
	struct exec  header;
	struct nlist *a;
	struct ramnl *s, *g;
	int          fd, i, t, zero = 0, h;
	char	     input[128];
	MLONG	     offset;

	for( ;; mode |= INTERACT ){
		if( (mode&INTERACT) && oflag ) clean=1, wrap(); /* horrible */
		strcpy( input, stab->path );
		if( (mode&INTERACT) && !oflag ){
			printf("argv[0] = %s\n", stab->path );
			scratch();
			scrapp( "argv[0]", "" );
			scrapp( "keyboard", 0 );
			scrapp( "none", "<none>" );
			printf( "symbol tables?" );
			switch( h = scrhit() ){
			case -1: strcpy( input, "<none>" );
				break;
			case 0:
				strcpy( input, "\vsymbol table object file? " );
				m_menu( 0, 3 ); 
				readuser( input );
				putchar( '\n' );
				break;
			default:
				strcpy( input, h );
			}
 			putchar( '\v' );
		}
		if( !strcmp( input, "<none>" ) ){
			stabfree( stab );
			return;
		}
		if( !strcmp( input, "" ) ) strcpy( input, stab->path );
		if( (fd = open( input, 0 )) < 0 ){
			printf( "cannot open: %s \n", input );
			continue;
		}
    		if( stab->size && !strcmp( input, stab->path ) ){
		    printf( "%s already loaded", stab->path );
		    if( stab->st_mtime != mtime( fd ) )
			printf( ", but out of date and reread.\n" );
		    else{
			close( fd );
			if( stab->text && (offset = peeklong(TEXT_INDEX) - stab->text) ){
			    printf( ", but must be relocated" );
			    for( i = 0; i < stab->size; ++i )
				switch( stab->base[i].n_type & ~N_EXT ){
					case N_TEXT: case N_BSS: case N_DATA:
						stab->base[i].n_value += offset;
				}
			    stab->text = peeklong(TEXT_INDEX);
			}
			printf( ".\n" );
			return;
		    }
		}
		lseek( fd, 0L, 0 );
		read( fd, (char *) &header, sizeof header );
		if( header.a_magic != MAGREL && header.a_magic != MAGABS ){
			printf( "not compiled for 68000: %s\n", input );
			close( fd );
			continue;
		}
		if( header.a_syms == 0){
			printf( "no symbols: %s\n", input );
			close( fd );
			continue;
		}
		if( stab->magic && header.a_magic != stab->magic )
			printf( "Warning: %s tables are %s.\n", stab->name,
			    header.a_magic == MAGREL ? "mpx" : "standalone" );
		break;
	}
	stabfree( stab );
	if( oflag ) stab->fd = fd;
	strcpy( stab->path, input );
	stab->header = header;
	stab->st_mtime = mtime( fd );
	lseek( fd, sizeof header + header.a_text + header.a_data, 0) ;
	if( !header.a_flag ){
		lseek( fd, header.a_text + header.a_data, 1 ); 
		stab->text = peeklong(TEXT_INDEX);
 	}
	a = (struct nlist *)calloc(stab->size=header.a_syms/(sizeof *a),sizeof *a);
	stab->base = (struct ramnl *) calloc( stab->size, sizeof *s );
	assert( a && stab->base && read( fd, (char *) a, header.a_syms ) );
	if( !oflag ) close( fd );
	for( i = stab->size-1, s = &stab->base[i]; i >= 0; --i, --s ){
 		* (struct nlist *) s = a[i];
		if( stab->text ) switch( s->n_type & ~N_EXT ){
			case N_BSS  :
			case N_DATA :
			case N_TEXT : s->n_value += stab->text;
		}
		if( s->n_other == S_SLINE && s->n_type != N_TEXT )
			s->n_other = S_DLINE;
		if( !s->n_other && !s->n_type ) zero = i;
		t = s->n_other ? s->n_other : S_shift(s->n_type);
		s->n_thread = stab->threader[t];
		stab->threader[t] = s;
	}
	if( zero ) printf( "table entry %d is invalid\n", zero );
	free( a );
	for( g = stab->threader[S_GSYM]; g; g = g->n_thread ){
		if( g->n_value == 0 ){
			s = lookup( table(g),
				IV(S_EDATA,S_EBSS,S_EABS,S_ETEXT), g->n_name, 0 );
			if( !s ){
				printf( "undefined: %0.8s in ", g->n_name );
				ramnldump(g);
				break;
			}
			g->n_type = s->n_type;
			g->n_value = s->n_value;
		}
	}
	verify( stab, S_BSTR );
	verify( stab, S_BENUM );
}

threadP( stab )
struct stable *stab;
{
	static struct ramnl P[3];

	if( stab->threader[S_GSYM] == &P[0] ) return;
	P[0].n_other = S_GSYM;
	strcpy( P[0].n_name, "P" );
	P[0].n_value = PROC_INDEX;
	P[0].n_desc = PTR+STRTY;
	P[1].n_other = S_TYID;
	strcpy( P[1].n_name, "Proc" );
	P[0].n_thread = stab->threader[S_GSYM];
	stab->threader[S_GSYM] = &P[0];
}

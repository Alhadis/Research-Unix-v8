#include "common.h"
#include "frame.h"
#include "user.h"
#include "../isp.h"
#include "../traps.h"

MLONG mpxloc( id )
char *id;
{
	struct ramnl *s = lookup( MPX, LDE, id, 0 );

	return s ? s->n_value : 0;
}

struct ramnl *bfunsline( b )
struct ramnl *b;
{
	struct ramnl *s = b;

	for( s = b; b && s->n_other != S_BFUN; ++s )
		if( s->n_other == S_SLINE ) return 0;
	return b;
}

struct frame *framechain( modes )
{
	struct	frame 	*p, *c, *user = 0;	
	struct  ramnl 	*b, *s;
	int		limit = 32, report = 0;
	MLONG		pc, trap = mpxloc( "trap" ), resume = mpxloc( "resume" ),
			sw = mpxloc( "sw" ), windowpr = mpxloc( "windowpr" );
#define sw_etc(f) ( f==sw || f==resume || f==windowpr || f== trap )
	int		verbose = modes&VERBOSE_CHAIN, mpx = modes&MPX_CHAIN,
			full = modes&FULL_CHAIN;

	if( !trap || !resume || !sw || !windowpr )
		printf( "mpx locations not found\n" );
	assert( p = (struct frame *) calloc( 1, sizeof *p ) );
	p->fp = peeklong( FRAME_INDEX );
	p->pc = resume;
	while( p->fp && limit-- ){
		strcpy( p->fname, "?" );
		addr_desc( p->fp, DO_STACK );
		p->retaddr = longfromjerq();
		p->function = longfromjerq();
		if( !p->retaddr ) break;
		if( b = lookup(USER_MPX, IV(S_BFUN,S_ETEXT,S_TEXT,S_EABS), 0, p->pc )){
			if( b->n_other == S_BFUN ){
				p->bfun = b;
				p->sline = lookup( USER_MPX, S_SLINE, 0, p->pc );
				if( !p->sline ) p->sline = bfunsline( b );
				p->so = lookup( USER_MPX, S_SO, 0, p->pc );
			}
			if( p->function && p->function != b->n_value ){
			 	if( idmatch(b->n_name, "ldiv1g")
				    && p->callee
					&& p->callee->function == trap ){
/* complete magic! ==>>	*/			p->pc = peeklong(p->callee->fp+86);
						continue;
				}
				if( !full || !verbose ) return p;
				printf( "\nmemory/tables inconsistency: %0.8s()\n",
								b->n_name );
				dumpframe( p );
				return p;
			}
			strncpy( p->fname, b->n_name, 8 );
		} else if( p->function ) strcpy( p->fname, doh(p->function) );
		if( !p->function ){
			if( b ) p->function = b->n_value;
			addr_desc( p->function, DO_FUNC );
		}
		if( !idmatch(b->n_name, "_start") &&
		    ( mpx || !sw_etc(p->function) ) ){
			p->visible = 1;
			if( mpx || verbose ){
				mpx = 1;
				++report;
				context( p, REGS_CONTEXT|LINE_CONTEXT );
				putchar( '\n' );
			}
		}
		if( p->function){
			MLONG maskdelta;

			jerqdo( DO_DELTAS );
			maskdelta = longfromjerq();
			p->regmask = maskdelta >> 16;
			p->regdelta = (MWORD) maskdelta;
			p->regbase = p->fp + p->regdelta;
		}
		if( trap && p->function == trap ){	/* see mpx trap code */
			p->regmask = 0x3FFF;
			p->regbase = p->fp + 3*4;
			p->retaddr = peeklong( p->regbase + 2 + 14*4 );
			if( peeklong( TYPE_INDEX ) == BPT_TRAP ) p->retaddr -= 2;
			pc = p->retaddr;
			if( mpx ) printf( "%s pc=%s\n",
				trapnames[peeklong( TYPE_INDEX )], doh(p->retaddr));
			if( verbose && (s = lookup( USER_MPX, S_BFUN, 0, pc ) )
				&&  pc < symbptloc( s ) ){
				    printf( "in prolog of %0.8s()\n", s->n_name );		    		    break;
			}
			if( verbose && (s = lookup( USER_MPX, S_EFUN, 0, pc-2 ) ) ){
				symbptloc(s);
				if( pc <= s->n_value+s->n_extra ){
				    printf( "in epilog of %0.8s()\n", s->n_name );
				    break;
				}
			}

		}
		if( trap && p->callee && p->callee->function==trap ){
			
			if( !full ){
				p->caller = &sentinel;
				return p;
			}
		}
		if( !user && region(USER, p->function) == N_TEXT ){
			user = p;
			if( !full ){
				p->caller = &sentinel;
				return user;
			}
		}
		assert( c = (struct frame *) calloc( 1, sizeof *c ) );
		c->fp = peeklong( p->fp );		
		c->pc = p->retaddr;
		p->caller = c;
		c->callee = p;
		p = c;
	}
	return user ? user : p;
}

dumpframe( p )
struct frame *p;
{
	printf( "fp: %d pc: %d fname: %s ", p->fp, p->pc, p->fname ); 
	printf( "function: %d\n", p->function );
	printf( "retaddr: %d regdelta: %d ", p->retaddr, p->regdelta );
	printf( "regmask: x%X regbase: %d\n", 0xFFFF & p->regmask, p->regbase );
	if( p->noregs) printf( "regs not live\n" );
	if( p->sline ) ramnldump( p->sline );
	if( p->so ) ramnldump( p->so );
	if( p->bfun ) ramnldump( p->bfun );
}


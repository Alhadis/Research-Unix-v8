#include "common.h"
#include "frame.h"
#include "mcc.h"
#include "user.h"
#include "../traps.h"

char *mcctype( s )
struct ramnl *s;
{
	char *buffer = talloc(32);
	int t = s->n_desc;

	for( t >>= BTSHIFT; t; t >>= TSHIFT )
				strcat( buffer,  &("*\0\0()\0[]\0"[(t&3)*3-3]) );
	strcat( buffer, basetypenames[s->n_desc&BTMASK] );
	if( (++s)->n_other == S_TYID )
		sprintf( &buffer[strlen(buffer)], " %0.8s", s->n_name );
	return buffer;
}

char *display( s, f )
struct ramnl *s;
struct frame *f;
{
	char   *d = talloc(32);
	MLONG  p = locate( s, f );
	MWORD  e;
	int    r = (s->n_other == S_RSYM);

	if( !p ) return "<location error>";
	switch( s->n_other ){
		case S_BFUN: snarfdot = s->n_value;
			snarfdotty = 0;
			return fmt( "%s()", doh(s->n_value) );
		case S_GSYM:
		case S_STSYM:
		case S_RSYM:
		case S_PSYM:
		case S_LSYM:
		case S_SSYM: break;
		case 0: return ntypenames[ s->n_type>>1 ];
		default :	return "<storage class error>";
	}
	switch( s->n_desc & C_TMASK ){
	    case PTR: if( s->n_desc != PTR+CHAR ) return doh(peeklong(p));
			else  return fmtstring( peeklong( p ) );
	    case FTN: return "<function>";
	    case ARY: snarfdot = p;
		      switch( DECREF(s->n_desc) ){
			default:    snarfdotty = 0; break;
			case CHAR:
			case UCHAR: snarfdotty = BYTEDOT; break;
			case INT:
			case UNSIGNED:
			case ENUMTY: snarfdotty = WORDDOT; break;
			case DOUBLE:
			case LONG:
			case ULONG: snarfdotty = LONGDOT; break;
		      }
		      return DECREF(s->n_desc)==CHAR ? fmtstring(p) : doh(p);
	    case 0:
		switch( s->n_desc & BTMASK ){
		default:   return "<unimplemented>";
		case UCHAR:	return fmt( "%u", peekbyte(r?p+3:p) );
		case CHAR:	return fmtchar( peekbyte( r ? p+3 : p ) );
		case INT: 	return fmt( "%d", (MWORD) peekword( r ? p+2 : p ) );
		case UNSIGNED:	return fmt( "%u", peekword( r ? p+2 : p ) );
		case LONG:	return fmt( "%d", peeklong(p) );
		case ULONG:	return fmt( "%u", peeklong(p) );
		case DOUBLE:	return fmt( "%g", f68ktovax(peeklong(p)) );
		case ENUMTY:
			if( (++s)->n_other != S_TYID ) return "<missing tables>";
			if( s->n_name[0] == '\0' ) return "<anon enum>";
			s = lookup( table(s), S_BENUM, s->n_name, 0 );
			if( !s ) return "<enum error>";
			e = peekword( r ? p+2 : p );
			for( ++s; s->n_other != S_EENUM; ++s )
			    if( s->n_other == S_ENUM && s->n_value == e )
					return strncpy( d, s->n_name, 8 );
			sprintf( d, "<enum %d>", e );
			return d;
		case UNIONTY:
		case STRTY:
#define			WIDTH 1024
			(d = talloc(WIDTH))[0] = '{';
			if( (++s)->n_other != S_TYID ) return "<missing tables>";
			if( s->n_name[0] == '\0' ) return "<anon struct>";
			s = lookup( table(s), S_BSTR, s->n_name, 0 );
			if( !s ) return "<struct error>";
			for( ++s; s->n_other != S_ESTR && strlen(d) < WIDTH-12; ){
				sprintf( &d[strlen(d)], "%0.8s=", s->n_name );
				s->n_value += p;
				strncat( d, display(s, 0), WIDTH - strlen(d) - 12 );
				s->n_value -= p;
				for( ++s; s->n_other == S_TYID
						|| s->n_other == S_DIM; ++s ){}
				if( s->n_other == S_SSYM ) strcat( d, "," );
			}
		 	return strcat( d, "}" );
		}
	}
	ramnldump( s );
	return "<internal error in display()>";
}

localsc()
{
	long	rslp = IV(S_RSYM, S_STSYM, S_LSYM, S_PSYM);
	long h = 1000;
	struct frame *f = train;
	struct ramnl	*s;

	assert( hatebs != ACTIVE );
	if( !visible( f->sline, rslp, "*", 0 ) ){
		printf( "no locals in %s()\n", f->fname );
		return;
	}
	for( ;; ){
	    scratch();
	    for( s = f->sline; s = visible( s, rslp, "*", 0 ); ){
		scrins(
		    fmt( "%0.8s\t%s", s->n_name, storagenames[s->n_other]), s );
	    }
	    if( (h = scrhit(h)) == -1 ) return;
	    chasesym( h );
	}
}

globalsc()
{
	long	h = 0;
	struct  ramnl *s;

	for( ;; ){
	scratch();
	    for( s = lookup( USER, S_GSYM, "*", 0 ); s; s = s->n_thread )
	       if( !lookup( USER, S_BFUN, s->n_name, 0 ) ){
	 	    scrsort( fmt( "%0.8s\t%s",
			s->n_name, storagenames[s->n_other]),
				s );
	    }
	    if( (h = scrhit(h)) == -1 ) return;
	    chasesym( h );
	}
}

stkframec()
{
	struct frame *f;
	long i, ct = 0;

	if( train == 0 ){
		jerqmenu( M_PENDULUM, 1 );
		train = framechain(FULL_CHAIN);
		jerqmenu( M_PENDULUM, 0 );
		if( !train ) return;
	}
	for( f = train; f; f = f->caller ) if( f == &sentinel ){
		jerqmenu( M_PENDULUM, 1 );
		train = framechain(FULL_CHAIN);
		jerqmenu( M_PENDULUM, 0 );
		if( !train ) return;
		break;
	}
	for( f = train; f->callee; f = f->callee ) {}
	scratch();
	for( ; f; f = f->caller ) if( f->visible ){
		scrapp( fmt( "%s()", f->fname ), f );
		++ct;
	}
	if( ct == 0 ){
		printf( "empty traceback.\n" );
		return;
	}
	switch(i = scrhit(0) ){
		case -1: return;
		default: context( train = (struct frame *) i,
						REGS_CONTEXT|LINE_CONTEXT );
			 putchar( '\n' );
	}
}
	
context( f, mode )
struct frame *f;
{
	int		sl = II( S_STSYM, S_LSYM );
	struct ramnl	*s, *psym, *rsym;
	int		psymct = 0;

	if( !f ){
		printf( "<context not determined>" );
		return;
	}
	if( !f->bfun || !f->so || !f->sline ){
		printf( "pc=%s in %s() <missing tables>", doh(f->pc), f->fname );
		return;
	}
	if( mode&LINE_CONTEXT ) printf( "%s in ", soline( f->pc ) );
	printf( "%s(", f->fname);
	for( psym = f->bfun; psym->n_other != S_EFUN; ++psym )
	    if( psym->n_other == S_PSYM ){
		rsym = 0;
		for( s = psym;  s->n_other != S_BFUN && !rsym; --s ){
		    if( s->n_other == S_RBRAC && s->n_desc != 0 ) break;	
		    if( s->n_other == S_RSYM && idmatch( s->n_name, psym->n_name ) )
			rsym = s;
		}
		if( psymct++ > 0 ) putchar( ',' );
		printf( "%0.8s=%s", psym->n_name, display( psym, f ) );
		if( rsym && (mode&REGS_CONTEXT) )
			printf( "/reg %0.8s=%s", rsym->n_name, display( rsym, f));
	}
	printf( ")" );
	if( !(mode&AUTO_CONTEXT) ) return;
	putchar( '\n' );
	for( s = f->sline; s = visible( s, sl, "*", 0 ); )
		printf( "%0.8s=%s\n", s->n_name, display( s, f ) );
}

MLONG locate( s, f )
struct ramnl *s;
struct frame *f;
{
	MWORD	mask;
	MLONG	p;
	int	r;
	struct ramnl *g;
	
	switch( s->n_other ){
	case S_BFUN: return s->n_value;
	case S_SSYM: assert( !f ); return s->n_value;
	case S_GSYM:
		if( s->n_value ) return s->n_value;
		if(g = lookup( USER_MPX, S_EBSS, s->n_name, 0 )) return g->n_value;
		return 0;
	case S_STSYM: return( s->n_value );
	case S_PSYM: assert( f ); return ( f->fp + s->n_value );
	case S_LSYM: assert( f ); return( f->fp + f->regdelta - s->n_value );
	case S_RSYM:
		r = s->n_value;
		if( !f && r > 15 ) return r;		/* function return in reg */
		for( ; f->callee; f = f->callee ){
			if( f->callee->regbase == 0 ) return 0;
			if( ( mask = f->callee->regmask ) & (1<<r) ){
				for( p = f->callee->regbase; r > 0; --r ){
					if( mask&1 ) p += 4;
					mask >>= 1;
				}
				return p;
			}
		}
		return 0;
	case 0: return s->n_value;
	}
	return 0;
} 


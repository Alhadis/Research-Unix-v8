#include "common.h"
#include "expr.h"
#include "user.h"
#include "frame.h"
#include "../traps.h"

exprdump( e )
struct expr *e;
{
	struct dim *d;

	if( !e ) return;
	printf( "%d: ", e );
	switch( e->e_tag ){
		case E_SYM: printf( "sym: " ); break;
		case E_ID: printf( "id: "); break;
		case E_CONST: printf( "const: " ); break;
		case E_UNARY: printf( "unary: " ); break;
		case E_BINARY: printf( "binary: " ); break;
		default: printf( "bad tag=%d: ", e->e_tag ); break;
	}
	printf("rval=%d ", e->e_rvalue );
	if(e->e_lvalue) printf( "lval=%d%s ", e->e_lvalue, e->e_hostr?"(host)":"" );
	if( e->e_op ) printf( "op=%c ", e->e_op );
	if( e->e_id ) printf( "id=%s ", e->e_id );
	if( e->e_sym ) printf( "sym=%0.8s ", e->e_sym->n_name );
	if( e->e_reg ) printf( "reg " );
	if( e->e_left ) printf( "left=%d ", e->e_left );
	if( e->e_right ) printf( "right=%d ", e->e_right );
	if( e->e_mcc ){
		struct ramnl dummy;
		dummy.n_desc = e->e_mcc;
		printf( "%s ", mcctype(&dummy) );
	}
	if( e->e_tyid ) printf( "%0.8s ", e->e_tyid->n_name );
	for( d = e->e_dim; d; d = d->d_next ) printf( "[%d]", d->d_dim );
	putchar( '\n' );
	if( e->e_sym ) ramnldump( e->e_sym );
}
		
char *fmtexpr( e )
struct expr *e;
{
	exprdump( e );
	if( !e ) return "<nil>";
	switch( e->e_tag ){
	case E_SYM:
		return fmt( "%0.8s", e->e_sym->n_name );
	case E_ID:
		return e->e_id;
	case E_CONST:
		return fmt( "%d", e->e_rvalue );
	case E_UNARY :
		return fmt( "(%c%s)", e->e_op, fmtexpr(e->e_left) );
	case E_BINARY:
		return fmt( "(%s%c%s)",
				fmtexpr(e->e_left), e->e_op, fmtexpr(e->e_right) );
	default: assert(0);
	}
}

struct expr *enode( t, s1, s2, s3 )
enum exprtag t;
{
	struct expr *e = (struct expr *) talloc( sizeof *e );

	switch( e->e_tag = t ){
	case E_SYM:
		e->e_sym = (struct ramnl *) s1;
		return e;
	case E_ID:
		e->e_id = (char *) s1;
		return e;
	case E_CONST:
		e->e_rvalue = s1;
		return e;
	case E_UNARY:
		e->e_op = s1;
		e->e_left = (struct expr *) s2;
		return e;
	case E_BINARY:
		e->e_op = s2;
		e->e_left = (struct expr *) s1;
		e->e_right = (struct expr *) s3;
		return e;
	default: assert(0);
	}
}

struct dim *dnode( dval, nval )
struct dim *nval;
{
	struct dim *d = (struct dim *) talloc( sizeof *d );

	d->d_dim = dval;
	d->d_next = nval;
	return d;
}

static exprmode;

zec()
{
	exprmode = exprmode ? 0 : 1;
}

exprc( e )
struct expr *e;
{
	struct ramnl pair[2];
	int i, n;
	MLONG argarea;		/* use arg area for scratch space */

	if( !eval( e ) ){
		if( exprmode ) printf( "%s\n", fmtexpr( e ) );
		return;
	}
	if( e->e_lvalue && e->e_hostr ){
		e->e_hostr = 0;
		n = emccsize(e);
		addr_desc(0,DO_INVOKE);
		argarea = longfromjerq();
		for( i = 0; i < n; ++i )
			pokebyte( argarea+i, *(char*) (e->e_lvalue+i) );
		e->e_lvalue = argarea;
	}
	if( exprmode ) printf( "%s\n", fmtexpr( e ) );
	if( e->e_lvalue ){
		pair[0].n_desc = e->e_mcc;
		pair[0].n_value = e->e_lvalue;
		pair[0].n_other = e->e_reg ? S_RSYM : S_GSYM;
		pair[1] = *(e->e_tyid);
		printf( "%s\n", display( &pair[0], 0 ) );
	} else
		printf( "%d\n", e->e_rvalue );
}

pokec( e )
struct expr *e;
{
	if( !eval( e ) ){
		if( exprmode ) printf( "%s\n", fmtexpr( e ) );
		return;
	}
	if( exprmode ) printf( "%s\n", fmtexpr( e ) );
	switch( dotty ){
		case BYTEDOT: showdot(); pokebyte( dot, e->e_rvalue ); break;
		case WORDDOT: showdot(); pokeword( dot, e->e_rvalue ); break;
		case LONGDOT: showdot(); pokelong( dot, e->e_rvalue ); break;
		default:
			printf( "use .b, .w, or .l to set operand length for =\n" );
			return;
	}
	showdot();
}

dotexprc( e )
struct expr *e;
{
	struct ramnl pair[2];
	MLONG val;

	if( !eval( e ) ){
		if( exprmode ) printf( "%s\n", fmtexpr( e ) );
		return;
	}
	if( exprmode ) printf( "%s\n", fmtexpr( e ) );
	if( !(val = e->e_rvalue) ) val = e->e_lvalue;
	if( val == 0 && e->e_tag == E_CONST ){
		dot = 0;
		return;
	}
	if( !inmemory(val) ){
		printf( "%d cannot be used as an address\n", val );
		return;
	}
	dot = val;
	showdot();
}

#define right e->e_right
#define left e->e_left
#define exprerr(err) { printf( "%s\n", err ); return 0; }

settyiddim( e, s )
struct expr *e;
struct ramnl *s;
{
	if( (++s)->n_other == S_TYID ) e->e_tyid = s++;
	while( s->n_other == S_DIM ) ++s;
	for( --s; s->n_other == S_DIM; --s )
		e->e_dim = dnode( s->n_desc, e->e_dim );
}

eval( e )
struct expr *e;
{
	int glb = II( S_GSYM, S_STSYM );
	int lcl = III(S_RSYM,S_LSYM,S_PSYM);
	struct ramnl *s = 0;
	long size;

	if( !e ) return 1;
	switch( e->e_tag ){
	default: assert(0);
	case E_SYM:
	case E_ID:
		if( !train && hatebs != ACTIVE && hatebs != ERRORED )
			train = framechain(0);
		if( e->e_tag == E_SYM ) s = e->e_sym;
		if( !s && train && train->sline )
			s = visible( train->sline, lcl, e->e_id, 0 );
		if( !s ) s = lookup( USER_MPX, glb, e->e_id, 0 );
		if( !s ){
			if( s = lookup( USER_MPX, S_ENUM, e->e_id, 0 ) ){
				e->e_rvalue = s->n_value;
				e->e_mcc = LONG;
				return 1;
			}
			if( s = lookup( USER_MPX, LDE, e->e_id, 0 ) ){
				e->e_rvalue = s->n_value;
				e->e_mcc = LONG;
				return 1;
			}
			if(s=lookup(USER_MPX,III(S_DATA,S_BSS,S_TEXT),e->e_id,0)){
				e->e_rvalue = s->n_value;
				e->e_mcc = LONG;
				return 1;
			}
			exprerr( fmt( "variable not found: %s", e->e_id ) );
		}
		e->e_lvalue = locate( s, train );
		e->e_mcc = s->n_desc;
		e->e_reg = s->n_other==S_RSYM;
		settyiddim( e, s );
		e->e_rvalue = rvalue(e->e_lvalue, e->e_mcc, e->e_reg );
		return 1;
	case E_CONST:
		e->e_mcc = LONG;
		return 1;
	case E_UNARY:
		if( !eval( left ) ) return 0;
		switch( e->e_op ){
		default:assert(0);
		case '-':
			if( !numeric(left->e_mcc) )
				exprerr( "invalid operand of unary -" );
			e->e_mcc = LONG;
			e->e_rvalue = -left->e_rvalue;
			return 1;
		case '*':
			if( !ISPTR(left->e_mcc) )
				exprerr( "invalid operand of *" );
			e->e_lvalue = left->e_rvalue;
			e->e_tyid = left->e_tyid;
			e->e_dim = left->e_dim;
			e->e_mcc = DECREF( left->e_mcc );
			e->e_rvalue = rvalue( e->e_lvalue, e->e_mcc, 0 );
			return 1;
		case '&':
			if( !left->e_lvalue || left->e_hostr )
				exprerr( "invalid operand of &" );
			e->e_rvalue = left->e_lvalue;
			e->e_tyid = left->e_tyid;
			e->e_dim = left->e_dim;
			e->e_mcc = INCREF(left->e_mcc );
			e->e_lvalue = 0;
			return 1;
		}
	case E_BINARY:
		switch( e->e_op ){
		default:assert(0);
		case ',':
			return eval(left) && eval(right);
		case '(':
			return apply( e );
		case '=':
			if( !eval(left) || !eval(right) ) return 0;
			return assign( e );
		case '>':
			if( !eval(left) ) return 0;
			if( left->e_mcc != (PTR+STRTY) )
				exprerr( "invalid left side of ->" );
			if( !field( left->e_tyid, right ) ) return 0;
			e->e_mcc = right->e_mcc;
			e->e_tyid = right->e_tyid;
			e->e_dim = right->e_dim;
			e->e_lvalue = left->e_rvalue + right->e_lvalue;
			e->e_rvalue = rvalue( e->e_lvalue, e->e_mcc, 0 );
			return 1;
		case '.':
			if( !eval(left) ) return 0;
			if( !left->e_lvalue ) exprerr( "invalid left side of ." );
			if( left->e_mcc!=STRTY && left->e_mcc!=UNIONTY )
				exprerr( "invalid left side of ." );
			if( !field( left->e_tyid, right ) ) return 0;
			e->e_mcc = right->e_mcc;
			e->e_tyid = right->e_tyid;
			e->e_dim = right->e_dim;
			e->e_lvalue = left->e_lvalue + right->e_lvalue;
			if( e->e_hostr = left->e_hostr )
				e->e_rvalue = hostrvalue( e->e_lvalue, e->e_mcc );
			else
				e->e_rvalue = rvalue( e->e_lvalue, e->e_mcc, 0 );
			return 1;
		case '[':
			if( !eval(left) || !eval(right) ) return 0;
			if( !numeric( right->e_mcc ) )
				exprerr( "invalid right side of [" );
			if( !ISARY(left->e_mcc) && !ISPTR(left->e_mcc) )
				exprerr( "invalid left operand of [" );
			e->e_tyid = left->e_tyid;
			e->e_mcc = DECREF(left->e_mcc);
			e->e_dim = left->e_dim ? left->e_dim->d_next : 0;
			size = emccsize(e);
			if( !size ) exprerr( "invalid left operand of [" );
			e->e_lvalue = size*right->e_rvalue +
			    (ISPTR(left->e_mcc) ? left->e_rvalue : left->e_lvalue);
			e->e_rvalue = rvalue( e->e_lvalue, e->e_mcc, 0 );
			return 1;
		case '%':
		case '*':
		case '/':
			if( !eval(left) || !eval(right) ) return 0;
			if( !numeric(left->e_mcc) )
				exprerr( fmt("invalid left side of %c", e->e_op) );
			if( !numeric(right->e_mcc) )
				exprerr( fmt("invalid right side of %c", e->e_op) );
			e->e_mcc = LONG;
			e->e_rvalue =
				e->e_op=='*' ? left->e_rvalue * right->e_rvalue
			      : e->e_op=='%' ? left->e_rvalue % right->e_rvalue
					     : left->e_rvalue / right->e_rvalue;
			return 1;
		case '+':
		case '-':
			if( !eval(left) || !eval(right) ) return 0;
			if( ISARY(left->e_mcc) ){
				left->e_rvalue = left->e_lvalue;
				left->e_mcc = INCREF(DECREF(left->e_mcc));
				if( left->e_dim ) left->e_dim = left->e_dim->d_next;
			}
			if( ISARY(right->e_mcc) ){
				right->e_rvalue = right->e_lvalue;
				right->e_mcc = INCREF(DECREF(left->e_mcc));
				if(right->e_dim)right->e_dim = right->e_dim->d_next;
			}
			if( ISPTR(left->e_mcc) && numeric(right->e_mcc) ){
			    	size = mccsize(DECREF(left->e_mcc),left->e_tyid );
				if( !size)
				    exprerr(fmt("invalid left side of %c",e->e_op));
				e->e_rvalue = left->e_rvalue +
				    (e->e_op=='-' ? -size : size )*right->e_rvalue;
				e->e_mcc = left->e_mcc;
				e->e_tyid = left->e_tyid;
				return 1;
			}
			if( numeric(left->e_mcc) && numeric(right->e_mcc) ){
				e->e_mcc = LONG;
				e->e_rvalue = left->e_rvalue + 
				    (e->e_op=='-' ? -1 : 1 )*right->e_rvalue;
				return 1;
			}
			if( e->e_op == '+' && 
				numeric(left->e_mcc) && ISPTR(right->e_mcc) ){
				size = mccsize( DECREF(right->e_mcc),right->e_tyid );
				if( !size ) exprerr( "invalid right side of +" );
				e->e_rvalue = right->e_rvalue + size*left->e_rvalue;
				e->e_mcc = right->e_mcc;
				e->e_tyid = right->e_tyid;
				return 1;
			}
			if( e->e_op == '-' &&
				ISPTR(left->e_mcc) && ISPTR(right->e_mcc) ){
				size = mccsize( DECREF(right->e_mcc),right->e_tyid );
				if( !size ||
				  size != mccsize( DECREF(left->e_mcc),left->e_tyid))
					exprerr( "invalid difference of pointers" );
				e->e_rvalue = (left->e_rvalue-right->e_rvalue)/size;
				e->e_mcc = LONG;
				return 1;
			}
			exprerr( fmt("invalid operands of %c", e->e_op)  );		
		}
	}
}

rvalue( p, t, r )
MLONG p;
{
	switch( t & C_TMASK ){
	case PTR:  return peeklong( p );
	case 0:
		switch( t & BTMASK ){
		case UCHAR:
		case CHAR:	return peekbyte( r ? p+3 : p );
		case ENUMTY:
		case INT: 	return (MWORD) peekword( r ? p+2 : p );
		case UNSIGNED:	return peekword( r ? p+2 : p );
		case DOUBLE:
		case LONG:
		case ULONG:	return peeklong( p );
		}
	}
	return 0;
}

hostrvalue( p, t )
char *p;
{
	long four;
	short two;

	switch( t & C_TMASK ){
	case PTR: four = * (long *) p;
		  swop( &four, 4 );
		  return four;
	case 0:
		switch( t & BTMASK ){
		case UCHAR:	return * (unsigned char *) p;
		case CHAR:	return *p;
		case ENUMTY:
		case INT: 	two = * (short *) p;
				swop( &two, 2 );
				return two;
		case UNSIGNED:	two = * (short *) p;
				swop( &two, 2 );
				return ((long)two)&0xFFFF;
		case DOUBLE:
		case LONG:
		case ULONG:	four = * (long *) p;
				swop( &four, 4 );
				return four;
		}
	}
	return 0;
}


field( tyid, e )
struct ramnl *tyid;
struct expr *e;
{
	struct ramnl *s;

	if( e->e_tag != E_ID ) assert(0);
	if( !(s = lookup( USER_MPX, S_BSTR, tyid->n_name, 0 ) ) )
		exprerr( fmt("missing tables for struct %0.8s", tyid->n_name) );
	for( ++s; s->n_other != S_ESTR; ++s )
		if( s->n_other == S_SSYM && idmatch( s->n_name, e->e_id ) ){
			e->e_lvalue = s->n_value;
			e->e_mcc = s->n_desc;
			settyiddim( e, s );
			return 1;
		}
	exprerr( fmt("field not found: %s", e->e_id) );
}
			
numeric( t )
{
	switch( t ){
		default:
		case DOUBLE:
			return 0;
		case ENUMTY:
		case CHAR:
		case UCHAR:
		case INT:
		case UNSIGNED:
		case LONG:
		case ULONG:
			return 1;
	}
}

emccsize( e )
struct expr *e;
{
	return mccsize( e->e_mcc, e->e_tyid, e->e_dim );
}

mccsize( t, tyid, dimen )
struct ramnl *tyid;
struct dim *dimen;
{
	struct ramnl *s;

	switch( t&C_TMASK ){
	case PTR: return 4;
	case ARY:
		if( !dimen ) return 0;
		return dimen->d_dim * mccsize( DECREF(t), tyid, dimen->d_next );
	case FTN: return 0;
	case 0:
		switch( t&BTMASK ){
		default: assert(0);
		case UCHAR:
		case CHAR:
			return 1;
		case ENUMTY:
		case INT:
		case UNSIGNED:
			return 2;
		case DOUBLE:
		case LONG:
		case ULONG:
			return 4;
		case UNIONTY:
		case STRTY:
			if( !(s = lookup( USER_MPX, S_ESTR, tyid->n_name, 0 )))
				return 0;
			return s->n_value;
		}
	}
}

apply( e )
struct expr *e;
{
	struct ramnl *s;

	assert( left->e_tag == E_ID );
	if( idmatch( left->e_id, "Pt" ) ) left->e_id = "Point";
	if( idmatch( left->e_id, "Rpt" ) ) left->e_id = "Rectangle";
	if( !strcmp(left->e_id,"sizeof") ) return esizeof(e);
	if( !(s = lookup( USER_MPX, II(S_BFUN,S_BSTR), left->e_id, 0 ) ) )
		exprerr( fmt("function/structure not found: %s", left->e_id) );
	switch( s->n_other ){
		default: assert(0);
		case S_BSTR: return strconst( e );
		case S_BFUN: return invokef( e );
	}
}

esizeof( e )
struct expr *e;
{
	struct expr *r = right;

	if( !r || r->e_right ) exprerr( "should be sizeof(expr)" );
	assert( r->e_op == ',' );
	if( !eval(r->e_left) ) return 0;
	e->e_mcc = LONG;
	e->e_rvalue = emccsize(r->e_left);
	e->e_lvalue = 0;
	return 1;
}

invokef( e )
struct expr *e;
{
	struct ramnl *bfun, *psym, *lastpsym = 0, *gstsym, *s;
	MLONG fp, *argsave;
	struct expr *r = right, *assignee, *equals;
	int i, argspace = 0;
	char *host;
	
	assert( left->e_tag == E_ID );
	assert( bfun = lookup( USER_MPX, S_BFUN, left->e_id, 0 ) );
	if( !(gstsym = lookup( USER_MPX, II(S_GSYM,S_STSYM), left->e_id, 0 ) ) )
		exprerr( fmt("missing tables: %s()", left->e_id) );
	e->e_mcc = DECREF(gstsym->n_desc);
	e->e_tyid = gstsym+1;
	addr_desc( bfun->n_value, DO_INVOKE );
	fp = longfromjerq() - 8;
	for( psym = bfun; psym->n_other != S_EFUN; ++psym )
		if( psym->n_other == S_PSYM ) lastpsym = psym;
	if( lastpsym )
	    argspace = lastpsym->n_value + mccsize( lastpsym->n_desc, psym+1 );
	argspace /= 4;
	argsave = (MLONG *) talloc( argspace*4 );
	for( i = 0; i < argspace; ++i ) argsave[i] = peeklong( fp+8+i*4 );
	for( psym = bfun; psym->n_other != S_EFUN; ++psym )
	    if( psym->n_other == S_PSYM ){
		if( !r ) exprerr( fmt("too few arguments to %s()", left->e_id) );
		assert( r->e_op == ',' );
		if( !eval(r->e_left) ) return 0;
		assignee = enode(E_ID,fmt("%s(%0.8s)",left->e_id,psym->n_name));
		assignee->e_lvalue = fp+psym->n_value;
		assignee->e_mcc = psym->n_desc;
		if( (psym+1)->n_other == S_TYID ) assignee->e_tyid = psym+1;
		equals = enode(E_BINARY, assignee, '=', r->e_left);
		if( exprmode) fmtexpr(equals);
		if( !assign(equals) ) return 0;
		r = r->e_right;
	}
	if( r ) exprerr( fmt("too many arguments to %s()", left->e_id) );
	addr_desc( bfun->n_value, DO_INVOKE );	/* again because of arg eval */
	longfromjerq();
	if( ISPTR( e->e_mcc) ){
		addr_desc( I_ADDR, DO_INVOKE );
		e->e_rvalue = longfromjerq();
	} else if( e->e_mcc == STRTY || e->e_mcc == UNIONTY ){
		switch( mccsize( e->e_mcc, e->e_tyid ) ){
			case 2:
				addr_desc( I_DATA, DO_INVOKE );
				pokeword( e->e_lvalue = fp+8, longfromjerq() );
				break;
			case 4:
				addr_desc( I_DATA, DO_INVOKE );
				pokelong( e->e_lvalue = fp+8, longfromjerq() );
				break;
			default:
				addr_desc( I_ADDR, DO_INVOKE );
				e->e_lvalue = longfromjerq();
				break;
		}
		if( !(s = lookup( USER_MPX, S_ESTR, e->e_tyid->n_name, 0 )) )
			exprerr( "missing structure tables" );
		host = talloc( s->n_value );
		for( i = 0; i < s->n_value; ++i )
			host[i] = peekbyte(e->e_lvalue+i);
		e->e_hostr = 1;
		e->e_lvalue = (long) host;
	} else {
		addr_desc( I_DATA, DO_INVOKE );
		e->e_rvalue = longfromjerq();
	}
	for( i = 0; i < argspace; ++i ) pokelong( fp+8+i*4, argsave[i] );
	return 1;
}

assign( e )
struct expr *e;
{
	if( left->e_mcc==STRTY || right->e_mcc==STRTY )
		return strassign( e );
	if( !left->e_lvalue || ISFTN(right->e_mcc) || ISARY(left->e_mcc) )
		exprerr( "invalid assignment" );
	if( ISPTR(left->e_mcc) || left->e_reg )
		pokelong( left->e_lvalue, right->e_rvalue );
	else switch( left->e_mcc ){
		default:
		case DOUBLE:
			exprerr( "invalid assignment" );
		case UCHAR:
		case CHAR:
			pokebyte( left->e_lvalue, right->e_rvalue );
			break;
		case INT:
		case UNSIGNED:
		case ENUMTY:
			pokeword( left->e_lvalue, right->e_rvalue );
			break;
		case LONG:
		case ULONG:
			pokelong( left->e_lvalue, right->e_rvalue );
			break;
	}
	e->e_reg = left->e_reg;
	e->e_mcc = left->e_mcc;
	e->e_tyid = left->e_tyid;
	e->e_lvalue = left->e_lvalue;
	e->e_rvalue = rvalue( e->e_lvalue, e->e_mcc, e->e_reg );
	return 1;
}

strassign( e )
struct expr *e;
{
	struct ramnl *ls, *rs;
	int i;
	char c;

	if( left->e_mcc!=STRTY || right->e_mcc!=STRTY )
		exprerr( "invalid structure assignment" );
	if( !(rs = lookup( USER_MPX, S_ESTR, right->e_tyid->n_name, 0 ))
	 || !(ls = lookup( USER_MPX, S_ESTR, left->e_tyid->n_name, 0 )) )
		exprerr( "missing structure tables" );
	if( rs->n_value != ls->n_value )
		exprerr( "incompatible structure assignment" );

	/* copy each byte separately */
	for( i = 0; i < rs->n_value; ++i ){
		if( right->e_hostr ) c = *(char *) (right->e_lvalue+i);
		else c = peekbyte(right->e_lvalue+i);
		if( left->e_hostr ) *(char *) (left->e_lvalue+i) = c;
		else pokebyte(left->e_lvalue+i, c);
	}
	e->e_lvalue = left->e_lvalue;
	e->e_hostr = left->e_hostr;
	e->e_mcc = left->e_mcc;
	e->e_tyid = left->e_tyid;
	return 1;
}	

strconst( e )
struct expr *e;
{
	struct ramnl *s;
	struct expr *r = right, *assignee, *equals;
	long base;

	assert( left->e_tag == E_ID );
	assert( s = lookup( USER_MPX, S_ESTR, left->e_id, 0 ) );
	e->e_lvalue = (long) talloc( s->n_value );
	e->e_hostr = 1;
	assert( s = lookup( USER_MPX, S_BSTR, left->e_id, 0 ) );
	e->e_mcc = STRTY;
	e->e_tyid = (struct ramnl *) talloc( sizeof *e->e_tyid );
	e->e_tyid->n_other = S_TYID;
	strncpy( e->e_tyid->n_name, left->e_id, 8 );
	for( ; s->n_other != S_ESTR; ++s ) if( s->n_other == S_SSYM ){
		if( !r ) exprerr( fmt("too few fields in %s(...)", left->e_id) );
		assert( r->e_op == ',' );
		if( !(eval(r->e_left) ) ) return 0;
		base = e->e_lvalue + s->n_value;
		if( s->n_desc == STRTY ){
			assignee = enode(E_ID,fmt("%s.%0.8s",left->e_id,s->n_name));
			assignee->e_lvalue = base;
			assignee->e_hostr = 1;
			assignee->e_mcc = STRTY;
			assignee->e_tyid = s+1;
			equals = enode(E_BINARY, assignee, '=', r->e_left);
			if( exprmode) fmtexpr(equals);
			if( !strassign(equals) ) return 0;
		} else if( ISPTR(s->n_desc) ){
			*(MLONG *) base = r->e_left->e_rvalue;
			swop( base, 4 );
		} else switch( s->n_desc ) {
			case DOUBLE:
			default: exprerr( "structure field error" );
			case CHAR:
			case UCHAR:
				*(MBYTE *) base = r->e_left->e_rvalue;
				break;
			case INT:
			case UNSIGNED:
			case ENUMTY:
				*(MWORD *) base = r->e_left->e_rvalue;
				swop( base, 2 );
				break;
			case LONG:
			case ULONG:
				*(MLONG *) base = r->e_left->e_rvalue;
				swop( base, 4 );
				break;
		}
		r = r->e_right;
	}
	if( r ) exprerr( fmt("too many fields in %s(...)", left->e_id) );
	return 1;
}

swop( p, n )
char *p;
{
	char c;

	switch( n ){
	case 2:
		c = p[0]; p[0] = p[1]; p[1] = c;
		break;
	case 4:
		swop( p+1, 2 );
		c = p[0]; p[0] = p[3]; p[3] = c;
		break;
	}
}

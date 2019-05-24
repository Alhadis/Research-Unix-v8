#include "common.h"
#include "../menu.h"
#include "user.h"
#include "../traps.h"
#include "frame.h"

extern int currmenu, currbutt, buttmenu[];

extern NewMenu menus[];

long keys[M_MENUS][M_ITEMS];

jerqmenu( m, c )
{
	assert( !oflag );
	putchar( ESCAPE );
	putchar( m );
	putchar( c+M_SHIFT );
}

m_item( i, s, a )
char *s;
long a;
{
	if( !menus[currmenu].item[i] ) m_null( i+1 );
	keys[currmenu][i] = a;
	if( !strcmp( menus[currmenu].item[i], s ) ) return;
	strncpy( menus[currmenu].item[i], s, M_SLOTLEN );
	printf( "%c%c%c%s%c",
		ESCAPE, M_ITEM, i+M_SHIFT, menus[currmenu].item[i], 0 );
}

long m_key( i )
{
	return keys[currmenu][i];
}

m_menu( m, b )
{
	if( b == 0 ) b = currbutt;
	if( currmenu == m && currbutt == b && buttmenu[b] == m ) return;
	buttmenu[b] = m;
	jerqmenu( M_MENU, currmenu = m );
	jerqmenu( M_BUTT, b );
}

m_null(i)
{
	int z;
	char *s;

	if( menus[currmenu].item[i] == 0 ) return;
	for( z = 0; z <= M_ITEMS; ++z ) if( !menus[currmenu].item[z] ){
		s = menus[currmenu].item[i];
		menus[currmenu].item[i] = 0;
		menus[currmenu].item[z] = s;
		jerqmenu( M_NULL, i );
		return;
	}
	assert( "zero" );
}


invoke( f )
int (*f)();
{
	(*f)();
}

menuhit( i, b )
{
	i -= M_SHIFT;
	currbutt = b-M_SHIFT;
	if( i == -1 ) return;
	switch( buttmenu[currbutt] ){
	default: assert(0);
	case M_MAIN:
		invoke( keys[M_MAIN][i] );
		return;
	case M_MEMORY:
		dotop( keys[M_MEMORY][i] );
		return;
	}
}

static addrlen, contlen;
static char *addrstr[DOTOPS+1], *contstr[DOTOPS+1];

measure(op)
{
	if( strlen(addrstr[op]=doh(dot)) > addrlen ) addrlen = strlen(addrstr[op]);
	if( strlen(contstr[op]=fmtdot()) > contlen ) contlen = strlen(contstr[op]);
}

meas_item( i, arrow, op )
{
	m_item( i,
		fmt( "%c %*s: %-*s",
			arrow, addrlen, addrstr[op], contlen, contstr[op] ),
				op );
}

m_set2()
{
	int i = 0, al = 0, cl = 0, op;
	char *d = 0;

	m_menu( M_MEMORY, 2 );
	if( !dot && snarfdot ){
		dot = snarfdot;
		snarfdot = 0;
		if( snarfdotty ) dotty = snarfdotty;
	}
	if( !dot ){
		m_item( i++, "Use . <expr> to", 0 );
		m_item( i++, "select location", 0 );
		m_null( i );
		return;
	}
	if( dot && dotty != INSTDOT ){
		for( op = 0; op <= DOTOPS; ++op ) addrstr[op] = contstr[op] = 0;
		addrlen = contlen = 0;
		movedot( -1 );
		if( inmemory(dot) ) measure(MINUSONE);
		movedot( 1 );
		if( inmemory(dot) ){
			measure(0);
			if( dotty == LONGDOT ){
				MLONG savedot = dot;
				dot = peeklong( dot );
				if( inmemory(dot) ) measure(INDIRECT);
				dot = savedot;
			}
		}
		movedot( 1 );
		if( inmemory(dot) ) measure(PLUSONE);
		movedot( -1 );
		if( addrstr[MINUSONE] ) meas_item( i++, ' ', MINUSONE );
		if( addrstr[0] ) meas_item( i++, '.', 0 );
		if( addrstr[PLUSONE] ) meas_item( i++, ' ', PLUSONE );
		if( addrstr[INDIRECT] ) meas_item( i++, '*', INDIRECT );
	}
	
	if( dotty == STRUCTDOT && (op=strop(dotstab[1].n_name)) && inmemory(dot) )
			m_item( i++, graphopnames(op), op );

	if( dotty != BYTEDOT ) m_item( i++, "byte", BYTEDOT );
	if( dotty != WORDDOT ) m_item( i++, "word", WORDDOT );
	if( dotty != LONGDOT ) m_item( i++, "long", LONGDOT );
	if( dotty != STRINGDOT ) m_item( i++, "string", STRINGDOT );
	if( dotfmt != HEX ) m_item( i++, "hex", HEX );
	if( dotfmt != DECIMAL ) m_item( i++, "decimal", DECIMAL );
	if( dotfmt != OCTAL ) m_item( i++, "octal", OCTAL );
	if( dotty != STRUCTDOT && dotfmt != ASCII ) m_item( i++, "ascii", ASCII );
	if( lookup( USER, S_BSTR, "*", 0 ) ) m_item( i++, "struct", STRUCTDOT );
	if( lookup( USER, S_ENUM, "*", 0 ) ) m_item( i++, "enum", ENUMDOT );
	if( d = fmtxref() ) m_item( i++, fmt( ". %8s", d), XREF );
	if( inmemory(snarfdot) && snarfdot != dot )
		m_item( i++, fmt( "%s: ", doh(snarfdot)), SNARF );
	m_null( i );
}

goc1(){ goc(1); }

m_set3()
{
	int i = 0;

	m_menu( M_MAIN, 3 );
	m_item( i++, "layer", newc );
	m_item( i++, "quit", quitc );
	if( hatebs != ERRORED ){
		m_item( i++, "breakpts", breakptmenu );
		m_item( i++, "globals", globalsc );
	}
	switch( hatebs  ){
	case MODIFIED:
	case BREAKPTED:
	case STEPPED:
		m_item( i++, "stmt step", stmtstepc );
	case HALTED:
		m_item( i++, "go", goc1 );
	case TRAPPED:
		m_item( i++, "traceback", tracebackc );
		m_item( i++, "function", stkframec );
		if( train && train->bfun && train->sline )
		    m_item( i++, fmt( "%s() vars", train->fname ), localsc );
		break;
	case ACTIVE:
		m_item( i++, "halt", haltc );
		break;
	case ERRORED: break;
	default: assert(0);
	}
	m_null(i);
}

menuset(){
	m_set2();
	m_set3();
}

menuforce( m )
{
	m_menu( m, currbutt );
	jerqmenu( M_HIT, currbutt ); flush();
	return (short) wordfromjerq() - M_SHIFT;
}

menuconfirm()
{
	jerqdo( M_CONFIRM );
	return getchar();
}

stmtstepc()
{
	cinglec( 1 );
}

resetmenus()
{
	assert(0);	/* -x option */
}

m_range(lo,hi,miss)
{
	int hit;

	jerqdo(M_LIMITS);
	longtojerq(lo);
	longtojerq(hi);
	hit = menuforce(M_NUMERIC);
	return hit == -1 ? miss : lo+hit;
}

bv_force(unpacked)
long unpacked[];
{
	int i;

	for( i = 0; i < M_BVSIZE/8; ++i ) bitvector[i] = 0;
	for( i = 0; i < M_BVSIZE; ++i )
		if( unpacked[i] ) bitvector[i/8] |= 1<<(i%8);
	jerqdo( M_BITVEC );
	for( i = 0; i < M_BVSIZE/8; ++i ) putchar(bitvector[i]);
	i = menuforce(M_NUMERIC);
	return i == -1 ? -1 : bvpos( i );
}

bv_dump()
{
	int i;

	printf( "bitvector: " );
	for( i = 0; bvpos(i) >= 0; ++i ) printf( "%d ", bvpos(i) );
	putchar( '\n' );
}

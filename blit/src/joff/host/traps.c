#include "common.h"
#include "../traps.h"
#include "user.h"
#include "../isp.h"
#include "frame.h"
#include "../menu.h"
#include "expr.h"

struct ramnl *bptsym[BPTS];
char bptscript[128];

MLONG trackbase;
int tracklen;

setbptscript( s )
char *s;
{
	if( !s ) bptscript[0] = 0;
	else strcpy( bptscript, s );
}

bptct()
{
	int ct = 0, i;

	for( i = 0; i < BPTS; ++i ) if( bptloc[i] ) ++ct;
	return ct;
}

MLONG symbptloc( s )
struct ramnl *s;
{
	switch( s->n_other ){
	default: assert(0);
	case S_SLINE:
	case S_EFUN:
		if( !s->n_extra )
			s->n_extra = peekword(s->n_value) == MOVMLRTS ? 8 : 2;
		return s->n_value;
	case S_BFUN:
		if( !s->n_extra )
			s->n_extra = peekword(s->n_value+4) == MOVML ? 10 : 4;
		return s->n_value+s->n_extra;
	}
}


bptstmt( file, line, setorclear )
char *file;
void (*setorclear)();
{
	struct ramnl *sline;

	if( !( sline = fileline( file, line ) ) ) return;
	(*setorclear)( symbptloc(sline), sline );
}

bptfunc( id, s_type, setorclear )
char *id;
void (*setorclear)();
{
	MLONG loc;

	struct ramnl *b; 

	if( !(b = lookup( USER_MPX, s_type, id, 0 ))){
		if( lookup( USER_MPX, S_TEXT, id, 0 ) ){
			printf( ".bc and .br require -g tables\n" );
			printf( "use the .bi command\n" );
			return;
		}
		printf( "not found: %s\n", id );
		return;
	}
	(*setorclear)( symbptloc(b), b );
}

setbpt( loc, s )
MLONG loc;
struct ramnl *s;
{
	int i, free = -1;

	if( !loc || (loc&1) ){
		printf( "cannot breakpoint (pc=%d)\n", loc );
		return 0;
	}
	for( i = 0; i < BPTS; ++i ){
		if( bptloc[i] == loc ){
			printf( "breakpoint already set (pc=%s)\n", doh(loc) );
			return 0;
		}
		if( bptloc[i] == 0 ) free = i;
	}
	if( free < 0 ){
		printf( "all %d breakpoints in use\n", BPTS );
		return 0;
	}
	if( region( USER, loc ) != N_TEXT )
		printf( "warning: breakpoint has been set outside user text!\n" );
	bptloc[free] = loc;
	bptsym[free] = s;
	addr_desc( loc, DO_BPTS );
	putchar( free );
	return 1;
}

clrbpt( loc )
long loc;
{
	int i;
	
	for( i = 0; i < BPTS; ++i ) if( bptloc[i] == loc ){
		bptloc[i] = 0;
		bptsym[i] = 0;
		jerqdo( DO_BPTC );
		putchar( i );
		return 1;
	}
	printf( "breakpoint not set (pc=%s)\n", doh(loc) );
	return 0;
}

verifybpts()
{
	int i, ct = 0;
	struct ramnl *s;

	for( i = 0; i < BPTS; ++i ) if( bptloc[i] )
		if( !(s = bptsym[i]) )
			printf( "%2d:      pc=%s\n", ++ct, doh(bptloc[i]));
		else switch( s->n_other ){
		default:      assert(0);
		case S_BFUN:  printf( "%2d: call %0.8s()\n", ++ct, s->n_name );
			      break;
		case S_EFUN:  printf( "%2d: ret  %0.8s()\n", ++ct, s->n_name );
			      break;
		case S_SLINE: printf( "%2d: stmt %s\n", ++ct, soline(s->n_value) );
			      break;
	}
	if( ct == 0 ) printf( "no breakpoints\n" );
}


clrbpts()
{
	int i;

	for( i = 0; i < BPTS; ++i ) if( bptloc[i] ) clrbpt( bptloc[i] );
}

pcstepped( pc )
MLONG pc;
{
	struct ramnl *s = lookup( USER_MPX, S_BFUN, 0, pc );

	if( !train || !s ){
		train = 0;
		return;
	}
	while( train->callee ) train = train->callee;
	while( train ){
		if( train->bfun == s ){
			train->pc = pc;
			train->sline = lookup( USER_MPX, S_SLINE, 0, pc );
			return;
		}
		train = train->caller;
	}
	train = 0;
	return;
}
		
hatebsmsg()
{
	long i;
	MLONG traploc, pc;
	struct ramnl *s;

	if( oflag ){
		printf( "offline mode\n" );
		return;
	}
	hatebs = peeklong( HATEBS_INDEX );
	putchar( STATELINE );
	switch( hatebs ){
	default:      printf( "joff internal error (hatebs=%d)", hatebs ); break;
	case ACTIVE:  printf( "running" );
		      break;
	case TRAPPED: if( (i = peeklong(TYPE_INDEX) ) > MAXTRAP){
				printf( "bad traptype %d\r", i );
				return;
		      }
		      printf( "trap %s: ", trapnames[i] );
		      context( train = framechain( 0 ), LINE_CONTEXT );
		      break;
	case HALTED:  printf( "halt: " );
		      context( train = framechain( 0 ), LINE_CONTEXT );
		      break;
	case MODIFIED: printf( "modified when " );
	case STEPPED: pc = peeklong(peeklong(TLOC_INDEX));
		      pcstepped( pc );
		      printf( "stepped: %s %s", soline(pc), srcline(pc) );
		      break;
	case ERRORED: printf( "no layer" );
		      break;
	case BREAKPTED:
	traploc = peeklong(peeklong(TLOC_INDEX));
	for( i = 0; i < BPTS; ++i ) if( traploc-2 == bptloc[i] ) break;
	if( i >= BPTS ) printf( "unexpected " );
	if( i >= BPTS || !(s = bptsym[i]) ){
		printf( "break: %s pc=%s", soline(traploc-2), doh(traploc-2) );
		train = framechain( 0 );
	}else switch( s->n_other ){
		case S_SLINE:
			printf( "break: %s  %s", soline(s->n_value),
							srcline(s->n_value) );
			train = framechain( 0 );
			break;
		case S_BFUN:
			printf( "call: " );
			context( train = framechain( 0 ), 0 );
			train->noregs = 1;
			break;
		case S_EFUN:
		assert( s = lookup( table(s), II(S_GSYM,S_STSYM), s->n_name, 0 ) );
			printf( "return: %s from ",  fmtreturn(s) );
			context( train = framechain( 0 ), 0 );
			break;
		default: assert(0);
		}
	putchar( '\r' );
	stream(bptscript);
	}
	putchar( '\r' );
}

haltc()
{
	if( hatebs != ACTIVE ) return;
	jerqdo( DO_HALT );
	hatebsmsg();
	jerqdo( DO_DISABLE );
}
	
goc(n)
{
	if( n <= 0 ) return;
	switch( hatebs ){
		case ERRORED:
		case ACTIVE:
		case TRAPPED: return;
	}
	addr_desc( n, DO_LIMIT );
	jerqdo( DO_ACT );
	hatebs = ACTIVE;
	printf( "%crunning", STATELINE );
	if( n>1 ) printf( " (for %d breaks)", n );
	putchar( '\r' );
	train = 0;
	jerqdo( DO_ENABLE );
	if( n > 1 ) jerqdo( DO_ANTIC );
}

MLONG singletrap()
{
	switch( peeklong( HATEBS_INDEX ) ){
	default:	 hatebsmsg(); return 0;
	case HALTED:
		printf( "Single step may only be used when halted at a trap.\n" );
		printf( "Set a breakpoint. Go. Wait for trap. THEN single step.\n");
		return 0;
	case MODIFIED:
	case STEPPED: return peeklong(peeklong(TLOC_INDEX));
	case BREAKPTED: return peeklong(peeklong(TLOC_INDEX))-2;
	}
}

singlestep()
{
	MLONG fp = peeklong( FRAME_INDEX );

	jerqdo( DO_SING );
	switch( hatebs = peeklong( HATEBS_INDEX ) ){
	case STEPPED:
			break;
	case MODIFIED:
		hatebsmsg();
		return 0;
	default:
		hatebsmsg();
		train = 0;
		return 0;
	}
	if( fp != peeklong( FRAME_INDEX ) ) train = 0;
	return 1;
}

singlec( instct )
{
	if( instct<= 0 ) return;
	if( !singletrap() ) return;
	dotrack(trackbase,tracklen);
	addr_desc( 0, DO_LIMIT );
	while( instct-- ) if( !singlestep() ) return;
	{MLONG savedot = dot; int savedotty = dotty;
		putchar( STATELINE );
		printf( "pc " );
		dotty = INSTDOT;
		dot = peeklong(peeklong(TLOC_INDEX));
		showdot();
		dot = savedot; dotty = savedotty;
		putchar( '\r' );
	}
	dotrack(0,0);
	jerqdo( DO_ANTIC );
}

MLONG next_stmt( pc )
MLONG pc;
{
	struct ramnl *s = lookup( USER, S_SLINE, 0, pc );
	
	if( !s ) return 0;
	s = s->n_thread;
	if( !s ) return pc+100;		/* last statement in program! */
	return s->n_value;
}

cinglec( stmtct )
{
	MLONG pc;
	struct ramnl *s;

	if( stmtct <= 0 ) return;
	dotrack(trackbase,tracklen);
	while( stmtct-- ){
		if( !(pc = singletrap()) ) return;
		if( !next_stmt( pc ) ){
			printf( "no tables for statement step\n" );
			return;
		}
		addr_desc( next_stmt(pc), DO_LIMIT );
		if( !singlestep() ) return;
		pc = peeklong(peeklong(TLOC_INDEX));
		if( !(s = lookup( USER, S_SLINE, 0, pc ) ) ){
			printf( "unexpected pc=%s\n", doh(pc) );
			hatebsmsg();
			return;
		}
	}
	hatebsmsg();
	dotrack(0,0);
	jerqdo( DO_ANTIC );
}

dotrack(base,len){
	addr_desc( base, DO_TRACK );
	putchar( len );
}

trackc( e )
struct expr *e;
{
	MLONG base;
	int len;

	if( !e ){
		printf( "tracking off\n" );
		dotrack(trackbase = 0, tracklen = 0);
		return;
	}
	if( !eval(e) ) return;
	if( !(base = e->e_lvalue) ){
		printf( "expression must yield L-value\n" );
		return;	
	}
	if( (len = emccsize(e)) <= 0 ){
		printf( "can't determine sizeof expression\n" );
		return;
	}
	if( e->e_reg && len!=4 ){
		printf( "tracks full register even tho variable doen't fill it\n" );
		len = 4;
	}
	if( !inmemory(base) ){
		printf( "expression doesn't start (%s) in memory\n", doh(base));
		return;
	}
	if( !inmemory(base+len-1) ){
		printf( "expression doesn't end (%s) in memory\n", doh(base+len-1));
		return;
	}
	if( len > TRACK_MAX ){
		printf( "track length > %d bytes\n", TRACK_MAX );
		return;
	}
	printf( "tracking (char*)%s", doh(base) );
	if( len > 1 ) printf( " thru (char*)%s", doh(base+len-1) );
	putchar( '\n' );
	trackbase = base;
	tracklen = len;
}

char *fmtreturn(s)
struct ramnl *s;
{
	static struct ramnl fake[2];
	MLONG	tloc = peeklong(TLOC_INDEX), pc = peeklong(tloc);

	fake[0].n_desc = DECREF(s->n_desc);
	fake[0].n_other = S_RSYM;
	fake[0].n_value = tloc-2-14*4; 		/* %d0 */
	fake[1] = *(s+1);			/* let display worry about it */
	switch( DECREF(s->n_desc) & C_TMASK ){
	    case FTN: 
	    case ARY:
	    case PTR:
		fake[0].n_value += 8*4; 	/* %a0 */
		break;
	    case 0:
		switch( s->n_desc & BTMASK ){
		default:   return "<C type err>";
		case VOID: return "<void>";
		case ENUMTY:
		case UCHAR:
		case CHAR:
		case INT:
		case UNSIGNED:
		case LONG:
		case ULONG:
		case DOUBLE:
			break;
		case UNIONTY:
		case STRTY:
			if( (++s)->n_other != S_TYID ||
				!(s = lookup( table(s), S_BSTR, s->n_name, 0)) ) 					return "<missing tables>";
			while( s->n_other != S_ESTR ) ++s;
			fake[0].n_other = S_GSYM;
			switch( s->n_value ){
			case 2:  fake[0].n_value += 2;		/* least sig word */
			case 4:  break;
			default: if( peekword(pc-2) != MOVL_IM ) return "<error>";
				 fake[0].n_value = peeklong(pc);
			}
		}
	}
	return display( &fake[0], 0 );
}

#define CLEARALL 0
#define LISTALL 1
#define STMTLEVEL 2
#define CLEARSTMT 3
#define CLEARBOTH 4
#define SETBOTH 5

breakptmenu()
{
	struct ramnl *s, *bfun;
	char *f;
	int ct, hit = 0;

	scratch();
	if( !(s = bfun = lookup( USER, S_BFUN, "*", 0 ) ) ){
		printf( "missing tables\n" );
		return;
	}
	for( ;; ){
		scratch();
		for( s = bfun; s; s = s->n_thread ){
			(f = fmt( "%0.8s()       ", s->n_name ))[10] = '\0';
			scrsort( f, s );
		}
		if( ct = bptct() ){
			scrins( fmt( "clear all" ), CLEARALL );
			scrins( fmt( "list  [%d]", ct ), LISTALL );
		} else scrins( "<none set>", LISTALL );
		scrins( "stmt bpts", STMTLEVEL );
		switch( (long) (s = (struct ramnl *) (hit = scrhit(hit)) ) ){
		case -1: return;
		case LISTALL:
			verifybpts();
			break;
		case CLEARALL:
			if( menuconfirm() ){
				clrbpts();
				return;
			}
			break;
		case STMTLEVEL:
			stmtbptmenu();
			break;
		default: breakfunc( s );
		}
	}
}


breakfunc( b )
struct ramnl *b;
{
	struct ramnl *e;
	long i, call = 0, ret = 0, differ;

	m_menu( M_CALLRET, 0 );				
	assert( e = lookup( USER, S_EFUN, b->n_name, 0 ) );
	differ = symbptloc(b) != symbptloc(e);
	for( i = 0; i < BPTS; ++i )
		if( bptsym[i] == b ) call = 1;
		else if( bptsym[i] == e ) ret = 1;
	i = 0;
	m_item( i++, call&&!ret ? ">call  " : " call  ", b );
	if( differ ) m_item( i++, ret&&!call ? ">return" : " return", e );
	if( differ ) m_item( i++, call&&ret ? ">both  " : " both  ", SETBOTH );
	m_item( i++, !call&&!ret ? ">none  " : " none  ", CLEARBOTH );
	m_null( i );
	if( (i = menuforce(M_CALLRET)) == -1 ) return;
	if( call ) clrbpt( symbptloc(b) );
	if( ret ) clrbpt( symbptloc(e) );
	switch( i = m_key(i) ){
	case CLEARBOTH:
		break;
	case SETBOTH:
		setbpt( symbptloc(b), b );
		setbpt( symbptloc(e), e );
		break;
	default:
		setbpt( symbptloc(i), i );	/* i is (struct ramnl *) */
	}
}

struct ramnl *lastsline(s)
struct ramnl *s;
{
	struct ramnl *stop, *sline = 0;

	assert( s->n_other == S_SO );
	for( ; s[1].n_other == S_SO; ++s ) {}
	stop = s->n_thread;
	for( ++s; stop ? s < stop : (long) s->n_thread; ++s )
		if( s->n_other == S_SLINE ) sline = s;
	return sline;
}

struct ramnl *linemap[M_BVSIZE];

stmtbitvec(s)
struct ramnl *s;
{
	struct ramnl *last = lastsline(s);
	int i;

	assert(last);
	for( i = 0; i < M_BVSIZE; ++i ) linemap[i] = 0;
	for( ; s <= last; ++s ) if( s->n_other == S_SLINE
		 && s->n_desc < M_BVSIZE && !linemap[s->n_desc] )
			linemap[s->n_desc] = s;
}

stmtbptmenu()
{
	struct ramnl *so;
	int n, hit;

	if( !(so = lookup(USER,S_SO,"*",0)) ) return;
	scratch();
	for( ; so; so = so->n_thread )
		if( lastsline(so) ) scrsort( basename(sofile(so)), so );
	if( (hit = scrhit(0)) == -1 ) return;
	so = (struct ramnl *) hit;
	stmtbitvec(so);
	while( (n = bv_force(linemap)) != -1 ){
		breakline(n);
	}
}

#define REACH	15
#define SHOW	4
#define STMTSET	(-2)
#define STMTCLR	(-3)
#define max(x,y) ((x)>(y)?(x):(y))
#define min(x,y) ((x)<(y)?(x):(y))
breakline(n)
{
for( ;; ){
	MLONG pc;
	int i, set = 0, ct;
	struct ramnl *b = linemap[n];

	assert( b && b->n_other == S_SLINE );
	pc = b->n_value;
	printf( "%s %s\n", soline(pc), srcline(pc) );
	for( i = 0; i < BPTS; ++i ) if( bptsym[i] == b ) set = 1;
	scratch();
	for( ct = 0, i = n-1; ct < SHOW && i >= max(0,n-REACH); --i )
		if( linemap[i] ){
			scrins(	fmt( "%d", linemap[i]->n_desc ), i );
			++ct;
		}
	scrapp( fmt( "%cbreak %d", set?'>':' ', b->n_desc ), STMTSET );
	scrapp( fmt( "%cclear %d", set?' ':'>', b->n_desc ), STMTCLR );
	for( ct = 0, i = n+1; ct < SHOW && i <= min(M_BVSIZE,n+REACH); ++i )
		if( linemap[i] ){
			scrapp(	fmt( "%d", linemap[i]->n_desc ), i );
			++ct;
		}
	switch( i = scrhit(0) ){
	case -1:	return;
	case STMTSET:	if( !set ) setbpt( symbptloc(b), b );
			return;
	case STMTCLR:	if( set ) clrbpt( symbptloc(b) );
			return;
	default:	n = i;	
	}
}
}


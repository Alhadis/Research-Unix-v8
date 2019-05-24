#include "common.h"
#include "user.h"
#include "frame.h"
#include "../traps.h"

static delta = 0;

char *unixline( str )
char *str;
{
	int i;
	char c;

	for( i = 0;; ){
		c = getchar();
		if( c == '\r' || c == '\n' ) break;
		if( c != '\b' ) printf( "%c", str[i++] = c );
		else if( i > 0 ) printf( "\b%c\b", str[--i] );
		if( c == 004 ) break;
	}
	str[i] = '\0';
	return str;
}

char *readuser( str )
char *str;
{
	char prompt[128];

	strncpy( prompt, str, 128 );
	for( ;; ){
		printf( "%s", prompt ); flush();
		if( oflag ) unixline( str );
		else jerqkbd( str );
		if( str[0] != '!' ) break;
		putchar( '\n' ); flush();
		system( &str[1] );
		printf( "\n!\n" );
	}
	return str;
}

char *doh( a )
{
	fmt( radix==16 ? "0x%X" : radix==10 ? "%u" : "0%o", a );
}

joff()
{
	int i;

	if( !oflag ){
		printf( "\r", STATELINE );
		jerqmenu( M_PENDULUM, 1 );
		flush();
		stabread( stabtab[MPX], 0 );
		jerqmenu( M_PENDULUM, 0 );
		hatebsmsg();
	} else newc();
	dotty = WORDDOT;
	radix = 10;
	dotfmt = DECIMAL;
	dotstrlen = 0;
	for( ;; ){
		tfree();
		scratch();
		if( !oflag ){
			if((hatebs = peeklong(HATEBS_INDEX)) == ACTIVE) train = 0;
			menuset();
		}
		sprintf( userline, dot ? "%s: " : ": ", doh(dot) );
		switch( *readuser( userline ) ){
		case ESCAPE:
			switch( userline[1] ){
			case DO_EVENT:
				hatebsmsg();
				break;
			case M_HIT:
				menuhit( (userline[2]<<8)|userline[3], userline[4]);
				break;
			}
			break;
		case '<' :
			putchar( '\n' );
			for( i = 1; userline[i] == ' '; ++i ){}
			stream(&userline[i]);
			break;
		case '\0' :
			putchar( oflag ? '\n' : '\v' );
			if( dot ){
				movedot( 1 );
				showdot();
			}
			break;
		default:
			if( !oflag ){
				m_menu( 0, 3 );
				m_menu( 0, 2 );
			}
			putchar( '\n' ); flush();
			lexindex = 0;
			strcat( userline, ";" );
			yyparse();
		}
	}
}

stream(script)
char *script;
{
	FILE *f;
	int i;

	if( !script[0] ) return;
	if( (f = fopen( script, "r" )) == NULL ){
		printf( "cannot open: %s\n", script );
		return;
	}
	while( fgets( userline, 256, f ) ){
		if( !strcmp( userline, "\n" ) ) continue;
		tfree();
		lexindex = 0;
		userline[strlen(userline)-1] = ';';
		printf( "%s \t ", userline );
		yyparse();
	}
	fclose( f );
}

tracebackc()
{
	if( hatebs == ACTIVE ) hatebsmsg();
	train = framechain( VERBOSE_CHAIN|FULL_CHAIN );
}

returnc( i )
{
	while( train && i-- ){
		if( train->caller == &sentinel ){
			printf( "use .t to obtain complete traceback\n" );
			return;
		}
		if( train->caller) train = train->caller;
	}
	context(train, LINE_CONTEXT|REGS_CONTEXT);
	putchar( '\n' );
}

callc( i )
{
	while( train && i-- ) if( train->callee) train = train->callee;
	context(train, LINE_CONTEXT|REGS_CONTEXT);
	putchar( '\n' );
}

framec()     { if( train ) dumpframe( train ); }

varsc()
{
	if( hatebs == ACTIVE ) hatebsmsg();
	if( train == 0) train = framechain( FULL_CHAIN );
	context( train, AUTO_CONTEXT|REGS_CONTEXT );
}

argvtables()
{
	char *str;
	struct ramnl *state, *fcn, *argv;
	MLONG argv0, udata, p = peeklong( PROC_INDEX );

	if( !(state = lookup( MPX, S_SSYM, "state", 0 ) ) ) return;
	if( !( peekword(p + state->n_value) & 8) ) return;	/* USER */

	if( !(fcn = lookup( MPX, S_SSYM, "fcn", 0 ) ) ) return;
	if( !(udata = peeklong( p + fcn->n_value ) ) ) return;

	if( !(argv = lookup( MPX, S_SSYM, "argv", 0 ) ) ) return;
	if( !(argv0 = peeklong( udata + argv->n_value ) ) )return;
	if( !(argv0 = peeklong( argv0 ) ) ) return;

	str = peeknstr( argv0, 128 );
	if( !str[0] ) return;
	if( strcmp( stabtab[USER]->path, str ) ){
		strcpy( stabtab[USER]->path, str );
		stabfree( stabtab[USER] );
	}
}

newc()
{
	int i;
	MLONG p = 1;

	train = 0;
	clrbpts();
	setbptscript(0);
	dot = snarfdot = 0;
	putchar( STATELINE );
	putchar( '\r' );
	if( !oflag ){
		flush();
		m_menu( 0, 2 ); 
		m_menu( 0, 3 ); 
		jerqdo( DO_DISABLE );
		jerqdo( DO_DEBUG );
		if( p = peeklong( PROC_INDEX) ){
			jerqdo( DO_VIDEO );
			argvtables();
		}
	}
	if( p ){
		stabread( stabtab[USER], oflag ? 0 : INTERACT );
		if( !oflag ){
			threadP(stabtab[USER]);
			makemap();
			hatebs = peeklong(HATEBS_INDEX);
			if( hatebs == ACTIVE || hatebs == ERRORED )
				hatebsmsg();
			jerqdo(DO_ENABLE);
		}
	}
}

inmemory( a )
MLONG a;
{
	if( a > LO_ADDR && a < HI_ADDR ) return 1;
	if( a >= LO_ROM && a < HI_ROM ) return 1;
	return 0;
}

dotop( i )
{
	switch( i ){
		default: assert(0);
		case 0:
		case MINUSONE:
		case PLUSONE:
			switch( i ){
			case MINUSONE: movedot( -1 ); break;
			case PLUSONE:  movedot( 1 ); break;
			}
			if( dotty == STRUCTDOT || dotty == STRINGDOT ) showdot();
			else printf( "%s: %s\n", doh(dot), fmtdot() );
			return;
		case INDIRECT: dot = peeklong(dot); return;
		case G_BITMAP:
		case G_POINT:
		case G_RECTANGLE:
		case G_TEXTURE:
			g_addr_desc( dot, i );
			return;
		case BYTEDOT: 
		case WORDDOT: 
		case LONGDOT: 
		case STRINGDOT: dotty = i; return;
		case STRUCTDOT: structenumdot( S_BSTR ); return;
		case ENUMDOT: structenumdot( S_BENUM ); return;
		case XREF: xrefc(); return;
		case DECIMAL: radix = 10; dotfmt = DECIMAL; return;
		case HEX: radix = 16; dotfmt = HEX; return;
		case OCTAL: radix = 8; dotfmt = OCTAL; return;
		case ASCII: dotfmt = ASCII; return;
		case SNARF: dot = snarfdot;
			    snarfdot = 0;
			    if( snarfdotty ) dotty = snarfdotty;
			    return;
	}
}

movedot( i )
{
	if( dotty != BYTEDOT && dotty != STRINGDOT )
		if( dot&1 ) ++dot;
	switch( dotty ){
		case BYTEDOT: dot += i; break;
		case WORDDOT: dot += i*2; break;
		case LONGDOT: dot += i*4; break;
		case STRINGDOT: dot += i*(dotstrlen ? dotstrlen : 4); break;
		case INSTDOT: dot += i == 1 ? delta : i; break;
		case ENUMDOT:
		case STRUCTDOT:  dot += i*delta; break;
		default: assert(0);
	}
	if( dot < 0 ) dot = 0;
}

showdot()
{
	int d, oh;

	if( dot == 0 ) return;
	if( dotty != BYTEDOT && dotty != STRINGDOT && dot&1 ) ++dot;
	switch( dotty ){
		case ENUMDOT:
		case STRUCTDOT: 
			dotstab[0].n_other = S_GSYM;
			dotstab[0].n_desc = (dotty == STRUCTDOT) ? STRTY : ENUMTY;
			dotstab[0].n_value = dot;
			printf( "%s: %s\n", doh(dot), display( &dotstab[0], 0 ) );
			return;
		case STRINGDOT:
			printf( "%s: %s\n", doh(dot), fmtstring( dot));
			return;
		case INSTDOT: delta = snap(dot) - dot; return;
		case BYTEDOT: oh = peekbyte( dot ); d = (int) (char) oh; break;
		case WORDDOT: oh = peekword( dot ); d = (int) (short) oh; break;
		case LONGDOT: d = oh = peeklong( dot ); break;
	}
	if( oh == 0 ){
		printf( "%s: 0\n", doh(dot) );
		return;
	}
	else printf( "%s: %d 0%o 0x%X \'", doh(dot), d , oh , oh );
	switch( dotty ){
		case LONGDOT:
			printf( "%s%s", fmtbyte( oh >> 24 ), fmtbyte( oh >> 16 ) );
		case WORDDOT: printf( "%s", fmtbyte( oh >> 8 ) );
		case BYTEDOT: printf( "%s", fmtbyte( oh ) );
	}
	printf( "\'\n" );
}

char *fmtshortstr( p )
MLONG p;
{
	int   width = 12, i = 0;
	char *buf = talloc( width+32 ), *s;

	if( !p ) return "0";
	assert( width >= 5 );
	strcpy( buf, "\"" );
	s = peeknstr( p, width );
	while( s[i] && strlen(buf) <= width ) strcat(buf, fmtbyte(s[i++]));
	strcat( buf, "\"" );
	if( strlen( buf ) > width ) strcpy( &buf[width-4], "...\"" );
	return buf;
}

char *fmtstruct( s )
struct ramnl *s;
{
	MLONG  p = locate( s, 0 );
	char *field, *val, *d;

#define	WIDTH 14
	(d = talloc(WIDTH+4))[0] = '{';
	if( (++s)->n_other != S_TYID ) return "<missing tables>";
	if( s->n_name[0] == '\0' ) return "<anon struct>";
	s = lookup( table(s), S_BSTR, s->n_name, 0 );
	if( !s ) return "<struct error>";
	for( ++s; s->n_other != S_ESTR; ){
		field = fmt( "%0.8s=", s->n_name );
		s->n_value += p;
		val =  display(s, 0);
		s->n_value -= p;
		if( strlen(field)+strlen(val)+strlen(d) >= WIDTH ) return d;
		sprintf( &d[strlen(d)], "%s%s", field, val );
		for( ++s; s->n_other == S_TYID || s->n_other == S_DIM; ++s ){}
		if( s->n_other == S_SSYM ) strcat( d, "," );
	}
	return strcat( d, "}" );
}

char *fmtdot()
{
	int d, oh, i;
	char *b[4];

	if( dot == 0 ) return "" ;
	if( dotty != BYTEDOT && dot&1 ) ++dot;
	switch( dotty ){
		case STRUCTDOT:
			dotstab[0].n_other = S_GSYM;
			dotstab[0].n_desc = STRTY;
			dotstab[0].n_value = dot;
			return fmtstruct( &dotstab[0] );
		case ENUMDOT:
			dotstab[0].n_other = S_GSYM;
			dotstab[0].n_desc = ENUMTY;
			dotstab[0].n_value = dot;
			return display( &dotstab[0], 0 );
		case STRINGDOT:
			return fmtshortstr( dot );
		case INSTDOT: assert(0);
		case BYTEDOT: oh = peekbyte( dot ); d = (int) (char) oh; break;
		case WORDDOT: oh = peekword( dot ); d = (int) (short) oh; break;
		case LONGDOT: d = oh = peeklong( dot ); break;
	}
	if( oh == 0 ) return "0";
	switch( dotfmt ){
	case DECIMAL: return fmt( "%d", d );
	case OCTAL:   return fmt( "0%o", oh );
	case HEX:     return fmt( "0x%X", oh );
	case ASCII:
		for( i = 0; i < 4; ++i ) b[i] = fmtbyte( oh >> (i*8) );
		switch( dotty ){
		case LONGDOT: return fmt( "\'%s%s%s%s\'", b[3], b[2], b[1], b[0] );
		case WORDDOT: return fmt( "\'%s%s\'", b[1], b[0] );
		case BYTEDOT: return fmt( "\'%s\'", b[0] );
		default: return "???";
		}
	}
}

idc( id )
char *id;
{
	struct ramnl *s, *sline;
	int glb = III( S_BFUN, S_GSYM, S_STSYM );
	int lcl = III(S_RSYM,S_LSYM,S_PSYM);;


	if( !train && hatebs != ACTIVE && hatebs != ERRORED ) train = framechain(0);
	if( ( train && (sline = lookup( USER_MPX, S_SLINE, 0, train->pc))
				&& (s = visible( sline, lcl, id, 0)) )
			|| ( s = lookup( USER_MPX, glb, id, 0)) ){}

	if( s ) printf( "%s\n", display(s,train));
	else if( (s = lookup( USER_MPX, LDE, id, 0) )
		|| (s = lookup( USER_MPX, LDL, id, 0)) ) 
			printf( "%s loaded at %s\n", id, doh(dot = s->n_value) );
	else if( s = lookup( USER_MPX, II(S_BSTR,S_BENUM), id, 0 ) ){
		dotty = s->n_other == S_BSTR ? STRUCTDOT : ENUMDOT;
		dotstab[1] = *s;
		dotstab[1].n_other = S_TYID;
		while( s->n_other != S_ESTR && s->n_other != S_EENUM ) ++s;
		delta = s->n_other == S_ESTR ? s->n_value : 2;
		showdot();
	}
	else printf( "not found: %s\n", id );
}

structenumdot( t )
long t;
{
	struct ramnl *s = lookup( USER, t, "*", 0 );
	long i;

	assert(s);
	scratch();
	for( ; s; s = s->n_thread ) scrsort( fmt( "%-8.8s", s->n_name ), s );
	if( ( i = scrhit(0) ) == -1 ) return;
	s = (struct ramnl *) i;
	dotty = s->n_other == S_BSTR ? STRUCTDOT : ENUMDOT;
	dotstab[1] = *s;
	dotstab[1].n_other = S_TYID;
	while( s->n_other != S_ESTR && s->n_other != S_EENUM ) ++s;
	delta = s->n_other == S_ESTR ? s->n_value : 2;
}
	
char *fmtxref()
{
	struct ramnl *s, *sline;
	MLONG  offset;
	struct frame *p;
	int r;

	for( p = train; p && p->caller && p->caller!= &sentinel; p = p->caller ){}
	for( ; p; p = p->callee )
	    if( p->sline )
		if( (s = visible( p->sline, S_PSYM, 0, dot - p->fp ) )
		 || (s = visible( p->sline, S_LSYM, 0, p->fp+p->regdelta-dot )))
			return fmt( "%0.8s in %s()", s->n_name, p->fname );
	if( s = lookup( USER_MPX, LDE, 0, dot ) ){
		if( (region(USER, dot) == N_TEXT || region(MPX, dot) == N_TEXT)
			&& (sline = lookup( USER_MPX, S_SLINE, 0, dot) )
				&& (offset = dot-sline->n_value) < 256 )
					return soline(dot);
		if( offset = dot - s->n_value )
			return fmt( "%0.8s+%s", s->n_name , doh(offset) );
		return fmt( "%0.8s", s->n_name );
	}
	return 0;
}
	
xrefc()
{
	struct ramnl *s;
	MLONG  offset;
	struct frame *p;
	int i, r;

	printf( "%s = ", doh( dot ) );
	for( i = 1; stabtab[i]; ++i ) if( r = region( i, dot ) )
		printf( "(%s %s) ", stabtab[i]->name, ntypenames[r] );
	for( p = train; p && p->caller && p->caller!= &sentinel; p = p->caller ){}
	for( ; p; p = p->callee )
	    if( p->sline )
		if( (s = visible( p->sline, S_PSYM, 0, dot - p->fp ) )
		 || (s = visible( p->sline, S_LSYM, 0, p->fp+p->regdelta-dot ))){
			printf( "%0.8s in %s()\n", s->n_name, p->fname );
			return;
		}
	if( s = lookup( USER_MPX, LDE, 0, dot ) ){
		printf( "%0.8s", s->n_name );
		if( offset = dot - s->n_value ) printf( "+%s", doh(offset) );
		if( (region(USER, dot) == N_TEXT || region(MPX, dot) == N_TEXT)
			&& (s = lookup( USER_MPX, S_SLINE, 0, dot) )
				&& (offset = dot-s->n_value) < 256 ){
			printf( " %s", soline(dot) );
		 	printf(" in %0.8s()",
					lookup(table(s),S_BFUN,0,dot)->n_name);
		}
		putchar( '\n' );
		return;
	}
	printf( "no symbolic form\n" );
}

regc( r )
{
	struct ramnl reg, *s;

	if( !train ){
		if( hatebs == STEPPED ) tracereg( r, peeklong(TLOC_INDEX) );
		else printf( "<no context>\n" );
		return;
	}
	if( r == -1  ){
		for( r = 0; r <= 15; ++r ) regc( r );
		return;
	}
	printf( "%s ", regnames[r] );
	if( s = visible( train->sline, S_RSYM, 0, r))
				printf( "(%0.8s) ", s->n_name );
	reg.n_other = S_RSYM;
	reg.n_value = r;
	if( dot = locate( &reg, train ) ){
		dotty = LONGDOT;
		showdot();
	}
	else printf( "not saved\n" );
}

tracereg( r, tloc )
MLONG tloc;
{
	switch( r ){
	case -1:
		for( r = 0; r <= 15; ++r ) tracereg( r, tloc );
		break;
	case 15:
		printf( "a7 (%%sp): %s\n", doh(tloc) );
		break;
	default:
		dot = tloc-2-14*4+( r==14 ? -3 : r )*4;	/* fp saved by trap() */
		dotty = LONGDOT;
		printf( "%s ", regnames[r] );
		showdot();
		return;
	}
}

struct ramnl *fileline( file, line )
char *file;
{
	struct ramnl *so = lookup( USER, S_SO, "*", 0 ), *sline, *nso;
	char *sof;

	if( !so ){
		printf( "missing tables\n" );
		return 0;
	}
	for( ; so; so = so->n_thread ){
		if( !strcmp( file, sof = sofile(so) ) ) break;
		if( !strcmp( file, basename(sof) ) ) break;
	}
	if( !so ){
		printf( "not found: %s\n", file );
		return 0;
	}
	for( sline = so; sline->n_other != S_SLINE; ++sline ){
		if( sline->n_other == S_EFUN ){
			printf( "missing tables\n" );
			return 0;
		}
	}
	for( nso = so; nso->n_thread == nso+1; ++nso ) {}
	nso = nso->n_thread;
	for( ; sline; sline = sline->n_thread ){
		if( nso && sline > nso ) break;
		if( sline->n_desc >= line ){
			if( sline->n_desc != line )
			printf( "using %d instead of %d\n", sline->n_desc, line );
			return sline;
		}
	}
	printf( "line %d not found in %s\n", line, sofile(so) );
	return 0;
}
	
srclinec( file, line )
char *file;
{
	struct ramnl *sline;

	if( !(sline = fileline( file, line ))) return;
	dot = sline->n_value;
	dotty = INSTDOT;
	xrefc();
	showdot();
} 	

quitc()
{
	if( !oflag && !menuconfirm() ) return;
	clean = 1;
	wrap();
}

zmc(f)
char *f;
{
	stabfree( stabtab[MPX] );
	strcpy( stabtab[MPX]->path, f );
	stabread( stabtab[MPX], 0);
}

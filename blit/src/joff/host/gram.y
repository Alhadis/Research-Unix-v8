%{
#include "common.h"
#include "user.h"
#include "expr.h"
#include "../traps.h"
#define online() if( oflag )\
{ yyerror("command needs a 68000 process; can't be used offline"); YYERROR; }\
	else if( hatebs==ERRORED )\
		{ yyerror("pick a layer before using this command"); YYERROR; }
%}

%union {
	char		cc;
	int		ii;
	char		*ss;
	struct expr	*ee;
}

%token CONST, SEMI, ID, QMARK, PCENT, EQUAL, SLASH, CNTRL_D
%token UNOP, STAR, PLUS, MINUS, AMPER, ARROW, DOT, LB, LP, COMMA, ERROR, RB, RP

%type <ii> CONST
%type <ss> ID
%type <ee> expr, list
%type <cc> UNOP, STAR, PLUS, MINUS, AMPER, ARROW, DOT, LB, LP, COMMA, ERROR, SLASH
%type <cc> EQUAL, PCENT, CNTRL_D

%right EQUAL
%left PLUS MINUS
%left STAR SLASH PCENT
%right UNOP
%left ARROW DOT LB LP

%type <cc> 'a' 'b' 'c' 'd' 'e' 'f' 'g' 'h' 'i' 'j' 'k' 'l' 'm' 'n' 'o' 'p' 'q' 'r'
%type <cc> 's' 't' 'u' 'v' 'w' 'x' 'y' 'z' 'A' 'B' 'C' 'D' 'E' 'F' 'G' 'H' 'I' 'J'
%type <cc> 'K' 'L' 'M' 'N' 'O' 'P' 'Q' 'R' 'S' 'T' 'U' 'V' 'W' 'X' 'Y' 'Z'
%type <cc> '0' '1' '2' '3' '4' '5' '6' '7' '8' '9' '/' '.' '-' '+' '_'

%type <ii> iconst dconst oconst hconst
%type <cc> nzdec seven dec oct hex HEX idleft idnext upper lower letter filechar
%type <ss> id fileid

%%

line:	command
|	line command

command:
	CNTRL_D semi			{ quitc(); }
|	ID SEMI				{ idc( $1 ); }
|	DOT expr SEMI			{ dotexprc( $2 ); }
|	PLUS CONST SEMI			{ movedot( $2 ); showdot(); }
|	PLUS SEMI			{ movedot( 1 ); showdot(); }
|	MINUS CONST SEMI		{ movedot( -$2 ); showdot(); }
|	MINUS SEMI			{ movedot( -1 ); showdot(); }
|	QMARK SEMI			{ hatebsmsg(); }
|	expr SEMI			{ exprc( $1 ); }
|	STAR SEMI			{ dot = peeklong( dot ); showdot(); }
|	DOT EQUAL expr SEMI		{ online(); pokec( $3 ); }

|	'%' 'd' oct semi		{ online(); regc( $3 - '0' ); }
|	'%' 'a' oct semi		{ online(); regc( $3 - '0' + 8 ); }
|	'%' 'f' 'p' semi		{ online(); regc( 6 + 8 ); }
|	'%' 's' 'p' semi		{ online(); regc( 7 + 8 ); }
|	'%' 'p' 'c' semi		{ online(); regc( 8 + 8 ); }
|	'%' SEMI			{ online(); regc( -1 ); }
|	'.' dotcommand

expr:	ID			{ $$ = enode( E_ID, $1 ); }
|	CONST			{ $$ = enode( E_CONST,  $1 ); }
|	STAR  expr  %prec UNOP	{ $$ = enode( E_UNARY,  $1, $2 ); }
|	AMPER expr  %prec UNOP	{ $$ = enode( E_UNARY,  $1, $2 ); }
|	MINUS expr  %prec UNOP	{ $$ = enode( E_UNARY,  $1, $2 ); }
|	expr DOT ID		{ $$ = enode( E_BINARY,  $1, $2, enode(E_ID,$3)); }
|	expr EQUAL expr		{ $$ = enode( E_BINARY, $1, $2, $3 ); }
|	expr MINUS expr		{ $$ = enode( E_BINARY, $1, $2, $3 ); }
|	expr ARROW ID		{ $$ = enode( E_BINARY, $1, $2, enode(E_ID,$3)); }
|	expr PCENT expr		{ $$ = enode( E_BINARY, $1, $2, $3 ); }
|	expr STAR expr		{ $$ = enode( E_BINARY, $1, $2, $3 ); }
|	expr SLASH expr		{ $$ = enode( E_BINARY, $1, $2, $3 ); }
|	expr PLUS expr		{ $$ = enode( E_BINARY, $1, $2, $3 ); }
|	expr LB expr RB		{ $$ = enode( E_BINARY, $1, $2, $3 );}
|	ID LP list RP		{ $$ = enode( E_BINARY, enode(E_ID,$1), $2, $3 ); }
|	ID LP RP 		{ $$ = enode( E_BINARY, enode(E_ID,$1), $2, 0 ); }
|	LP expr RP		{ $$ = $2; }

list:	expr			{ $$ = enode( E_BINARY, $1, ',', 0 ); }
|	expr COMMA list		{ $$ = enode( E_BINARY, $1, $2, $3 ); } 

track:	expr semi		{ trackc( $1 ); }
|	semi			{ trackc( 0 ); }
dotcommand:
|	'1' '6' semi		{ radix = 16; }
|	'1' '0' semi		{ radix = 10; }
|	'8' semi		{ radix = 8; }

|	't' 'r' {online(); setmode();} track

|	'v' semi		{ online(); varsc(); }
|	't' semi		{ online(); tracebackc(); }
|	'c' w iconst semi 	{ online(); callc( $3 ); }
|	'c' semi		{ online(); callc(1); }
|	'r' w iconst semi	{ online(); returnc( $3 ); }
|	'r' semi		{ online(); returnc(1); }
|	'b' semi		{ dotty = BYTEDOT; showdot(); }
| 	'w' semi		{ dotty = WORDDOT; showdot(); }
|	'l' semi		{ dotty = LONGDOT; showdot(); }
|	'i' semi		{ dotty = INSTDOT; showdot(); }
|	's' w iconst semi	{ dotty = STRINGDOT; dotstrlen = $3; showdot(); }
|	's' semi		{ dotty = STRINGDOT; showdot(); }
|	'x' semi		{ xrefc(); }

|	'q' semi		{ clean = 1; wrap(); }
|	'l' 'a' semi		{ newc(); }
|	'h' semi		{ online(); haltc(); }
|	'g' w iconst semi	{ online(); goc( $3 ); }
|	'g' semi		{ online(); goc( 1 ); }
|	's' 'i' w iconst semi	{ online(); singlec( $4 ); }
|	's' 'i' semi		{ online(); singlec( 1 ); }
|	's' 's' w iconst semi	{ online(); cinglec( $4 ); }
|	's' 's' semi		{ online(); cinglec( 1 ); }

|	'b' 'l' semi		{ online(); verifybpts(); }
|	'b' 'r' w id semi	{ online(); bptfunc( $4, S_EFUN, setbpt ); }
|	'b' 'c' w id semi	{ online(); bptfunc( $4, S_BFUN, setbpt ); }
|	'b' 'i' semi		{ online(); setbpt( dot, 0 ); }
| 	'b' 's' w fileid w iconst semi
				{ online(); bptstmt( $4, $6, setbpt ); }
|	'c' 'r' w id semi	{ online(); bptfunc( $4, S_EFUN, clrbpt ); }
|	'c' 'c' w id semi	{ online(); bptfunc( $4, S_BFUN, clrbpt ); }
|	'c' 'a' semi		{ online(); clrbpts(); }
|	'c' 'i' semi		{ online(); clrbpt( dot ); }
|	'c' 's' w fileid w iconst semi
				{ online(); bptstmt( $4, $6, clrbpt ); }
|	'b' 'x' w fileid semi	{ online(); setbptscript( $4 ); }
|	'b' 'x' semi		{ setbptscript( 0 ); }
|	'f' 'f' semi		{ printf( "\f" ); }
|	'l' 'i' w fileid w iconst semi
				{ srclinec( $4, $6 ); }
|	'c' 'd' w fileid semi	{ cdc( $4 ); }
|	'c' 'd' semi		{ cdc( "" ); }
|	'p' 'w' 'd' semi	{ pwdc(); }

|	's' 'd' w fileid semi	{ searchdirc( $4 ); }
|	'z' 'f' semi		{ online(); framec(); }
|	'z' 's' semi		{ stabstats(); }
|	'z' 'h' w id w id semi
				{ stabhelp( $4, $6 ); }
|	'z' 'h' w '*' w id semi	{ stabhelp( "*", $6 ); }
|	'z' 'P' semi		{ online(); dot = peeklong(PROC_INDEX); }
|	'z' 'e'	semi		{ zec(); }
|	'z' 'v'	semi		{ zvc(); }
|	'z' 'x' w fileid semi	{ reload( $4 ); }
|	'z' 'd' w iconst w iconst semi
				{ zdc( $4, $6 ); }
|	'z' 'm' w fileid semi	{ online(); zmc($4); }

semi:	ow SEMI

id:	idleft { ($$ = talloc(128))[0] =  $1; }
|	id idnext { $1[strlen($1)] = $2; $$ = $1; }

fileid:	filechar { ($$ = talloc(128))[0] =  $1; }
|	fileid filechar { $1[strlen($1)] = $2; $$ = $1; }

iconst:	dconst | oconst | hconst

dconst: nzdec { $$ = $1 - '0'; }
|	dconst dec { $$ = $1*10 + $2-'0'; }

oconst: '0' { $$ = 0; }
|	oconst oct { $$ = ($1<<3) + $2-'0'; }

hconst: '0' xX { $$ = 0; }
|	hconst dec { $$ = ($1<<4) + $2-'0'; }
|	hconst hex { $$ = ($1<<4) + $2-'a'+10; }
|	hconst HEX { $$ = ($1<<4) + $2-'A'+10; }

ow:	ow ' ' |
w:	w ' ' | ' '
seven:	'1' | '2' | '3' | '4' | '5' | '6' | '7'
nzdec:	'8' | '9' | seven
oct:	'0' | seven
dec:	'0' | nzdec
lower:  'a'|'b'|'c'|'d'|'e'|'f'|'g'|'h'|'i'|'j'|'k'|'l'|'m'|'n'|'o'|'p'|'q'|'r'
|	's'|'t'|'u'|'v'|'w'|'x'|'y'|'z'
upper:	'A'|'B'|'C'|'D'|'E'|'F'|'G'|'H'|'I'|'J'|'K'|'L'|'M'|'N'|'O'|'P'|'Q'|'R'
|	'S'|'T'|'U'|'V'|'W'|'X'|'Y'|'Z'
xX:	'x' | 'X'
hex:	'a' | 'b' | 'c' | 'd' | 'e' | 'f'
HEX:	'A' | 'B' | 'C' | 'D' | 'E' | 'F'
letter: upper | lower
idleft: letter | '_'
idnext: idleft | dec
filechar: lower | upper | dec | '.' | '/' | '_' | '-' | '+'

%%

#define look (userline[ lexindex ])
#define take (userline[ lexindex++ ])
#define more (userline[ lexindex+1 ])
#define ys (yylval.ss)
#define yi (yylval.ii)
#define yc (yylval.cc)
#define ishex(x) (isdigit(x) || (x>='a'&&x<='f') || (x>='A'&&x<='F'))
#define isoct(x) ( x>='0' && x<='7' )
static enum { FULL_LEX, NO_LEX } mode;

setmode()
{
	mode = FULL_LEX;
	if( look == '%' ) mode = NO_LEX;
	if( look == '.' && more != ' ' && more != '=' ) mode = NO_LEX;
}

yylex()
{
	if( lexindex == 0 ) setmode();
	if( mode == NO_LEX ){
		if( look == ';' ){
			take;
			setmode();
			return SEMI;
		}
		return yc = take;
	}
	while( isspace(look) ) take;
	if( isalpha(look) || look=='_' ){
		(ys = talloc(16))[0] = take;
		while( isalnum(look) || look=='_' )
			if( strlen(ys) < 8 ) ys[strlen(ys)] = take; else take;
		return ID;
	}
	if( look=='0' && (more =='x' || more=='X' ) ){
		for( take, take, yi = 0; ishex(look); take )
		    yi = (yi<<4) + (isalpha(look) ? (look|' ')+10-'a' : look-'0');
		return CONST;
	}
	if( look=='0' ){
		for( take, yi = 0; isoct(look); take ) yi = (yi<<3) + look - '0';
		return CONST;
	}
	if( isdigit(look) ){
		for( yi = 0; isdigit(look); take ) yi = yi*10 + look - '0';
		return CONST;
	}
	if( look=='-' && more=='>' ) return take, yc = take, ARROW;
	switch( yc = take ){
		case 004 : return CNTRL_D;
		case '/' : return SLASH;
		case '*' : return STAR;
		case '+' : return PLUS;
		case '-' : return MINUS;
		case '.' : return DOT;
		case '(' : return LP;
		case ')' : return RP;
		case '[' : return LB;
		case ']' : return RB;
		case '&' : return AMPER;
		case ',' : return COMMA;
		case ';' : return SEMI;
		case '%' : return PCENT;
		case '?' : return QMARK;
		case '=' : return EQUAL;
		default  : return yc;
	}
}


yyerror( str )
char *str;
{
	printf( "%s\n", str );
}



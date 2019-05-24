%{
#include "gram.h"
#include "expr.pub"
#include <ctype.h>
int DotDot;
Expr *E_IConst(long), *E_DConst(double);
%}

%union {
	char		cc;
	long		ll;
	char		ss[32];
	struct Expr	*ee;
	double		dd;
}

%token G_EXPR G_DOTEQ_CONEX G_DOLEQ_CONEX G_CONEX G_DOTDOT
%token ICONST, ID, PCENT, EQUAL, SLASH, DOLLAR, SIZEOF, TYPEOF, QMARK, SEMI
%token UNOP, STAR, PLUS, MINUS, AMPER, ARROW, DOT, LB, LP, COMMA, ERROR, RB, RP
%token PLUSPLUS, MINUSMINUS, EQUALEQUAL, GREATER, LESS, BAR, BARBAR
%token AMPERAMPER, HAT, TILDE, GREATEREQUAL, LESSEQUAL, FABS
%token GREATERGREATER, LESSLESS, BANG, BANGEQUAL, DCONST, LC, RC, DOTDOT

%type <dd> DCONST
%type <ll> ICONST
%type <ss> ID
%type <ee> expr, list, conex
%type <cc> UNOP, STAR, PLUS, MINUS, AMPER, ARROW, DOT, LB, LP, COMMA, ERROR, SLASH
%type <cc> EQUAL, PCENT, QMARK, SEMI
%type <cc> PLUSPLUS, MINUSMINUS, EQUALEQUAL, GREATER, LESS, BAR, BARBAR
%type <cc> AMPERAMPER, HAT, TILDE, GREATEREQUAL, LESSEQUAL
%type <cc> GREATERGREATER, LESSLESS, BANG, BANGEQUAL, FABS, LC, RC, DOTDOT

%left DOTDOT
%left COMMA
%right EQUAL
%left BARBAR
%left AMPERAMPER
%left BAR
%left HAT
%left AMPER
%left EQUALEQUAL BANGEQUAL
%left LESS GREATER LESSEQUAL GREATEREQUAL
%left PLUS MINUS
%left STAR SLASH PCENT
%left SIZEOF TYPEOF BANG TILDE
%right UNOP
%left ARROW DOT LB LP

%%

start:	G_EXPR   { DotDot=0; } expr SEMI	{ yyres = (long) $expr; }
|	G_DOTDOT { DotDot=1; } expr SEMI	{ yyres = (long) $expr; }
|	G_DOTEQ_CONEX DOT EQUAL conex SEMI	{ yyres = (long) $conex; }
|	G_DOLEQ_CONEX DOLLAR EQUAL conex SEMI	{ yyres = (long) $conex; }
|	G_CONEX conex SEMI			{ yyres = (long) $conex; }

expr:	DOLLAR			{ if( !CurrentExpr){
					yyerror("$ cannot be used here");
					YYACCEPT;
				  }
				  $$ = CurrentExpr;
				}
|	LC expr RC ID		{ $$ = E_Binary( $expr, O_ENV, E_Id($ID) ); }
|	ID			{ $$ = E_Id( $ID ); }
|	DCONST			{ $$ = E_DConst( $DCONST ); }
|	ICONST			{ $$ = E_IConst( $ICONST ); }
|	STAR  expr %prec UNOP 	{ $$ = E_Unary( O_DEREF, $expr ); }
|	AMPER expr  		{ $$ = E_Unary( O_REF, $expr ); }
|	SIZEOF expr 		{ $$ = E_Unary( O_SIZEOF, $expr ); }
|	TYPEOF expr 		{ $$ = E_Unary( O_TYPEOF, $expr ); }
|	BANG expr		{ $$ = E_Unary( O_LOGNOT, $expr ); }
|	MINUS expr %prec UNOP	{ $$ = E_Unary( O_MINUS, $expr ); }
|	TILDE expr		{ $$ = E_Unary( O_1SCOMP, $expr ); }
|	expr DOTDOT expr	{ if( !DotDot ){
					yyerror(".. cannot be used here");
					YYACCEPT;
				  }
				  $$ = E_Binary( $expr, O_RANGE, $expr#2 );}
|	expr DOT ID		{ $$ = E_Binary( $expr, O_DOT, E_Id($ID)); }
|	expr COMMA expr		{ $$ = E_Binary( $expr, O_COMMA, $expr#2 ); }
|	expr EQUAL expr		{ $$ = E_Binary( $expr, O_ASSIGN, $expr#2 ); }
|	expr BARBAR expr	{ $$ = E_Binary( $expr, O_LOGOR, $expr#2 ); }
|	expr AMPERAMPER expr	{ $$ = E_Binary( $expr, O_LOGAND, $expr#2 ); }
|	expr BAR expr		{ $$ = E_Binary( $expr, O_BITOR, $expr#2 ); }
|	expr AMPER expr		{ $$ = E_Binary( $expr, O_BITAND, $expr#2 ); }
|	expr HAT expr		{ $$ = E_Binary( $expr, O_BITXOR, $expr#2 ); }
|	expr EQUALEQUAL expr	{ $$ = E_Binary( $expr, O_EQ, $expr#2 ); }
|	expr BANGEQUAL expr	{ $$ = E_Binary( $expr, O_NE, $expr#2 ); }
|	expr LESS expr		{ $$ = E_Binary( $expr, O_LT, $expr#2 ); }
|	expr GREATER expr	{ $$ = E_Binary( $expr, O_GT, $expr#2 ); }
|	expr LESSEQUAL expr	{ $$ = E_Binary( $expr, O_LE, $expr#2 ); }
|	expr GREATEREQUAL expr	{ $$ = E_Binary( $expr, O_GE, $expr#2 ); }
|	expr MINUS expr		{ $$ = E_Binary( $expr, O_MINUS, $expr#2 ); }
|	expr ARROW ID		{ $$ = E_Binary( $expr, O_ARROW, E_Id($ID)); }
|	expr PCENT expr		{ $$ = E_Binary( $expr, O_MOD, $expr#2 ); }
|	expr STAR expr		{ $$ = E_Binary( $expr, O_MULT, $expr#2 ); }
|	expr SLASH expr		{ $$ = E_Binary( $expr, O_DIV, $expr#2 ); }
|	expr PLUS expr		{ $$ = E_Binary( $expr, O_PLUS, $expr#2 ); }
|	expr LB expr RB		{ $$ = E_Binary( $expr, O_INDEX, $expr#2 );}
|	ID LP list RP		{ $$ = E_Binary( E_Id($ID), O_CALL, $list ); }
|	ID LP RP 		{ $$ = E_Binary( E_Id($ID), O_CALL, 0 ); }
|	LP expr RP		{ $$ = $expr; }
|	FABS LP expr RP		{ $$ = E_Unary( O_FABS, $expr ); }

conex:	DOLLAR			{ if( !CurrentExpr){
					yyerror("no current expression for $");
					YYACCEPT;
				  }
				  $$ = CurrentExpr;
				}
|	ID			{ $$ = E_Id( $ID ); }
|	ICONST			{ $$ = E_IConst( $ICONST ); }
|	AMPER conex  %prec UNOP	{ $$ = E_Unary( O_REF, $conex ); }
|	MINUS conex %prec UNOP	{ $$ = E_Unary( O_MINUS, $conex ); }
|	conex MINUS conex	{ $$ = E_Binary( $conex, O_MINUS, $conex#2 ); }
|	conex PCENT conex	{ $$ = E_Binary( $conex, O_MOD, $conex#2 ); }
|	conex STAR  conex	{ $$ = E_Binary( $conex, O_MULT, $conex#2 ); }
|	conex SLASH conex	{ $$ = E_Binary( $conex, O_DIV, $conex#2 ); }
|	conex PLUS  conex	{ $$ = E_Binary( $conex, O_PLUS, $conex#2 ); }
|	conex LB conex RB	{ $$ = E_Binary( $conex, O_INDEX, $conex#2 );}
|	LP conex RP		{ $$ = $conex; }

list:	expr	%prec COMMA	{ $$ = $expr; }
|	list COMMA expr		{ $$ = E_Binary( $list, O_COMMA, $expr ); } 

%%

#define LOOK (LexString[LexIndex ])
#define TAKE (AddToken(), LexString[LexIndex++])
#define MORE (LexString[LexIndex+1])
#define yc (yylval.cc)
#define yd (yylval.dd)
#define yl (yylval.ll)
#define ys (yylval.ss)
#define ishex(x) (isdigit(x) || (x>='a'&&x<='f') || (x>='A'&&x<='F'))
#define isoct(x) ( x>='0' && x<='7' )

yylex()
{
	int token = doyylex();

	return token;
}

void AddToken()
{
	int l = strlen(Token);

	if( l < 64 ){
		Token[l] = LOOK;
		Token[l+1] = '\0';
	}
}

doyylex()
{
	double atof(char*);

	if( LexIndex < 0 ){
		LexIndex = 0;
		return LexGoal;
	}
	while( isspace(LOOK) ) TAKE;
	Token[0] = '\0';
	if( isalpha(LOOK) || LOOK=='_' || LOOK=='$' ){
		TAKE;
		while( isalnum(LOOK) || LOOK=='_' ) TAKE;
		strcpy( ys, Token );
		if( !strcmp(ys,"sizeof") ) return SIZEOF;
		if( !strcmp(ys,"typeof") ) return TYPEOF;
		if( !strcmp(ys,"fabs") ) return FABS;
		if( !strcmp(ys,"$") ) return DOLLAR;
		return ID;
	}
	if( LOOK == '\'' ){
		TAKE;
		if( LOOK == '\\' ){
			TAKE;
			if( MORE != '\'' ) return 0;
			char *trans = "bnftv", *late = "\b\n\f\t\v";
			yl = LOOK;
			for( int i = 0; trans[i]; ++i )
				if( LOOK == trans[i] ) yl = late[i];
			TAKE; TAKE; return ICONST;
		}
		if( MORE != '\'' ) return 0;
		yl = TAKE;
		TAKE;
		return ICONST;
	}
	if( LOOK=='0' && (MORE=='x' || MORE=='X') ){
		TAKE; TAKE;
		if( !ishex(LOOK) ) return 0;
		for( yl = 0; ishex(LOOK); TAKE )
		    yl = (yl<<4) + (isalpha(LOOK) ? (LOOK|' ')+10-'a' : LOOK-'0');
		return ICONST;
	}
	if( LOOK=='0' ){
		for( TAKE, yl = 0; isoct(LOOK); TAKE ) yl = (yl<<3) + LOOK - '0';
		goto IorD;
	}
	if( isdigit(LOOK) ){
		for( yl = 0; isdigit(LOOK); TAKE ) yl = yl*10 + LOOK - '0';
		goto IorD;
	}
	if( LOOK == '.' && isdigit(MORE) ) goto Point;
	if( LOOK=='.' && MORE=='.' ) return (TAKE, TAKE, DOTDOT);
	if( LOOK=='-' && MORE=='>' ) return (TAKE, TAKE, ARROW);
	if( LOOK=='-' && MORE=='-' ) return (TAKE, TAKE, MINUSMINUS);
	if( LOOK=='+' && MORE=='+' ) return (TAKE, TAKE, PLUSPLUS);
	if( LOOK=='=' && MORE=='=' ) return (TAKE, TAKE, EQUALEQUAL);
	if( LOOK=='!' && MORE=='=' ) return (TAKE, TAKE, BANGEQUAL);
	if( LOOK==':' && MORE=='=' ) return (TAKE, TAKE, EQUAL);
	if( LOOK=='>' && MORE=='=' ) return (TAKE, TAKE, GREATEREQUAL);
	if( LOOK=='<' && MORE=='=' ) return (TAKE, TAKE, LESSEQUAL);
	if( LOOK=='&' && MORE=='&' ) return (TAKE, TAKE, AMPERAMPER);
	if( LOOK=='|' && MORE=='|' ) return (TAKE, TAKE, BARBAR);
	switch( TAKE ){
		case '>' : return GREATER;
		case '<' : return LESS;
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
		case '%' : return PCENT;
		case '=' : return EQUAL;
		case ';' : return SEMI;
		case '|' : return BAR;
		case '^' : return HAT;
		case '~' : return TILDE;
		case '!' : return BANG;
		case '{' : return LC;
		case '}' : return RC;
		default  : return 0;
	}
IorD:
	if( LOOK=='.' && MORE=='.' ) return ICONST;
	if( LOOK=='.' ) goto Point;
	if( LOOK=='e' || LOOK=='E' ) goto Exp;
	return ICONST;
Point:
	for( TAKE; isdigit(LOOK); TAKE) {}
	if( LOOK!='e' && LOOK!='E' ) goto Double;
Exp:
	TAKE;
	if( LOOK=='+' || LOOK=='-' ) TAKE;
	if( !isdigit(LOOK) ) return 0;
	while( isdigit(LOOK) ) TAKE;
Double:
	yd = atof(Token);
	return DCONST;	

}

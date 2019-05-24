
# line 2 "gram.y"
#include "gram.h"
#include "expr.pub"
#include <ctype.h>
int DotDot;
Expr *E_IConst(long), *E_DConst(double);

# line 9 "gram.y"
typedef union  {
	char		cc;
	long		ll;
	char		ss[32];
	struct Expr	*ee;
	double		dd;
} YYSTYPE;
# define G_EXPR 257
# define G_DOTEQ_CONEX 258
# define G_DOLEQ_CONEX 259
# define G_CONEX 260
# define G_DOTDOT 261
# define ICONST 262
# define ID 263
# define PCENT 264
# define EQUAL 265
# define SLASH 266
# define DOLLAR 267
# define SIZEOF 268
# define TYPEOF 269
# define QMARK 270
# define SEMI 271
# define UNOP 272
# define STAR 273
# define PLUS 274
# define MINUS 275
# define AMPER 276
# define ARROW 277
# define DOT 278
# define LB 279
# define LP 280
# define COMMA 281
# define ERROR 282
# define RB 283
# define RP 284
# define PLUSPLUS 285
# define MINUSMINUS 286
# define EQUALEQUAL 287
# define GREATER 288
# define LESS 289
# define BAR 290
# define BARBAR 291
# define AMPERAMPER 292
# define HAT 293
# define TILDE 294
# define GREATEREQUAL 295
# define LESSEQUAL 296
# define FABS 297
# define GREATERGREATER 298
# define LESSLESS 299
# define BANG 300
# define BANGEQUAL 301
# define DCONST 302
# define LC 303
# define RC 304
# define DOTDOT 305
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern short yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

# line 127 "gram.y"


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
short yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 60
# define YYLAST 711
short yyact[]={

  64,  50,  66, 124,  79,  70, 123,  42,   9,  65,
  67,  62,  54,  63,  48,  68,  10,  49,  35,  34,
 125, 122, 106,  56,  59,  58,  53,  51,  52,  55,
  91,  61,  60,  64,  50,  66,   8,  57,  63,  48,
  68,  47,  65,  67,  62,  54,  63,  48,  68,   7,
  49,   1, 121, 113,   0,   0,  56,  59,  58,  53,
  51,  52,  55,   0,  61,  60,  64,  50,  66,   0,
  57,   0,   0,   0,  47,  65,  67,  62,  54,  63,
  48,  68,   0,  49,   0,   0, 116,   0,   0,  56,
  59,  58,  53,  51,  52,  55,   0,  61,  60,  64,
  50,  66,   0,  57,   0,   0,   0,  47,  65,  67,
  62,  54,  63,  48,  68,   0,  49,   2,   4,   5,
   6,   3,  56,  59,  58,  53,  51,  52,  55,   0,
  61,  60,   0,   0,   0,   0,  57,   0,   0, 112,
  47,  64,  50,  66,   0,   0,   0,   0,  80,   0,
  65,  67,  62,  54,  63,  48,  68,   0,  49,   0,
   0,   0,   0,   0,  56,  59,  58,  53,  51,  52,
  55,   0,  61,  60,  64,  50,  66,   0,  57,   0,
   0,  46,  47,  65,  67,  62,  54,  63,  48,  68,
   0,  49,   0,   0,   0,   0,   0,  56,  59,  58,
  53,  51,  52,  55,   0,  61,  60,  64,  50,  66,
   0,  57,   0,   0,   0,  47,  65,  67,  62,  54,
  63,  48,  68,   0,   0,   0,   0,   0,   0,   0,
  56,  59,  58,  53,  51,  52,  55,   0,  61,  60,
  64,  50,  66,   0,  57,   0,   0,   0,  47,  65,
  67,  62,  54,  63,  48,  68,   0,  49,   0,   0,
   0,   0,   0,  56,  59,  58,  53,  51,  52,  55,
   0,  61,  60,  64,  50,  66,   0,  57,   0,   0,
   0,   0,  65,  67,  62,  54,  63,  48,  68,   0,
   0,   0,   0,   0,   0,   0,  56,  59,  58,  53,
  51,  52,  55,   0,  61,  60,  64,   0,  66,   0,
  57,   0,   0,   0,   0,  65,  67,  62,  54,  63,
  48,  68,   0,   0,   0,   0,   0,   0,   0,  56,
  59,  58,  53,   0,  52,  55,   0,  61,  60,  64,
   0,  66,   0,  57,   0,   0,   0,   0,  65,  67,
  62,  54,  63,  48,  68,   0,   0,   0,   0,   0,
   0,   0,  56,  59,  58,  53,   0,   0,  55,   0,
  61,  60,  23,  21,   0,   0,  57,  19,  26,  27,
   0,   0,  38,  24,  40,  29,  25,   0,   0,   0,
  31,  39,  41,  37, 114,   0,  38,  42,  40,   0,
   0,   0,  89,   0,  30,  39,   0,  32,   0,   0,
  28,  42,  22,  20,  23,  21,   0,   0,   0,  19,
  26,  27,   0,   0,  38,  24,  40,  29,  25,   0,
   0,   0,  31,  39,  41,  37,   0,   0,   0,  42,
   0,   0,   0, 120,   0,   0,  30,   0,  64,  32,
  66,   0,  28,   0,  22,  20,   0,  65,  67,  62,
  54,  63,  48,  68,   0,   0,   0,   0,   0,   0,
   0,  56,  59,  58,   0,   0,   0,  55,   0,  61,
  60,  64,   0,  66,   0,  57,   0,   0,   0,   0,
  65,  67,  62,  54,  63,  48,  68,   0,   0,   0,
  38,   0,  40,   0,  56,  59,  58, 119,   0,  39,
  41,  37,  61,  60,  64,  42,  66,  64,  57,  66,
   0,   0,   0,  65,  67,  62,  65,  63,  48,  68,
  63,  48,  68,  38,   0,  40,   0,  56,  59,  58,
 118,   0,  39,  41,  37,  61,  60,  64,  42,  66,
   0,  57,   0,   0,   0,   0,  65,  67,  62,   0,
  63,  48,  68,  14,  13,   0,   0,   0,  12,   0,
   0,  59,  58,  64,   0,  66,  16,  15,  61,  60,
   0,  17,  65,  67,  62,  18,  63,  48,  68,  38,
   0,  40,   0,   0,  33,   0,  36,   0,  39,  41,
  37,   0,   0,   0,  42,   0,  69,   0,   0,   0,
  71,  72,  73,  74,  75,  76,  77,  78,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,  90,   0,  92,  93,  94,  95,  96,
  97,  98,  99, 100, 101, 102, 103, 104, 105,   0,
 107, 108, 109, 110, 111,  11, 115,   0,   0,   0,
   0,   0,   0,   0,   0, 117,   0,   0,   0,   0,
   0,  43,  44,  45,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  81,  82,   0,  83,  84,  85,  86,  87,  88,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
 126 };
short yypact[]={

-140,-1000,-1000,-1000,-270,-251, 301, 152, 152,-246,
-247, 325,-1000,-1000,-1000, 301, 301, 301, -90,-1000,
 152,-275,-1000,-1000, 152, 152, 152, 152, 152, 152,
 152, 152,-276,-123, 301, 301,-1000, 301, 301, 301,
 301, 301, 301,-272,-272, 118,-1000, 152,-233, 152,
 152, 152, 152, 152, 152, 152, 152, 152, 152, 152,
 152, 152, 152,-241, 152, 152, 152, 152, 152,-165,
 110,-239, 250,-239,-239,-239,-239,-239,-198, 152,
-1000, 269, 236, 132,-272,-272,-272, 132, 160,-1000,
 -24,-1000,   9,   9,  42,  75, 184, 250, 217, 283,
 283, 309, 309, 309, 309, 253,-1000,-239,-239,-239,
 253,-231,-242,-278,-1000, -57,-1000,-264,-1000,-1000,
-1000,-1000,-1000,-1000, 152,-1000, -57 };
short yypgo[]={

   0, 585,  53, 655,  51,  49,  36 };
short yyr1[]={

   0,   5,   4,   6,   4,   4,   4,   4,   1,   1,
   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
   1,   1,   1,   1,   1,   1,   3,   3,   3,   3,
   3,   3,   3,   3,   3,   3,   3,   3,   2,   2 };
short yyr2[]={

   0,   0,   4,   0,   4,   5,   5,   3,   1,   4,
   1,   1,   1,   2,   2,   2,   2,   2,   2,   2,
   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
   3,   4,   4,   3,   3,   4,   1,   1,   1,   2,
   2,   3,   3,   3,   3,   3,   4,   3,   1,   3 };
short yychk[]={

-1000,  -4, 257, 261, 258, 259, 260,  -5,  -6, 278,
 267,  -3, 267, 263, 262, 276, 275, 280,  -1, 267,
 303, 263, 302, 262, 273, 276, 268, 269, 300, 275,
 294, 280, 297,  -1, 265, 265, 271, 275, 264, 273,
 266, 274, 279,  -3,  -3,  -3, 271, 305, 278, 281,
 265, 291, 292, 290, 276, 293, 287, 301, 289, 288,
 296, 295, 275, 277, 264, 273, 266, 274, 279,  -1,
 280,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 280,
 271,  -3,  -3,  -3,  -3,  -3,  -3,  -3,  -3, 284,
  -1, 263,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
  -1,  -1,  -1,  -1,  -1,  -1, 263,  -1,  -1,  -1,
  -1,  -1, 304,  -2, 284,  -1, 284,  -1, 271, 271,
 283, 283, 263, 284, 281, 284,  -1 };
short yydef[]={

   0,  -2,   1,   3,   0,   0,   0,   0,   0,   0,
   0,   0,  46,  47,  48,   0,   0,   0,   0,   8,
   0,  10,  11,  12,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   7,   0,   0,   0,
   0,   0,   0,  49,  50,   0,   2,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,  13,  14,  15,  16,  17,  18,  19,   0,   0,
   4,   0,   0,  51,  52,  53,  54,  55,   0,  57,
  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,
  40,   0,   0,   0,  43,  58,  44,   0,   5,   6,
  56,  41,   9,  42,   0,  45,  59 };
# ifdef YYDEBUG
# include "y.debug"
# endif

# define YYFLAG -1000
# define YYERROR goto yyerrlab
# define YYACCEPT return(0)
# define YYABORT return(1)

/*	parser for yacc output	*/

#ifdef YYDEBUG
int yydebug = 0; /* 1 for debugging */
#endif
YYSTYPE yyv[YYMAXDEPTH]; /* where the values are stored */
int yychar = -1; /* current input token number */
int yynerrs = 0;  /* number of errors */
short yyerrflag = 0;  /* error recovery flag */

yyparse()
{	short yys[YYMAXDEPTH];
	int yyj, yym;
	register YYSTYPE *yypvt;
	register int yystate, yyn;
	register short *yyps;
	register YYSTYPE *yypv;
	register short *yyxi;

	yystate = 0;
	yychar = -1;
	yynerrs = 0;
	yyerrflag = 0;
	yyps= &yys[-1];
	yypv= &yyv[-1];

yystack:    /* put a state and value onto the stack */
#ifdef YYDEBUG
	if(yydebug >= 3)
		if(yychar < 0 || yytoknames[yychar] == 0)
			printf("char %d in %s", yychar, yystates[yystate]);
		else
			printf("%s in %s", yytoknames[yychar], yystates[yystate]);
#endif
	if( ++yyps >= &yys[YYMAXDEPTH] ) { 
		yyerror( "yacc stack overflow" ); 
		return(1); 
	}
	*yyps = yystate;
	++yypv;
	*yypv = yyval;
yynewstate:
	yyn = yypact[yystate];
	if(yyn <= YYFLAG) goto yydefault; /* simple state */
	if(yychar<0) {
		yychar = yylex();
#ifdef YYDEBUG
		if(yydebug >= 2) {
			if(yychar <= 0)
				printf("lex EOF\n");
			else if(yytoknames[yychar])
				printf("lex %s\n", yytoknames[yychar]);
			else
				printf("lex (%c)\n", yychar);
		}
#endif
		if(yychar < 0)
			yychar = 0;
	}
	if((yyn += yychar) < 0 || yyn >= YYLAST)
		goto yydefault;
	if( yychk[ yyn=yyact[ yyn ] ] == yychar ){ /* valid shift */
		yychar = -1;
		yyval = yylval;
		yystate = yyn;
		if( yyerrflag > 0 ) --yyerrflag;
		goto yystack;
	}
yydefault:
	/* default state action */
	if( (yyn=yydef[yystate]) == -2 ) {
		if(yychar < 0) {
			yychar = yylex();
#ifdef YYDEBUG
			if(yydebug >= 2)
				if(yychar < 0)
					printf("lex EOF\n");
				else
					printf("lex %s\n", yytoknames[yychar]);
#endif
			if(yychar < 0)
				yychar = 0;
		}
		/* look through exception table */
		for(yyxi=yyexca; (*yyxi!= (-1)) || (yyxi[1]!=yystate);
			yyxi += 2 ) ; /* VOID */
		while( *(yyxi+=2) >= 0 ){
			if( *yyxi == yychar ) break;
		}
		if( (yyn = yyxi[1]) < 0 ) return(0);   /* accept */
	}
	if( yyn == 0 ){ /* error */
		/* error ... attempt to resume parsing */
		switch( yyerrflag ){
		case 0:   /* brand new error */
#ifdef YYDEBUG
			yyerror("syntax error\n%s", yystates[yystate]);
			if(yytoknames[yychar])
				yyerror("saw %s\n", yytoknames[yychar]);
			else if(yychar >= ' ' && yychar < '\177')
				yyerror("saw `%c'\n", yychar);
			else if(yychar == 0)
				yyerror("saw EOF\n");
			else
				yyerror("saw char 0%o\n", yychar);
#else
			yyerror( "syntax error" );
#endif
yyerrlab:
			++yynerrs;
		case 1:
		case 2: /* incompletely recovered error ... try again */
			yyerrflag = 3;
			/* find a state where "error" is a legal shift action */
			while ( yyps >= yys ) {
				yyn = yypact[*yyps] + YYERRCODE;
				if( yyn>= 0 && yyn < YYLAST && yychk[yyact[yyn]] == YYERRCODE ){
					yystate = yyact[yyn];  /* simulate a shift of "error" */
					goto yystack;
				}
				yyn = yypact[*yyps];
				/* the current yyps has no shift onn "error", pop stack */
#ifdef YYDEBUG
				if( yydebug ) printf( "error recovery pops state %d, uncovers %d\n", *yyps, yyps[-1] );
#endif
				--yyps;
				--yypv;
			}
			/* there is no state on the stack with an error shift ... abort */
yyabort:
			return(1);
		case 3:  /* no shift yet; clobber input char */
#ifdef YYDEBUG
			if( yydebug ) printf( "error recovery discards char %d\n", yychar );
#endif
			if( yychar == 0 ) goto yyabort; /* don't discard EOF, quit */
			yychar = -1;
			goto yynewstate;   /* try again in the same state */
		}
	}
	/* reduction by production yyn */
#ifdef YYDEBUG
	if(yydebug) {	char *s;
		printf("reduce %d in:\n\t", yyn);
		for(s = yystates[yystate]; *s; s++) {
			putchar(*s);
			if(*s == '\n' && *(s+1))
				putchar('\t');
		}
	}
#endif
	yyps -= yyr2[yyn];
	yypvt = yypv;
	yypv -= yyr2[yyn];
	yyval = yypv[1];
	yym=yyn;
	/* consult goto table to find next state */
	yyn = yyr1[yyn];
	yyj = yypgo[yyn] + *yyps + 1;
	if( yyj>=YYLAST || yychk[ yystate = yyact[yyj] ] != -yyn ) yystate = yyact[yypgo[yyn]];
	switch(yym){
		
case 1:
# line 52 "gram.y"
{ DotDot=0; } break;
case 2:
# line 52 "gram.y"
{ yyres = (long) yypvt[-1].ee; } break;
case 3:
# line 53 "gram.y"
{ DotDot=1; } break;
case 4:
# line 53 "gram.y"
{ yyres = (long) yypvt[-1].ee; } break;
case 5:
# line 54 "gram.y"
{ yyres = (long) yypvt[-1].ee; } break;
case 6:
# line 55 "gram.y"
{ yyres = (long) yypvt[-1].ee; } break;
case 7:
# line 56 "gram.y"
{ yyres = (long) yypvt[-1].ee; } break;
case 8:
# line 58 "gram.y"
{ if( !CurrentExpr){
					yyerror("$ cannot be used here");
					YYACCEPT;
				  }
				  yyval.ee = CurrentExpr;
				} break;
case 9:
# line 64 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_ENV, E_Id(yypvt[-0].ss) ); } break;
case 10:
# line 65 "gram.y"
{ yyval.ee = E_Id( yypvt[-0].ss); } break;
case 11:
# line 66 "gram.y"
{ yyval.ee = E_DConst( yypvt[-0].dd); } break;
case 12:
# line 67 "gram.y"
{ yyval.ee = E_IConst( yypvt[-0].ll); } break;
case 13:
# line 68 "gram.y"
{ yyval.ee = E_Unary( O_DEREF, yypvt[-0].ee); } break;
case 14:
# line 69 "gram.y"
{ yyval.ee = E_Unary( O_REF, yypvt[-0].ee); } break;
case 15:
# line 70 "gram.y"
{ yyval.ee = E_Unary( O_SIZEOF, yypvt[-0].ee); } break;
case 16:
# line 71 "gram.y"
{ yyval.ee = E_Unary( O_TYPEOF, yypvt[-0].ee); } break;
case 17:
# line 72 "gram.y"
{ yyval.ee = E_Unary( O_LOGNOT, yypvt[-0].ee); } break;
case 18:
# line 73 "gram.y"
{ yyval.ee = E_Unary( O_MINUS, yypvt[-0].ee); } break;
case 19:
# line 74 "gram.y"
{ yyval.ee = E_Unary( O_1SCOMP, yypvt[-0].ee); } break;
case 20:
# line 75 "gram.y"
{ if( !DotDot ){
					yyerror(".. cannot be used here");
					YYACCEPT;
				  }
				  yyval.ee = E_Binary( yypvt[-2].ee, O_RANGE, yypvt[-0].ee );} break;
case 21:
# line 80 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_DOT, E_Id(yypvt[-0].ss)); } break;
case 22:
# line 81 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_COMMA, yypvt[-0].ee ); } break;
case 23:
# line 82 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_ASSIGN, yypvt[-0].ee ); } break;
case 24:
# line 83 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_LOGOR, yypvt[-0].ee ); } break;
case 25:
# line 84 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_LOGAND, yypvt[-0].ee ); } break;
case 26:
# line 85 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_BITOR, yypvt[-0].ee ); } break;
case 27:
# line 86 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_BITAND, yypvt[-0].ee ); } break;
case 28:
# line 87 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_BITXOR, yypvt[-0].ee ); } break;
case 29:
# line 88 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_EQ, yypvt[-0].ee ); } break;
case 30:
# line 89 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_NE, yypvt[-0].ee ); } break;
case 31:
# line 90 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_LT, yypvt[-0].ee ); } break;
case 32:
# line 91 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_GT, yypvt[-0].ee ); } break;
case 33:
# line 92 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_LE, yypvt[-0].ee ); } break;
case 34:
# line 93 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_GE, yypvt[-0].ee ); } break;
case 35:
# line 94 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_MINUS, yypvt[-0].ee ); } break;
case 36:
# line 95 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_ARROW, E_Id(yypvt[-0].ss)); } break;
case 37:
# line 96 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_MOD, yypvt[-0].ee ); } break;
case 38:
# line 97 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_MULT, yypvt[-0].ee ); } break;
case 39:
# line 98 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_DIV, yypvt[-0].ee ); } break;
case 40:
# line 99 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_PLUS, yypvt[-0].ee ); } break;
case 41:
# line 100 "gram.y"
{ yyval.ee = E_Binary( yypvt[-3].ee, O_INDEX, yypvt[-1].ee );} break;
case 42:
# line 101 "gram.y"
{ yyval.ee = E_Binary( E_Id(yypvt[-3].ss), O_CALL, yypvt[-1].ee); } break;
case 43:
# line 102 "gram.y"
{ yyval.ee = E_Binary( E_Id(yypvt[-2].ss), O_CALL, 0 ); } break;
case 44:
# line 103 "gram.y"
{ yyval.ee = yypvt[-1].ee; } break;
case 45:
# line 104 "gram.y"
{ yyval.ee = E_Unary( O_FABS, yypvt[-1].ee); } break;
case 46:
# line 106 "gram.y"
{ if( !CurrentExpr){
					yyerror("no current expression for $");
					YYACCEPT;
				  }
				  yyval.ee = CurrentExpr;
				} break;
case 47:
# line 112 "gram.y"
{ yyval.ee = E_Id( yypvt[-0].ss); } break;
case 48:
# line 113 "gram.y"
{ yyval.ee = E_IConst( yypvt[-0].ll); } break;
case 49:
# line 114 "gram.y"
{ yyval.ee = E_Unary( O_REF, yypvt[-0].ee); } break;
case 50:
# line 115 "gram.y"
{ yyval.ee = E_Unary( O_MINUS, yypvt[-0].ee); } break;
case 51:
# line 116 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_MINUS, yypvt[-0].ee ); } break;
case 52:
# line 117 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_MOD, yypvt[-0].ee ); } break;
case 53:
# line 118 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_MULT, yypvt[-0].ee ); } break;
case 54:
# line 119 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_DIV, yypvt[-0].ee ); } break;
case 55:
# line 120 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_PLUS, yypvt[-0].ee ); } break;
case 56:
# line 121 "gram.y"
{ yyval.ee = E_Binary( yypvt[-3].ee, O_INDEX, yypvt[-1].ee );} break;
case 57:
# line 122 "gram.y"
{ yyval.ee = yypvt[-1].ee; } break;
case 58:
# line 124 "gram.y"
{ yyval.ee = yypvt[-0].ee; } break;
case 59:
# line 125 "gram.y"
{ yyval.ee = E_Binary( yypvt[-2].ee, O_COMMA, yypvt[-0].ee); } break;
	}
	goto yystack;  /* stack new state and value */
}

# define MACHINE 257
# define LP 258
# define RP 259
# define NUMBER 260

# line 4 "ps.y"
#include "stdio.h"
FILE *infd;
#define YYDEBUG
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern short yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
#ifndef YYSTYPE
#define YYSTYPE int
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

# line 34 "ps.y"

int lineno = 1;
yyerror(s)
char *s;
{	char buf[128];
	sprintf(buf, "%s line %d", s, lineno);
	perror(buf);
}

pparse(fd)
FILE *fd;
{
	infd = fd;
	yyparse();
}

yylex()
{	int c;
again:
	c = getc(infd);
	if(c == '\n')
		lineno++;
loop:
	if(c == EOF)
		return(0);
	if(c == '#') {
		do {
			c = getc(infd);
		} while(c != '\n' && c != EOF);
		if(c == '\n')
			lineno++;
		goto loop;
	}
	if(c == ' ' || c == ',' || c == '\n' || c == '\t')
		goto again;
	if(c == '(')
		return(LP);
	if(c == ')')
		return(RP);
	if(c == '+' || c == '-' || c >= '0' && c <= '9') {
		donumber(c);
		return(NUMBER);
	}
	if(c >= 'a' && c <= 'z' || c == '/') {
		domachine(c);
		lineno++;
		return(MACHINE);
	}
	debug("bad char in people (%c) 0%o ignored\n", c, c);
	goto again;
}

donumber(c)
int c;
{	int n, i, sign;
	n = 0;
	sign = 1;
	if(c == '-')
		sign = -1;
	else if(c != '+')
		n = c - '0';
	for(c = getc(infd); c >= '0' && c <= '9'; c = getc(infd))
		n = 10 * n + c - '0';
	ungetc(c, infd);
	yylval = sign * n;
}

static char nmbuf[128];
domachine(c)
{	char *p = nmbuf;
	*p++ = c;
	for(c = getc(infd); (c >= 'a' && c <= 'z' || c == '/') && p < nmbuf + 127; c=getc(infd))
		*p++ = c;
	*p = 0;
	newhost(nmbuf);
}
short yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 16
# define YYLAST 32
short yyact[]={

  16,  12,  13,  13,  21,  13,  24,  23,  18,   8,
   4,  20,  11,   7,   3,  14,  10,   5,   6,   2,
   9,   1,   0,  15,   0,  17,   0,   0,   0,  19,
   0,  22 };
short yypact[]={

-247,-1000,-247,-1000,-249,-1000,-249,-1000,-257,-1000,
-258,-1000,-255,-1000,-251,-1000,-255,-256,-1000,-256,
-252,-1000,-253,-1000,-1000 };
short yypgo[]={

   0,  21,  19,  14,  18,  13,  16,  15,  12,  11 };
short yyr1[]={

   0,   1,   1,   2,   2,   3,   4,   4,   4,   5,
   6,   6,   7,   7,   8,   9 };
short yyr2[]={

   0,   0,   1,   1,   2,   2,   0,   1,   2,   4,
   1,   4,   1,   4,   1,   1 };
short yychk[]={

-1000,  -1,  -2,  -3, 257,  -3,  -4,  -5, 258,  -5,
  -6,  -8, 258, 260,  -7,  -8, 258,  -8, 259,  -8,
  -9, 260,  -9, 259, 259 };
short yydef[]={

   1,  -2,   2,   3,   6,   4,   5,   7,   0,   8,
   0,  10,   0,  14,   0,  12,   0,   0,   9,   0,
   0,  15,   0,  11,  13 };
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
		
case 10:
# line 23 "ps.y"
{ doclient(yypvt[-0], -1); } break;
case 11:
# line 25 "ps.y"
{ doclient(yypvt[-2], yypvt[-1]); } break;
case 12:
# line 28 "ps.y"
{ dohost(yypvt[-0], -1); } break;
case 13:
# line 30 "ps.y"
{ dohost(yypvt[-2], yypvt[-1]); } break;
	}
	goto yystack;  /* stack new state and value */
}

# define COLON 257
# define PLUS 258
# define MINUS 259
# define MUL 260
# define DIV 261
# define AMP 262
# define OFFOP 263
# define CM 264
# define NL 265
# define LB 266
# define RB 267
# define LP 268
# define RP 269
# define SEMI 270
# define INT 271
# define NAME 272
# define REG 273
# define NOCHAR 274
# define SPC 275
# define ALPH 276
# define DIGIT 277
# define SQ 278
# define SH 279
# define DQ 280
# define EXCL 281
# define EOR 282
# define IOR 283
# define STRING 284
# define SIZE 285
# define ISPACE 286
# define IBYTE 287
# define ILONG 288
# define INST0 289
# define INST1 290
# define INST2 291
# define ISHORT 292
# define IDATA 293
# define IGLOBAL 294
# define ISET 295
# define ITEXT 296
# define ICOMM 297
# define ILCOMM 298
# define IEVEN 299
# define IORG 300
# define IOPTIM 301
# define IPCREL 302
# define ISTABS 303
# define ISTABD 304
# define ISTABN 305
# define NOTYET 306
# define FILENAME 307

# line 37 "mas0.y"
#include <stdio.h>
#include <ctype.h>
#include "mas.h"

struct arg	arglist[3];
struct arg *ap	= { arglist};
struct	exp	explist[10];
struct	exp	*xp = { explist};
int	i, ii;
long	li;
extern struct	exp	usedot[];
struct	exp	*dotp = &usedot[0];
int	anyerrs;
int	passno	= 1;
FILE	*tmpfil;
FILE	*relfil;
FILE	*txtfil;
int	hshused;
long	tsize;
long	dsize;
long	bitfield;
int	bitoff;
int	usrname;
char	yytext[NCPS+2];
char	yystring[NCPS];
struct	nlist stabsym;
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern short yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;

# line 68 "mas0.y"
	struct exp *pval;
# define YYERRCODE 256

# line 511 "mas0.y"


/* VARARGS1 */
yyerror(s, a)
char *s;
{
	static char *lastfile = NULL;

	if (anyerrs==0)
		fprintf(stderr, "Assembler:\n");
	anyerrs++;
	fprintf(stderr, "line %d: ", lineno);
	fprintf(stderr, " (pass %d) ", passno);
	fprintf(stderr, s, a);
	if(lastfile != filename)
		fprintf(stderr, "\t(%s)", lastfile = filename);
	fprintf(stderr, "\n");
	yyerrok;
}
short yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
-1, 2,
	0, 1,
	265, 6,
	270, 6,
	272, 6,
	286, 6,
	287, 6,
	288, 6,
	289, 6,
	290, 6,
	291, 6,
	292, 6,
	293, 6,
	294, 6,
	295, 6,
	296, 6,
	297, 6,
	298, 6,
	299, 6,
	300, 6,
	301, 6,
	302, 6,
	303, 6,
	304, 6,
	305, 6,
	-2, 0,
	};
# define YYNPROD 66
# define YYLAST 223
short yyact[]={

   9, 129, 125,  51,  55,  80,  87,  82,  49,  35,
  33, 130, 128, 117,  18,  15,  16,  22,  23,  24,
  17,  13,  12,  11,  14,  30,  31,  21,  19,  29,
  25,  26,  28,  27,  77, 108,   6,  79,  78,  94,
  41,   7, 110,  76,  96,   8,  39,  38,  80,  40,
 123,  41,  39,  38,  80, 122,  42,  41, 124, 113,
  40, 112,  42,  39,  38, 101,  98, 115,  93,  39,
  38,  85, 114,  42,  60,  61,  62,  63,  84,  42,
 116,  83,  72,  59,  58,  92,  60,  61,  62,  63,
  62,  63,  32,   4,  69,  50,  68,  73,  75,  37,
  46,  45,  44,  67,  34,  10,   5,   3,  36,  43,
   2,   1,  20,  47,  48,   0,   0,   0,   0,  52,
  53,  54,  56,  57,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  64,  65,  66,   0,   0,   0,   0,  74,  74,  70,
  71,  81,   0,  86,   0,   0,   0,   0,   0,   0,
  88,  89,  90,  91,   0,   0,   0,  95,   0,   0,
   0,   0,   0,  99, 100,  97,  64,  65, 102, 103,
 104,   0,   0,   0,   0,   0,   0,   0,   0, 106,
 105,   0,   0,   0,   0, 107,  74, 109,  64, 111,
   0,   0,   0,   0,   0,   0,   0, 119, 120, 118,
   0,   0,   0,   0, 121,   0,   0,   0,   0, 127,
   0,   0, 126 };
short yypact[]={

-1000,-1000,-163,-229,-220,-272,-1000,-1000,-1000,-165,
-1000,-262,-263,-208,-208,-1000,-1000,-1000,-208,-208,
-264,-1000,-282,-282,-282,-282,-280,-208,-208,-1000,
-1000,-1000,-1000,-180,-181,-1000,-1000,-172,-1000,-1000,
-208,-208,-208,-1000,-208,-208,-208,-1000,-1000,-182,
-1000,-1000,-225,-225,-265,-183,-186,-193,-208,-266,
-208,-208,-208,-208,-184,-1000,-1000,-196,-1000,-218,
-196,-196,-208,-1000,-224,-1000,-219,-202,-208,-208,
-1000,-199,-1000,-208,-208,-208,-1000,-1000,-170,-170,
-1000,-1000,-1000,-208,-208,-1000,-268,-234,-219,-226,
-1000,-225,-203,-205,-1000,-1000,-1000,-197,-178,-256,
-268,-1000,-208,-208,-1000,-268,-1000,-1000,-214,-206,
-1000,-283,-1000,-268,-208,-257,-284,-1000,-1000,-258,
-1000 };
short yypgo[]={

   0, 112,  97,  98,  95,  94,  99, 111, 110, 107,
 106, 105, 104, 102, 103, 101, 100,  96 };
short yyr1[]={

   0,   7,   8,   8,   8,   8,  10,  10,   9,  11,
  11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
  11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
  11,  12,  12,   4,   4,   1,   1,  14,  14,  14,
  17,  17,  13,  15,  16,   2,   2,   2,   2,   2,
   2,   2,   2,   2,   2,   3,   5,   6,   6,   6,
   6,   6,   6,   6,   6,   6 };
short yyr2[]={

   0,   1,   0,   3,   3,   3,   0,   3,   2,   4,
   2,   1,   2,   1,   2,   3,   3,   3,   2,   2,
   4,   1,   2,   3,   5,   3,   8,   6,   4,   1,
   0,   1,   3,   1,   0,   1,   1,   0,   3,   1,
   1,   3,   0,   0,   0,   1,   1,   3,   4,   4,
   4,   5,   7,   8,   2,   1,   1,   1,   1,   3,
   3,   3,   3,   3,   2,   2 };
short yychk[]={

-1000,  -7,  -8,  -9, 256, -10, 265, 270, 265, 272,
 -11, 295, 294, 293, 296, 287, 288, 292, 286, 300,
  -1, 299, 289, 290, 291, 302, 303, 305, 304, 301,
 297, 298, 257, 272, -12, 272,  -5,  -6, 272, 271,
 268, 259, 281,  -5, -13, -15, -16,  -5,  -5, 272,
  -4, 285,  -4,  -4,  -4, 284,  -5,  -5, 264, 264,
 258, 259, 260, 261,  -6,  -6,  -6, -14, -17,  -5,
 -14, -14, 264,  -2,  -5,  -3, 268, 259, 263, 262,
 273,  -2, 272, 264, 264, 264,  -5, 272,  -6,  -6,
  -6,  -6, 269, 264, 257,  -5, 268,  -3, 268,  -5,
  -5, 264,  -5,  -5,  -5, -17,  -5,  -3, 269,  -3,
 268,  -2, 264, 264, 269, 264, 258, 269,  -3,  -5,
  -5,  -3, 269, 264, 264, 285,  -3,  -5, 269, 285,
 269 };
short yydef[]={

   2,  -2,  -2,   0,   0,  30,   3,   4,   5,   0,
   8,   0,   0,  11,  13,  42,  43,  44,   0,   0,
   0,  21,  34,  34,  34,  34,   0,   0,   0,  29,
  35,  36,   7,   0,  10,  31,  12,  56,  57,  58,
   0,   0,   0,  14,  37,  37,  37,  18,  19,   0,
  22,  33,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,  64,  65,  15,  39,  40,
  16,  17,   0,  23,  45,  46,   0,   0,   0,   0,
  55,   0,  25,   0,   0,   0,   9,  32,  60,  61,
  62,  63,  59,   0,   0,  20,   0,   0,   0,   0,
  54,   0,   0,   0,  28,  38,  41,   0,  47,   0,
   0,  24,   0,   0,  50,   0,  49,  48,   0,   0,
  27,   0,  51,   0,   0,   0,   0,  26,  52,   0,
  53 };
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
# line 71 "mas0.y"
 {
		curlen = NBPW/2;
		dotp->xvalue &= ~01;
		flushfield(NBPW/2);
	} break;
case 2:
# line 77 "mas0.y"
 {
		goto reset;
	} break;
case 3:
# line 80 "mas0.y"
 {
		lineno++;
		goto reset;
	} break;
case 4:
# line 84 "mas0.y"
 {
		goto reset;
	} break;
case 5:
# line 87 "mas0.y"
 {
		lineno++;
		yyerrok;
	reset:
		ap = arglist;
		xp = explist;
		usrname = 0;
	} break;
case 7:
# line 98 "mas0.y"
 {
		flushfield(NBPW/4);
		if ((yypvt[-1].yname->n_type&XTYPE)!=N_UNDF) {
			if((yypvt[-1].yname->n_type&XTYPE)!=dotp->xtype || yypvt[-1].yname->n_value!=dotp->xvalue
			 || passno==1 && yypvt[-1].yname->index != dotp->xloc) {
				yyerror("%.8s redefined", yypvt[-1].yname->n_name);
			}
		}
		yypvt[-1].yname->n_type &= ~(XTYPE|XFORW);
		yypvt[-1].yname->n_type |= dotp->xtype;
		yypvt[-1].yname->n_value = dotp->xvalue;
		if (passno==1)
			yypvt[-1].yname->index = dotp-usedot;
	} break;
case 9:
# line 118 "mas0.y"
 {
		yypvt[-2].yname->n_type &= (N_EXT|XFORW);
		/*  what's this for?
		if($2->n_type&N_EXT && ($2->n_type&XTYPE)==N_UNDF && passno==2)
			$2->n_type &= ~N_EXT;
		*/
		yypvt[-2].yname->n_type |= yypvt[-0].yexp->xtype&(XTYPE|XFORW);
		yypvt[-2].yname->n_value = yypvt[-0].yexp->xvalue;
		if (passno==1)
			yypvt[-2].yname->index = yypvt[-0].yexp->xloc;
		else if ((yypvt[-0].yexp->xtype&XTYPE)==N_UNDF)
			yyerror("Illegal set");
	} break;
case 11:
# line 132 "mas0.y"
 {
		i = -IDATA;
		goto chloc;
	} break;
case 12:
# line 136 "mas0.y"
 {
		i = IDATA;
		goto chloc;
	} break;
case 13:
# line 140 "mas0.y"
 {
		i = -ITEXT;
		goto chloc;
	} break;
case 14:
# line 144 "mas0.y"
 {
		i = ITEXT;
	chloc:
		if (i < 0) {
			ii = 0;
			i = -i;
		} else {
			if (yypvt[-0].yexp->xtype != N_ABS || (ii=yypvt[-0].yexp->xvalue) >= NLOC) {
				yyerror("illegal location counter");
				ii = 0;
			}
		}
		if (i == IDATA)
			ii += NLOC;
		flushfield(NBPW/2);
		dotp = &usedot[ii];
		if (passno==2) {
			if (usefile[ii] == NULL) {
				tmpn2[TMPC] = 'a'+ii;
				if ((usefile[ii]=fopen(tmpn2, "w"))==NULL) {
					yyerror("cannot create temp");
					delexit();
				}
				tmpn3[TMPC] = 'a'+ii;
				if ((rusefile[ii]=fopen(tmpn3, "w"))==NULL) {
					yyerror("cannot create temp");
					delexit();
				}
			}
			txtfil = usefile[ii];
			relfil = rusefile[ii];
		}
	} break;
case 15:
# line 177 "mas0.y"
 {
		flushfield(NBPW/4);
		if (bitoff)
			dotp->xvalue++;
	} break;
case 16:
# line 182 "mas0.y"
 {
		flushfield(NBPW);
	} break;
case 17:
# line 185 "mas0.y"
 {
		flushfield(NBPW/2);
	} break;
case 18:
# line 188 "mas0.y"
 {
		if (yypvt[-0].yexp->xtype != N_ABS)
			yyerror("space size not absolute");
		li = yypvt[-0].yexp->xvalue;
	ospace:
		flushfield(NBPW/4);
		if (dotp->xvalue&01) {
			dotp->xvalue--;
			li--;
			curlen = NBPW/2;
			flushfield(NBPW/2);
		}
		while (li>1) {
			li -= 2;
			dotp->xvalue += 2;
			outhw(0, N_ABS, SNULL, 0);
		}
		if (li>0) {
			bitfield = 0;
			bitoff= NBPW/4;
			dotp->xvalue++;
		}
	} break;
case 19:
# line 211 "mas0.y"
 {
		if (yypvt[-0].yexp->xtype==N_ABS)
			orgwarn++;
		else if (yypvt[-0].yexp->xtype!=dotp->xtype)
			yyerror("Illegal 'org'");
		li = yypvt[-0].yexp->xvalue - dotp->xvalue;
		if (li < 0)
			yyerror("Backwards 'org'");
		goto ospace;
	} break;
case 20:
# line 221 "mas0.y"
 {
		if (yypvt[-0].yexp->xtype != N_ABS)
			yyerror("comm size not absolute");
		if (passno==1 && (yypvt[-2].yname->n_type&XTYPE)!=N_UNDF)
			yyerror("Redefinition of %.8s", yypvt[-2].yname->n_name);
		if (passno==1) {
			yypvt[-2].yname->n_value = yypvt[-0].yexp->xvalue;
			if (yypvt[-3].yint==ICOMM)
				yypvt[-2].yname->n_type |= N_EXT;
			else {
				yypvt[-2].yname->n_type &= ~XTYPE;
				yypvt[-2].yname->n_type |= N_BSS;
			}
		}
	} break;
case 21:
# line 236 "mas0.y"
 {
		flushfield(NBPW/2);
	} break;
case 22:
# line 239 "mas0.y"
 {
		insout(yypvt[-1].yname, (struct arg *)NULL, (struct arg *)NULL, yypvt[-0].yint);
	} break;
case 23:
# line 242 "mas0.y"
 {
		insout(yypvt[-2].yname, yypvt[-0].yarg, (struct arg *)NULL, yypvt[-1].yint);
	} break;
case 24:
# line 245 "mas0.y"
 {
		insout(yypvt[-4].yname, yypvt[-2].yarg, yypvt[-0].yarg, yypvt[-3].yint);
	} break;
case 25:
# line 248 "mas0.y"
 {
		li = yypvt[-0].yname->n_value - dotp->xvalue;
		if (yypvt[-1].yint==0)
			yypvt[-1].yint = W;
		if (passno == 2 && (dotp->xtype != yypvt[-0].yname->n_type
		 || (yypvt[-1].yint==W && (li < -65536 || li > 65535))))
			yyerror("Illegal pcrel value");
		if (yypvt[-1].yint==B)
			yyerror("Illegal size");
		if (yypvt[-1].yint==L) {
			outhw((short)(li>>16), yypvt[-0].yname->n_type, yypvt[-0].yname, XPCREL+X2WDS);
			outhw((short)li, N_ABS, SNULL, 0);
		} else
			outhw((short)li, yypvt[-0].yname->n_type, yypvt[-0].yname, XPCREL);
		dotp->xvalue += 2;
	} break;
case 26:
# line 264 "mas0.y"
 {
		nstabs++;
		if (passno==2) {
			strncpy(stabsym.n_name, yypvt[-6].ystring, NCPS);
			stabsym.n_dtype = ckabs(yypvt[-4].yexp);
			stabsym.n_desc = ckabs(yypvt[-2].yexp);
			ckrel(&stabsym, yypvt[-0].yexp);
			goto writestab;
		}
	} break;
case 27:
# line 274 "mas0.y"
 {
		nstabs++;
		if (passno==2) {
			strncpy(stabsym.n_name, "", NCPS);
			stabsym.n_dtype = ckabs(yypvt[-4].yexp);
			stabsym.n_desc = ckabs(yypvt[-2].yexp);
			ckrel(&stabsym, yypvt[-0].yexp);
			goto writestab;
		}
	} break;
case 28:
# line 284 "mas0.y"
 {
		nstabs++;
		if (passno==2) {
			strncpy(stabsym.n_name, "", NCPS);
			stabsym.n_dtype = ckabs(yypvt[-2].yexp);
			stabsym.n_desc = ckabs(yypvt[-0].yexp);
			ckrel(&stabsym, dotp);
		writestab:
			fwrite((char *)&stabsym, sizeof(stabsym), 1, stabfil);
		}
	} break;
case 31:
# line 299 "mas0.y"
 {
		yypvt[-0].yname->n_type |= N_EXT;
		} break;
case 32:
# line 302 "mas0.y"

		yypvt[-0].yname->n_type |= N_EXT; break;
case 33:
# line 305 "mas0.y"
 {
		yyval.yint = yypvt[-0].yint;
	} break;
case 34:
# line 308 "mas0.y"
 {
		yyval.yint = 0;
	} break;
case 35:
# line 313 "mas0.y"
 {
		yyval.yint = ICOMM;
	} break;
case 36:
# line 316 "mas0.y"
 {
		yyval.yint = ILCOMM;
	} break;
case 40:
# line 325 "mas0.y"
 {
		i = curlen;
		pval = yypvt[-0].yexp;
		flushfield(curlen);
		goto outx;
	} break;
case 41:
# line 331 "mas0.y"
 {
		if (yypvt[-2].yexp->xtype != N_ABS)
			yyerror("Width expression not absolute");
		i = yypvt[-2].yexp->xvalue;
		pval = yypvt[-0].yexp;
		if (bitoff+i > curlen)
			flushfield(curlen);
		if (i > curlen)
			yyerror("Expression crosses field boundary");
	outx:
		 if ((pval->xtype&XTYPE)!=N_ABS && passno==2) {
			if (curlen==NBPW/2 && bitoff==0) {
				outhw((short)pval->xvalue, pval->xtype, pval->xname, 0);
				dotp->xvalue += 2;
			} else if (curlen!=NBPW || bitoff) {
				yyerror("Illegal relocation in field");
				bitoff += i;
			} else {
				output(pval->xvalue, pval->xtype, pval->xname);
				dotp->xvalue += 4;
			}
		} else {
			if (i<NBPW) {
				li = pval->xvalue & ((1L<<i)-1);
				bitoff += i;
				ii = NBPW-bitoff;
				bitfield |= li << ii;
			} else {
				bitfield = pval->xvalue;
				bitoff = NBPW;
			}
		}
		ap = arglist;
		xp = explist;
	} break;
case 42:
# line 367 "mas0.y"
 {
		curlen = NBPW/4;
		dotp->xvalue &= ~01;
	} break;
case 43:
# line 372 "mas0.y"
 {
		curlen = NBPW;
		align(FW);
	} break;
case 44:
# line 377 "mas0.y"
 {
		curlen = NBPW/2;
		align(HW);
	} break;
case 45:
# line 382 "mas0.y"
 {
		ap->atype = AEXP;
		ap->xp = yypvt[-0].yexp;
		ap->areg1 = 0;
		ap->areg2 = 0;
		yyval.yarg = ap++;
	} break;
case 46:
# line 389 "mas0.y"
 {
		ap->atype = AREG;
		ap->areg1 = yypvt[-0].yint;
		ap->areg2 = 0;
		yyval.yarg = ap++;
	} break;
case 47:
# line 395 "mas0.y"
 {
		ap->atype = AIREG;
		ap->areg1 = yypvt[-1].yint;
		ap->areg2 = 0;
		yyval.yarg = ap++;
	} break;
case 48:
# line 401 "mas0.y"
 {
		ap->atype = ADEC;
		ap->areg1 = yypvt[-1].yint;
		ap->areg2 = 0;
		yyval.yarg = ap++;
	} break;
case 49:
# line 407 "mas0.y"
 {
		ap->atype = AINC;
		ap->areg1 = yypvt[-2].yint;
		ap->areg2 = 0;
		yyval.yarg = ap++;
	} break;
case 50:
# line 413 "mas0.y"
 {
		ap->atype = AOFF;
		ap->xp = yypvt[-3].yexp;
		ap->areg1 = yypvt[-1].yint;
		ap->areg2 = 0;
		yyval.yarg = ap++;
	} break;
case 51:
# line 420 "mas0.y"
 {
		ap->atype = APIC;
		ap->xp = yypvt[-3].yexp;
		ap->areg1 = yypvt[-1].yint;
		ap->areg2 = 0;
		yyval.yarg = ap++;
	} break;
case 52:
# line 427 "mas0.y"
 {
		ap->atype = ANDX;
		ap->xp = yypvt[-6].yexp;
		ap->areg1 = yypvt[-4].yint;
		ap->areg2 = yypvt[-2].yint;
		ap->asize = yypvt[-1].yint;
		yyval.yarg = ap++;
	} break;
case 53:
# line 435 "mas0.y"
 {
		ap->atype = API2;
		ap->xp = yypvt[-6].yexp;
		ap->areg1 = yypvt[-4].yint;
		ap->areg2 = yypvt[-2].yint;
		ap->asize = yypvt[-1].yint;
		yyval.yarg = ap++;
	} break;
case 54:
# line 443 "mas0.y"
 {
		ap->atype = AIMM;
		ap->xp = yypvt[-0].yexp;
		ap->areg1 = 0;
		ap->areg2 = 0;
		yyval.yarg = ap++;
	} break;
case 55:
# line 453 "mas0.y"
 {
		if ( (yypvt[-0].yname->n_type&XTYPE)!=N_ABS || yypvt[-0].yname->n_value<0 || yypvt[-0].yname->n_value>SRREG)
			if (passno!=1)
				yyerror("Illegal register");
		yyval.yint = yypvt[-0].yname->n_value;
	} break;
case 57:
# line 464 "mas0.y"
 {
		yyval.yexp = xp++;
		yyval.yexp->xtype = yypvt[-0].yname->n_type;
		if ((yypvt[-0].yname->n_type&XTYPE)==N_UNDF) {
			yyval.yexp->xname = yypvt[-0].yname;
			yyval.yexp->xvalue = 0;
			if (passno==1)
				yypvt[-0].yname->n_type |= XFORW;
		} else {
			yyval.yexp->xvalue = yypvt[-0].yname->n_value;
			yyval.yexp->xname = NULL;
			yyval.yexp->xloc = yypvt[-0].yname->index;
		}
	} break;
case 58:
# line 478 "mas0.y"
 {
		yyval.yexp = xp++;
		yyval.yexp->xtype = N_ABS;
		yyval.yexp->xvalue = * (long *)yypvt[-0].ynumber;
		yyval.yexp->xloc = 0;
	} break;
case 59:
# line 484 "mas0.y"
 {
		yyval.yexp = yypvt[-1].yexp;
	} break;
case 60:
# line 487 "mas0.y"
 {
		goto comb;
	} break;
case 61:
# line 490 "mas0.y"
 {
		goto comb;
	} break;
case 62:
# line 493 "mas0.y"
 {
		goto comb;
	} break;
case 63:
# line 496 "mas0.y"
 {
	comb:
		yyval.yexp = combine(yypvt[-1].yint, yypvt[-2].yexp, yypvt[-0].yexp);
	} break;
case 64:
# line 500 "mas0.y"
 {
		xp->xtype = N_ABS;
		xp->xvalue = 0;
		yyval.yexp = combine(yypvt[-1].yint, xp++, yypvt[-0].yexp);
	} break;
case 65:
# line 505 "mas0.y"
{
		yyval.yexp = yypvt[-0].yexp;
		yyval.yexp->xvalue = ~yyval.yexp->xvalue;
	} break;
	}
	goto yystack;  /* stack new state and value */
}

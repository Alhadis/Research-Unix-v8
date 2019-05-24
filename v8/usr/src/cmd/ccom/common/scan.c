/* @(#) scan.c: 1.2 2/27/84				*/

# include "mfile1.h"
# include <ctype.h>
/* temporarily */

/* character-set translation */
# ifndef CCTRANS
# define CCTRANS(x) (x)
# endif

int flflag = 0;
# ifndef FLOATCVT
double atof();
# define FLOATCVT(x) atof(x)
# endif

# ifndef ASMBUF
# define ASMBUF 200
# endif
char asmbuf[ASMBUF];
char *asmp;
int asm_esc = 0; /* asm escaped used in file */
/* lexical actions */

# define A_ERR 0		/* illegal character */
# define A_LET 1		/* saw a letter */
# define A_DIG 2		/* saw a digit */
# define A_1C 3			/* return a single character */
# define A_STR 4		/* string */
# define A_CC 5			/* character constant */
# define A_BCD 6		/* GCOS BCD constant */
# define A_SL 7			/* saw a / */
# define A_DOT 8		/* saw a . */
# define A_PL 9		/* + */
# define A_MI 10		/* - */
# define A_EQ 11		/* = */
# define A_NOT 12		/* ! */
# define A_LT 13		/* < */
# define A_GT 14		/* > */
# define A_AND 16		/* & */
# define A_OR 17		/* | */
# define A_WS 18		/* whitespace (not \n) */
# define A_NL 19		/* \n */

/* character classes */

# define LEXLET 01
# define LEXDIG 02
# define LEXOCT 04
# define LEXHEX 010
# define LEXWS 020
# define LEXDOT 040

/* reserved word actions */

# define AR_TY 0		/* type word */
# define AR_RW 1		/* simple reserved word */
# define AR_CL 2		/* storage class word */
# define AR_S 3		/* struct */
# define AR_U 4		/* union */
# define AR_E 5		/* enum */
# define AR_A 6		/* asm */

/* text buffer */
# define LXTSZ BUFSIZ		/* lots of room for FLEXNAMES */
char yytext[LXTSZ];
char * lxgcp;
/* If TRUNCATE is undefined, let -T determine whether symbols are truncated.
** Otherwise, forcibly truncate all names.
*/

static int truncate_flag =
#ifndef	TRUNCATE
0;		/* truncate based on -T (non-zero to truncate */
#else
1;		/* force all names <= NCHNAM chars */
#endif

int zflag = 0;


/* ARGSUSED */
mainp1( argc, argv )
int argc;
char *argv[];
{
	/* control multiple files */

	register i;
	register char *cp;
	extern int idebug, bdebug, tdebug, edebug, ddebug, xdebug, opdebug;
	extern int asdebug, nowarn, proflag, bothdebug, syncstdio, Pflag;
	extern unsigned int maxoffset;
	int fdef = 0;

	unsigned int temp = 1;
	i = 0;
	do {
		temp <<= 1;
		++i;
	} while ( temp != 0 );
	maxoffset = 1 << (i-1);

	for( i=1; i<argc; ++i ) {
		if( *(cp=argv[i]) == '-' && *++cp == 'X' ) {
			while( *++cp ) switch(++syncstdio, *cp) {
			case 'P': ++Pflag; break;
			case 'B':
				++bothdebug; break;
			case 'd':
				++ddebug; break;
			case 'i':
				++idebug; break;
			case 'b':
				++bdebug; break;
			case 't':
				++tdebug; break;
			case 'e':
				++edebug; break;
			case 'x':
				++xdebug; break;
			case 'o':
				++opdebug; break;
			case 'p':
				++proflag; break;
			case 's':
				++asdebug; --syncstdio; break;
			case 'w':
				++nowarn; --syncstdio; break;
			case 'z':	/* stin file debug info */
				++zflag; break;
			case 'T':	/* truncate names */
				++truncate_flag; break;
			default:
				cerror( "bad option: X%c", *cp );
			}
		} else {
			if( *(argv[i]) != '-' ) switch( fdef++ ) {
			case 0:
				close(0);
				if (open(argv[i], 0) == 0) break;
				fprintf(stderr, "ccom: can't open %s\n", argv[i]);
				exit(1);

			case 1:
				if (freopen(argv[i], "w", stdout)) break;
				fprintf(stderr, "ccom: can't open %s\n", argv[i]);
				exit(1);

			default:
				;
			}
		}
	}
	if (isatty(1)) ++syncstdio;

	p2init( argc, argv );

	for( i=0; i<SYMTSZ; ++i ) stab[i].stype = TNULL;

	lxinit();
	tinit();
	mkdope();

	/* dimension table initialization */

	dimtab[NULL] = 0;
	dimtab[VOID] = 0;
	dimtab[CHAR] = SZCHAR;
	dimtab[INT] = SZINT;
	dimtab[FLOAT] = SZFLOAT;
	dimtab[DOUBLE] = SZDOUBLE;
	dimtab[LONG] = SZLONG;
	dimtab[SHORT] = SZSHORT;
	dimtab[UCHAR] = SZCHAR;
	dimtab[USHORT] = SZSHORT;
	dimtab[UNSIGNED] = SZINT;
	dimtab[ULONG] = SZLONG;
	/* starts past any of the above */
	curdim = 16;
	reached = 1;

	yyparse();
	yyaccpt();

# ifdef GDEBUG
	ejsdb();
# endif
# ifdef ENDJOB
	ENDJOB(nerrors?1:0);
# endif
	return(nerrors?1:0);

}

# ifdef ibm

# define CSMASK 0377
# define CSSZ 256

# else

# define CSMASK 0177
# define CSSZ 128

# endif

/*
*	We buffer the input on a line by line basis
*	so that we can output the ``C'' source as comments
*	in the assembly language
*/
int	asdebug = 0;
static	char	linebuf[BUFSIZ+1];
static	char	*linecp = &linebuf[0], *endcp = &linebuf[0];
#undef	ungetc
#undef	getchar

#define	getchar()	(*linecp ? *linecp++ : eatline() )
#define	ungetchar()	--linecp
#define	ungetc(c)	*--linecp=(c)

eatline()
{
	static int savechar; char *index(), *memcpy();
	register n; register char *cp;
	if (asdebug) for (*linecp = savechar, n = 0;;) {
		if ((cp = index(linecp, '\n')) || (cp = endcp-1, n)) {
			savechar = *++cp;
			*cp = '\0';
			spitline(linecp);
			return *linecp++;
		}
		linecp = memcpy(linebuf, linecp, n = endcp - linecp);
		if ((n = read(0, endcp = linebuf + n, BUFSIZ - n)) <= 0) {
			*(endcp = linebuf) = '\0';
			return EOF;
		}
		*(endcp += n) = '\0';
	} else {
		if ((n = read(0, linebuf, BUFSIZ)) <= 0) {
			(linecp = linebuf)[0] = 0;
			return EOF;
		} else {
			(linecp = linebuf)[n] = 0;
			return *linecp++;
		}
	}
}

/*
** add the original C line to the assembler output as comment,
** unless it is empty.
*/
#define	MARKERCNT	20	/* how often to spit out line markers */
spitline(l)
register char *l;
{
	extern int lineno;

	if( !(lineno%MARKERCNT) )
		printx( "%s -- line %d %s\n", COMMENTSTR, lineno, ftitle );
				/* line marker */
	while( isspace( *l ) )
		l++;
	if( *l )
		printx( "%s %s", COMMENTSTR, l );
}

short lxmask[CSSZ+1];

lxenter( s, m )
register char *s;
register short m;
{
	/* enter a mask into lxmask */
	register c;

	while( c= *s++ ) lxmask[c+1] |= m;

}


# define lxget(c,m) (lxgcp=yytext,lxmore(c,m))

lxmore( c, m )
register c, m;
{
	register char *cp;

	*(cp = lxgcp) = c;
	while ((c = getchar()), lxmask[c+1]&m) {
		if( cp < &yytext[LXTSZ-1])
			*++cp = c;
	}
	ungetchar();
	*(lxgcp = cp+1) = '\0';
}

struct lxdope
{
	short lxch;	/* the character */
	short lxact;	/* the action to be performed */
	short lxtok;	/* the token number to be returned */
	short lxval;	/* the value to be returned */
} lxdope[] =
{
	'$',	A_ERR,	0,	0,	/* illegal characters go here... */
	'_',	A_LET,	0,	0,	/* letters point here */
	'0',	A_DIG,	0,	0,	/* digits point here */
	' ',	A_WS,	0,	0,	/* whitespace goes here */
	'\n',	A_NL,	0,	0,
	'"',	A_STR,	0,	0,	/* character string */
	'\'',	A_CC,	0,	0,	/* character constant */
	'`',	A_BCD,	0,	0,	/* GCOS BCD constant */
	'(',	A_1C,	LP,	0,
	')',	A_1C,	RP,	0,
	'{',	A_1C,	LC,	0,
	'}',	A_1C,	RC,	0,
	'[',	A_1C,	LB,	0,
	']',	A_1C,	RB,	0,
	'*',	A_1C,	MUL,	MUL,
	'?',	A_1C,	QUEST,	0,
	':',	A_1C,	COLON,	0,
	'+',	A_PL,	PLUS,	PLUS,
	'-',	A_MI,	MINUS,	MINUS,
	'/',	A_SL,	DIVOP,	DIV,
	'%',	A_1C,	DIVOP,	MOD,
	'&',	A_AND,	AND,	AND,
	'|',	A_OR,	OR,	OR,
	'^',	A_1C,	ER,	ER,
	'!',	A_NOT,	UNOP,	NOT,
	'~',	A_1C,	UNOP,	COMPL,
	',',	A_1C,	CM,	CM,
	';',	A_1C,	SM,	0,
	'.',	A_DOT,	STROP,	DOT,
	'<',	A_LT,	RELOP,	LT,
	'>',	A_GT,	RELOP,	GT,
	'=',	A_EQ,	ASSIGN,	ASSIGN,
	-1,	A_1C,	0,	0,
};

struct lxdope *lxcp[CSSZ+1];

lxinit()
{
	register struct lxdope *p;
	register i;
	register char *cp;
	/* set up character classes */

	lxenter( "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_", LEXLET );
	lxenter( "0123456789", LEXDIG );
	lxenter( "0123456789abcdefABCDEF", LEXHEX );
	/* \013 should become \v someday; \013 is OK for ASCII and EBCDIC */
	lxenter( " \t\r\b\f\013", LEXWS );
	lxenter( "01234567", LEXOCT );
	lxmask['.'+1] |= LEXDOT;
	lxhinit();

	/* make lxcp point to appropriate lxdope entry for each character */

	/* initialize error entries */

	for( i= 0; i<=CSSZ; ++i ) lxcp[i] = lxdope;

	/* make unique entries */

	for( p=lxdope; ; ++p )
	{
		lxcp[p->lxch+1] = p;
		if( p->lxch < 0 ) break;
	}

	/* handle letters, digits, and whitespace */
	/* by convention, first, second, and third places */

	cp = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	while( *cp ) lxcp[*cp++ + 1] = &lxdope[1];
	cp = "123456789";
	while( *cp ) lxcp[*cp++ + 1] = &lxdope[2];
	cp = "\t\b\r\f\013";
	while( *cp ) lxcp[*cp++ + 1] = &lxdope[3];

	/* first line might have title */
	lineno = 1;
	lxtitle();

}

int lxmatch;  /* character to be matched in char or string constant */

lxstr(ct)
register ct;
{
	/* match a string or character constant, up to lxmatch */

	register c, val, i = 0;

	while ((c = getchar()) != lxmatch) {
		switch( c ) {

		case EOF:
			uerror( "lxstr() unexpected EOF" );
			break;

		case '\n':
			uerror( "newline in string or char constant" );
			++lineno;
			break;

		case '\\':
			switch (c = getchar()) {
			case '\n':
				++lineno; continue;
			default:
				val = c; goto mkcc;
			case 'n':
				val = '\n'; goto mkcc;
			case 'r':
				val = '\r'; goto mkcc;
			case 'b':
				val = '\b'; goto mkcc;
			case 't':
				val = '\t'; goto mkcc;
			case 'f':
				val = '\f'; goto mkcc;
			case 'v':
				val = '\013'; goto mkcc;
			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
				val = c-'0';
				c = getchar();  /* try for 2 */
				if (lxmask[c+1] & LEXOCT) {
					val = (val<<3) | (c-'0');
					c = getchar();  /* try for 3 */
					if (lxmask[c+1] & LEXOCT)
						val = (val<<3) | (c-'0');
					else
						ungetchar();
				} else
					ungetchar();
				goto mkcc1;
			}
		default:
			val =c;
mkcc:
			val = CCTRANS(val);
mkcc1:
			if (lxmatch == '\'') {
				/* it is, after all, a "character" constant */
				val = ccast((CONSZ)val,CHAR);
				if (i == 0)
					lastcon = val;
				else {
# ifdef MAKECC
					MAKECC( val, i );
# else
# ifdef RTOLBYTES
					lastcon <<= SZCHAR;
					lastcon |= (val&BITMASK(SZCHAR));
# else
					lastcon &= BITMASK(SZCHAR*i);
					lastcon |= val << (SZCHAR*i);
# endif
# endif
				}
			} else {
				 /* stash the byte into the string */
				if (strflg) {
					if (ct==0 || i<ct)
						putbyte( val );
					else
						if( i == ct )
							werror(
				"non-null byte ignored in string initializer" );
				} else
					bycode( val, i );
			}
			++i;
			continue;
		}
		break;
	}
	/* end of string or char constant */

	if (lxmatch == '"') {
		if (strflg) {	/* end the string */
			if (ct==0 || i<ct)
				putbyte( 0 );  /* the null at the end */
		} else {	/* the initializer gets a null byte */
			bycode( 0, i++ );
			bycode( -1, i );
			dimtab[curdim] = i;  /* in case of later sizeof ... */
		}
	} else {	/* end the character constant */
		if (i == 0)
			uerror( "empty character constant" );
		if (i>(SZINT/SZCHAR) || ((pflag||hflag)&&i>1))
			uerror( "too many characters in character constant" );
	}
}

lxcom()
{
	register c;
	/* saw a /*: process a comment */

	for (;;) {
		switch (c = getchar()) {

		case EOF:
			uerror( "lxcom() unexpected EOF" );
			return;

		case '\n':
			++lineno;

		default:
			continue;

		case '*':
			if( (c = getchar()) == '/' ) return;
			else ungetchar();
			continue;

		}
	}
}

#define Return(e)	return(yylval.lineno = lineno, (e))

yylex()
{
	for(;;)
	{

		register lxchar;
		register struct lxdope *p;
		register struct symtab *sp;
		register char *cp;
		register id;

		switch( (p = lxcp[(lxchar = getchar())+1])->lxact )
		{

onechar:
			ungetc( lxchar );

		case A_1C:
			/* eat up a single character, and return an opcode */

			yylval.intval = p->lxval;
			Return( p->lxtok );

		case A_ERR:
			uerror( "illegal character: %03o (octal)", lxchar );
			break;

		case A_LET:
			/* collect an identifier, and check for reserved word */
			lxget( lxchar, LEXLET|LEXDIG );

			/* 0 means some kind of low level error, >0 reserved,
			** <0 means id
			*/
			if( (lxchar=lxres()) > 0 ) Return( lxchar );
			if( lxchar== 0 ) continue;

			if (truncate_flag )
				yytext[NCHNAM] = '\0';	/* truncate name */
			id = lookup( hash(yytext),
					/* tag name for struct/union/enum */
				(stwart&TAGNAME)? STAG:
					/* member name for struct/union */
				(stwart&(INSTRUCT|INUNION|FUNNYNAME))?SMOS:0 );
			sp = &stab[id];
			if( sp->sclass == TYPEDEF && !stwart )
			{
				stwart = instruct;
				yylval.nodep = mkty( sp->stype, sp->dimoff,
				sp->sizoff );
				Return( TYPE );
			}
			stwart = (stwart&SEENAME) ? instruct : 0;
			yylval.intval = id;
			Return( NAME );

		case A_DIG:
			/* collect a digit string, then look at last one... */
			lastcon = 0;
			lxget( lxchar, LEXDIG );
			switch( lxchar = getchar() )
			{

			case 'x':
			case 'X':
				if( yytext[0] != '0' && !yytext[1] )
					uerror( "illegal hex constant" );
				lxmore( lxchar, LEXHEX );
				/* convert the value */
				{
					for( cp = yytext+2; *cp; ++cp )
					{
						/* this code won't work for all character sets,
						** but seems ok for ascii and ebcdic
						*/
						lastcon <<= 4;
						if( isdigit( *cp ) )
							lastcon += *cp-'0';
						else if( isupper( *cp ) )
							lastcon += *cp - 'A'+ 10;
						else
							lastcon += *cp - 'a'+ 10;
					}
				}

hexlong:
				/* hex and octal constants are long if they
				** fit within 0177777
				*/
				if( lastcon & ~0177777L ) yylval.intval = 1;
				else yylval.intval = 0;

				goto islong;

			case '.':
				lxmore( lxchar, LEXDIG );

getfp:
				if( (lxchar = getchar()) == 'e' || lxchar == 'E' )
				{
					 /* exponent */
				case 'e':
				case 'E':
					if( (lxchar = getchar()) == '+' ||
					    lxchar == '-' )
					{
						*lxgcp++ = 'e';
					}
					else
					{
						ungetc(lxchar);
						lxchar = 'e';
					}
					lxmore( lxchar, LEXDIG );
					/* now have the whole thing... */
				}
				else
				{
					  /* no exponent */
					ungetc( lxchar );
				}
# ifdef NOFLOAT
				if( !flflag )
				{
					flflag = 1;
					werror( "sorry, no floating point" );
				}
				for( cp = yytext; *cp; ++cp )
				{
					if( *cp == 'e' || *cp == 'E' ||
					    *cp == '.' )
					{
						*cp = '\0';
						break;
					}
				}
				goto conv10;
# else
				flflag = 1;
				dcon = FLOATCVT(yytext);
				Return( FCON );
# endif

			default:
				ungetc( lxchar );
				if( yytext[0] == '0' )
				{
					/* convert in octal */
					for( cp = yytext+1; *cp; ++cp )
					{
						lastcon <<= 3;
						lastcon += *cp - '0';
					}
					goto hexlong;
				}
				else
				{
					/* convert in decimal */
# ifdef NOFLOAT
conv10:
# endif
					for( cp = yytext; *cp; ++cp )
					{
						lastcon = lastcon*10 + *cp - '0';
					}
				}

				/* decide if it is long or not (decimal case) */

				/* if it is positive and fits in 15 bits,
				** or negative and and fits in 15 bits
				** with an extended sign, it is int;
				** otherwise long
				*/
				/* if there is an l or L following,
				** all bets are off...
				*/

				{
					register CONSZ v;
					v = lastcon & ~077777L;
					if( v == 0 || v == ~077777L )
						yylval.intval = 0;
					else yylval.intval = 1;
				}

islong:
				/* finally, look for trailing L or l */
				if( (lxchar=getchar()) == 'L' || lxchar == 'l' )
						yylval.intval = 1;
				else ungetc( lxchar );
				Return( ICON );
			}

		case A_DOT:
			/* a dot: if followed by a digit, floating point */
			lxchar = getchar();
			if( lxmask[lxchar+1] & LEXDIG )
			{
				ungetc(lxchar);
				lxget( '.', LEXDIG );
				goto getfp;
			}
			stwart = FUNNYNAME;
			goto onechar;

		case A_STR:
			/* string constant */
			lxmatch = '"';
			Return( STRING );

		case A_CC:
			/* character constant */
			lxmatch = '\'';
			lastcon = 0;
			lxstr(0);
			yylval.intval = 0;
			Return( ICON );

		case A_SL:
			/* / */
			if( (lxchar = getchar()) != '*' ) goto onechar;
			lxcom();
		case A_WS:
			continue;

		case A_NL:
			++lineno;
			lxtitle();
			continue;

		case A_NOT:
			/* ! */
			if( (lxchar = getchar()) != '=' ) goto onechar;
			yylval.intval = NE;
			Return( EQUOP );

		case A_MI:
			/* - */
			if( (lxchar = getchar()) == '-' )
			{
				yylval.intval = DECR;
				Return( INCOP );
			}
			if( lxchar != '>' ) goto onechar;
			stwart = FUNNYNAME;
			yylval.intval=STREF;
			Return( STROP );

		case A_PL:
			/* + */
			if( (lxchar = getchar()) != '+' ) goto onechar;
			yylval.intval = INCR;
			Return( INCOP );

		case A_AND:
			/* & */
			if( (lxchar = getchar()) != '&' ) goto onechar;
			Return( yylval.intval = ANDAND );

		case A_OR:
			/* | */
			if( (lxchar = getchar()) != '|' ) goto onechar;
			Return( yylval.intval = OROR );

		case A_LT:
			/* < */
			if( (lxchar = getchar()) == '<' )
			{
				yylval.intval = LS;
				Return( SHIFTOP );
			}
			if( lxchar != '=' ) goto onechar;
			yylval.intval = LE;
			Return( RELOP );

		case A_GT:
			/* > */
			if( (lxchar = getchar()) == '>' )
			{
				yylval.intval = RS;
				Return(SHIFTOP );
			}
			if( lxchar != '=' ) goto onechar;
			yylval.intval = GE;
			Return( RELOP );

		case A_EQ:
			/* = */
			switch( lxchar = getchar() )
			{

			case '=':
				yylval.intval = EQ;
				Return( EQUOP );

			default:
				goto onechar;

			}


		default:
			cerror( "yylex error, character %03o (octal)", lxchar );

		}

		/* ordinarily, repeat here... */
		cerror( "out of switch in yylex" );

	}

}

struct lxrdope
{
	/* dope for reserved, in alphabetical order */

	char *lxrch;	/* name of reserved word */
	short lxhash;
	short lxract;	/* reserved word action */
	short lxrval;	/* value to be returned */
} lxrdope[] =
{
	"asm",		0,	AR_A,	0,
	"auto",		0,	AR_CL,	AUTO,
	"break",	0,	AR_RW,	BREAK,
	"case",		0,	AR_RW,	CASE,
	"char",		0,	AR_TY,	CHAR,
	"continue",	0,	AR_RW,	CONTINUE,
	"default",	0,	AR_RW,	DEFAULT,
	"do",		0,	AR_RW,	DO,
	"double",	0,	AR_TY,	DOUBLE,
	"else",		0,	AR_RW,	ELSE,
	"enum",		0,	AR_E,	ENUM,
	"extern",	0,	AR_CL,	EXTERN,
#ifdef ONEFLOAT
	"float",	0,	AR_TY,	DOUBLE,
#else
	"float",	0,	AR_TY,	FLOAT,
#endif
	"for",		0,	AR_RW,	FOR,
	"fortran",	0,	AR_CL,	FORTRAN,
	"goto",		0,	AR_RW,	GOTO,
	"if",		0,	AR_RW,	IF,
	"int",		0,	AR_TY,	INT,
	"long",		0,	AR_TY,	LONG,
	"register",	0,	AR_CL,	REGISTER,
	"return",	0,	AR_RW,	RETURN,
	"short",	0,	AR_TY,	SHORT,
	"sizeof",	0,	AR_RW,	SIZEOF,
	"static",	0,	AR_CL,	STATIC,
	"struct",	0,	AR_S,	0,
	"switch",	0,	AR_RW,	SWITCH,
	"typedef",	0,	AR_CL,	TYPEDEF,
	"union",	0,	AR_U,	0,
	"unsigned",	0,	AR_TY,	UNSIGNED,
	"void",		0,	AR_TY,	VOID,
	"while",	0,	AR_RW,	WHILE,
	0,		0,	0,	0,
};

#define HASH(s)	(((lxrch0[(s)[0]&037]<<4) | lxrch2[(s)[2]&037]) & 0377)

unsigned char lxrch0[040], lxrch2[040], lxrindx[0400];

lxhinit()
{
	register struct lxrdope *lp; struct lxrdope dptemp;
	register char *s; register j;
	int k0, k2;
	for (lp = lxrdope; s = lp->lxrch; ++lp) {
		++lxrch0[s[0] & 037];
		++lxrch2[s[2] & 037];
	}
	for (k0=k2=j=0; j<040; j++) {
		if (lxrch0[j]) lxrch0[j] = ++k0;
		if (lxrch2[j]) lxrch2[j] = ++k2;
	}
	for (lp = lxrdope; s = lp->lxrch; ++lp)
		lp->lxhash = HASH(s);
	do {
		j = 0;
		for (lp = lxrdope+1; lp->lxrch; ++lp)
			if (lp->lxhash < (lp-1)->lxhash) {
				dptemp = *lp;
				*lp = *(lp-1);
				*(lp-1) = dptemp;
				++j;
			}
	} while (j);
	for (k0 = 1, lp = lxrdope; s = lp->lxrch; ++lp, ++k0)
		if (lxrindx[j = lp->lxhash] == 0)
			lxrindx[j] = k0;
/*
	for (lp = lxrdope; lp->lxrch; ++lp)
		printf("%d %d %s\n", lxrindx[lp->lxhash], lp->lxhash, lp->lxrch);
*/
}

lxres()
{
	/*
	** check to see of yytext is reserved; if so,
	** do the appropriate action and return
	** otherwise, return -1
	*/
	register c;
	register struct lxrdope *p;

	if (yytext[1] == 0) return -1;

	if ((c = HASH(yytext)) == 0 || (c = lxrindx[c]) == 0) return -1;

	p = lxrdope + c - 1;
	do if (strcmp(yytext, p->lxrch) == 0) switch (p->lxract) {

	case AR_TY:	/* type word */
		stwart = instruct;
		yylval.nodep = mkty( (TWORD)p->lxrval, 0, p->lxrval );
		if (p->lxrval==FLOAT || p->lxrval==DOUBLE) {
# ifdef NOFLOAT
			if (!flflag)
				werror( "sorry, no floating point" );
			yylval.nodep->tn.type = LONG;
# endif
			flflag = 1;
		}
		return( TYPE );

	case AR_RW:	/* ordinary reserved word */
		return( yylval.intval = p->lxrval );

	case AR_CL:	/* class word */
		yylval.intval = p->lxrval;
		return( CLASS );

	case AR_S:	/* struct */
		stwart = INSTRUCT|SEENAME|TAGNAME;
		yylval.intval = INSTRUCT;
		return( STRUCT );

	case AR_U:	/* union */
		stwart = INUNION|SEENAME|TAGNAME;
		yylval.intval = INUNION;
		return( STRUCT );

	case AR_E:	/* enums */
		stwart = SEENAME|TAGNAME;
		return( yylval.intval = ENUM );

	case AR_A:	/* asm */
		asm_esc = 1; /* warn the world! */
		lxget(' ', LEXWS);
		if (getchar() != '(') goto badasm;
		lxget(' ', LEXWS);
		if (getchar() != '"') goto badasm;
		asmp = asmbuf;
		while ((c = getchar()) != '"') {
			if (c=='\n' || c==EOF) goto badasm;
			*asmp++ = c;
			if (asmp >= &asmbuf[ASMBUF-1])
				uerror( "asm > %d chars", ASMBUF);
		}
		lxget(' ', LEXWS);
		if( getchar() != ')' ) goto badasm;
		*asmp++ = '\0';
		return( ASM );
badasm:
		uerror( "bad asm construction" );
		return( 0 );

	default:
		cerror("bad AR_?? action");
	} while ((++p)->lxhash == c);
	return -1;
}

lxtitle()
{
	/* called after a newline; set linenumber and file name */

	register c, val; register char *cp;

	for(;;) {	/* might be several such lines in a row */
		if ((c = getchar()) != '#') {
			if (c != EOF) ungetchar();
			return;
		}
		lxget( ' ', LEXWS);
		val = 0;
		for (c = getchar(); isdigit(c); c = getchar())
			val = val*10+ c - '0';
		ungetchar();
		lineno = val;
		lxget(' ', LEXWS);
		if ((c = getchar()) != '\n') {
			cp = ftitle; *cp++ = c;
			while ((c=getchar()) != '\n')
				*cp++ = c;
			*cp = '\0';
		}
	}
}

# ifndef MYASMOUT
asmout()
{
#ifdef GDEBUG
	dbline();
#endif
	printx("%s\n%s\n%s\n", ASM_COMMENT, asmbuf, ASM_END);
}
# endif

#include <stdio.h>
#include "mas.h"
#include "mas.yh"
#include <ctype.h>


/*
 * Tables for combination of operands.
 */

/* table for + */
char	pltab[6][6] = {
/*	UND	ABS	TXT	DAT	BSS	EXT */
/*UND*/	N_UNDF,	N_UNDF,	N_UNDF,	N_UNDF,	N_UNDF,	N_UNDF,
/*ABS*/	N_UNDF,	N_ABS,	N_TEXT,	N_DATA,	N_BSS,	N_EXT,
/*TXT*/	N_UNDF,	N_TEXT,	ERR,	ERR,	ERR,	ERR,
/*DAT*/	N_UNDF,	N_DATA,	ERR,	ERR,	ERR,	ERR,
/*BSS*/	N_UNDF,	N_BSS,	ERR,	ERR,	ERR,	ERR,
/*EXT*/	N_UNDF,	N_EXT,	ERR,	ERR,	ERR,	ERR,
};

/* table for - */
char	mintab[6][6] = {
/*	UND	ABS	TXT	DAT	BSS	EXT */
/*UND*/	N_UNDF,	N_UNDF,	N_UNDF,	N_UNDF,	N_UNDF,	N_UNDF,
/*ABS*/	N_UNDF,	N_ABS,	ERR,	ERR,	ERR,	ERR,
/*TXT*/	N_UNDF,	N_TEXT,	N_ABS,	ERR,	ERR,	ERR,
/*DAT*/	N_UNDF,	N_DATA,	ERR,	N_ABS,	ERR,	ERR,
/*BSS*/	N_UNDF,	N_BSS,	ERR,	ERR,	N_ABS,	ERR,
/*EXT*/	N_UNDF,	N_EXT,	ERR,	ERR,	ERR,	ERR,
};

/* table for other operators */
char	othtab[6][6] = {
/*	UND	ABS	TXT	DAT	BSS	EXT */
/*UND*/	N_UNDF,	N_UNDF,	N_UNDF,	N_UNDF,	N_UNDF,	N_UNDF,
/*ABS*/	N_UNDF,	N_ABS,	ERR,	ERR,	ERR,	ERR,
/*TXT*/	N_UNDF,	ERR,	ERR,	ERR,	ERR,	ERR,
/*DAT*/	N_UNDF,	ERR,	ERR,	ERR,	ERR,	ERR,
/*BSS*/	N_UNDF,	ERR,	ERR,	ERR,	ERR,	ERR,
/*EXT*/	N_UNDF,	ERR,	ERR,	ERR,	ERR,	ERR,
};

struct exp *
combine(op, r1, r2)
register struct exp *r1, *r2;
{
	register t1, t2, type;

	t1 = r1->xtype&XTYPE;
	t2 = r2->xtype&XTYPE;
	if (r1->xtype==N_EXT+N_UNDF)
		t1 = XTXRN;
	if (r2->xtype==N_EXT+N_UNDF)
		t2 = XTXRN;
	if (passno==1)
		if (r1->xloc!=r2->xloc && t1==t2 && t1!=N_ABS)
			t1 = t2 = XTXRN;	/* error on != loc ctrs */
	t1 >>= 1;
	t2 >>= 1;
	switch (op) {

	case PLUS:
		r1->xvalue += r2->xvalue;
		type = pltab[t1][t2];
		break;

	case MINUS:
		r1->xvalue -= r2->xvalue;
		type = mintab[t1][t2];
		break;

	case MUL:
		r1->xvalue *= r2->xvalue;
		goto comm;

	case DIV:
		if (r2->xvalue == 0)
			yyerror("Divide check");
		else
			r1->xvalue /= r2->xvalue;
		goto comm;

	comm:
		type = othtab[t1][t2];
		break;

	case EOR:
		r1->xvalue ^= r2->xvalue;
		goto comm;
	case IOR:
		r1->xvalue |= r2->xvalue;
		goto comm;
	default:
		yyerror("Internal error: unknown operator");
	}
	if (t2==(XTXRN>>1))
		r1->xname = r2->xname;
	r1->xtype = type | (r1->xtype|r2->xtype)&(XFORW|N_EXT);
	if (type==ERR)
		yyerror("Relocation error");
	return(r1);
}

int	type[] = {
	EOF,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	SPC,	NL,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	SPC,	EXCL,	DQ,	SH,	OFFOP,	REG,	AMP,	SQ,
	LP,	RP,	MUL,	PLUS,	CM,	MINUS,	SIZE,	DIV,
	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,
	DIGIT,	DIGIT,	COLON,	SEMI,	0,	0,	0,	0,
	0,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	LB,	0,	RB,	EOR,	ALPH,
	0,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	0,	IOR,	0,	ALPH,	0,
};

yylex()
{
	register short val;
	register base;
	register char *cp;
	struct nlist *op, **opp;
	int tildeflag;
	static long intval;
	register int n;

loop:
	switch(yylval.yint = type[(val = getchar())+1]) {

	case SH:
		while ((val = getchar()) != '\n' && val>0)
			;
		val = NL;
		goto ret;

	case SPC:
		goto loop;

	case EOF:
		return(0);

	case ALPH:
	case REG:
		tildeflag = 1;
		if (val=='~') {
			val = getchar();
			tildeflag = 2;
		}
		cp = yytext;
		do {
			if (cp <= &yytext[NCPS])
				*cp++ = val;
		} while (type[(val=getchar())+1]==ALPH||type[val+1]==DIGIT||type[val+1]==REG);
		*cp = '\0';
		while (val=='\t' || val==' ')
			val = getchar();
		ungetc(val, stdin);
		base = 1;
		if (val==':') {
			base = usrname;
			usrname = 1;
		}
		if ((op = *(opp=lookup(tildeflag)))->tag) {
			yylval.yname = op;
			val = op->tag+256;
			goto ret;
		} else {
			yylval.yname = op;
			if (tildeflag==2)
				*opp = 0;
			usrname = base;
			val = NAME;
			if (yytext[0]=='%')
				val = REG;
			goto ret;
		}

	case DIGIT:
		intval = val-'0';
		if (val=='0') {
			val = getchar();
			if (val=='x' || val=='X') {
				base = 16;
			} else {
				ungetc(val, stdin);
				base = 8;
			}
		} else
			base = 10;
		while (type[(val=getchar())+1]==DIGIT
		    || (base==16 && (val>='a' && val<='f'||val>='A' && val<='F'))) {
			if (base==8)
				intval <<= 3;
			else if (base==10)
				intval *= 10;
			else
				intval <<= 4;
			if (val>='a' && val<='f')
				val -= 'a' - 10 - '0';
			else if (val>='A' && val<='F')
				val -= 'A' - 10 - '0';
			intval += val-'0';
		}
		ungetc(val, stdin);
		val = INT;
		yylval.ynumber = &intval;
		goto ret;

	case SIZE:
		n = getchar();
		if (isupper(n))
			n = tolower(n);
		yylval.yint = 0;
		if (n=='b')
			yylval.yint = B;
		else if (n=='w')
			yylval.yint = W;
		else if (n=='l')
			yylval.yint = L;
		else
			yyerror("Unknown size specification");
		val = SIZE;
		goto ret;

	case SQ:
		if ((intval = getchar()) == '\n')
			lineno++;
		if(intval=='\\') switch(intval = getchar()) {
			case 'n':
				intval = '\n';
				break;
			case 'r':
				intval = '\r';
				break;
			case '\\':
			case '\'':
				break;
			case 'b':
				intval = '\b';
				break;
			case 't':
				intval = '\t';
				break;
			case 'v':
				intval = '\013';
				break;
			case 'f':
				intval = '\f';
				break;
			default:
				yyerror("Illegal escaped character");
		}
		val = INT;
		yylval.ynumber = &intval;
		goto ret;

	case DQ:
		cp = yystring;
		while ((val = getchar()) != '"' && val!='\n' && val!=EOF) {
			if (cp < &yystring[NCPS])
				*cp++ = val;
		}
		if (cp < &yystring[NCPS])
			*cp = '\0';
		if (val!='"') {
			yyerror("Nonterminated string");
			val = NL;
		} else
			val = STRING;
		yylval.ystring = yystring;
		goto ret;

	case 0:
		yyerror("Illegal character");
		val = NOCHAR;
		goto ret;

	default:
		val = yylval.yint;
		goto ret;
	}
ret:
	return(val);
}

p2filename(s)
char *s;
{
	put2(FILENAME, tmpfil);
	fwrite((char *)&s, sizeof(s), 1, tmpfil);
}

align(m)
register m;
{
	if (dotp->xvalue&m) {
		yyerror("Alignment error");
		dotp->xvalue |= m;
		dotp->xvalue += 1;
	}
}

ckrel(sp, xp)
register struct nlist *sp;
register struct exp *xp;
{
	if ((xp->xtype&XTYPE) == N_UNDF) {
		sp->n_type = N_UNDF|N_EXT;
		sp->n_value = 0;
		return;
	}
	sp->n_type = xp->xtype&XTYPE;
	sp->n_value = xp->xvalue;
}

long
ckabs(xp)
register struct exp *xp;
{
	if ((xp->xtype & XTYPE)!=N_ABS)
		yyerror("Absolute expression required");
	return(xp->xvalue);
}

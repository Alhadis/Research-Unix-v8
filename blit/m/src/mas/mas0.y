/*
 * Motorola 68000 assembler
 */
%token COLON
%token <yint> PLUS MINUS
%token <yint> MUL DIV
%token AMP OFFOP
%token CM NL LB RB LP RP SEMI
%token <ynumber> INT
%token <yname> NAME
%token <yname> REG
%token NOCHAR SPC ALPH DIGIT SQ SH DQ EXCL EOR IOR
%token <ystring> STRING
%token <yint> SIZE
%token <yint> ISPACE IBYTE ILONG
%token <yname> INST0 INST1 INST2
%token <yint> ISHORT IDATA IGLOBAL ISET ITEXT ICOMM ILCOMM IEVEN
%token <yint> IORG IOPTIM IPCREL
%token ISTABS ISTABD ISTABN
%token NOTYET
%token FILENAME

%nonassoc COLON
%left PLUS MINUS
%left MUL DIV
%left AMP OFFOP
%left EXCL EOR IOR

%type <yint> comm
%type <yarg> arg
%type <yint> reg
%type <yint> optsize
%type <yexp> expr
%type <yexp> sexpr

%{
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
%}

%%

%{
	struct exp *pval;
%}

wholefile: file = {
		curlen = NBPW/2;
		dotp->xvalue &= ~01;
		flushfield(NBPW/2);
	}

file:	/* empty */ = {
		goto reset;
	}
	| file linstruction NL = {
		lineno++;
		goto reset;
	}
	| file linstruction SEMI = {
		goto reset;
	}
	| file error NL = {
		lineno++;
		yyerrok;
	reset:
		ap = arglist;
		xp = explist;
		usrname = 0;
	}
	;

labels:		/* empty */
	| labels NAME COLON = {
		flushfield(NBPW/4);
		if (($2->n_type&XTYPE)!=N_UNDF) {
			if(($2->n_type&XTYPE)!=dotp->xtype || $2->n_value!=dotp->xvalue
			 || passno==1 && $2->index != dotp->xloc) {
				yyerror("%.8s redefined", $2->n_name);
			}
		}
		$2->n_type &= ~(XTYPE|XFORW);
		$2->n_type |= dotp->xtype;
		$2->n_value = dotp->xvalue;
		if (passno==1)
			$2->index = dotp-usedot;
	}
	;

linstruction:	labels instruction
		;

instruction:
	ISET NAME CM expr = {
		$2->n_type &= (N_EXT|XFORW);
		/*  what's this for?
		if($2->n_type&N_EXT && ($2->n_type&XTYPE)==N_UNDF && passno==2)
			$2->n_type &= ~N_EXT;
		*/
		$2->n_type |= $4->xtype&(XTYPE|XFORW);
		$2->n_value = $4->xvalue;
		if (passno==1)
			$2->index = $4->xloc;
		else if (($4->xtype&XTYPE)==N_UNDF)
			yyerror("Illegal set");
	}
	| IGLOBAL names
	| IDATA = {
		i = -IDATA;
		goto chloc;
	}
	| IDATA expr = {
		i = IDATA;
		goto chloc;
	}
	| ITEXT = {
		i = -ITEXT;
		goto chloc;
	}
	| ITEXT expr = {
		i = ITEXT;
	chloc:
		if (i < 0) {
			ii = 0;
			i = -i;
		} else {
			if ($2->xtype != N_ABS || (ii=$2->xvalue) >= NLOC) {
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
	}
	| IBYTE setchar explist = {
		flushfield(NBPW/4);
		if (bitoff)
			dotp->xvalue++;
	}
	| ILONG setint explist = {
		flushfield(NBPW);
	}
	| ISHORT sethalf explist = {
		flushfield(NBPW/2);
	}
	| ISPACE expr = {
		if ($2->xtype != N_ABS)
			yyerror("space size not absolute");
		li = $2->xvalue;
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
	}
	| IORG expr = {
		if ($2->xtype==N_ABS)
			orgwarn++;
		else if ($2->xtype!=dotp->xtype)
			yyerror("Illegal 'org'");
		li = $2->xvalue - dotp->xvalue;
		if (li < 0)
			yyerror("Backwards 'org'");
		goto ospace;
	}
	| comm NAME CM expr = {
		if ($4->xtype != N_ABS)
			yyerror("comm size not absolute");
		if (passno==1 && ($2->n_type&XTYPE)!=N_UNDF)
			yyerror("Redefinition of %.8s", $2->n_name);
		if (passno==1) {
			$2->n_value = $4->xvalue;
			if ($1==ICOMM)
				$2->n_type |= N_EXT;
			else {
				$2->n_type &= ~XTYPE;
				$2->n_type |= N_BSS;
			}
		}
	}
	| IEVEN = {
		flushfield(NBPW/2);
	}
	| INST0 optsize = {
		insout($1, (struct arg *)NULL, (struct arg *)NULL, $2);
	}
	| INST1 optsize arg = {
		insout($1, $3, (struct arg *)NULL, $2);
	}
	| INST2 optsize arg CM arg = {
		insout($1, $3, $5, $2);
	}
	| IPCREL optsize NAME = {
		li = $3->n_value - dotp->xvalue;
		if ($2==0)
			$2 = W;
		if (passno == 2 && (dotp->xtype != $3->n_type
		 || ($2==W && (li < -65536 || li > 65535))))
			yyerror("Illegal pcrel value");
		if ($2==B)
			yyerror("Illegal size");
		if ($2==L) {
			outhw((short)(li>>16), $3->n_type, $3, XPCREL+X2WDS);
			outhw((short)li, N_ABS, SNULL, 0);
		} else
			outhw((short)li, $3->n_type, $3, XPCREL);
		dotp->xvalue += 2;
	}
	| ISTABS STRING CM expr CM expr CM expr = {
		nstabs++;
		if (passno==2) {
			strncpy(stabsym.n_name, $2, NCPS);
			stabsym.n_dtype = ckabs($4);
			stabsym.n_desc = ckabs($6);
			ckrel(&stabsym, $8);
			goto writestab;
		}
	}
	| ISTABN expr CM expr CM expr = {
		nstabs++;
		if (passno==2) {
			strncpy(stabsym.n_name, "", NCPS);
			stabsym.n_dtype = ckabs($2);
			stabsym.n_desc = ckabs($4);
			ckrel(&stabsym, $6);
			goto writestab;
		}
	}
	| ISTABD expr CM expr = {
		nstabs++;
		if (passno==2) {
			strncpy(stabsym.n_name, "", NCPS);
			stabsym.n_dtype = ckabs($2);
			stabsym.n_desc = ckabs($4);
			ckrel(&stabsym, dotp);
		writestab:
			fwrite((char *)&stabsym, sizeof(stabsym), 1, stabfil);
		}
	}
	| IOPTIM	/* ignore */
	|  /* empty */
	;

names:	NAME = {
		$1->n_type |= N_EXT;
		}
	| names CM NAME =
		$3->n_type |= N_EXT;

optsize: SIZE = {
		$$ = $1;
	}
	| /* empty */ = {
		$$ = 0;
	}
	;

comm:	ICOMM = {
		$$ = ICOMM;
	}
	| ILCOMM = {
		$$ = ILCOMM;
	}
	;
explist: /* empty */
	| explist CM outexpr
	| outexpr
	;

outexpr: expr = {
		i = curlen;
		pval = $1;
		flushfield(curlen);
		goto outx;
	}
	| expr COLON expr = {
		if ($1->xtype != N_ABS)
			yyerror("Width expression not absolute");
		i = $1->xvalue;
		pval = $3;
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
	}

setchar: = {
		curlen = NBPW/4;
		dotp->xvalue &= ~01;
	}

setint: = {
		curlen = NBPW;
		align(FW);
	}

sethalf: = {
		curlen = NBPW/2;
		align(HW);
	}

arg:	expr = {
		ap->atype = AEXP;
		ap->xp = $1;
		ap->areg1 = 0;
		ap->areg2 = 0;
		$$ = ap++;
	}
	| reg = {
		ap->atype = AREG;
		ap->areg1 = $1;
		ap->areg2 = 0;
		$$ = ap++;
	}
	| LP reg RP = {
		ap->atype = AIREG;
		ap->areg1 = $2;
		ap->areg2 = 0;
		$$ = ap++;
	}
	| MINUS LP reg RP = {
		ap->atype = ADEC;
		ap->areg1 = $3;
		ap->areg2 = 0;
		$$ = ap++;
	}
	| LP reg RP PLUS = {
		ap->atype = AINC;
		ap->areg1 = $2;
		ap->areg2 = 0;
		$$ = ap++;
	} 
	| expr LP reg RP = {
		ap->atype = AOFF;
		ap->xp = $1;
		ap->areg1 = $3;
		ap->areg2 = 0;
		$$ = ap++;
	}
	| OFFOP expr LP reg RP = {
		ap->atype = APIC;
		ap->xp = $2;
		ap->areg1 = $4;
		ap->areg2 = 0;
		$$ = ap++;
	}
	| expr LP reg CM reg SIZE RP = {
		ap->atype = ANDX;
		ap->xp = $1;
		ap->areg1 = $3;
		ap->areg2 = $5;
		ap->asize = $6;
		$$ = ap++;
	}
	| OFFOP expr LP reg CM reg SIZE RP = {
		ap->atype = API2;
		ap->xp = $2;
		ap->areg1 = $4;
		ap->areg2 = $6;
		ap->asize = $7;
		$$ = ap++;
	}
	| AMP expr = {
		ap->atype = AIMM;
		ap->xp = $2;
		ap->areg1 = 0;
		ap->areg2 = 0;
		$$ = ap++;
	}
	;

reg:
	REG = {
		if ( ($1->n_type&XTYPE)!=N_ABS || $1->n_value<0 || $1->n_value>SRREG)
			if (passno!=1)
				yyerror("Illegal register");
		$$ = $1->n_value;
	};

expr:	sexpr
	/* OFFOP would go here */
	;

sexpr:	NAME = {
		$$ = xp++;
		$$->xtype = $1->n_type;
		if (($1->n_type&XTYPE)==N_UNDF) {
			$$->xname = $1;
			$$->xvalue = 0;
			if (passno==1)
				$1->n_type |= XFORW;
		} else {
			$$->xvalue = $1->n_value;
			$$->xname = NULL;
			$$->xloc = $1->index;
		}
	}
	| INT = {
		$$ = xp++;
		$$->xtype = N_ABS;
		$$->xvalue = * (long *)$1;
		$$->xloc = 0;
	}
	| LP sexpr RP = {
		$$ = $2;
	}
	| sexpr PLUS sexpr = {
		goto comb;
	}
	| sexpr MINUS sexpr = {
		goto comb;
	}
	| sexpr MUL sexpr = {
		goto comb;
	}
	| sexpr DIV sexpr = {
	comb:
		$$ = combine($2, $1, $3);
	}
	| MINUS sexpr %prec AMP = {
		xp->xtype = N_ABS;
		xp->xvalue = 0;
		$$ = combine($1, xp++, $2);
	}
	| EXCL sexpr ={
		$$ = $2;
		$$->xvalue = ~$$->xvalue;
	}
	;

%%

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

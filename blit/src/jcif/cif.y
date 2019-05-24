%{
#include <stdio.h>
#include <ctype.h>
#include "cif.h"

int sym,width,wpath;    /* current Symbol number */
Rectangle rect;		/* a Rectangle accumulator */
Rectangle mbb;		/* the mbb so far */
int nullmbb = 1;
%}

%token  MINUS,SEMI,INT,DS,DF,DD,E,P,R,B,M,X,Y,T,W,C,L,NE,NM,NP,NC,ND,NI,NG,NB,
%union {
    int intval;
    Point ptval;
    Rectangle rectval;
    Transform transval;
}

%type <intval> integer,INT;
%type <ptval> Point;
%type <transval> Transformation tform;

/* beginning token */
%start file

%%
file:           program end
	    ;
program:        body
	    |   program body
	    ;
body:           statement
	    |   defdelete
	    |   defstart statements deffinish
	    ;
statements:     statement
	    |   statements statement
	    ;
statement:      SEMI
	    |   primitive SEMI {
		    if (wpath == 1) { /* Lord forgive me for this! */
			inttobin(MZERO);
			wpath = 0;
		    }
		}
	    ;
primitive:      polygon
	    |   box
	    |   wire
	    |   layer
	    |   call
	    ;
polygon:        P {
			wpath = 1;
			inttobin(POLYGON);
		}
	    |   polygon Point {
		    pttobin($2);
		    if (nullmbb == 1) {
			mbb = pttobox($2,0);
			nullmbb = 0;
		    }
		    else
			mbb = mbbrect(mbb,pttobox($2,0));
		}
	    ;
box:            B integer integer Point {
		    if ($2 == 0 || $3 == 0)
			inttobin(ERRBOX);
		    else
		    inttobin(BOX);
		    rect = boxtorect($2,$3,$4);
		    recttobin(rect);
		    if (nullmbb == 1) {
			mbb = rect;
			nullmbb = 0;
		    }
		    else
			mbb = mbbrect(mbb,rect);
		}
	    |   B integer integer Point Point
	    ;
wire:           W integer {
		    wpath = 1;  /* so that SEMI can clean up - ugh! */
		    inttobin(WIRE);
		    inttobin(width = $2);
		}
	    |   wire Point {
		    pttobin($2);
		    if (nullmbb == 1) {
			mbb = pttobox($2,width);
			nullmbb = 0;
		    }
		    else
			mbb = mbbrect(mbb,pttobox($2,width));
		}
	    ;
layer:          L NM {
		    inttobin(METAL);
		}
	    |   L NE {
		    inttobin(ERRS);
		}
	    |   L ND {
		    inttobin(DIFF);
		}
	    |   L NP {
		    inttobin(POLY);
		}
	    |   L NI {
		    inttobin(IMPLANT);
		}
	    |   L NC {
		    inttobin(CUT);
		}
	    |   L NB {
		    inttobin(POLYCON);
		}
	    |   L NG {
		    inttobin(GLASS);
		}
	    ;
defstart:       DS integer {
		    sym = $2;
		    markbos(sym);
		}
	    |   DS integer integer integer {
		    sym = $2;
		    markbos(sym);
		}
	    ;
deffinish:      DF {
		    syms[sym].mbb = mbb;
		    nullmbb = 1;
		    inttobin(MZERO);
		    sym = 0;
		}
	    ;
defdelete:      DD INT
	    ;
call:           C INT Transformation {
		    if ($2 > 1 && syms[$2].pc == 0)
			yyerror("reference before definition!");
		    if (nullmbb == 1) {
			mbb = transrect($3,syms[$2].mbb);
			nullmbb = 0;
		    }
		    else
			mbb = mbbrect(mbb,transrect($3,syms[$2].mbb));
		    syms[$2].refcnt++;
		    if (sym != 0) {
			inttobin(CALL);
			inttobin($2);
			transtobin($3);
		    }
		    else {
			plot($2,$3);
		    }
		}
	    ;
end:            E SEMI
	    |
		E
	    ;
Transformation: /* empty */ {
		    $$ = nulltrans;
		}
	    |   Transformation tform {
		    $$ = transtrans($2,$1);
		}
	    ;
tform:          T Point {
		    $$ = newtrans(1,0,0,1,$2.x,$2.y);
		}
	    |   M X {
		    $$ = newtrans(-1,0,0,1,0,0);
		}
	    |   M Y {
		    $$ = newtrans(1,0,0,-1,0,0);
		}
	    |   R Point {
		    int x,y,r;  /* Manhattan rotations ONLY */
		    x = $2.x;
		    y = $2.y;
		    r = abs(x + y);
		    $$ = newtrans(x/r,y/r,-y/r,x/r,0,0);
		}
	    ;
Point:          integer integer {
		    $$ = newpt($1,$2);
		}
	    ;
integer:        INT {
		    $$ = $1;
		}
	    |   MINUS INT {
		    $$ = - $2;
		}
	    ;
%%

#include "lex.yy.c"

yyerror(s)
{
    fprintf(stderr,"line %d: %s\n",linecnt,s);
}

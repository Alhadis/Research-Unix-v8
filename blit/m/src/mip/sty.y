/*	This program turns machine descriptions into a table.c file */
/*	The input language uses many C conventions and notations */
/*	Types are represented as a shorthand:
	c	char
	i	int
	s	short
	l	long
	p	pointer
	P	alternate form of pointer
	t	structure
	v	void
	ux	unsigned x
	f	float
	d	double
/*	There are a number of builtin cost names:
	NONAME		a constant with no name field (not an address)
	CONVAL n	the constant n
	NCONVAL n	the constant -n
	POSRANGE n	constants in the range [0,2**n -1]
	SGRANGE n	constants in the range [-2**n, 2**n - 1]
	*/
/*	There are also several incedental needs, etc, specified
	1,2,3	Number of needed registers
	$P	need pairs
	$<	share left
	$>	share right
	$L	result left
	$R	result right
	$1,2,3	result in reg 1, 2, 3
	$C	operation produces correct condition codes
	$N	no value produced: side effects only
	$A	need them all
	$[	share left, RHS is preferred (if a temp register)
	$]	share right, RHS is preferred (if a temp register)
	$l	left side is referenced once more
	$r	right side is referenced once more
	*/

/* NOTE:
	When several templates match with equal cost, the first is chosen.
	Thus, when side-effects only are desired, the first template matching
	is taken, if it is of lowest cost.
	Moral:  $N templates should be cheaper or should preceed $L, $1, etc.
	*/

%{
# include "stdio.h"
# include "ctype.h"
# include "mfile2"
# include "common"

typedef struct {
	int sha;
	int ty;
	} SHTY;

SHTY lshty, rshty;

int dcost = 0;
int op, tyop, needs, rewrite, cost;
int lcount, rcount;
int opno;
int ophd[DSIZE];
int optl[DSIZE];
int opsh[DSIZE];

typedef struct optb OPTB;

# ifndef NOPTB
# define NOPTB 400
# endif

struct optb {
	int op;  /* what operation is to be matched */
	int tyop; /* the type associated with the op node */
	int nxop;  /* the next op */
	int line; /* the input line number */
	SHTY l, r;  /* the shapes of the left and right */
	int oneeds, orew, ocost; /* needs, rewrite info, op cost */
	char *string;  /* the output */
	int olcount, orcount;  /* the usage count for lhs, rhs */
	};

OPTB optb[NOPTB];

# ifndef NSTRING
# define NSTRING 10000
# endif

# ifndef NSTYSHPS
# define NSTYSHPS 4000
# endif

typedef struct {
	int sop, sleft, sright, ssh, scost;
	char shname[8];
	} STYSHP;

int nshp = 0;
int nmshp = NSTYSHPS-1;

STYSHP shp[NSTYSHPS];
char strng[NSTRING];
char *string;
char *pstring = strng;
char	*asstring;

%}

%union {
	int ival;
	char *str;
	SHTY shh;
	};

%token STR DEF LPRF RPRF LSHR RSHR GOODCC NOVAL PNEED LRES RRES LCOUNT RCOUNT
%token USERCOST CONVAL NCONVAL POSRANGE SGRANGE NONAME DCOST SHAPES OPCODES
%token <ival> OP NM DIG STYPE RREG
%left OP
%left STYPE

%type <str> STR
%type <ival> opcost num slist nterm opop shape
%type <ival> costnexp nexp
%type <ival> cost cexpr cterm
%type <shh> sref

	/*
	OP	the ops <, <<, >, >>, <=, >=, +, -, *, /, %, &, ^, ~, !
	NM	names (letter, followed by 0 or more letters or digits
	STR	a string in ""
	DIG	a digit, 0-9
	Other letters are returned: (, ), {, }, etc.
	*/

%%

file	:	SHAPES lshapes OPCODES lops
		{	finished(); }
	;

lshapes	:	/* EMPTY */
	|	lshapes NM ':' slist ',' costnexp ';'
		{	shp[$2].sop = MANY;
			shp[$2].sleft = $4;
			shp[$2].sright = $6;
			shp[$2].scost = 0;
			shp[$2].ssh = 0;
			}
	|	lshapes NM ':' costnexp ';'
		{	shp[$2].sop = MANY;
			shp[$2].sleft = $4;
			shp[$2].sright = -1;
			shp[$2].scost = 0;
			shp[$2].ssh = 0;
			}
	|	lshapes NM ':' opop shape opcost ';'
		{	shp[nshp].scost = $6;
			shp[nshp].ssh = $5;
			shp[nshp].sop = $4;
			shp[nshp].sleft = -1;
			shp[nshp].sright = -1;
			++nshp;
			checkit( nshp-1 );
			shp[$2].sop = MANY;
			shp[$2].sleft = nshp-1;
			shp[$2].sright = -1;
			shp[$2].scost = 0;
			shp[$2].ssh = 0;
			if( nshp >= nmshp ) {
				yyerror( "out of node space" );
				exit( 1 );
				}
			}
	;

opop	:	/* EMPTY */
		{	$$ = ICON; }
	|	OP
	;

lops	:	/* EMPTY */
		{	needs = op = rewrite = cost = 0;
			lcount = rcount = 1; }

	|	lops lop
	;

lop	:	OP sref ',' sref ltail
		{	lshty = $2;
			rshty = $4;
			op = $1;
			tyop = TANY;
			output();
			tyop = needs = op = rewrite = cost = 0;
			lcount = rcount = 1;
			}
	|	OP STYPE sref ',' sref ltail
		{	lshty = $3;
			rshty = $5;
			op = $1;
			tyop = $2;
			output();
			tyop = needs = op = rewrite = cost = 0;
			lcount = rcount = 1;
			}
	|	OP sref ltail
		{	lshty = $2;
			rshty.sha = -1;
			rshty.ty = TANY;
			op = $1;
			tyop = TANY;
			output();
			tyop = needs = op = rewrite = cost = 0;
			lcount = rcount = 1;
			}
	|	OP STYPE sref ltail
		{	lshty = $3;
			rshty.sha = -1;
			rshty.ty = $2;
			op = $1;
			tyop = TANY;
			output();
			tyop = needs = op = rewrite = cost = 0;
			lcount = rcount = 1;
			}
	|	sref ltail
		{	rshty = $1;
			lshty.sha = -1;
			lshty.ty = TANY;
			loutput();
			needs = op = tyop = rewrite = cost = 0;
			lcount = rcount = 1;
			}
	|	DCOST {dcost=0;} opcost ';'
			{	dcost = $3; }
	;

ltail	:	needs STR opcost ';'
		{	
			cost = $3;
			asstring = $2;
			}
	;

needs	:	/* EMPTY */
		{	needs = 0; }
	|	'{' nlist '}'
	;

nlist	:	/* EMPTY */
		{	needs = 0; }
	|	nlist DIG
		{	needs = (needs&~NCOUNT) | $2; }
	|	nlist RPRF
		{	needs |= RSHARE|RPREF; }
	|	nlist LPRF
		{	needs |= LSHARE|LPREF; }
	|	nlist RSHR
		{	needs |= RSHARE; }
	|	nlist LSHR
		{	needs |= LSHARE; }
	|	nlist PNEED
		{	needs |= NPAIR; }
	|	nlist GOODCC
		{	rewrite |= RESCC; }
	|	nlist NOVAL
		{	rewrite = RNULL; }
	|	nlist LRES
		{	rewrite |= RLEFT; }
	|	nlist RRES
		{	rewrite |= RRIGHT; }
	|	nlist RREG
		{	if( !(needs&NCOUNT) ) needs |= $2;
			rewrite |= (($2==1)?RESC1:(($2==2)?RESC2:RESC3));
			}
	|	nlist LCOUNT
		{	lcount += 1; }
	|	nlist RCOUNT
		{	rcount += 1; }
	;

num	:	DIG
	|	num DIG
		{	$$ = 10*$1 + $2; }
	;

opcost	:	cost
	|	/* EMPTY */
		{	$$ = dcost; }
	;

cost	:	':' cexpr
		{	$$ = $2; }
	;

cexpr	:	cterm
	|	OP cterm
		{	$$ = uopcost( $1, $2 ); }
	|	cterm OP cterm
		{	$$ = bopcost( $2, $1, $3 ); }
	;

cterm	:	'(' cexpr ')'
		{	$$ = $2; }
	|	num
	;

shape	:	/* EMPTY */
		{	$$ = 0; }
	|	NONAME
		{	$$ = NACON;  /* constant with no name */ }
	|	USERCOST num
		{	$$ = $2|SUSER;  /* user's cost ftn */ }
	|	CONVAL num
		{	$$ = $2|SVAL;  /* positive constant value */ }
	|	NCONVAL num
		{	$$ = $2|SNVAL;  /* negative constant value */ }
	|	POSRANGE num
		{	$$ = $2|SRANGE0;  /* positive range */ }
	|	SGRANGE num
		{	$$ = $2|SSRANGE;  /* signed range */ }
	;

sref	:	nterm
		{	$$.ty = 0;
			$$.sha = $1;
			}
	|	nterm STYPE     /* do this before doing in nterm */
		{	$$.sha = $1;
			$$.ty = $2; }
	;

slist	:	costnexp
	|	slist ',' costnexp
		{	$$ = bop( MANY, $1, $3, 0 ); }
	;

costnexp:	nexp opcost
		{	$$ = sharecost($1, $2); }
	;
	
nexp	:	nterm
	|	OP nterm
		{	$$ = uop( $1, $2 ); }
	|	nterm OP nterm
		{	$$ = bop( $2, $1, $3, 0 ); }
	;

nterm	:	'(' nexp ')'
		{	$$ = $2; }
	|	NM
	|	nterm STYPE
		{	$$ = top( $1, $2 );  }
	;

%%
int	lineno = 1;
char	filename[100] = "<stdin>";

main(argc, argv) int argc; char **argv;{
	int i;
	for( i=0; i<DSIZE; ++i ) {
		opsh[i] = 0;
		optl[i] = ophd[i] = -1;
		}
	mkdope();
	yyparse();
	exit( 0 );
	}

checkit( n ) {
	/* check a shape */
	STYSHP *p;
	if( n<0 ) return;
	if( n>=nshp && n<=nmshp || n>=NSTYSHPS ) {
		yyerror( "out of range shape: %d", n );
		}
	p = &shp[n];
	if( p->sop < 0 || p->sop > MANY ) {
		yyerror( "out of range op: %d", p->sop );
		}

	switch( optype(p->sop) ) {

	case BITYPE:
		if( p->sright < 0 && p->sop != MANY ) {
			yyerror( "right side empty" );
			}
		checkit( p->sright );
	case UTYPE:
		if( p->sleft < 0 ) {
			yyerror( "left side empty" );
			}
		checkit( p->sleft );
		}
	}

	/* VARARGS */
yyerror( s, a ) char *s; {
	fprintf( stderr, s, a );
	fprintf( stderr, ", file \"%s\", line %d\n", filename, lineno );
	fprintf( stderr, "for now, I'm dumping a core\n" );
	abort();
	exit( 1 );
	}

int otb[20];  /* ops that are part of the shape */
int notb;  /* number of otb entries */

loutput(){
	/*
	 * rhs has a leaf: output templates for all interesting
	 * operators in this leaf. 
	 * An interesting operator is one that is NOT MANY.
	 * This allows us to access the table for each operator
	 * that this leaf may implement.
	 */
	int s, i;
	int ocost = cost;
	notb = 0;
	lout1( s = rshty.sha );
	if( !notb ) yyerror( "lout1 err" );
	for( i=notb-1; i>=0; --i ){
		op = otb[i];
		tyop = TANY;
		if( optype(op) == LTYPE ) {
			lcount = 0;
			cost = ocost + (rcount * refine(s));
			}
		else if( optype(op) == BITYPE ) {
			yyerror( "binary op in shape not done right" );
			}
		else {  /* op is unary */
			lcount = rcount;
			rcount = 0;
			cost = ocost + (lcount * urefine(s));
			}
		output();
		}
	}

urefine( s ) {  /* construct an entry for a unary op matching shape s */
	/* for now, punt on the costs */
	rcount = lcount;
	lcount = 0;
	return( refine( s ) );
	}

smat(s) {
	/*
	 *	return 1 if we can find the op anyplace in the
	 *	shape tree `s'
	 */
	if( s<0 ) return( 0 );
	if( shp[s].sop == MANY ) {
		if( shp[s].sleft>=0 && smat( shp[s].sleft ) ) return( 1 );
		if( shp[s].sright>=0 && smat( shp[s].sright ) ) return( 1 );
		return( 0 );
		}
	return( shp[s].sop == op );
	}

refine(s){
	/*
	 * find the largest subshape of `s' that contains
	 * the operator `op' (op is global).
	 * We descend MANY nodes until we run out,
	 * or until the operator `op' is found in both descendents.
	 * Since we are throwing away costs buried in the MANY nodes
	 * on the way down, we must keep track of those, to
	 * adjust the costs appropriately in the operator table.
	 */
	int bc = 0;

/*
	printf( "refine(%d), op=(%d)%s, left=%d, right=%d\n",
		s, shp[s].sop, opst[shp[s].sop], shp[s].sleft, shp[s].sright );
*/
	while( shp[s].sop == MANY ) {
		bc += shp[s].scost;
		if( smat( shp[s].sleft ) ){
			if( smat( shp[s].sright ) ) break;
			else s = shp[s].sleft;
			}
		else s = shp[s].sright;
/*
		printf( "\ts=>(%d), op=%s, left=%d, right=%d\n",
			s, opst[shp[s].sop], shp[s].sleft, shp[s].sright );
*/
		}
/*
	printf( "ends with s=%d\n", s );
*/
	lshty.sha = -1;
	rshty.sha = mkshp(s);
	return(bc + shp[s].scost );
	}

lout1( n ) {
	int i;
	while( n>=0 && shp[n].sop == MANY ) {
		lout1( shp[n].sleft );
		n = shp[n].sright;
		}
	if( n<0 ) return;
	for( i=0; i<notb; ++i ){
		if( otb[i] == shp[n].sop ) return;
		}
	otb[notb++] = shp[n].sop;
	}

mkshp(s) {
	/* make a shape that yields s */
	/* first, make s a MANY node */
	register i;

	if( s < 0 ) return( s );
	if( shp[s].sop == MANY ) i = s;
	else {
		/* look for a MANY node pointing to s */
		for( i= NSTYSHPS; i>nmshp; --i ) {
			if( shp[i].sright >= 0 ) continue;
			if( shp[i].sleft == s ) goto foundit;
			}
		/* must make a MANY node */
		i = bop( MANY, s, -1, 0 );
		}

	/* now, make sure that i has a name, so it will be output */

	foundit: ;

	if( !shp[i].shname[0] ) {
		strcpy( shp[i].shname, "mkshp" );
		}
	return(i);
	}

onebit(x) {
	/* return 1 if x has at most 1 bit on, 0 otherwise */
	return( !(x&(x-1)) );
	}

output(){
	OPTB *q;
	int j;

	if( lshty.ty == 0 ) lshty.ty = TANY;
	if( rshty.ty == 0 ) rshty.ty = TANY;

	switch( op ) {

	case 0:
		yyerror( "0 op" );

	case STAR:
	case REG:
	case UNARY MINUS:
	case UNARY AND:
	case FLD:
		if( needs&(LSHARE|RSHARE) ) needs |= (LSHARE|RSHARE);
		break;

	case ASSIGN:
	case ASG PLUS:
	case ASG MINUS:
	case ASG MUL:
	case ASG DIV:
	case ASG MOD:
	case ASG AND:
	case ASG OR:
	case ASG ER:
	case ASG LS:
	case ASG RS:
		if( !(rewrite & (RNULL|RESC1|RESC2|RESC3|RRIGHT)) ){
			rewrite |= RLEFT;
			}
		}
	if( !rewrite ) rewrite = RNULL;

	if( !onebit( rewrite & (RNULL|RLEFT|RRIGHT|RESC1|RESC2|RESC3) ) ) {
		yyerror( "multiple results -- illegal (%o)", rewrite );
		}
	if( ((rewrite&RLEFT)&&(needs&LSHARE)) ||
			((rewrite&RRIGHT)&&(needs&RSHARE)) ){
		if( asstring[0] != 'Y' )
			yyerror( "don't share on result side of tree" );
		}
	if( needs && (needs&(LSHARE|RSHARE)) == needs ) {
		yyerror( "don't share without allocating something" );
		}
	checkout(asstring);

	q = &optb[opno];
	if( opno >= NOPTB ) yyerror( "too many templates" );
	q->line = lineno;
	q->op = op;
	q->tyop = tyop;
	if( ophd[op]>=0 ) {
		optb[optl[op]].nxop = opno;
		optl[op] = opno;
		}
	else {
		optl[op] = ophd[op] = opno;
		}
	q->nxop = -1;
	q->l = lshty;
	q->r = rshty;
	q->oneeds = needs;
	q->orew = rewrite;
	q->ocost = cost;
	q->string = asstring;
	q->olcount = lcount;
	q->orcount = rcount;
	/* now, take care of special cases */
	if( optype(op) == LTYPE ) {  /* leaf */
		int s;
		if( (s=q->r.sha) >= 0 && trivial(s) ) {
			q->r.sha = -1;  /* clobber any right shape */
			}
		}
	else if( optype(op) == UTYPE ) {
		if( q->r.sha >=0 ) {
			/* someday, look for things below op */
			/* for now, we get the cost wrong so back up */
			}
		}
	++opno;
	}

trivial( s ) {
	/* is shape s a trivial match for op */
	if( shp[s].sop == MANY ) {
		if( shp[s].sright >= 0 ) {
			return( 0 );  /* nontrivial */
			}
		s = shp[s].sleft;
		}
	if( shp[s].sop != op ) {
		return( 0 );
		}
	if( shp[s].ssh ) {
		return( 0 );
		}
	return( 1 );  /* ok to clobber */
	}

checkout(string) char *string;{
	/* check out the string, looking at rewrite and needs */
	/* look for {U,I,C,A}{L,R,1,2,3} and \n */
	/* complain about:
	***	1, 2, 3 used, not allocated
	***	shared side after \n after temp used
	***	AL or AR used, w. side effect possible, more than once 
	*/

	/* flagl and flagr are 1 if L and R legal, 0 if not, -1 if
	/* they will be illegal after the next \n */

	int flagl, flagr, prn, min, cond;
	register char *s;

	flagl = flagr = 1;
	cond = 0;

	for( s=string; *s; ++s ) {
		switch( *s ) {

		case '\\':
			++s;
			if( *s == '\\' ) ++s;
			else if( *s == 'n' ){
				if( flagl<0 ) flagl=0;
				if( flagr<0 ) flagr=0;
				}
			break;

		case 'Z':
			++s;
			if( *s=='(' ) {
				while( *++s != ')' ) {;}
				}
			break;

		case 'Y':
			/* this string is asserted to be good; don't check */
			return;

		case 'R':
		case 'D':
			/* conditional; a lot of stuff no longer is true */
			cond = 1;
			break;

		case 'A':
		case 'C':
		case 'U':
		case 'I':
			++s;
			if( *s == '-' ) {
				++s;
				min = 1;
				}
			else min = 0;
			if( *s == '(' ) {
				++s;
				prn = 1;
				}
			else prn = 0;
			switch( *s ){

			case 'L':
				if( !flagl && !cond ) {
					yyerror( "illegal L just at \"%s\"",
							s );
					}
				/* look for side-effects here */
				if( !min && seff(lshty.sha)) flagl = 0;
				break;

			case 'R':
				if( !flagr && !cond ) {
					yyerror( "illegal R just at \"%s\"",
							s );
					}
				/* look for side-effects here */
				if( !min && seff(rshty.sha)) flagr = 0;
				break;

			case '1':
			case '2':
			case '3':
				if( (*s - '0') > (needs&NCOUNT) ){
					yyerror( "reg %c used, not allocated",
						*s );
					}
				if( (needs&LSHARE) && flagl ) flagl = -1;
				if( (needs&RSHARE) && flagr ) flagr = -1;

			case '.':
				break;

			default:
				yyerror( "illegal qualifier just at \"%s\"",

					s );
				}
			if( prn ) while( *s != ')' ) ++s;
			}
		}
	}

seff( s ) {
	if( shp[s].sop == INCR || shp[s].sop == DECR ) return( 1 );
	if( shp[s].sleft >= 0 && seff( shp[s].sleft ) ) return( 1 );
	if( shp[s].sright >= 0 && seff( shp[s].sright ) ) return( 1 );
	return( 0 );
	}

uop( o, a ) {
	if( o == MUL ) o = STAR;
	else if( o == MINUS ) o = UNARY MINUS;
	else if( o == AND ) o = UNARY AND;
	return( bop( o, a, -1, 0 ) );
	}

top( a, ty ) {
	/* build a type node over a */
	/* must be done differently than uop, since types must be copied */

	if( a<0 ) return( a );
	checkit( a );
	if( shp[a].sop == MANY ) {
		int l, r;
		l = shp[a].sleft;
		r = shp[a].sright;
		if( l>=0 ) l = top( l, ty );
		if( r>=0 ) r = top( r, ty );
		return( bop( MANY, l, r, 0 ) );
		}
	if( shp[a].ssh ) {
		yyerror( "can't type a special node" );
		}
	shp[nshp] = shp[a];
	shp[nshp].shname[0] = '\0';
	shp[nshp].ssh = ty|SPTYPE;
	++nshp;
	checkit( nshp-1 );
	return( nshp-1 );
	}

bop( o, a, b, cst ) {
	STYSHP *p;
	int l, r, ret;

	checkit( a );
	checkit( b );

	if( o != MANY ){
		while( shp[a].sop == MANY && shp[a].sright < 0 ) {
			a = shp[a].sleft;
			}
		while( shp[b].sop == MANY && shp[b].sright < 0 ) {
			b = shp[b].sleft;
			}
		if( a>=0 && shp[a].sop == MANY ) {
			/* distribute MANY nodes to top */
			l = bop( o, shp[a].sleft, b, cst + shp[a].scost );
			r = bop( o, shp[a].sright, b, cst + shp[a].scost );
			return( bop( MANY, l, r, 0 ) );
			}
		if( b>=0 && shp[b].sop == MANY ) {
			/* distribute MANY nodes to top */
			l = bop( o, a, shp[b].sleft, cst + shp[b].scost );
			r = bop( o, a, shp[b].sright, cst + shp[b].scost );
			return( bop( MANY, l, r, 0 ) );
			}
		}

	if( o == MANY ) {
		/* don't generate node if not needed */
/*
		if( a<0 ) return( b );
		else if( b<0 ) return( a );
*/
		/* MANY nodes go at the end */
		p = &shp[ret=nmshp--];
		}
	else p = &shp[ret=nshp++];
	if( nshp >= nmshp ) {
		yyerror( "out of node space" );
		exit( 1 );
		}
	p->sop = o;
	p->sleft = a;
	p->sright =b;
	p->scost = cst;
	p->ssh = 0;
	p->shname[0] = '\0';
	checkit( ret );
	return( ret );
	}

int olist[] = { PLUS, MINUS, MUL, DIV, MOD, LS, RS, OR, ER, AND, -1 };

finished() {
	STYSHP *p;
	int i, j;

	/* terminate the templates */
	printf( "# include \"mfile2\"\n\n" );
	printf( "# define PSHNL ((SHAPE **)0)\n" );
	printf( "# define P(x) (&pshape[x])\n" );
	printf( "# define SHNL ((SHAPE *)0)\n" );
	printf( "# define S(x) (&shapes[x])\n" );

	/* chain binary ops together with the op= form */

	for( i=0; (op=olist[i])>=0; ++i ){
		if( ophd[op]<0 ) ophd[op] = ophd[ASG op];
		else optb[optl[op]].nxop = ophd[ASG op];
		}
	if( ophd[STCALL]<0 ) ophd[STCALL] = ophd[CALL];
	if( ophd[UNARY STCALL]<0 ) ophd[UNARY STCALL] = ophd[UNARY CALL];

	/* everything that gets used should be a MANY shape with name */
	/* mkshp is called to cause this to be true */

	for( i=0; i<opno; ++i ) {
		OPTB *q;
		q = &optb[i];
		q->l.sha = mkshp(q->l.sha);
		q->r.sha = mkshp(q->r.sha);
		}

	/* set the ssh flags in shp, but don't print yet */

	for( j=0,i=NSTYSHPS-1; i>nmshp; --i ) {
		if( !shp[i].shname[0] ) continue;
		shp[i].ssh = j;
		j += manycount( i );
		++j;  /* count the null */
		}

	printf( "\nSHAPE shapes[] = {\n" );

	for( i=0, p=shp; i<nshp; ++i,++p ){
		if( p->sop<0 )
			yyerror( "undefined shape: %.8s", p->shname );
		printf( "/*  %d */ ", i);
		printf( "%d,\t",p->sop );
		saaddr(p->sleft);
		saaddr(p->sright);
		printf( "0%o, %d,\n", p->ssh, p->scost );
		if( p->shname[0] ) printf( "\t/* %.8s  ***** */\n", p->shname );
		}

	printf( "\n};\n" );

	/* out out pointers to the shape table */

	printf( "SHAPE *pshape[] = {\n" );
	for( j=0,i=NSTYSHPS-1; i>nmshp; --i ) {
		/* put out the pointers to the shape table */
		if( !shp[i].shname[0] ) continue;
		if( shp[i].ssh != j ) {
			fprintf( stderr, "shape %d, ssh=%d, j=%d\n",
				i, shp[i].ssh, j );
			}
		printf( "/* %d */ ", j );
		j = manylist( i, j );
		printf( " SHNL,  /* %.8s */\n", shp[i].shname );
		++j;
		}

	printf( "};\n" );

	printf( "struct optab table[] = {\n\n" );

	for( i=0; i<opno; ++i ){
		OPTB *q;
		q = &optb[i];
		printf( "/* # %d, line %d */\n", i, q->line );
		if( q->nxop >= 0 )
			printf( "%d,\t0%o,\t&table[%d],\n", q->op, q->tyop,
				 q->nxop );
		else printf( "%d,\t0%o,\t0,\n", q->op, q->tyop );

		shpprint(q->l.sha, q->l.ty);
		shpprint(q->r.sha, q->r.ty);
		printf( "\t\t0%o,\t0%o,\n\t\t\"%s\",\n\t\t%d,%d,%d, %d,\n",
			q->oneeds, q->orew, q->string,
			q->ocost, q->olcount, q->orcount, q->line );
		}
	printf( "\n};\n" );

	printf( "OPTAB *ophead[] = {\n" );

	for( i=0; i<DSIZE; ++i ){
		if( ophd[i] < 0 ) printf( "	0,\n" );
		else printf( "	&table[%d],\n", ophd[i] );
		}
	printf( "	};\n" );
	}

manycount( i ) {
	/* count the descendents of shp[i] */
	int c;
	if( shp[i].sop == MANY ) {
		c = 0;
		if( shp[i].sleft >= 0 ) c += manycount( shp[i].sleft );
		if( shp[i].sright >= 0 ) c += manycount( shp[i].sright );
		return( c );
		}
	else return( 1 );
	}

manylist( i, j ) {
	/* put out a list of S(x) for shp[i], starting at j */
	/* first, put out CCODES and FREE */
	/* next, put out REG and TEMP */
	/* next, everything else */
	int k;

	if( i<0 ) return(j);

	setopsh( i );  /* set opsh[k] if op k is legal at head of i */
	if( opsh[FREE] ) {
		j = manyop( i, j, FREE );
		opsh[FREE] = 0;
		}
	if( opsh[CCODES] ) {
		j = manyop( i, j, CCODES );
		opsh[CCODES] = 0;
		}
	if( opsh[REG] ) {
		j = manyop( i, j, REG );
		opsh[REG] = 0;
		}
	if( opsh[TEMP] ) {
		j = manyop( i, j, TEMP );
		opsh[TEMP] = 0;
		}
	for( k=0; k<DSIZE; ++k ) {
		if( opsh[k] ) {
			j = manyop( i, j, k );
			opsh[k] = 0;
			}
		}
	return( j );
	}

manyop( i, j, o ) {
	/* put out a list of S(x) for shapes matching op */

	if( i<0 ) return( j );
	if( shp[i].sop == MANY ) {
		j = manyop( shp[i].sleft, j, o );
		return( manyop( shp[i].sright, j, o ) );
		}
	if( shp[i].sop == o ) {
		printf( "S(%d), ", i );
		if( !(j&07) ) printf( "\n" );
		return( j+1 );
		}
	return( j );
	}

setopsh( i ) {
	/* set opsh[k] to 1 for every op appearing in shp[i] */
	int s;

	if( i<0 ) return;
	s = shp[i].sop;

	if( s == MANY ) {
		setopsh( shp[i].sleft );
		setopsh( shp[i].sright );
		}
	else opsh[s] = 1;
	}

saaddr(sp){
	if( sp < 0 ) printf( "SHNL,\t" );
	else printf( "S(%d),\t", sp );
	}

shpprint(sha, ty){
	if( sha < 0 ) printf( "\tPSHNL,\t0%o,\n", ty );
	else printf( "\tP(%d),\t0%o,\n", shp[sha].ssh, ty );
	}

# define NAMESZ 100
char name[NAMESZ];  /* had better be long enough */

yylex() {
	int c, i;

	for(;;) {

		c = getchar();
		if( c<0 ) return( -1 );
		switch( c ) {

		case '\n':
			++lineno;
		case ' ':
		case '\b':
		case '\f':
		case '\v':
		case '\t':
			continue;

		case '<':
		case '>':
		case '+':
		case '-':
		case '=':
		case '*':
		case '%':
		case '/':
		case '&':
		case '|':
		case '^':
		case '!':
			name[0] = c;
			name[1] = getchar();
			name[2] = '\0';
			if( oplook() ) {
				if( yylval.ival == LS || yylval.ival == RS ) {
					if( (c=getchar()) == '=' )
						yylval.ival = ASG yylval.ival;
					else ungetc( c, stdin );
					}
				return( OP );
				}
			ungetc( name[1], stdin );
			name[1] = '\0';
			if( oplook() ) return( OP );
			yyerror( "cannot deal with %c", name[0] );
			return( OP );

		case '~':
			yylval.ival = COMPL;
			return( OP );

		case '"':
			string = pstring;
			for( i=0; i<NAMESZ-1; ++i ) {
				c = getchar();
				if( c == '\n' ) ++lineno;
				else if( c == '\\' ) { /* escape */
					if( pstring >= &strng[NSTRING] ){
						yyerror( "s_table overflow" );
						}
					*pstring++ = '\\';
					c = getchar();
					if( c == '\n' ) ++lineno;
					else if( c<0 ) yyerror( "missing \"" );
					else if( isalpha(c) && isupper(c) ){
						if( pstring>=&strng[NSTRING] ){
						yyerror( "s_table overflow" );
							}
						*pstring++ = '\\';
						}
					/* fall thru and stuff c away */
					}
				else if( c == '"' ) {
					if( pstring >= &strng[NSTRING] ){
						yyerror( "s_table overflow" );
						}
					*pstring++ = '\0';
					break;
					}
				else if( c<0 ) yyerror( "missing \"" );
				if( pstring >= &strng[NSTRING] ){
					yyerror( "s_table overflow" );
					}
				*pstring++ = c;
				}
			yylval.str = string;
			return( STR );

		case '\'':
			for( i=0; i<NAMESZ-1; ++i ){
				c = getchar();
				if( c == '\'' ) break;
				if( c == '\n' ) yyerror( "missing '" );
				name[i] = c;
				}
			name[i] = '\0';
			if( oplook() ) return( OP );
			yyerror( "bad op name: '%s'", name );

		case '[':
			for( i=0; i<NAMESZ-1; ++i ){
				c = getchar();
				if( c == ']' ) break;
				if( c == '\n' ) yyerror( "missing '" );
				name[i] = c;
				}
			name[i] = '\0';
			yylval.ival = tystr();
			return( STYPE );

		case '#':  /* comment */
			while( (c = getchar()) != '\n' ) {
				if( c < 0 ) yyerror( "unexpected EOF" );
				}
			++lineno;
			continue;

		case '$':
			c = getchar();
			if( isdigit(c) ) {
				yylval.ival = c-'0';
				return( RREG );
				}
			switch( c ) {
			case '[':	return( LPRF );
			case ']':	return( RPRF );
			case '<':	return( LSHR );
			case '>':	return( RSHR );
			case 'L':	return( LRES );
			case 'R':	return( RRES );
			case 'P':	return( PNEED );
			case 'C':	return( GOODCC );
			case 'N':	return( NOVAL );
			case 'r':	return( RCOUNT );
			case 'l':	return( LCOUNT );
			case 'A':	yylval.ival = NRGS;
					return( DIG );
				}
			yyerror( "$%c illegal", c );

		default:
			if( isdigit(c) ){
				yylval.ival = c-'0';
				return( DIG );
				}
			if( isalpha(c) ){
				/* collect the name */
				i = 1;
				name[0] = c;
				while( isalpha( (c=getchar()) ) || isdigit(c) ){
					name[i++] = c;
					}
				name[i] = '\0';
				ungetc( c, stdin );
				return( lookup() );
				}
			return( c );
			}
		}
	}

struct nlist {
	char *shop;
	int vop;
	} ot[] = {

	"++",	INCR,
	"+",	PLUS,
	"--",	DECR,
	"-",	MINUS,
	"*",	MUL,
	"%",	MOD,
	"/",	DIV,
	"&",	AND,
	"^",	ER,
	"!=",	NE,
	"==",	EQ,
	"UMINUS",	UNARY MINUS,
	"UAND",	UNARY AND,
	"STAR",	STAR,
	"+=",	ASG PLUS,
	"-=",	ASG MINUS,
	"*=",	ASG MUL,
	"/=",	ASG DIV,
	"%=",	ASG MOD,
	"&=",	ASG AND,
	"|=",	ASG OR,
	"|",	OR,
	"^=",	ASG ER,
	"=",	ASSIGN,
	"<",	LT,
	"<=",	LE,
	"<<",	LS,
	">",	GT,
	">=",	GE,
	">>",	RS,
	"FLD",	FLD,
	"CMP",	CMP,
	"COMOP",	COMOP,
	"CM",		CM,
	"GENLAB",	GENLAB,
	"GENUBR",	GENUBR,
	"GENBR",	GENBR,
	"ARG",		FUNARG,
	"STARG",	STARG,
	"STASG",	STASG,
	"INIT",		INIT,
	"GOTO",		GOTO,
	"CALL",		CALL,
	"UCALL",	UNARY CALL,
	"STCALL",	STCALL,
	"USTCALL",	UNARY STCALL,
	"CONV",		CONV,
	"PDIV",		PDIV,
	"PMUL",		PMUL,
	"REG",	REG,
	"CON",	ICON,
	"TEMP",	TEMP,
	"AUTO",	VAUTO,
	"PARAM",	VPARAM,
	"NAME",	NAME,
	"RNODE",	RNODE,
	"SNODE",	SNODE,
	"QNODE",	QNODE,
	"CC",	CCODES,
	"FREE",	FREE,
	"UOP0", UOP0,
	"UOP1", UOP1,
	"UOP2", UOP2,
	"UOP3", UOP3,
	"UOP4", UOP4,
	"UOP5", UOP5,
	"UOP6", UOP6,
	"UOP7", UOP7,
	"UOP8", UOP8,
	"UOP9", UOP9,
	"",	-1 };

oplook() {
	/* look up the first n chars of name in the above table */
	/* return 1 if it is an op, after setting yylval */
	int i;
	for( i=0; ot[i].vop >= 0; ++i ){
		if( !strcmp( name, ot[i].shop ) ) {
			yylval.ival = ot[i].vop;
			return( 1 );
			}
		}
	return( 0 );
	}

tystr() {
	register char *p;
	register i;
	/* lookup the types in name */
	p = name;
	if( !*p ) return( TANY );
	else i = 0;
	for(;;) {
		switch( *p++ ) {

		case '\0':
			return( i );

		case 'c':
			i |= TCHAR;
		case ' ':
		case ',':
			continue;

		case 's':
			i |= TSHORT;
			continue;

		case 'i':
			i |= TINT;
			continue;

		case 'l':
			i |= TLONG;
			continue;

		case 'f':
			i |= TFLOAT;
			continue;

		case 'd':
			i |= TDOUBLE;
			continue;

		case 'P':
			i |= TPOINT2;
			continue;

		case 'p':
			i |= TPOINT;
			continue;

		case 't':
			i |= TSTRUCT;
			continue;

		case 'v':
			/* later..
			i |= TVOID;
			*/
			continue;

		case 'u':
			if( *p == 'i' ) i |= TUNSIGNED;
			else if( *p == 's' ) i |= TUSHORT;
			else if( *p == 'c' ) i |= TUCHAR;
			else if( *p == 'l' ) i |= TULONG;
			else yyerror( "bad u%c type", *p );
			++p;
			continue;

		default:
			yyerror( "illegal type: %c", p[-1] );
			}
		}
	}

struct nlist resw[] = {
	"DCOST",	DCOST,
	"SHAPES",	SHAPES,
	"OPCODES",	OPCODES,
	"USERCOST",	USERCOST,
	"CONVAL",	CONVAL,
	"NCONVAL",	NCONVAL,
	"POSRANGE",	POSRANGE,
	"SGRANGE",	SGRANGE,
	"NONAME",	NONAME,
	"",	-1,
	};

lookup() {
	/* look up the shape name in name, and return the index */
	register STYSHP *p;
	int i;
	for( i=0; resw[i].vop >= 0; ++i ){
		if( !strcmp( name, resw[i].shop ) ) return( resw[i].vop );
		}
	for( i=NSTYSHPS-1, p= &shp[NSTYSHPS-1]; i>nmshp; --i,--p ){
		if( !strncmp( name, p->shname, 8 ) ) { /* match */
			yylval.ival = i;
			return( NM );
			}
		}
	/* new entry */
	strncpy( p->shname, name, 8 );
	p->ssh = p->scost = 0;
	p->sleft = p->sright = -1;
	p->sop = -1;
	yylval.ival = nmshp--;
	if( nmshp <= nshp ) {
		yyerror( "out of node space" );
		exit( 1 );
		}
	return( NM );
	}

sharecost ( sn, cst ) {
	if ( sn == nshp - 1 ){
		shp[sn].scost = cst;
		return(sn);
	}
	if ( cst == 0 )
		return( sn );

	if (nshp >= nmshp )
		yyerror("too many shapes");
	shp[nshp] = shp[sn];
	shp[nshp].scost += cst;
	++nshp;
	checkit( nshp-1 );
	return( nshp-1 );
}
	
uopcost (o, a) {
	switch(o){
	default:
	case MUL:
	case AND:	yyerror("Illegal unary cost operator");
			return(0);
	case MINUS:
		return(-a);
		}
	}

bopcost (o, a, b) {
	switch(o){
	case INCR:
	case PLUS:	return(a+b);
	case DECR:
	case MINUS:	return(a-b);
	case MUL:	return(a*b);
	case DIV:	return(a/b);
	case MOD:	return(a%b);
	case LS:	return(a>>b);
	case RS:	return(a<<b);
	case OR:	return(a|b);
	case ER:	return(a^b);
	case AND:	return(a&b);
	case LT:	return(a<b);
	case GT:	return(a>b);
	case LE:	return(a<=b);
	case GE:	return(a>=b);
	case NE:	return(a!=b);
	case EQ:	return(a==b);
	default:	yyerror("Illegal binary operator");
			return(0);
		}
	}

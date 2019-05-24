# include "mfile2"
# define DB(x) if(((int)x)&01) cerror( "oddpt(%o)", x );

/*	some storage declarations */

# ifdef TWOPASS
int ftnno;  /* number of current function */
#endif

int lflag;
int e2debug;
int udebug;
int fast;

	/* maxtemp is the maximum size (in bits) needed for temps so far */
	/* maxarg is ditto for outgoing arguments */
	/* maxboff is ditto for automatic variables */
	/* earlier attempts to keep these on a per-block basis were silly */
int maxtemp;
int maxarg;
int maxboff;
NODE * condit();

NODE *
force(p) NODE *p; {
	/* take care to get the type right... */
	register NODE *q, *r;
	if( !p ) cerror( "force" );
	p = ind2type(p);  /* force p to be the proper type */
	q = talloc();
	*q = *p;
	r = talloc();
	*r = *p;
	q->tn.op = ASSIGN;
	q->in.right = p;
	q->in.left = r;
	r->tn.op = QNODE;
	r->tn.rval = callreg(p); /* the reg where the value will be forced */
	return( q );
	}

p2init( argc, argv ) char *argv[];{
	/* set the values of the pass 2 arguments */

	register int c;
	register char *cp;
	register files;

	allo0();  /* free all regs */
	files = 0;

	for( c=1; c<argc; ++c ){
		if( *(cp=argv[c]) == '-' ){
			while( *++cp ){
				switch( *cp ){

				case 'X':  /* pass1 flags */
					while( *++cp ) { /* VOID */ }
					--cp;
					break;

				case 'l':  /* linenos */
					++lflag;
					break;

				case 'e':  /* expressions */
					++e2debug;
					break;

				case 'o':  /* orders */
					++odebug;
					break;

				case 'r':  /* register allocation */
					++rdebug;
					break;

				case 's':  /* shapes */
					++sdebug;
					break;

				case 'u':  /* Sethi-Ullman testing
						(machine dependent) */
					++udebug;
					break;

				case 'f':  /* try for faster compile speed */
					++fast;
					break;

				default:
					cerror( "bad option: %c", *cp );
					}
				}
			}
		else files = 1;  /* assumed to be a ftitle */
		}

	mkdope();
	return( files );

	}

NODE *
dlabel( p, l ) NODE *p; {
	/* define a label after p is executed */
	NODE *q;
	if( !p ) cerror( "dlabel" );
	q = talloc();
	q->tn.type = p->tn.type;
	q->in.left = p;
	q->tn.op = GENLAB;
	q->bn.label = l;
	return( q );
	}

NODE *
genbr( o, l, p ) NODE *p; {
	/* after evaluating p, generate a branch to l */
	/* if o is 0, unconditional */
	NODE *q;
	if( !p ) cerror( "genbr" );
	if( l < 0 ) cerror( "genbr1" );
	q = talloc();
	q->tn.op = o?GENBR:GENUBR;
	q->tn.type = p->tn.type;
	q->in.left = p;
	q->bn.label = l;
	q->bn.lop = o;
	if( o && logop(p->tn.op) ) p->tn.op = CMP;
	return( q );
	}

int negrel[] = { NE, EQ, GT, GE, LT, LE, UGT, UGE, ULT, ULE } ;  /* negatives of relationals */

tcond( p ) register NODE *p; {
	/* return 1 if p is always true, 0 otherwise */
	register o = p->tn.op;
	register NODE *q;

	switch( o ) {

	case ICON:
		return( p->tn.lval || p->tn.name[0] );

	case COMOP:
		return( tcond( p->in.right ) );

	case ANDAND:
		return( tcond( p->in.left ) && tcond( p->in.right ) );

	case OROR:
		return( tcond( p->in.left ) || tcond( p->in.right ) );

	case NOT:
		return( fcond( p->in.left ) );

	case QUEST:
		q = p->in.right;
		if( tcond( p->in.left ) ) return( tcond( q->in.left ) );
		if( fcond( p->in.left ) ) return( tcond( q->in.right ) );
		return( tcond( q->in.left ) && tcond( q->in.right ) );

	default:
		return( 0 );
		}
	}

fcond( p ) register NODE *p; {
	/* return 1 if p is always false, 0 otherwise */
	register o = p->tn.op;
	register NODE *q;

	switch( o ) {

	case ICON:
		return( !p->tn.lval && !p->tn.name[0] );

	case COMOP:
		return( fcond( p->in.right ) );

	case ANDAND:
		return( fcond( p->in.left ) || fcond( p->in.right ) );

	case OROR:
		return( fcond( p->in.left ) && fcond( p->in.right ) );

	case NOT:
		return( tcond( p->in.left ) );

	case QUEST:
		q = p->in.right;
		if( tcond( p->in.left ) ) return( fcond( q->in.left ) );
		if( fcond( p->in.left ) ) return( fcond( q->in.right ) );
		return( fcond( q->in.left ) && fcond( q->in.right ) );

	default:
		return( 0 );
		}
	}

NODE *
rcomma( p ) register NODE *p; {
	/* p is a COMOP; return the shrunken version thereof */

	if( p->tn.op != COMOP ) cerror( "rcomma" );

	if( p->in.left && p->in.right ) return( p );
	p->tn.op = FREE;
	if( !p->in.left ) return( p->in.right );
	return( p->in.left );
	}

NODE *
gtb( p, l ) NODE *p; {
	register NODE *q;
	/* replace p by a trivial branch to l */
	/* if l is -1, return NULL */
	DB(p);
	q = condit( p, CEFF, -1, -1 );
	DB(q);
	if( l<0 ) return( q );
	if( !q ) {
		q = talloc();
		q->tn.op = ICON;
		q->tn.lval = 0;
		q->tn.name[0] = '\0';
		q->tn.type = TINT;
		}
	return( genbr( 0, l, q ) );
	}

NODE *
condit( p, goal, t, f ) NODE *p; {
	/* generate code for conditionals in terms of GENLAB and GENBR nodes */
	/* goal is either CEFF, NRGS, or CCC */
	/* also, delete stuff that never needs get done */
	/* if goal==CEFF, return of null means nothing to be done */

	int o, lt, lf, l;
	NODE *q, *q1, *q2;

	o = p->tn.op;

	if( odebug >2 ) {
		printf( "condit( %d (%s), %s, %d, %d )\n", p-node, opst[o],
			goal==CCC?"CCC":(goal==NRGS?"NRGS":"CEFF"),
			t, f );
		}
	if( o == CBRANCH ) {
		p->in.right->tn.op = p->tn.op = FREE;
		l = p->in.right->tn.lval;
		p = p->in.left;
		if( fcond( p ) ) return( gtb(p,l) );
		if( tcond( p ) ) return( gtb(p,-1) );
		return( condit( p, CCC, -1, l ) );
		}

		/* a convenient place to diddle a few special ops */
	if( callop(o) ){
		if( optype(o) == UTYPE ) p->stn.argsize = 0;
		else p->stn.argsize = argsize(p->in.right);
		if( p->stn.argsize > maxarg ) maxarg = p->stn.argsize;
		if( goal==CEFF ) goal = NRGS;
		/* flow on, so that we can handle if( f(...) )... */
		}
	else if( goal==CEFF && (asgop(o) || o==STASG || o==INIT)) goal=NRGS;

	/* do a bit of optimization */

	if( goal == NRGS ) {
		if( logop(o) ){
			/* must make p into ( p ? 1 : 0 ), then recompile */
			q1 = talloc();
			q1->tn.op = ICON;
			q1->tn.name[0] = '\0';
			q1->tn.lval = 1;
			q1->tn.type = TINT;
			q2 = talloc();
			*q2 = *q1;
			q2->tn.lval = 0;
			q = talloc();
			q->tn.op = COLON;
			q->tn.type = TINT;
			q->in.left = q1;
			q->in.right = q2;
			q1 = talloc();
			q1->tn.op = o = QUEST;
			q1->tn.type = TINT;
			q1->in.left = p;
			q1->in.right = q;
			p = q1;  /* flow on, and compile */
			}
		}

	if( goal != CCC ) {
		if( o == QUEST ) {
			/* rewrite ? : when goal not CCC */
			lf = getlab();
			l = getlab();
			p->tn.op = COMOP;
			q = p->in.right;
			q1 = condit( q->in.left, goal, -1, -1 );
			q->in.right = condit( q->in.right, goal, -1, -1 );
			if( tcond( p->in.left ) ) {
				q->tn.op = FREE;
				tfree( q->in.right );
				p->in.right = q1;
				p->in.left=condit( p->in.left, CEFF, -1, -1 );
				return( rcomma( p ) );
				}
			if( fcond( p->in.left ) ) {
				q->tn.op = FREE;
				tfree( q1 );
				p->in.right = q->in.right;
				p->in.left=condit( p->in.left, CEFF, -1, -1 );
				return( rcomma( p ) );
				}
			if( !q1 ) {
				if( !q->in.right ) {
					/* nothing to do */
					tfree( p->in.left );
					p->tn.op = q->tn.op = FREE;
					return( q1 );
					}
				/* rhs done if condition is false */
				p->in.left = condit( p->in.left, CCC, l, -1 );
				p->in.right = dlabel( q->in.right, l );
				q->tn.op = FREE;
				return( p );
				}
			else if( !q->in.right ) {
				/* lhs done if condition is true */
				p->in.left=condit( p->in.left, CCC, -1, lf );
				p->in.right = dlabel( q1, lf );
				q->tn.op = FREE;
				return( p );
				}

			/* both sides exist and the condition is nontrivial */
			p->in.left = condit( p->in.left, CCC, -1, lf );
			q1 = force(q1);
			q->in.right = force(q->in.right);
			q1 = genbr( 0, l, q1 );
			q->in.left = dlabel( q1, lf );
			q->tn.op = COMOP;
			return( dlabel( p, l ) );
			}

		if( goal == CEFF ) {
			/* some things may disappear */
			switch( o ) {

			case CBRANCH:
			case GENBR:
			case GENUBR:
			case CALL:
			case UNARY CALL:
			case FORTCALL:
			case UNARY FORTCALL:
			case STCALL:
			case UNARY STCALL:
			case STASG:
			case INIT:
			case MOD:   /* do these for the side effects */
			case DIV:
			case UOP0:
			case UOP1:
			case UOP2:
			case UOP3:
			case UOP4:
			case UOP5:
			case UOP6:
			case UOP7:
			case UOP8:
			case UOP9:
				goal = NRGS;
				}
			}

		switch( optype(o) ) {
		case LTYPE:
			if( goal == CEFF ) {
				p->tn.op = FREE;
				return( NIL );
				}
			break;

		case BITYPE:
			p->in.right = condit( p->in.right, goal, -1, -1 );
		case UTYPE:
			p->in.left = condit( p->in.left, o==COMOP?CEFF:goal,
				-1, -1 );
			}

		if( goal == CEFF || o==COMOP ) {
			/* lhs or rhs may have disappeared */
			/* op need not get done */

			switch( optype(o) ){

			case BITYPE:
				p->tn.op = COMOP;
				return( rcomma( p ) );

			case UTYPE:
				p->tn.op = FREE;
				return( p->in.left );

			case LTYPE:
				p->tn.op = FREE;
				return( NIL );
				}
			}
		return( p );
		}

	/* goal must = CCC from here on */

	switch( o ) {

	case ULE:
	case ULT:
	case UGE:
	case UGT:
	case EQ:
	case NE:
	case LE:
	case LT:
	case GE:
	case GT:
		if(t<0 ) {
			o = p->tn.op = negrel[o-EQ];
			t = f;
			f = -1;
			}

#ifndef NOOPT
		if( p->in.right->in.op == ICON &&
			p->in.right->tn.lval == 0 &&
			p->in.right->in.name[0] == '\0' ) {

			/* the question here is whether we can assume that */
			/* unconditional branches preserve condition codes */
			/* if this turned out to be no, we would have to */
			/* explicitly handle this case here */

			switch( o ) {

			case UGT:
			case ULE:
				o = p->in.op = (o==UGT)?NE:EQ;
			case EQ:
			case NE:
			case LE:
			case LT:
			case GE:
			case GT:
				if( logop( p->in.left->tn.op ) ){
					/* situation like (a==0)==0 */
					/* ignore optimization */
					goto noopt;
					}
				break;

			case ULT:  /* never succeeds */
				return( gtb( p, f ) );

			case UGE:
				/* always succeeds */
				return( gtb( p, t ) );
				}
			p->tn.op = p->in.right->tn.op = FREE;
			p = condit( p->in.left, NRGS, -1, -1 );
			p = genbr( o, t, p );
			if( f<0 ) return( p );
			else return( genbr( 0, f, p ) );
			}
		noopt:
# endif

		p->in.left = condit( p->in.left, NRGS, -1, -1 );
		p->in.right = condit( p->in.right, NRGS, -1, -1 );
		p = genbr( o, t, p );
		if( f>=0 ) p = genbr( 0, f, p );
		return( p );

	case COMOP:
		p->in.left = condit( p->in.left, CEFF, -1, -1 );
		p->in.right = condit( p->in.right, CCC, t, f );
		return( rcomma( p ) );

	case NOT:
		p->tn.op = FREE;
		return( condit( p->in.left, CCC, f, t ) );

	case ANDAND:
		lf = f<0 ? getlab() : f;
		p->tn.op = COMOP;
		if( tcond( p->in.left ) ){
			p->in.left = condit( p->in.left, CEFF, -1, -1 );
			}
		else p->in.left = condit( p->in.left, CCC, -1, lf );
		if( tcond( p->in.right ) ){
			p->in.right = condit( p->in.right, CEFF, -1, -1 );
			}
		else p->in.right = condit( p->in.right, CCC, t, f );
		p = rcomma( p );
		if( f<0 ) p = dlabel( p, lf );
		return( p );
		
	case OROR:
		lt = t<0 ? getlab() : t;
		p->tn.op = COMOP;
		if( fcond( p->in.left ) ){
			p->in.left = condit( p->in.left, CEFF, -1, -1 );
			}
		else p->in.left = condit( p->in.left, CCC, lt, -1 );
		if( fcond( p->in.right ) ){
			p->in.right = condit( p->in.right, CEFF, -1, -1 );
			}
		else p->in.right = condit( p->in.right, CCC, t, f );
		p = rcomma( p );
		if( t<0 ) p = dlabel( p, lt );
		return( p );

		/* do ICON here, someday ... */

	case QUEST:
		lf = f<0 ? getlab() : f;
		lt = t<0 ? getlab() : t;
		p->in.left = condit( p->in.left, CCC, -1, l=getlab() );
		q = p->in.right;
		q1 = condit( q->in.left, goal, lt, lf );
		q->in.left = dlabel( q1, l );
		q->in.right = condit( q->in.right, goal, t, f );
		p->tn.op = COMOP;
		q->tn.op = COMOP;
		if( t<0 ) p = dlabel( p, lt );
		if( f<0 ) p = dlabel( p, lf );
		return( p );

	default:
		/* get the condition codes, generate the branch */
		switch( optype(o) ){
	
		case BITYPE:
			p->in.right = condit( p->in.right, NRGS, -1, -1 );
		case UTYPE:
			p->in.left = condit( p->in.left, NRGS, -1, -1 );
			}
		if( t>=0 ) p = genbr( NE, t, p );
		if( f>=0 ) p = genbr( (t>=0)?0:EQ, f, p );
		return( p );
		}
	}

# ifndef TWOPASS

p2compile( p ) NODE *p; {

	if( lflag ) lineid( lineno, ftitle );
	tmpoff = 0;  /* expression at top level reuses temps */
	/* generate code for the tree p */

# ifdef MYREADER
	MYREADER(p);  /* do your own laundering of the input */
# endif
	/* eliminate the conditionals */
# ifndef NODBG
	if( p && odebug>2 ) e2print(p);
# endif
	p = condit( p, CEFF, -1, -1 );
	if( p ) {
		/* expression does something */
		/* generate the code */
# ifndef NODBG
		if( odebug>2 ) e2print(p);
# endif
		codgen( p );
		}
# ifndef NODBG
	else if( odebug>1 ) printf( "null effect\n" );
# endif
	allchk();
	/* tcheck will be done by the first pass at the end of a ftn. */
	/* first pass will do it... */
	}

p2bbeg( aoff, myreg ) {
	static int myftn = -1;
	SETOFF( aoff, ALSTACK );
	if( myftn != ftnno ){ /* beginning of function */
		maxboff = aoff;
		myftn = ftnno;
		maxtemp = 0;
		maxarg = 0;
		}
	else {
		if( aoff > maxboff ) maxboff = aoff;
		}
# ifdef SETREGS
	SETREGS(myreg);
# endif
	}

p2bend(){
	SETOFF( maxboff, ALSTACK );
	SETOFF( maxarg, ALSTACK );
	SETOFF( maxtemp, ALSTACK );
	eobl2();
	maxboff = maxarg = maxtemp = 0;
	}

# endif

char *cnames[] = {
	"CEFF",
	"NRGS",
	"CCC",
	0,
	};

prgoal( goal ) {

	/* print a nice-looking description of goal */

	int i, flag;

	flag = 0;
	for( i=0; cnames[i]; ++i ){
		if( goal & (1<<i) ){
			if( flag ) printf( "|" );
			++flag;
			printf( cnames[i] );
			}
		}
	if( !flag ) printf( "?%o", goal );

	}

#ifndef NODBG
e2print( p ) NODE *p; {
	printf( "\n********* costs=(0,...,NRGS;EFF;TEMP;CC)\n" );
	e22print( p );
	}

e22print( p ) NODE *p; {
	static down=0;
	register ty;

	ty = optype( p->tn.op );
	if( ty == BITYPE ){
		++down;
		e22print( p->in.right );
		--down;
		}
	e222print( down, p );

	if( ty != LTYPE ){
		++down;
		e22print( p->in.left );
		--down;
		}
	}

e222print( down, p ) NODE *p; {
	/* print one node */
	int i, d;

	for( d=down; d>1; d -= 2 ) printf( "\t" );
	if( d ) printf( "    " );

	printf( "%d) %s", p-node, opst[p->in.op] );
	switch( p->in.op ) { /* special cases */

	case REG:
		printf( " %s", rnames[p->tn.rval] );
		break;

	case ICON:
	case NAME:
	case VAUTO:
	case VPARAM:
	case TEMP:
		printf( " " );
		adrput( p );
		break;

	case STCALL:
	case UNARY STCALL:
		printf( " args=%d", p->stn.argsize );
	case STARG:
	case STASG:
		printf( " size=%d", p->stn.stsize );
		printf( " align=%d", p->stn.stalign );
		break;

	case GENBR:
		printf( " %d (%s)", p->bn.label, opst[p->bn.lop] );
		break;

	case CALL:
	case UNARY CALL:
		printf( " args=%d", p->stn.argsize );
		break;

	case GENUBR:
	case GENLAB:
		printf( " %d", p->bn.label );
		break;

	case FUNARG:
		printf( " offset=%d", p->tn.rval );

		}

	printf( ", " );
	t2print( p->in.type );
	printf( ", c=[" );
	for( i=0; i<NRGS; ++i ) cprt( p->in.cst[i], "," );
	cprt( p->in.cst[NRGS], "; " );
	cprt( p->in.cst[CEFF], "; " );
	cprt( p->in.cst[CTEMP], "; " );
	cprt( p->in.cst[CCC], "]" );
	if( p->tn.goal == CEFF ) printf( " (EFF)\n" );
	else if( p->tn.goal == CCC ) printf( " (CC)\n" );
	else if( p->tn.goal != NRGS ) printf( "(BAD GOAL: %d)\n", p->tn.goal );
	else printf( "\n" );
	}

t2print( t ) TWORD t; {
	int i;
	static struct {
		TWORD mask;
		char * string;
		} t2tab[] = {
			TANY, "ANY",
			TINT, "INT",
			TUNSIGNED, "UNSIGNED",
			TCHAR, "CHAR",
			TUCHAR, "UCHAR",
			TSHORT, "SHORT",
			TUSHORT, "USHORT",
			TLONG, "LONG",
			TULONG, "ULONG",
			TFLOAT, "FLOAT",
			TDOUBLE, "DOUBLE",
			TPOINT, "POINTER",
			TPOINT2, "POINTER2",
			TSTRUCT, "STRUCT",
			TVOID, "VOID",
			0, 0
			};

	for( i=0; t && t2tab[i].mask; ++i ) {
		if( (t&t2tab[i].mask) == t2tab[i].mask ) {
			printf( " %s", t2tab[i].string );
			t ^= t2tab[i].mask;
			}
		}
	}

# else
e2print( p ) NODE *p; {
	werror( "e2print not compiled" );
	}
e222print( down, p ) NODE *p; {
	werror( "e222print not compiled" );
	}
# endif
cprt( c, s ) char *s; {
	if( c >= INFINITY ) printf( "*%s", s );
	else printf( "%d%s", c, s );
	}

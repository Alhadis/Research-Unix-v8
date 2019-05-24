# include "mfile1"

/*	this file contains code which is dependent on the target machine */
/*	the following used to be on the old local.c file */

/* the register variables are %d2 through %d7 and %a2 through %a5 */
/* everything is saved on the stack as a 4 byte quantity */
/* the machine-independent part maintains a quantity regvar that is */
/* saved and restored across block boundaries */
/* if cisreg likes the type, it calls rallc to get a register number */
/* rallc puts the register number into nextrvar, and updates regvar */
/* rallc returns 0 if there are no free regs of that type, else 1 */

fincode( d, sz ) double d; {
	/* initialization of floats: treat like vax */
	union {
		float f;
		long l;
		struct { short slo, shi; } sh;
		} x;
	int z;
	x.f = d;
	z = x.sh.slo;
	x.sh.slo = x.sh.shi;
	x.sh.shi = z;
	printf( "	long	%d\n", x.l );
	inoff += sz;
	}

rallc( t ) TWORD t; {
	int i;
	if( ISPTR( t ) ){
		/* address register */
		for( i=2; i<=5; ++i ){
			if( regvar & (0400<<i) ) continue;
			nextrvar = i;
			regvar |= (0400<<i);
			return(1);
			}
		return(0);
		}
	else {
		/* data register */
		for( i=2; i<=7; ++i ){
			if( regvar & (1<<i) ) continue;
			nextrvar = i;
			regvar |= (1<<i);
			return( 1 );
			}
		return(0);
		}
	}

NODE *
clocal(p) NODE *p; { return(p); }

myinit( p, sz ) NODE *p; { 
	TWORD t;
	NODE *q;
	if( p->in.left->tn.op != ICON ) yyerror( "illegal initializer" );
	t = p->tn.type;
	q = p->in.left;
	inoff += sz;
	if( t == CHAR || t == UCHAR ) {
		printf( "	byte	%d\n", (int) q->tn.lval );
		return;
		}
	if( t == INT || t == UNSIGNED ) {
		printf( "	short	%d\n", (int) q->tn.lval );
		return;
		}
	/* do pointers, longs in pass 2 */
	ecode( p );
	}

cisreg( t ) TWORD t; { /* is an automatic variable of type t OK for a register variable */

	if( (t==LONG||t==ULONG||t==INT||t==UNSIGNED||t==CHAR||t==UCHAR
		||ISPTR(t)) && rallc(t) ) return(1);
	return(0);
	}

static inwd	/* current bit offsed in word */;
static word	/* word being built from fields */;

incode( p, sz ) register NODE *p; {
	register v;

	/* generate initialization code for assigning a constant c
		to a field of width sz */
	/* we assume that the proper alignment has been obtained */
	/* inoff is updated to have the proper final value */
	/* we also assume sz  < SZINT */

	if((sz+inwd) > SZINT) cerror("incode: field > int");
	/* right to left for the 68000 */
	v = p->tn.lval & ((1<<sz)-1);
	if( SZINT-inwd-sz ) v <<= (SZINT-inwd-sz);
	word |= v;
	inwd += sz;
	inoff += sz;
	if(inoff%SZINT == 0) {
		printf( "	short	0%o\n", word);
		word = inwd = 0;
		}
	}

vfdzero( n ){ /* define n bits of zeros in a vfd */

	if( n <= 0 ) return;

	inwd += n;
	inoff += n;
	if( inoff%ALINT ==0 ) {
		printf( "	short	0%o\n", word );
		word = inwd = 0;
		}
	}

char *
exname( p ) char *p; {
	/* make a name look like an external name in the local machine */
	/* better not have two calls outstanding at once! */
	static char text[NCHNAM+1];
	strncpy( text, p, NCHNAM );
	return( text );
	}

commdec( id ){ /* make a common declaration for id, if reasonable */
	register struct symtab *q;
	int sz;

	q = &stab[id];
	sz = tsize( q->stype, q->dimoff, q->sizoff )/SZCHAR;

	if( q->sclass==STATIC ) {
		/* always local */
		if( q->slevel ) printf( "\tlcomm\tL%%%d,%ld\n", q->offset, sz );
		else printf( "\tlcomm\t%s,%ld\n", exname( q->sname ), sz );
		}
	else printf( "	comm	%s,%ld\n", exname( q->sname ), sz );
	}

/* the following is the stuff on the old code.c file */

branch( n ){
	/* output a branch to label n */
	if( !reached ) return;
	printf( "	br	L%%%d\n", n );
	}

deflab( n ){
	/* output something to define the current position as label n */
	printf( "L%%%d:\n", n );
	}

defalign(n) {
	/* cause the alignment to become a multiple of n */
	n /= SZCHAR;
	if( curloc != PROG && n > 1 ) printf( "	even\n" );
	}

char *locnames[] = {
	/* location counter names for PROG, ADATA, DATA, ISTRNG, STRNG */
	"	text\n",
	0,
	"	data	1\n",
	0,
	"	data	2\n",
	};

efcode(){
	/* code for the end of a function */

	deflab( retlab );  /* define the return location */

	if( strftn ){  /* copy output (in r0) to caller */
		printf( "	mov.l	&__StRet,%%a0\n" );
		}
	printf( "	movm.l	S%%%d(%%fp),&M%%%d\n", ftnno, ftnno );
	printf( "	unlk	%%fp\n" );
	printf( "	rts\n" );
	}

bfcode( a, n ) int a[]; {
	/* code for the beginning of a function; a is an array of
		indices in stab for the arguments; n is the number */

	retlab = getlab();

	/* routine prolog */

	printf( "	link	%%fp,&F%%%d\n", ftnno );
/* no longer do we push the mask...
	printf( "	mov.w	&M%%%d,-2(%%fp)\n", ftnno );
*/
	printf( "	movm.l	&M%%%d,S%%%d(%%fp)\n", ftnno, ftnno );
	}

defnam( p ) register struct symtab *p; {
	/* define the current location as the name p->sname */
	if( p->sclass == EXTDEF ){
		printf( "	global	%s\n", exname( p->sname ) );
		}
	printf( "%s:\n", exname( p->sname ) );
	}

bycode( t, i ){
	/* put byte i+1 in a string */

	i &= 07;
	if( t < 0 ){ /* end of the string */
		if( i != 0 ) printf( "\n" );
		}

	else { /* stash byte t into string */
		if( i == 0 ) printf( "	byte	" );
		else printf( "," );
		printf( "0%o", t );
		if( i == 07 ) printf( "\n" );
		}
	}

zecode( n ){
	/* n integer words of zeros */
	OFFSZ temp;

	if( n <= 0 ) return;
	printf( "	space	%d\n", 2*n );
	temp = n;
	inoff += temp*SZINT;
	}

main( argc, argv ) char *argv[]; {
	int r;
	char errbuf[BUFSIZ];

	/* for( r=1; r<argc; ++r ) {
		/* if( argv[r][0] == '-' ) continue;
		/* /* first file argument: input */
		/* if( !freopen( argv[r], "r", stdin ) ) {
			/* uerror( "cannot open input file: %s", argv[r] );
			/* exit( 1 );
			/* }
		/* }
	/* for( ++r; r<argc; ++r ) {
		/* if( argv[r][0] == '-' ) continue;
		/* /* second file argument: output */
		/* if( !freopen( argv[r], "w", stdout ) ) {
			/* uerror( "cannot open output file: %s", argv[r] );
			/* exit( 1 );
			/* }
		/* }
	*/
	setbuf(stderr, errbuf);	/* rob */
	r = mainp1( argc, argv );

	return( r );
	}

genswitch(p,n) register struct sw *p;{
	/*	p points to an array of structures, each consisting
		of a constant value and a label.
		The first is >=0 if there is a default label;
		its value is the label number
		The entries p[1] to p[n] are the nontrivial cases
		*/
	register i;
	register long j, range;
	register dlab, swlab;

	range = p[n].sval-p[1].sval;

	if( range>0 && range <= 3*n && n>=5 ){ /* implement a direct switch */

		dlab = p->slab >= 0 ? p->slab : getlab();

		if( p[1].sval ){
			printf( "	sub.w	&%ld,%%d0\n", p[1].sval );
			}

		/* note that this is a logical compare; it thus checks
		   for numbers below range as well as out of range.
		   */
		printf( "	cmp.w	%%d0,&%ld\n", range );
		printf( "	bhi	L%%%d\n", dlab );

		printf( "	add.w	%%d0,%%d0\n" );
		printf( "	mov.w	6(%%pc,%%d0.w),%%d0\n" );
		printf( "	jmp	2(%%pc,%%d0.w)\n" );
		/* output table */

		deflab( swlab = getlab() );

		for( i=1,j=p[1].sval; i<=n; ++j ){

			printf( "	short	L%%%d-L%%%d\n",
				 (j == p[i].sval) ? p[i++].slab : dlab, swlab );
			}

		if( p->slab< 0 ) deflab( dlab );
		return;
		}


	/* debugging code */

	/* out for the moment
	if( n >= 4 ) werror( "inefficient switch: %d, %d", n, (int) (range/n) );
	*/

	/* simple switch code */

	for( i=1; i<=n; ++i ){
		/* already in 0 */

		if( p[i].sval ) printf( "\tcmp.w\t%%d0,&%ld\n", p[i].sval );
		else printf( "\ttst.w\t%%d0\n" );
		printf( "	beq	L%%%d\n", p[i].slab );
		}

	if( p->slab>=0 ) printf( "	br	L%%%d\n", p->slab );
	}

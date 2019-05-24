# include "mfile2"
/* a lot of the machine dependent parts of the second pass */

int rsmask;

	/* the stack frame looks as follows:

	incoming args
	saved PC (4 bytes)
	saved old FP (%a6) (4 bytes)
fp->
	saved regs (the mask is M%n)
	autos (maxboff keeps track of this)
	temps (maxtemp keeps track of this)
	outgoing args (4 byte hole left for the first one )
sp->

	the first saved reg is at S%n(%fp)
	the last auto ends at S%n(%fp)
	the last temp ends at T%n(%fp)
	the first incoming arg is at 8(%fp)
	F%n is the size of the frame (argument to "link" instruction)
	*/

lineid( l, fn ) char *fn; {
	/* identify line l and file fn */
	printf( "#	line %d, file %s\n", l, fn );
	}

eobl2(){
	int off;
	int rsave;
	int i, m;
	/* count bits in rsmask */
	for( rsave=i=0, m=1; i<16; ++i, m<<=1 ) {
		if( m&rsmask ) rsave += 4;
		}
	maxboff /= SZCHAR;
	maxtemp /= SZCHAR;
	maxarg /= SZCHAR;

	off = rsave;
	printf( "	set	S%%%d,%d\n", ftnno, -off );
	off += maxboff;
	printf( "	set	T%%%d,%d\n", ftnno, -off );
	off += maxtemp+4;  /* 4 byte hole for last arg */
	printf( "	set	F%%%d,%d\n", ftnno, -off );
	printf( "	set	M%%%d,0%o\n", ftnno, rsmask );
	rsmask = 0;
	}

char *
rnames[]= {  /* keyed to register number tokens */

	"0", "1", "2", "3", "4", "5", "6", "7", "?8", "?9", "?10"
	};

special( sh, p ) NODE *p; {
	cerror( "special called!" );
	return( INFINITY );
	}

int toff;
int toffbase;

popargs( size ) {
	/* pop arguments from the stack */
	toff -= size/2;
	if( toff == 0 ) {
		size -= 2*toffbase;
		toffbase = toff = 0;
		}
	if( size ) printf( "	add.l	&%d,%%sp\n", size );
	}

zzzcode( p, ppc ) NODE *p; char **ppc; {
	int c, ii;

	switch( c= *++(*ppc) ){

	case 'p':
		popargs( 4 );
		return;

	case 'q':
		popargs( 8 );
		return;

	case 'r':
		popargs( 2 );
		return;

	case 'c':
		popargs( p->stn.argsize/SZCHAR );
		return;

	case '1':
		if( toff ) {
			putchar( '-' );
			}
		printf( "(%%sp)" );
		if( !toff ) toffbase = 1;
		++toff;
		return;

	case '2':
		if( toff ) {
			putchar( '-' );
			}
		printf( "(%%sp)" );
		if( !toff ) toffbase = 2;
		toff+=2;
		return;

	case '0':
		/* like 2 but doesn't print -(%%sp) */
		/* used for pea */  /* leaves toffbase at 0, also */
		toff += 2;
		return;

	case 's':  /* like s, but needs value afterwards */
	case 'S':  /* structure assignment */
		if( p->in.op == STASG ){
			stas( p, p->stn.stsize, c );
			}
		else if( p->in.op == STARG ){  /* store an arg onto the stack */
			star( p->in.left, p->stn.stsize );
			}
		else cerror( "ZS bad" );
		break;

	case 'z':  /* adjust register on right */
		{	int adj;
			if( p->stn.stsize == SZINT || p->stn.stsize == SZLONG )
				return;
			adj = ((p->stn.stsize+SZSHORT)/SZLONG) - 1;
			if( adj ) {
				printf( "\tsub.l\t&%ld,%%a%d\n", 4*adj,
					p->in.right->tn.rval );
				}
			}
		return;

	case 'I':
		cbgen( p->bn.lop, p->bn.label, c );
		return;

	case 'i':
		c = p->in.left->in.left->tn.rval;  /* register number */
		dbccgen( p->bn.lop, p->bn.label, c );
		return;

	case 'b':
		++*ppc;
		p = getadr(p,ppc);
		if((ii = ispow2(p->tn.lval))>=0) {
			printf( "&%d", ii );
			break;
			}
		cerror( "Zb but not power of 2" );

	default:
		cerror( "illegal zzzcode: %c", c );
		}
	}

star( p, sz ) NODE *p; {
	/* put argument onto the stack */
	/* p is a REG */

	sz /= SZCHAR;

	if( p->tn.op != REG ) uerror( "star of non-REG" );
	while( (sz -= 4) >= 0 ) {
		printf( "\tmov.l\t%d", sz );
		expand( p, INREG, "U.,Z2\n" );
		}
	if( sz == -2 ) expand( p, INREG, "\tmov.w\t0U.,Z1\n" );
	else if( sz != -4 ) cerror( "ZS sz %d", sz );
	}

stas( p, sz, c ) NODE *p; {
	/* p is an STASG node; left and right are regs */
	/* all structures are aligned on int boundaries */

	int rn, ln, zz;

	rn = p->in.right->tn.rval;
	if( c == 's' ) { /* lhs is a temp moved into a reg: kluge! */
		ln = getlr( p, '1' )->tn.rval;  /* A1 */
		}
	else ln = p->in.left->tn.rval;

	for( zz = (sz /= SZCHAR); sz > 4; sz -= 4 ) {
		printf( "\tmov.l\t(%%a%d)+,(%%a%d)+\n", rn, ln );
		}

	if( sz == 4 ) printf( "\tmov.l\t0(%%a%d),0(%%a%d)\n", rn, ln );
	else if( sz == 2 ) printf( "\tmov.w\t0(%%a%d),0(%%a%d)\n", rn, ln );
	else cerror( "ZS sz %d", sz );
	if( ln > 1 ) { /* restore clobbered register variable */
		if( zz > sz ) printf( "\tsub.l\t&%d,%%a%d\n", zz-sz, ln);
		}
	if( rn > 1 ) { /* restore clobbered register variable */
		if( zz > sz ) printf( "\tsub.l\t&%d,%%a%d\n", zz-sz, rn);
		}
	}

conput( p ) register NODE *p; {

	switch( p->in.op ){

	case ICON:
		acon( p );
		return;

	case REG:
		if(p->tn.type&TPOINT ) printf( "%%a" );
		else printf( "%%d" );
		printf( "%s", rnames[p->tn.rval] );
		if( p->tn.rval > 7 ) werror( "bad register output" );
		return;

	default:
		cerror( "illegal conput" );
		}
	}

insput( p ) NODE *p; {
	cerror( "insput" );
	}

upput( pp ) NODE *pp; {
	/* output the address of the second word in the
	   pair pointed to by p (for LONGs)*/
	long v;
	NODE *r, *l, *p;

	v=0;
	p = pp;
	if( p->tn.op == PLUS ) {
		v=0;
		r = p->in.right;
		l = p->in.left;
		if( r->tn.op == ICON &&  l->tn.op == PLUS ){
			v = r->tn.lval;
			p = l;
			r = p->in.right;
			l = p->in.left;
			}
		if( r->tn.op == REG ) {
			l = p->in.left;
			if( l->tn.op == REG ){
				printf( "%ld(%%a%d,%%d%d.l)", v, r->tn.rval,
					l->tn.rval );
				return;
				}
			if( l->tn.op != CONV ) goto ill;
			l = l->in.left;
			if( l->tn.op != REG ) goto ill;
			printf( "%ld(%%a%d,%%d%d.w)", v, r->tn.rval, l->tn.rval 
				);
			return;
			}
		if( r->tn.op != ICON ) goto ill;
		v = r->tn.lval;
		p = p->in.left;
		}
	else if( p->tn.op == MINUS ){
		r = p->in.right;
		if( r->tn.op != ICON || r->in.name[0] ) goto ill;
		v = -r->tn.lval;
		p = p->in.left;
		}
	else if( p->tn.op == ASG MINUS ) {
		r = p->in.right;
		if( r->tn.op != ICON ) goto ill;
		r = p->in.left;
		if( r->tn.op != REG ) goto ill;
		/* always do the side effect */
		else printf( "-(%%a%s)", rnames[r->tn.rval] );
		sideff = 1;  /* cream it */
		return;
		}
	else if( p->tn.op == INCR ) {
		r = p->in.right;
		if( r->tn.op != ICON ) goto ill;
		r = p->in.left;
		if( r->tn.op != REG ) goto ill;
		if( sideff ) printf( "(%%a%s)+", rnames[r->tn.rval] );
		else printf( "(%%a%s)", rnames[r->tn.rval] );
		return;
		}
	if( p->tn.op != REG ) goto ill;
	if( v )
		printf( "%ld(%%a%s)", v, rnames[p->tn.rval] );
	else
		printf( "(%%a%s)", rnames[p->tn.rval] );
	return;

	ill:
	e2print(pp);
	cerror( "illegal address: upput" );

	}

adrput( p ) register NODE *p; {
	/* output an address, with offsets, from p */

	while( p->in.op == FLD || p->in.op == CONV ){
		p = p->in.left;
		}
	switch( p->in.op ){

	case NAME:
		acon( p );
		sideff = 0;
		return;

	case ICON:
		/* addressable value of the constant */
		printf( "&" );
		acon( p );
		sideff = 0;
		return;

	case REG:
		conput( p );
		sideff = 0;
		return;

	case STAR:
		upput( p->in.left );
		return;

	case VAUTO:
		printf( "%ld+S%%%d(%%fp)", p->tn.lval, ftnno );
		sideff = 0;
		return;

	case VPARAM:
		printf( "%ld(%%fp)", p->tn.lval );
		sideff = 0;
		return;

	case TEMP:
		printf( "%ld+T%%%d(%%fp)", p->tn.lval, ftnno );
		sideff = 0;
		return;

	default:
		e2print(p);
		cerror( "illegal address" );
		return;
		}
	}

acon( p ) register NODE *p; { /* print out a constant */

	if( p->in.name[0] == '\0' ){	/* constant only */
		printf( "%ld", p->tn.lval);
		}
	else if( p->tn.lval == 0 ) {	/* name only */
		printf( "%.8s", p->in.name );
		}
	else {				/* name + offset */
		printf( "%.8s+%ld", p->in.name , p->tn.lval );
		}
	}

char *
ccbranches[] = {
	"	beq	L%%%d\n",
	"	bne	L%%%d\n",
	"	ble	L%%%d\n",
	"	blt	L%%%d\n",
	"	bge	L%%%d\n",
	"	bgt	L%%%d\n",
	"	bls	L%%%d\n",
	"	blo	L%%%d\n",
	"	bhs	L%%%d\n",
	"	bhi	L%%%d\n",
	};

cbgen( o, lab, mode ) { /*   printf conditional and unconditional branches */
	if( o == 0 ) printf( "	br	L%%%d\n", lab );
	else if( o > UGT ) cerror( "bad conditional branch: %s", opst[o] );
	else printf( ccbranches[o-EQ], lab );
	}

char *
dbccops[] = {
	"	dbeq	%%d%d,L%%%d\n",
	"	dbne	%%d%d,L%%%d\n",
	"	dble	%%d%d,L%%%d\n",
	"	dblt	%%d%d,L%%%d\n",
	"	dbge	%%d%d,L%%%d\n",
	"	dbgt	%%d%d,L%%%d\n",
	"	dbls	%%d%d,L%%%d\n",
	"	dblo	%%d%d,L%%%d\n",
	"	dbhs	%%d%d,L%%%d\n",
	"	dbhi	%%d%d,L%%%d\n",
	};

dbccgen( o, lab, reg ) { /*   printf conditional and unconditional branches */
	if( o == 0 ) printf( "	dbt	%%d%d,L%%%d\n", reg, lab );
	else if( o > UGT ) cerror( "bad conditional branch: %s", opst[o] );
	else printf( dbccops[o-EQ], reg, lab );
	}

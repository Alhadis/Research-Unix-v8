# include "mfile1"

# define ISCON(p) (p->in.op==ICON)

int opdebug = 0;
NODE *doptim();

NODE *
aadjust( p, adj ) NODE *p; {
	/* try to adjust p by adj bits */
	NODE *q;
	adj = BITOOR(adj);
	switch( p->tn.op ){

	case ICON:
		p->tn.lval += adj;
		return( p );

	default:
		/* construct a + node */
	mkplus:
		return( block( PLUS, p, bcon(adj), p->fn.type,
			p->fn.cdim, p->fn.csiz ) );

	case PLUS:
		q = p->in.right;
		if( q->tn.op != ICON ) goto mkplus;
		q->tn.lval += adj;
		return( p );

	case MINUS:
		q = p->in.right;
		if( q->tn.op != ICON ) goto mkplus;
		q->tn.lval -= adj;
		return( p );
		}
	}

adjust( p, adj ) NODE *p; {
	/* handle adjustment of scalars by adj bits */

	switch( p->tn.op ){

	case NAME:
	case VAUTO:
	case VPARAM:
		p->tn.lval += BITOOR(adj);
		return( 1 );

	case STAR:
		p->in.left = aadjust( p->in.left, adj );
		return( 1 );

	default:
		return( 0 );
		}
	}

# ifdef NOSIMPSTR
# ifndef MYSIMPSTR
simpstr( d, s ) {  return( STRTY ); }
# endif
# else
TWORD
simpstr( d, s ) {
	/* return STRTY if not, and CHAR, INT, SHORT, or LONG if simple */
	register sz, al;

	sz = tsize( STRTY, d, s );
	al = talign( STRTY, s );
	if( sz == SZINT && !( al % ALINT) ) return( INT );
	else if( sz == SZCHAR && !( al % ALCHAR) ) return( CHAR );
	else if( sz == SZLONG && !( al % ALLONG) ) return( LONG );
	else if( sz == SZSHORT && !( al % ALSHORT) ) return( SHORT );
	return( STRTY );
	}
# endif

tydown( p ) NODE *p; {
	/* reflect the type of p downwards, as appropriate */
	/* returns 1 if it makes a real changed */
	TWORD t;
	NODE *l, *r;
	int flag;

	if( opdebug ) {
		printf( "tydown(%d) called with:\n", p-node );
		eprint( p );
		}
	t = p->tn.type;
	if( ISPTR(t) || ISARY(t) ) return(0);

	/* work these types down into the tree */

	flag = 0;

	switch( p->tn.op ) {

	case AND:
	case OR:
	case PLUS:
	case MINUS:
	case ER:
		if( opdebug ) {
			printf( "tydown:\n" );
			eprint( p );
			}
		r = p->in.right;
		if( bigsize(r->tn.type) > bigsize(p->tn.type ) ) {
			r = makety( r, t, 0, t );
			tydown( r );
			p->in.right = doptim( r );
			if( opdebug ) {
				printf( "tydown(%d), after doptim(R):\n",
					p-node );
				eprint(p);
				}
			flag = 1;
			}
		/* FALLTHRU */

	case UNARY MINUS:
	case COMPL:
		l = p->in.left;
		if( bigsize(l->tn.type) > bigsize(p->tn.type ) ) {
			l = makety( l, t, 0, t );
			tydown( l );
			p->in.left = doptim( l );
			if( opdebug ) {
				printf( "tydown(%d), after doptim(L):\n",
					p-node );
				eprint(p);
				}
			flag = 1;
			}
		return(flag);
		}
	}

# ifndef MYCONVERT
NODE *
sconvert( p ) register NODE *p; {
	TWORD t, lt;
	register NODE *l;
	int o;

	/* optimize CONV nodes */
	/* the unsigned-ness is ignored */
	/* if the CONV involves floats or doubles, retain unless null */
	/* if the CONV makes things bigger, retain */
	/* if the CONV keeps things the same size, just paint the type */
	/* if the CONV makes things smaller, adjust the addressing with
		memory references, and paint the new type */
	/* if a pointer is being converted, convert as if it were PTRTYPE */
	/* finally, if CONV converts a constant, do it in place */

	again:

	if( opdebug ) {
		printf( "sconvert(%d) called:\n", p-node );
		eprint( p );
		}
	if( p->tn.op != CONV ) cerror( "sconvert" );
	l = p->in.left;
	t = p->tn.type;
	lt = l->tn.type;
	o = l->tn.op;

	if( o==CONV && cbigger( l ) ){
	merge:
		p->in.left = l->in.left;
		l->tn.op = FREE;
		tydown( p );
		goto again;
		}

	if( ISUNSIGNED(t) ) t = DEUNSIGN(t);
	if( ISUNSIGNED(lt) ) lt = DEUNSIGN(lt);
	if( ISPTR(lt) )
# ifdef MEMONLY
		if( o==STAR || o==NAME || o==VAUTO || o==VPARAM )
# endif
		lt = PTRTYPE;

	if( t == lt ) goto paint;

	if( ISPTR(lt) || ISARY(lt) ) return(p);

	if( o == FCON ){  /* for a floating point const, paint type,
				round when it is output */
		if( t == FLOAT || t == DOUBLE ) goto paint;
		/* otherwise, convert it to long and treat as long conversion */
		l->tn.op = ICON;
		l->tn.lval = l->fpn.dval;  /* MACHINE-DEPENDENT CONVERSION */
		goto icon;
		}

	if( t==FLOAT || t==DOUBLE || lt==FLOAT || lt==DOUBLE ) return( p );

	if( o == ICON ){
	icon:
		l->tn.lval = ccast( l->tn.lval, p->tn.type );
	paint:
		l->tn.type = p->tn.type;
		l->fn.csiz = p->fn.csiz;
		l->fn.cdim = p->fn.cdim;
		p->tn.op = FREE;
		if( tydown(l) ) {
			l = doptim(l);
			}
		return( l );
		}

	if( cbigger(p) ) return( p );

	/* p makes things smaller */

	if( o==CONV ){
		/* two conversions in a row: the second makes things smaller */
		/* make them into one */
		goto merge;
		}

	if( o==STAR || o==NAME || o==VAUTO || o==VPARAM ) {
		/* memory reference: determine the adjustment */
# ifdef RTOLBYTES
# ifdef LOWINT
		if( lt == LONG ) if( !adjust( l, LOWINT )) cerror( "adj" );
# endif
# else 
		int adj = 0;
		if( lt == LONG ) adj = SZLONG;
		else if( lt == INT ) adj = SZINT;
		else if( lt == SHORT ) adj = SZSHORT;
		else cerror( "sconv:lt 0%o", lt );
		if( t == INT ) adj -= SZINT;
		else if( t == SHORT ) adj -= SZSHORT;
		else if( t == CHAR ) adj -= SZCHAR;
		else cerror( "sconv:t 0%o", t );
# ifdef LOWINT
		if( lt == LONG ) adj += LOWINT;
# endif
		if( adj ) if( !adjust( l, adj ) ) cerror( "adj1" );
# endif
		}

	/* other cases are where it is computed into a reg; */
	/* simply paint the type */
	/* we must copy (e.g., apply the CONV) for register vars */
	if( o == REG ) return( p );
	goto paint;
	}
# endif

NODE *
pvconvert( p ) register NODE *p; {
	/* p is a CONV node; convert */
	/* this does something only when the descendent is not a ptr */
	NODE *l;
	int o;
	l = p->in.left;
	if( ISPTR( l->tn.type ) ) return( clocal(p) );
# ifdef MEMONLY
	o = l->tn.op;
	if( o==STAR || o==NAME || o==VAUTO || o==VPARAM || o==ICON )
# endif
		{ /* optimize this reference */
		/* sconvert and optimize to PTRTYPE */
		l = makety( l, PTRTYPE, 0, PTRTYPE );
		if( l->tn.op == CONV ) l = sconvert( l );
		l->tn.type = p->tn.type;
		l->fn.cdim = p->fn.cdim;
		l->fn.csiz = p->fn.csiz;
		p->tn.op = FREE;
		p = l;
		}
	return( clocal(p) );
	}

NODE *
fortarg( p ) NODE *p; {
	/* fortran function arguments */

	if( p->in.op == CM ){
		p->in.left = fortarg( p->in.left );
		p->in.right = fortarg( p->in.right );
		return(p);
		}

	while( ISPTR(p->in.type) ){
		p = buildtree( STAR, p, NIL );
		}
	return( optim(p) );
	}

	/* mapping relationals when the sides are reversed */
short revrel[] ={ EQ, NE, GE, GT, LE, LT, UGE, UGT, ULE, ULT };
# define REPORT(x) if(opdebug)printf( "optim turns %d into %d\n",p-node,x-node);

NODE *
doptim(p) register NODE *p; {
	/* local optimizations, most of which are machine independent */
	/* doptim is called for each node by optim; it assumes that
	/* the children of p are already optimized */
	/* p is not a leaf */

	register NODE *l, *r, *sp;
	register o, i;
	TWORD t;

#ifndef NODBG
	if( opdebug ) {
		printf( "doptim called on:\n" );
		eprint(p);
		}
#endif

	if( (t=BTYPE(p->in.type))==ENUMTY || t==MOETY ) econvert(p);

	switch( optype( o = p->tn.op ) ) {

	case BITYPE:
		r = p->in.right;
		/* FALLTHRU */
	case UTYPE:
		l = p->in.left;
		break;

	case LTYPE:
		/* nothing more to do (after doing the enum stuff) */
		return( p );
		}

	sp = conval( p );
	/* return only if conval did something */
	if( sp != p ) return( doptim(sp) );

#ifndef NODBG
	if( opdebug ) {
		printf( "doptim works on:\n" );
		eprint(p);
		}
#endif

	switch(o){

	case CONV:
		/* someday, make pvconvert and sconvert the same */
		return( ISPTR(p->tn.type)?pvconvert(p):sconvert(p) );

	case ASG PLUS:
	case ASG MINUS:
	case ASG AND:
	case ASG OR:
	case ASG ER:
	case ASG LS:
	case ASG RS:
	case ASG MUL:
	case ASG DIV:
	case ASG MOD:
		/* if conversion ops on the lhs, transfer them to the rhs */
		t = l->in.type;
		if( l->tn.op == CONV && t!=FLOAT && t!=DOUBLE ) {
			p->in.left = l->in.left;
			l->tn.op = FREE;
			r = makety( r, p->in.type, p->fn.cdim, p->fn.csiz );
			p->in.right = doptim( r );
			}

		if( !nncon(r) ) break;  /* no more optimization */

		/* get rid of 0 ops that don't change anything... */
		if( !r->tn.lval && (o==ASG PLUS || o==ASG MINUS || o==ASG OR ||
				o==ASG ER || o==ASG LS || o==ASG RS) ){
			/* the answer is the lhs */
			goto bless;
			}
		if( r->tn.lval == 1 && (o==ASG MUL || o==ASG DIV) ) {
			/* the answer is the lhs */
			goto bless;
			}
		if( (i = ispow2( r->tn.lval ))>=0 && o==ASG MUL ) {
			o = p->in.op = ASG LS;
			r->in.type = r->fn.csiz = INT;
			r->tn.lval = i;
			}
		break;

	case LS:
	case RS:
		if( !nncon(r) || r->tn.lval ) break;  /* do nothing */
		goto bless;  /* shifts by 0 */

	case FORTCALL:
		p->in.right = fortarg( r );
		break;

	case UNARY AND:
		switch( l->tn.op ) {

		case STAR:
			/* fake up to use setuleft */
			l->tn.op = FREE;
			l=l->in.left;
			goto setuleft;

		case VAUTO:
		case VPARAM:
			/* the next two lines come from short structs */
		case CALL:
		case UNARY CALL:
			break;

		case RNODE:
# ifdef ARGSRET
			/* RNODE disappears if structure simple */
			if( simpstr( p->fn.cdim, p->fn.csiz ) == STRTY ) {
				/* complicated: make it look like first arg */
				l->tn.op = VPARAM;
				l->tn.lval = BITOOR(ARGINIT);
				l->tn.rval = NONAME;
				}
			break;
#else
# ifdef STATSRET
			/* simple structures will disappear */
			if( simpstr( p->fn.cdim, p->fn.csiz ) != STRTY ) break;
			/* otherwise, make & RNODE into ICON for static area */
			l->tn.rval = -strftn;
# else
			break; /* & of RNODE is just fine */
#endif
#endif
		case NAME:

# ifdef ANDABLE
			if( !ANDABLE(l) ) return(p);
# endif
			l->tn.op = ICON;

			setuleft:
			/* set the type of lhs with the type of the top */
			l->in.type = p->in.type;
			l->fn.cdim = p->fn.cdim;
			l->fn.csiz = p->fn.csiz;
			p->in.op = FREE;
			REPORT(l);
			return( l );

		default:
			cerror( "& error" );
			}
		break;

	case STCALL:
	case UNARY STCALL:
		/* use l in case return type overwritten */
		t = simpstr( l->fn.cdim, l->fn.csiz );
		if( t != STRTY ) {
			/* take some care to keep the types OK */
			/* the type of the return might well have been
			/* overwritten by (say) a structure reference */
			/* MAY NOT BE QUITE RIGHT IF TWO FLAVORS OF PTR */
			l = p;
			p->tn.type = DECREF( p->tn.type );
			p = buildtree( UNARY AND, l, NIL );
			if( o == STCALL ) l->tn.op = CALL;
			else l->tn.op = UNARY CALL;
			l->fn.type = l->fn.csiz = t;
			l->fn.cdim = 0;
			}
		break;

	case STAR:
		if( p->tn.type == STRTY || p->tn.type == UNIONTY ) {
			p->tn.op = FREE;
			REPORT(l);
			return( l );
			}
		if( l->tn.op == UNARY AND && !callop(l->in.left->tn.op) ) {
			/* & of call used in structure optimization */
			/* fake up to use setuleft */
			l->tn.op = FREE;
			l = l->in.left;
			goto setuleft;
			}
		if( l->tn.op != ICON ) break;
		l->tn.op = NAME;
		goto setuleft;

	case MINUS:
		if( !nncon(r) ) break;
		r->tn.lval = - r->tn.lval;
		o = p->in.op = PLUS;

	case MUL:
	case PLUS:
	case AND:
	case OR:
	case ER:
		/* commutative ops; for now, just collect constants */
		/* someday, do it right */
		if( o==r->tn.op || nncon(l) || ( ISCON(l) && !ISCON(r) ) ) {
			/* make ops tower to the left, not the right */
			/* also, put constants on the right */
			sp = l;
			l = p->in.left = r;
			r = p->in.right = sp;
			}
		/* do (A + C1) + C2, etc. */
		/* the number of special cases is horrifying */
		/* many bugs have been found here; this code is very cautious */
		/* (A + C1) + C2, where C2 can be a ptr, C1 not */
		if( o==PLUS && l->tn.op==PLUS && ISCON(r) &&
				nncon(l->in.right) ){
			p->in.left = l->in.left;
			l->tn.op = FREE;
			l = l->in.right;
			r->tn.lval += l->tn.lval;
			l->tn.op = FREE;
			return( doptim( p ) );
			}
		/* (A + C1) + C2, where C1 can be a ptr, C2 not */
		if( o==PLUS && l->tn.op==PLUS && nncon(r) &&
				ISCON(l->in.right) ){
			l->in.right->tn.lval += r->tn.lval;
			goto bless;  /* return l as the result */
			}
		/* (A - C1) + C2, where C2 can be a ptr, C1 not */
		if( o==PLUS && l->tn.op==MINUS && ISCON(r) &&
				nncon(l->in.right) ){
			p->in.left = l->in.left;
			l->tn.op = FREE;
			l = l->in.right;
			r->tn.lval -= l->tn.lval;
			l->tn.op = FREE;
			return( doptim( p ) );
			}
		/* (&A)+C */
		if( o==PLUS && l->tn.op == UNARY AND && ISCON(r) ) {
			switch( l->in.left->tn.op ) {
			case NAME:
			case VPARAM:
			case VAUTO:
				l->in.left->tn.lval += r->tn.lval;
				goto bless;
				}
			}

		/* change muls to shifts */

		if( o==MUL && nncon(r) && (i=ispow2(r->tn.lval))>=0){
			if( i == 0 ){ /* multiplication by 1 */
			bless:
				/* return l, with the type of p */
				l = makety( l, p->tn.type, p->fn.cdim,
						p->fn.csiz );
				r->tn.op = FREE;
				p->tn.op = FREE;
				/* if a conversion op was added, optimize */
				l = doptim( l );
#ifndef NODBG
				if( opdebug ) {
					printf( "optim replaces op1 (%d) by:\n",
						p-node );
					eprint(l);
					}
#endif
				return( l );
				}
			o = p->in.op = LS;
			r->in.type = r->fn.csiz = INT;
			r->tn.lval = i;
			}

		/* change +'s of negative consts back to - */
		if( o==PLUS && nncon(r) && r->tn.lval<0 ){
			r->tn.lval = -r->tn.lval;
			o = p->in.op = MINUS;
			}

		if( nncon(r) && !r->tn.lval && (o==PLUS||o==MINUS) ){
			/* get rid of add or subtract of 0 */
			goto bless;
			}

# ifdef PTRLEFT
		if( o==PLUS && ISPTR(p->tn.type) && ISPTR(r->tn.type)
# ifdef CONSRIGHT
			&& r->tn.op != ICON
# endif
			) { sp = l; p->in.left = r; p->in.right = sp; }
# endif
# ifdef PTRRIGHT
		if( o==PLUS && ISPTR(p->tn.type) && ISPTR(l->tn.type)
# ifdef CONSRIGHT
			&& r->tn.op != ICON
# endif
		 	) { sp = l; p->in.left = r; p->in.right = sp; }
# endif
		break;

	case DIV:
		if( nncon( r ) && r->tn.lval == 1 ) goto bless;
		break;

	case EQ:
	case NE:
	case LT:
	case LE:
	case GT:
	case GE:
	case ULT:
	case ULE:
	case UGT:
	case UGE:
		if( ISCON(l) && !ISCON(r) ){
			/* exchange operands */
			p->in.op = revrel[p->in.op - EQ ];
			sp = l;
			l = p->in.left = r;
			r = p->in.right = sp;
			}
		break;

	case STASG:
		if( (t=simpstr( p->fn.cdim, p->fn.csiz ) ) != STRTY ) {
			/* rewrite = as simpler */
			if( ISPTR(r->tn.type) ) {
				r = buildtree( STAR, r, NIL );
				r->fn.type = r->fn.csiz = t;
				r->fn.cdim = 0;
				p->in.right = r = doptim( r );
				}
			l = buildtree( STAR, l, NIL );
			l->fn.type = l->fn.csiz = t;
			l->fn.cdim = 0;
			p->in.left = l = doptim( l );
			p->fn.type = p->fn.csiz = (t==LONG ? LONG : INT);
			p->fn.cdim = 0;
			p->fn.op = ASSIGN;
			return( p );
			}

	case STARG:
		if( (t=simpstr( p->fn.cdim, p->fn.csiz ) ) != STRTY ) {
			/* rewrite as simpler */
			if( ISPTR(l->fn.type) ) {
				l = buildtree( STAR, l, NIL );
				l->fn.type = l->fn.csiz = t;
				l->fn.cdim = 0;
				p->in.left = l = doptim( l );
				}
			p->fn.type = p->fn.csiz = (t==LONG ? LONG : INT);
			p->fn.cdim = 0;
			p->fn.op = FUNARG;
			return( p );
			}
# ifdef ENDSTRUCT
		p->in.left = aadjust( l, p->stn.stsize );
# endif
		break;

		}

	return(p);
	}

NODE *
optim( p ) register NODE *p; {

	switch( optype( p->tn.op ) ){

	case BITYPE:
		p->in.right = optim( p->in.right );
		/* FALLTHRU */
	case UTYPE:
		p->in.left = optim( p->in.left );
		}
	return( doptim( p ) );
	}

ispow2( c ) CONSZ c; {
	register i;
	if( c <= 0 || (c&(c-1)) ) return(-1);
	for( i=0; c>1; ++i) c >>= 1;
	return(i);
	}

nncon( p ) NODE *p; {
	/* is p a constant without a name */
	return( p->tn.op == ICON && p->tn.rval == NONAME && !ISPTR(p->tn.type));
	}

	/* some routines for debugging and tree transformation */
#ifndef MYOFFCON
NODE *
offcon( off, t, d, s ) OFFSZ off; TWORD t; {

	/* return a node, for structure references, which is suitable for
	   being added to a pointer of type t, in order to be off bits offset
	   into a structure */

	register NODE *p;

	/* t, d, and s are the type, dimension offset, and size offset */
	/* in general they may be necessary for offcon */

	p = bcon(0);
	p->tn.lval = BITOOR(off);
	p->fn.type = p->fn.csiz = PTRTYPE;
	return(p);
	}
# endif

bccode(){
	/* called just before executing code */
	/* beware: called several times if there is auto. initialization */
# ifdef SDB
	static bcclev;
	if( blevel != bcclev ){
		bcclev = blevel;
		pstab( S_LBRAC, blevel-1 );
		}
# endif
# ifdef MYBCCODE
	MYBCCODE;
# endif
	p2bbeg( autooff, regvar );
	}

# ifdef SDB
int stabflag = 0;

	/* symbol table stuff */

pstab(  op, val ) {
	/* write out stabs, stabn, or stabd */
	struct symtab *p;

	if( !stabflag ) return;

	switch( op ) {
	case S_BFUN:
	case S_EFUN:
		p = &stab[curftn];
		printf( "\tstabs\t\"%.8s\",0x%x,%d,", p->sname, op, lineno );
		if( op == S_BFUN ) printf( "%s\n", exname( p->sname ) );
		else {
			printf( LABFMT, retlab );
			printf( "\n" );
			}
		return;

	case S_TYID:
		printf( "\tstabs\t\"%.8s\",0x%x,0,0\n", stab[val].sname, op );
		return;

	default:
		printf( "	stabd	0x%x,0%o\n", op, val );

		}
	}

# ifndef OUTREGNO
# define OUTREGNO(p) ((p)->offset)
# endif

ppstab( p ) struct symtab *p; {
	/* write out stabs for the symbol p */
	TWORD t;
	int off, op, i;

	if( !stabflag ) return;

	if( p->sclass & FIELD ) return;
	if( p->stype == TNULL ) return;

	printf( "\tstabs\t\"%.8s\",", p->sname );
	switch( p->sclass ){

	case AUTO:
		op = S_LSYM;
	mkoff:
		off = p->offset;
		if( off < 0 ) off = -off;
		printf( "0x%x,0%o,%d\n", op, p->stype, off/SZCHAR );
		break;

	case REGISTER:
		printf( "0x%x,0%o,%d\n", S_RSYM, p->stype, OUTREGNO(p) );
		break;

	case PARAM:
		op = S_PSYM;
		goto mkoff;

	case EXTERN:
		printf( "0x%x,0%o,0\n", S_GSYM, p->stype );
		break;

	case EXTDEF:
		printf( "0x%x,0%o,%s\n", S_GSYM, p->stype, exname(p->sname) );
		break;

	case STATIC:
		printf( "0x%x,0%o", S_STSYM, p->stype );
		if( p->slevel && p->sclass==STATIC ) {
			printf( ",L%%%d\n", p->offset );
			}
		else printf( ",%s\n", exname(p->sname) );
		break;

	case STNAME:
	case UNAME:
		printf( "0x%x,0%o,0\n", S_BSTR, p->stype, 0 );
		for( i = dimtab[p->sizoff+1]; dimtab[i] >= 0; ++i ){
			ppstab( &stab[dimtab[i]] );
			}
		printf( "\tstabs\t\"%.8s\",0x%x,0%o,%d\n", p->sname, S_ESTR,
			p->stype, dimtab[p->sizoff]/SZCHAR );
		return;

	case MOS:
	case MOU:
		op = S_SSYM;
		goto mkoff;

	case ENAME:
		printf( "0x%x,0%o,0\n", S_BENUM, p->stype, 0 );
		for( i = dimtab[p->sizoff+1]; dimtab[i] >= 0; ++i ){
			ppstab( &stab[dimtab[i]] );
			}
		printf( "\tstabs\t\"%.8s\",0x%x,0%o,0\n", p->sname, S_EENUM,
			p->stype, 0 );
		return;

	case MOE:
		printf( "0x%x,0%o,%d\n", S_ENUM, p->stype, p->offset );
		return;

	default:
		cerror( "illegal stab class: %d", p->sclass );
		}

	switch( BTYPE(p->stype) ) {
	/* make another entry to describe structs, unions */
	case STRTY:
	case UNIONTY:
	case ENUMTY:
		/* write out TYID with the name, if any */
		pstab( S_TYID, dimtab[p->sizoff+3] );
		}
	/* make other entries with the dimensions */
	for( t=p->stype, i=p->dimoff; t&TMASK; t = DECREF(t) ) {
		if( ISARY(t) ) pstab( S_DIM, dimtab[i++] );
		}
	}

aobeg(){
	/* called before removing automatics from stab */
	}

aocode(p) struct symtab *p; {
	/* called when automatic p removed from stab */

	if( p->sclass & FIELD ) return;
	switch( p->sclass ){

	case REGISTER:
	case AUTO:
	case STATIC:
	case EXTDEF:
	/*	for now, do not report externs
	/*	case EXTERN:
	*/
	case STNAME:
	case UNAME:
	case ENAME:
		if( !ISFTN( p->stype ) ) ppstab( p );
		return;
		}
	}

aoend(){
	/* called after removing all automatics from stab */
	pstab( S_RBRAC, blevel );
	}

ejsdb(){
	/* called at the end of the entire file */
	register struct symtab *p;
	for( p = &stab[SYMTSZ]; --p>=stab; ){
		if( p->stype != TNULL && !ISFTN(p->stype) ) {
			switch( p->sclass ) {

			case EXTERN:
				if( p->suse > 0 ) continue;  /* unused */
			case EXTDEF:
			case STATIC:
			case STNAME:
			case UNAME:
			case ENAME:
				ppstab(p);
				}
			}
		}
	}

char sdbfile[100];
int sdbfop = S_SO;

sdbline(){
	/* print out linenumber, filenumber */
	if( !stabflag ) return;
	if( strcmp( ftitle, sdbfile ) ) {
		/* new file */
		register char *pc;
		int i, l, la;

		l = locctr( PROG );
		deflab( la = getlab() );
		strcpy( sdbfile, ftitle );
		for( pc=sdbfile,i=0; *pc; ++pc ){
			if( *pc == '"' ) continue;
			if( i==0 ) printf( "\tstabs\t\"" );
			putchar( *pc );
			if( ++i == 8 ){
				i=0;
				printf( "\",0x%x,0,", sdbfop );
				printf( LABFMT, la );
				putchar( '\n' );
				}
			}
		if( i ) {
			printf( "\",0x%x,0,", sdbfop );
			printf( LABFMT, la );
			putchar( '\n' );
			}
		sdbfop = S_SOL;
		locctr( l );
		}
	pstab( S_SLINE, lineno );
	}
# endif

# include "mfile2.h"

char *cnames[] = 
{
	"CEFF",
	"NRGS",
	"CCC",
	0,
};

prgoal( goal ) 
register goal;
{
	/* print a nice-looking description of goal */

	register i, flag;

	flag = 0;
	for( i=0; cnames[i]; ++i )
	{
		if( goal & (1<<i) )
		{
			if( flag ) printf( "|" );
			++flag;
			printf( cnames[i] );
		}
	}
	if( !flag ) printf( "?%o", goal );

}

#ifndef NODBG
e2print( p )
register NODE *p; 
{
	printf( "\n********* costs=(0,...,NRGS;EFF;TEMP;CC)\n" );
	e22print( p ,"T");
	printf("=========\n");
}

e22print( p ,s)
register NODE *p; 
char *s;
{
	static down=0;
	register ty;

	ty = optype( p->tn.op );
	if( ty == BITYPE )
	{
		++down;
		e22print( p->in.right ,"R");
		--down;
	}
	e222print( down, p, s );

	if( ty != LTYPE )
	{
		++down;
		e22print( p->in.left, "L" );
		--down;
	}
}

e222print( down, p, s )
NODE *p; 
char *s;
{
	/* print one node */
	int i, d;

	for( d=down; d>1; d -= 2 ) printf( "\t" );
	if( d ) printf( "    " );

	printf( "%s.%d) op= '%s'",s, (int)(p-node), opst[p->in.op] );
	switch( p->in.op ) 
	{
		 /* special cases */
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

t2print( t )
TWORD t;
{
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
e2print( p )
NODE *p; 
{
	werror( "e2print not compiled" );
}
e222print( p, s )
NODE *p; 
char *s;
{
	werror( "e222print not compiled" );
}
# endif
cprt( c, s )
register char *s; 
register c;
{
	if( c >= INFINITY ) printf( "*%s", s );
	else printf( "%d%s", c, s );
}

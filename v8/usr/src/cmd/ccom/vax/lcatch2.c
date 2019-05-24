# include "mfile2.h"
extern int Pflag, bbcnt;
/* a lot of the machine dependent parts of the second pass */

char *
rnames[]= {  /* keyed to register number tokens */

	"r0", "r1",
	"r2", "r3", "r4",
	"r5", "r6", "r7", "r8", "r9", "r10", "r11",
	"ap", "fp", "sp", "pc" 
	};

eobl2()			/* end of function stuff */
{
}

char *
exname( ix )
char *ix;
{
	/* make a name look like an external name in the local machine */

	static char text[100] = "_";

	if( ix == NULL ) cerror("no name in exname");
	strcpy(text+1, ix);
	return( text );
}

lineid( l, fn )
int l;
char *fn;
{
	/* identify line l and file fn */
	printx( "#	line %d, file %s\n", l, fn );
}

deflab( n )
int n;
{
	printx( "L%d:\n", n );
}


genubr( n )
int n;
{
	/* output a branch to label n */
	printx( "\tjbr\tL%d\n", n );
	if(Pflag)
		printx("#jmp L%d\n", n);
}
genret( s, l, n )
int s, l, n;
{
	/* a return: s nonzero means a structure returned */
	/* n has the value of "retlab", a common spot for returns */
	deflab(n);
	if( s ) printx( "\tmovab\tL%d,r0\n", l );
	if(Pflag) {
		printx("#ret %d\n", ++bbcnt);
		printx("\tincl\tlocprof+%d\n", 4*(bbcnt+3));
	}
	dbfunret();
	printx( "\tret\n" );
}

defalign(n)
int n;
{
	/* cause the alignment to become a multiple of n bits */
	if( n % SZCHAR ) cerror( "funny alignment: %d", n );
	else n /= SZCHAR;
	if( n == 1 ) return;
	else if( n==2 ) n=1;
	else if( n==4 ) n=2;
	else cerror( "funny alignment: %d", n );
	printx( "\t.align\t%d\n", n );
}

char *locnames[] = {
	/* location counter names for PROG, DATA, ADATA, ISTRNG, STRNG */
	"	.text\n",
	"	.data\n",
	"	.data\n",
	"	.data	2\n",
	"	.data	1\n",
	};

bycode( t, i )
int t, i;
{
	/* put byte i+1 in a string */

	if ( t < 0 ) { /* end of the string */
		printx("\n");
	} else { /* stash byte t into string */
		if( (i&7) == 0 ) printx( "\n	.byte	" );
		else printx(",");
		printx("0x%x", t);
	}
}

genshort( s )
short s;
{
	/* write out a short value */
	printx( "	.short	%d\n", (short) s );
}

genlong( l )
long l;
{
	/* write out a long initializer */
	printx( "	.long	0x%lx\n", l );
}

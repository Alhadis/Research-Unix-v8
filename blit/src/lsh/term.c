#include <jerq.h>
#include <jerqio.h>

#define	INSET	2
#define	C_NEW	2

struct udata{
	short	bra_b_start;	/* branch over data */
	Rectangle Drect;
	struct Mouse mouse;
	Layer	*Jdisplayp;
	char	**argv;
	int	argc;
};

typedef union Fptr{
	long	l;
	int	(*f)();
} Fptr;

long proctab, windowstart;
Fptr qputc;

char Terminator;
long getlong()
{
	register char c;
	long i = 0;

	while( c = getchar(), c == ' ' || c == '\t' || c == ',' ) {}
	while( c >= '0' && c <= '9' ){
		i = i*10 + c - '0';
		c = getchar();
	}
	Terminator = c;
	return i;
}

main()
{
	static char buf[512];
	Rectangle r;

	proctab = getlong();
	qputc.l = getlong();
	windowstart = getlong();
	Output();
	while( r.origin.x = getlong() ){
		r.origin.y = getlong();
		r.corner.x = getlong();
		r.corner.y = getlong();
		buf[0] = '\0';
		if( Terminator == ' ' ) fgets(buf, 512, stdin);
		rectlayer( r, buf );
	}
	exit();
}

Output()
{
	struct Proc *p = (struct Proc *) proctab;
	register i;
	register struct udata *udp;
	Rectangle r;

	for( i = 2; i < 8; ++i ) if( &p[i] != P ){
		if( !(p[i].state) ) continue;
		r = inset(p[i].rect,-INSET);
		printf( "%d", r.origin.x );
		printf( " %d", r.origin.y );
		printf( " %d", r.corner.x );
		printf( " %d", r.corner.y );
		udp = (struct udata *) p[i].fcn;
		if( ((long) udp ) != windowstart && udp->argv[0] )
			printf( " [%s]", udp->argv[0] );
		printf( "\n" );
	}
}

Texture shademap={ 0x1000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
shade(l)
Layer *l;
{
	texture(l, inset(l->rect, INSET), &shademap, F_XOR);
}

rectlayer( r, s )
Rectangle r;
char *s;
{
	register struct Proc *p;
	register unsigned char *pout;

	if(r.corner.x-r.origin.x<=100 || r.corner.y-r.origin.y<=50) return;
	if(!(p=newproc(windowstart))) return;
	p->rect=inset(r, INSET);
	if( !rectclip(&p->rect,Jrect) || !(p->layer=newlayer(r))){
		p->state=0;
		return;
	}
	mpxnewwind(p, C_NEW);
	shade(p->layer);
	tolayer(p->layer);
	pout = p->cbufpout;
	do sleep( 60 ); while( pout == p->cbufpout );
	while( *s ) ( *qputc.f )( &p->kbdqueue, *s++ );
}

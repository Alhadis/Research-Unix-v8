#include "/usr/jerq/src/joff/host/a.out.h"
#include <stdio.h>

#define MAGIC_ABS 0407

long proctab, windowst, qputc;

error(s)
char *s;
{
	fprintf( stdout, "%s\n", s );
	exit(1);
}

main(argc,argv)
char **argv;
{
	struct exec hdr;
	int fd, size, i;
	struct nlist *s;

	if((fd = open(argv[1],0)) == -1 )
		error( "can't open symbol table" );
	lseek( fd, 0L, 0 );
	if(read(fd, (char*) &hdr, sizeof hdr) != sizeof hdr)
		error( "can't read header" );
	if(hdr.a_magic != MAGIC_ABS ) 
		error( "bad magic" );
	if(hdr.a_syms == 0)
		error( "no symbols" );
	s = (struct nlist*) calloc(hdr.a_syms,1);
	if(!s) error( "malloc error" );
	lseek( fd, sizeof hdr + hdr.a_text + hdr.a_data, 0) ;
	if( read( fd, (char *) s, hdr.a_syms ) != hdr.a_syms )
		error( "can't read stabs" );
	size = hdr.a_syms/sizeof *s;
	for( i = 0; i < size; ++i, ++s ){
		if( !strncmp( s->n_name, "proctab", 8 ) )
			if( s->n_value ) proctab = s->n_value;
		if( !strncmp( s->n_name, "windowst", 8 ) )
			if( s->n_value ) windowst = s->n_value;
		if( !strncmp( s->n_name, "qputc", 8 ) )
			if( s->n_value ) qputc = s->n_value;
		if( proctab && windowst && qputc ) break;
	}
	printf( "%d\n%d\n%d\n", proctab, qputc, windowst );
}

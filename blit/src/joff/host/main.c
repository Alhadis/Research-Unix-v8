#include "common.h"
#include "stab.h"


main( argc, argv)
char *argv[];
{
	int i, xflag = 0;
	char *download = "/usr/blit/lib/joff.m";
	char *loader = "/usr/blit/bin/68ld\0 -z";
#define argstr() ( argv[i][2] ? &argv[i][2] : argv[++i] )

	assert( stabtab[USER] = (struct stable *) calloc(1, sizeof(struct stable)));
	assert( stabtab[MPX] = (struct stable *) calloc(1, sizeof(struct stable) ));
	stabtab[USER]->name = "user";
	stabtab[MPX]->name = "mpx";
	stabtab[USER]->magic = MAGREL;
	stabtab[MPX]->magic = MAGABS;
	stabtab[MPX]->rom = 262146;
	strcpy( stabtab[USER]->path, "a.out" );
	strcpy( stabtab[MPX]->path, "/usr/blit/lib/mpxterm" );
	for( i = 1; argv[i]; ++i ){
	    if( argv[i][1] != '=' && argv[i][0] != '-' ) useerr();
	    switch( argv[i][1] == '=' ? argv[i][0] : argv[i][1] ){
		case 'd' : searchdirc( argstr() ); break;
		case 'x' : ++xflag; break;
		case 'z' : strcat( loader, " -z" ); break;
		case '6' : loader = argstr(); break;
		case 't' : download = argstr(); break;
		case 'm' : strcpy( stabtab[MPX]->path, argstr() ); break;
		case 'u' : strcpy( stabtab[USER]->path, argstr() ); break;
		case 'o' : ++oflag;
			stabtab[USER]->name = "object";
			stabtab[USER]->magic = 0;
			strcpy( stabtab[USER]->path, argstr() ); break;
			break;
		default: useerr();
	    }
	}
	if( oflag ) stabtab[MPX] = 0;
		
	if( !oflag && !xflag && system( fmt("%s %s", loader,  download) ) ){
		clean = 1;
		wrap();
	}
	settty();
	if( xflag ) printf( "reloaded\n" );
	m_init();
	joff();
}

useerr(){ 
	fprintf( stderr, "use: joff [-ofile]\n" );
	clean = 1;
	wrap();
}

reload( obj )
char *obj;
{
	resetmenus();
	execl( 	obj,
		obj,
		"-x",
		fmt( "-m%s", stabtab[MPX]->path ),
		fmt( "-u%s", stabtab[USER]->path ),
		NULL
	     );
	printf( "execl() failed.\n" );
}

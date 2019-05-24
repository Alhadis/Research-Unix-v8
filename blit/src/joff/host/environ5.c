

#include "common.h"
#include <signal.h>
#include <sys/termio.h>

/*#include <sys/ioctl.h>*/

static struct termio *savetty;
static struct termio *worktty;

wrap(junk){
	if( !oflag ) jerqdo( DO_WRAP );
	resettty();
	if( !clean ) abort();
	exit();
}

static char stdoutbuf[BUFSIZ];

settty()
{
	if( signal( SIGINT, SIG_IGN ) != SIG_IGN ) signal( SIGINT, wrap );
	assert( savetty = (struct termio *) calloc( 1, sizeof *savetty) );
	assert( worktty = (struct termio *) calloc( 1, sizeof *worktty) );
	(void)ioctl( 1, TCGETA, savetty );
	worktty->c_iflag = IGNBRK;
	worktty->c_cflag = (savetty->c_cflag & (CBAUD|CLOCAL)) | CS8 | CREAD;
	worktty->c_cc[VMIN] = 1;
	(void)ioctl( 1, TCSETA, worktty );
	if( !oflag ){
		 setbuf( stdout, stdoutbuf );
	}
}

resettty(){
	if( savetty ) ioctl( 1, TCSETA, savetty );
}

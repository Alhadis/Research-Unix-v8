#include "common.h"
#include <signal.h>
#include <sgtty.h>

/*#include <sys/ioctl.h>*/

static struct sgttyb *savetty;
static struct sgttyb *worktty;

breaksig()
{
	clean = 1;
	wrap();
}

wrap(junk){
	if( !oflag ) jerqdo( DO_WRAP );
	resettty();
	if( !clean ) abort();
	exit();
}

static char stdoutbuf[BUFSIZ];

settty()
{
	if( signal( SIGINT, SIG_IGN ) != SIG_IGN ) signal( SIGINT, breaksig );
	assert( savetty = (struct sgttyb *) calloc( 1, sizeof *savetty) );
	assert( worktty = (struct sgttyb *) calloc( 1, sizeof *worktty) );
	ioctl( 1, TIOCGETP, savetty );
	*worktty = *savetty;
	worktty->sg_flags &= ~ECHO;
	worktty->sg_flags |= CBREAK;
	if( !oflag ){
		setbuf( stdout, stdoutbuf );
		worktty->sg_flags |= RAW;
	}
	ioctl( 1, TIOCSETP, worktty );
}

resettty(){
	if( savetty ) ioctl( 1, TIOCSETP, savetty );
}

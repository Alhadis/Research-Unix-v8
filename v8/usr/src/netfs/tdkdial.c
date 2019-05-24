#define DKMSGS 1
#include <dk.h>
#include <dkerr.h>
#include <sgtty.h>
#include <stdio.h>
#include <signal.h>
#include "dkdial.h"

#define SUBR "/etc/dkdialsub"

int	dkverbose;
int	dkrchan;

static char	*argbuf;
static char	**bufptr;
extern	int	rdk_ld;
extern 	int	errno ;

int
tdkdial(dialstr, traffic)
int traffic ;
char * dialstr ;
{
	return (_tdkdial(1, traffic, dialstr)) ;
}

int
tdkserv(srvname, maxtraf)
int maxtraf ;
char * srvname ;
{
	return (_tdkdial(2, maxtraf, srvname)) ;
}


int
_tdkdial (type, traffic, dialstr)
int type ;
int traffic ;
char * dialstr ;
{
	register int	fd, pid, rc;
	char	args[100];
	char	*argptrs[10];
	int	status;
	struct dialout reply;
	int	pipefd[2];
	register i;
	char outname[64];
	int	popld ;
	int	oldchdie ;

	/* establish the initial connection */
	strcpy(outname, "/dev/dk/dkxx");
	for (i=3; i<64; i+=2) {
		outname[10] = i/10 + '0';
		outname[11] = i%10 + '0';
		if ((fd = open(outname, 2)) >= 0)
			break;
	}
	if (fd < 0) {
		tdkerrmess ("dk busy or no listener\n");
		return fd;
	}
	ioctl(fd, FIOPUSHLD, &rdk_ld) ;

	/* create a pipe to the sub-process */
	if (pipe (pipefd) < 0) {
		tdkerrmess ("cannot create a pipe\n");
		close (fd);
		return -1;
	}

	/* build the arg list for the sub-process */
	arginit (args, argptrs);
	strarg ("tdkdial");
	intarg (fd);
	intarg (pipefd[PIPEWRITE]);
	intarg (type);
	intarg(traffic) ;
	strarg(dialstr) ;
	argend();
	oldchdie = (int)signal(SIGCHLD, SIG_IGN) ;

	/* start up a sub-process */
	switch (pid = fork()) {

		/* child */
	case 0:
		close (pipefd[PIPEREAD]);
		setuid (geteuid());
		execv (SUBR, argptrs);
		tdkerrmess ("cannot execute dialer\n");
		exit (GENERR);

		/* error */
	case -1:
		close (pipefd[PIPEREAD]);
		close (pipefd[PIPEWRITE]);
		close(fd);
		tdkerrmess ("cannot fork\n");
		signal(SIGCHLD, oldchdie) ;
		return -1;

		/* parent */
	default:
		close (pipefd[PIPEWRITE]);

		/* wait for the child to complete */
		do 
			rc = wait (&status);
		while (rc > 0 && rc != pid);

		signal(SIGCHLD, oldchdie) ;
		/* figure out what happened */
		if (rc < 0) {
			tdkerrmess ("tdkdial process disappeared\n");
			goto bad;
		}

		/* read the reply */
		rc = read (pipefd[PIPEREAD], &reply, sizeof (reply));
		if (rc != sizeof (reply)) {
			tdkerrmess ("wrong length in reading pipe\n");
			goto bad;
		}
		close (pipefd[PIPEREAD]);

		/* save the real channel number in case someone wants it */
		dkrchan = reply.param2;

		/* pretty normal return -- figure out what happened */
		switch (status) {

			/* successfully connected */
		case NORM << 8:
			ioctl(fd, FIOPOPLD, 0) ;
			return fd;


			/* couldn't connect */
		default:
			if ((status & 0xff) == 0) {
				register int	x = status >> 8;
				if (x > ERRBASE && x < ERRBASE + NDKERR)
					tdkerrmess(dkmsgs[x-ERRBASE]);
			}
			goto bad;
		}
	}

bad:
	close (pipefd[PIPEREAD]);
	close (fd);
	return -1;
}


tdkerrmess (s)
	register char	*s;
{
	if (dkverbose)
		write (2, s, strlen(s));
}


/*
 *	The following subroutines help build argument lists
 */

static
arginit (x, y)
	char	*x;
	char	**y;
{
	argbuf = x;
	bufptr = y;
}


static
strarg (x)
	register char	*x;
{
	*bufptr++ = x;
}


static
intarg (x)
	int	x;
{
	*bufptr++ = argbuf;
	if (x < 0)
		*argbuf++ = '-';
	else
		x = -x;
	convint (x);
	*argbuf++ = '\0';
}


static
convint (x)
	register int	x;
{
	if (x <= -10)
		convint (x / 10);
	*argbuf++ = '0' - x % 10;
}


static
argend()
{
	*bufptr++ = 0;
}



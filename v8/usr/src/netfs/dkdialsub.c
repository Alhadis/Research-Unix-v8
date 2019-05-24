#define DKMSGS 1
#include <sys/types.h>
#include <stdio.h>
#include <dk.h>
#include <dkerr.h>
#include <pwd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include "dkdial.h"
#include <sys/stat.h>

struct passwd *getpwuid();

main (argc, argv)
	int argc;
	char **argv;
{
	int fd, server ;
	int retval, listener;
	struct dialout reply;
	struct listenin d;
	register char **p;
	struct passwd *pw;
	struct stat st;
	int pipefd;
	char dialstr[64] ;
	char dialtone[2] ;
	int rc ;
	int traffic ;
	char *cp ;

	/* pick up and decode arguments */
	if (argc != 6)
		exit (GENERR);
	p = argv + 1;
	fd = atoi (*p++);
	pipefd = atoi (*p++);
	server = atoi (*p++);
	traffic = atoi(*p++) ;
	strcpy(dialstr, *p) ;
	cp = dialstr ;
	while (*cp) {
		if (*cp == '\n')
			*cp = '\0' ;
		cp++ ;
	}

	/* set up the parameter block for the initial request */
	fstat(fd, &st);
	d.l_type = T_SRV;
	d.l_lchan = minor(st.st_rdev);
	d.l_srv = 256 - server;
	d.l_param0 = traffic ;
	d.l_param1 = 0;
	d.l_param2 = 0;
	d.l_param3 = 0;
	d.l_param4 = 0;
	d.l_param5 = 0;
	strcat(dialstr, "\n") ;
	pw = getpwuid(getuid()) ;
	if (pw == 0)
		exit(NOUID) ;
	strcat(dialstr, pw->pw_name) ;
	strcat(dialstr, "\n") ;

	/*
	 *	Send and read the reply to the initial request.
	 *	Send the reply down the pipe to the caller.
	 *	If no reply in 15 seconds, something's wrong;
	 *	just terminate, and let the caller deal with
	 *	the abnormal return code.
	 */
	signal (SIGALRM, SIG_DFL);
	alarm (60);

	listener = dkchan1() ;
	if (listener < 0)
		exit(GENERR);
	write(listener, (char *)&d, sizeof(d));
	close(listener);
	rc = read(fd, dialtone, 1) ;
	if (rc < 0)
		exit(GENERR) ;
	if (dialtone[0] != 'O')
		exit(GENERR) ;
	write(fd, dialstr, strlen(dialstr)) ;
	rc = read (fd, &reply, sizeof reply);
	alarm (0);
	if (rc <= 0)
		exit (GENERR);
	write (pipefd, &reply, sizeof reply);

	/* assume normal exit */
	retval = NORM;

	/* successful reply, analyze it */
	switch (reply.srv) {
	case D_OPEN:
		break ;
	case D_FAIL:
		if (reply.param1 > 0 && reply.param1 < NDKERR)
			exit (reply.param1 + ERRBASE);
		exit (ERRBASE);
	}
	return retval;
}

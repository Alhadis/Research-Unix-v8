/*
 *	ct - call terminal
 */

#include <sgtty.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <utmp.h>

#define GETTY "/etc/getty"
#define ROOT 0
#define OTHER 1

extern int optind;
extern char *optarg;

char *ttyname();

char *num, *class = "D1200";

extern int tty_ld;

int hangup = 0;
int verbose = 0;
int errflg = 0;
int count = 5;		/* how many times to try to get through */
int waitint = 60;	/* how many seconds to wait between attempts */

#define SCPYN(a, b)	strncpy(a, b, sizeof(a))
#define SCMPN(a, b)	strncmp(a, b, sizeof(a))

char utmp[] = "/etc/utmp";
char wtmpf[] = "/usr/adm/wtmp";

struct utmp wtmp;

main (argc, argv)
	int argc;
	char **argv;
{
	register int c;
	int p, pid, status, fd;
	char *tty;

	while ((c = getopt (argc, argv, "hvc:w:")) != EOF) {
		switch (c) {
		case 'h':
			hangup = 1;
			break;
		
		case 'v':
			verbose = 1;
			break;
		
		case 'c':
			count = atoi (optarg);
			break;
		
		case 'w':
			waitint = atoi (optarg);
			break;
		
		case '?':
			errflg++;
			break;
		}
	}

	if (errflg || optind > argc-1) {
		if (verbose)
			fprintf (stderr, "ct: arg count\n");
		exit (1);
	}

	num = argv[optind];
	if (optind < argc-1)
		class = argv[optind+1];

	/* hang up the phone if requested and standard input is a terminal */
	if (hangup) {
		struct sgttyb s;
		if (gtty (0, &s) >= 0) {
			setsigs (SIG_IGN);
			verbose = 0;
			s.sg_ispeed = s.sg_ospeed = 0;
			stty (0, &s);
			sleep (8);	/* let modems quiesce */
		}
	}

	while (--count >= 0) {
		fd = dialout (num, class);
		if (fd >= 0)
			break;
		if (count >= 0)
			sleep (waitint);
	}

	if (fd < 0) {
		if (verbose)
			fprintf (stderr, "ct: connect failed\n");
		exit (1);
	}

	/* we don't want exclusive use of the line */
	ioctl (fd, TIOCNXCL, 0);

	/* figure out the name we were given */
	tty = ttyname (fd);
	if (tty == NULL) {
		if (verbose)
			fprintf (stderr, "??? can't find tty\n");
		close (fd);
		exit (1);
	}
	
	setsigs (SIG_IGN);

	if (verbose)
		fprintf (stderr, "connected\n");
	session (fd, tty);
	cleanup (tty);

	return 0;
}

/* conduct a terminal session on file "fd" */
session (fd, tty)
	register int fd;
	register char *tty;
{
	int status, pid;
	register int p;

	switch (pid = fork()) {
	default:	/* parent */
		do p = wait (&status);
		while (p >= 0 && p != pid);
		if (verbose)
			fprintf (stderr, "exit status %d\n", status);
		break;
	
	case 0:		/* child */
		/* Allow the terminal session to run unmasked */
		setsigs (SIG_DFL);

		/* become the head of a new process group */
		ioctl (fd, TIOCSPGRP, 0);

		/* set up standard input, output, error, tty */
		close (0); close (1); close (2); close (3);
		dup (fd); dup (fd); dup (fd); dup (fd); close (fd);

		/* start the session */
		execl (GETTY, "-", strchr (class, '3')? "5": "3", 0);
		exit (1);	/* exec failed */
	
	case -1:
		exit (1);
	}
	return status;
}

cleanup (dev)
	char *dev;
{
	register f;
	register char *line, *p;

	/* find last component of path name */
	p = line = dev;
	while (*p)
		if (*p++ == '/')
			line = p;

	/* indicate this user is no longer signed on */
	f = open(utmp, 2);
	if(f >= 0) {
		while(read(f, (char *)&wtmp, sizeof(wtmp)) == sizeof(wtmp)) {
			if (SCMPN(wtmp.ut_line, line))
				continue;
			lseek(f, -(long)sizeof(wtmp), 1);
			SCPYN(wtmp.ut_name, "");
			time(&wtmp.ut_time);
			write(f, (char *)&wtmp, sizeof(wtmp));
		}
		close(f);
	}

	/* write logout information for accounting */
	f = open(wtmpf, 1);
	if (f >= 0) {
		SCPYN(wtmp.ut_line, line);
		SCPYN(wtmp.ut_name, "");
		time(&wtmp.ut_time);
		lseek(f, (long)0, 2);
		write(f, (char *)&wtmp, sizeof(wtmp));
		close(f);
	}

	/* reset device to pristine state */
	chown (dev, ROOT, OTHER);
	chmod (dev, 0666);
}

/* set all important signals to f */
setsigs (f)
	register int (*f)();
{
	signal (SIGHUP, f);
	signal (SIGINT, f);
	signal (SIGQUIT, f);
}

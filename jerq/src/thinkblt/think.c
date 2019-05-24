#include <signal.h>
#include <stdio.h>

#define Signal(s, f)	if (signal(s, SIG_IGN) != SIG_IGN) (void)signal(s, f)

#define PARGVAL	((*argv)[2] ? (*argv)+2 : --argc ? *++argv : (char *)0)

char linkbuf[64], *linkname = 0;
char *getenv(), *strcat(), *strcpy();
int outfd, debug, finish();

#define Getc()	(--nchin > 0 ? *inp++ : (inp = getbuf()) ? *inp++ : -1)

int nchin, infd; char inbuf[256], *getbuf();

int Argc; char **Argv;

main(argc,argv)
int argc; char **argv;
{
	register char *inp;

	Signal(SIGHUP, finish);
	Signal(SIGINT, finish);
	Signal(SIGQUIT, finish);
	Signal(SIGPIPE, finish);

	while (--argc > 0 && (*++argv)[0] == '-' && (*argv)[1])
		switch ((*argv)[1]) {
		case 'o':
			linkname = PARGVAL; break;
		case 'D':
			debug++; break;
		default:
			fprintf(stderr, "unknown option %s\n", *argv);
			exit(1);
		}
	Argc = ++argc; Argv = --argv;

	if (linkname == 0)
		linkname = getenv("THINK");
	if (linkname == 0 && (linkname = getenv("HOME")))
		linkname = strcat(strcpy(linkbuf, linkname), "/.THINK");
	if (linkname == 0) {
		fprintf(stderr, "no output device\n");
		exit(1);
	}
	if ((outfd = open(linkname, 1)) < 0) {
		fprintf(stderr, "cannot open %s\n", linkname);
		exit(1);
	}
	if (debug)
		fprintf(stderr, "%s\n", linkname);
	if ((infd = (argc > 1) ? Open() : 0) < 0)
		exit(0);

	inp = getbuf();
	if (nchin >= 2 && inp[0] == 033 && inp[1] == 'N') {
		nchin--;
		thinksort(inp += 2, outfd);
	} else do {
		write(outfd, inbuf, nchin);
	} while (getbuf());

	finish(0);
}

char *
getbuf()
{
	while ((nchin = read(infd, inbuf, sizeof inbuf)) <= 0) {
		if (infd) close(infd);
		infd = Open();
		if (infd < 0) break;
	}
	return (nchin > 0) ? inbuf : 0;
}

Open()
{
	int infd = -1;
	while (--Argc > 0) {
		if (strcmp("-",*++Argv) == 0) infd = 0;
		else infd = open(*Argv,0);
		if (infd >= 0) break;
		fprintf(stderr,"cannot open %s\n",*Argv);
	}
	return infd;
}

finish(s)
{
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	if (outfd) {
		write(outfd, inbuf, 0);
		close(outfd);
	}
	exit(s != 0);
}

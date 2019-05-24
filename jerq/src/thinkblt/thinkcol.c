#include <stdio.h>

int nchin; char inbuf[256];

main(argc, argv)
char **argv;
{
	static char outbuf[BUFSIZ];
	setbuf (stdout, outbuf);
	thinksort(0, 1);
	exit(0);
}

char *
getbuf()
{
	nchin = read(0, inbuf, sizeof inbuf);
	return (nchin > 0) ? inbuf : 0;
}

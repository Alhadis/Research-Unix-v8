#include <stdio.h>

main(argc, argv)
char **argv;
{
	long i, lim1, lim2;
	long atol();
	register c;

	if (argc < 3) {
		fputs("usage: trim ncopy ndelete\n", stderr);
		exit(1);
	}
	lim1 = atol(argv[1]);
	lim2 = atol(argv[2]);
	for (i=0; i<lim1; i++)
		putchar(getchar());
	for (i=0; i<lim2; i++)
		getchar();
	while ((c=getchar())>=0)
		putchar(c);
}

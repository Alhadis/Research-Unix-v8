#include "asd.h"

main (argc, argv)
	int argc;
	char **argv;
{
	register int i;

	getargs (argc, argv, "vD:", (int (*)) 0);

	pkgstart();
	for (i = optind; i < argc; i++)
		pkgfile (argv[i]);

	i = pkgend();

	return i;
}

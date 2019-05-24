/*
 * print y given x=y
 */

char	*index();

main(argc, argv)
char **argv;
{
	register char *p;

	if (argc>=2) {
		p = index(argv[1], '=');
		printf("%s\n", p? p+1: argv[1]);
	}
}

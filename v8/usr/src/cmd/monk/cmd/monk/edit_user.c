#include	<stdio.h>
#include	<ctype.h>
#include	"search.h"
#include	"rd.h"

#define	USAGE		"type [ -t type | filename ]\n"
#define	OPTIONS		"t:"
#define	EDIT		"edit"
#define	MYEDITOR	"EDITOR"

main(argc, argv)
int	argc;
char	**argv;
{
	char	*choosefile(), *getenv();

	char	*filename, *command, *editor;

	filename = choosefile(argc, argv);
	if ((editor = getenv(MYEDITOR)) == NULL) {
		fprintf(stderr, "Please define EDITOR in your login\n");
		exit(-1);
	}
	command = strjoin(editor, filename);
	exit(system(command));
}

char *
choosefile(argc, argv)
int	argc;
char	**argv;
{
	extern int	optind;
	extern char	*optarg;

	char	*buildname(), *strip();

	int	c;
	char	*program;

	program = strip(argv[0]);
	if (strcmp(program, EDIT) != 0)
		return(buildname(program));
	while ((c = getopt(argc, argv, OPTIONS)) != EOF )
		switch (c) {
			/* format -t selects prototype, e.g. letter.sample */
			case 't':
				return(buildname(optarg));
			case '?':
				fprintf(stderr, "USAGE: %s; no option %c\n",
					USAGE, c);
		}
	if ((argc - optind) > 0)
		return(argv[optind]);
	else
		fprintf(stderr, "USAGE: %s; no filename or type given\n", USAGE);
}

char *
strip(name)
char	*name;
{
	char	*pe;

	for (pe = name + strlen(name); --pe > name; )
		if (*pe == '/')
			return(++pe);
	return(name);
}

char *
buildname(name)
char	*name;
{
	char	*typename, *filename;

	typename = strconcat(name, DB_SAMPLE_EXT);
	filename = strconcat(DB_PATH, typename);
	free(typename);
	return(filename);
}

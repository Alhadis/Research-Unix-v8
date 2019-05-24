#include	<stdio.h>
#include	<ctype.h>
#include	"search.h"
#include	"warn.h"
#include	"rd.h"

#define	USAGE	"monk [-d database_dir -n ] filename\n"
#define	OPTIONS	"d:n"

main(argc, argv)
int	argc;
char	**argv;
{
	extern int	optind;
	extern char	*optarg;
	extern short	database_mode;

	struct environment	*env;
	int	c;
	char	*database_source, *database_path;
	char	*filename;

	database_path = DB_PATH;
	database_mode = COMPRESSED;
	database_source = DB_COMPRESSED;
	filename = (char *) 0;
	while ((c = getopt(argc, argv, OPTIONS)) != EOF )
		switch (c) {
			case 'd':
				database_path = optarg;
				break;
			case 'n':
				database_mode = STANDARD;
				break;
			case '?':
				warn_user(0, "usage: %s; no option %c\n",
								USAGE, c);
		}
	if ((argc - optind) > 0)
		filename = argv[optind];
/* until remove ungetstring at end of style cannot handle stdin */
	else {
		warn_user(0, "usage: %s\n", USAGE);
		exit(0);
	}
	env = read_userfile(filename, database_path, (struct environment *) 0);
	end_allenvir(env);
	exit(0);
}

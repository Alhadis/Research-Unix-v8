#include	"cem.h"

/*
 *	Cemantics.  Intelligent C loader and binder.
 *
 *	Bruce Ellis	- Sept 1984.
 */

int	debug;
int	errors;
int	file_errors;
int	in_lib;
int	modtimes	= 1;
int	out_fid;
int	pedantic;
int	tell_times;
int	verbose;
char	*load_out;

char	*alloc_end;
char	*alloc_ptr;
char	*data_base;
char	*data_end;
char	*data_ptr;
char	*my_name;
char	*str_base;
inst	*global_list;
long	new_type_index;
long	str_num;
long	type_index;
long	var_index;
symbol	**str_trans;
symbol	*src_file;
type	**type_trans;
var	**var_trans;

/*
 *	cem
 *		-o file		output file
 *		-d		debug
 *		-m		override modtime quick check
 *		-p		be pedantic about types
 *		-t		tell all file times
 *		-v		verbose format for types
 *		-l lib		load library
 *		file		load file
 */
static void
usage()
{
	fprintf(stderr, "usage: %s [-o file] [-dmptv] [-l lib] file ...\n", my_name);
	exit(1);
}

int
main(argc, argv)
int	argc;
char	*argv[];
{
	register int	i;
	register int	j;
	extern char	*strrchr();
	extern void	check_externs();
	extern void	load_lib();
	extern void	load_obj();

	if ((my_name = strrchr(argv[0], '/')) == NULL || *++my_name == '\0')
		my_name = argv[0];

	setbuf(stdout, salloc((long)BUFSIZ));

	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			for (j = 1; argv[i][j] != '\0'; j++)
			{
				switch (argv[i][j])
				{
				case 'd':
					debug = 1;
					continue;

				case 'l':
					if (argv[i][++j] == '\0')
					{
						if (++i >= argc)
							usage();

						load_lib(argv[i]);
					}
					else
						load_lib(&argv[i][j]);

					break;

				case 'm':
					modtimes = 0;
					continue;

				case 'o':
					if (argv[i][++j] == '\0')
					{
						if (++i >= argc)
							usage();

						load_out = argv[i];
					}
					else
						load_out = &argv[i][j];

					break;

				case 'p':
					pedantic = 1;
					continue;

				case 't':
					tell_times = 1;
					continue;

				case 'v':
					verbose = 1;
					continue;

				default:
					usage();
				}

				break;
			}
		}
		else
			load_obj(argv[i]);
	}

	check_externs();

	return errors;
}

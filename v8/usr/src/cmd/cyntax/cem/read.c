#include	<errno.h>
#include	"cem.h"
#define	STD_OBJ	1
#include	"stdobj.h"

/*
 *	Read and load an object file.
 */
static void
read_obj(s, fd)
register char	*s;
register int	fd;
{
	register int	i;
	register long	sz;
	header		obj_header;
	static char	cmd[]	= CMD;
	static char	mac[]	= MAC;
	extern void	enter_types();
	extern void	enter_vars();
	extern void	install_strings();

	switch (read(fd, (char *)&obj_header, sizeof (header)))
	{
	case SYSERROR:
		fprintf(stderr, "%s: could not read ", my_name);
		fflush(stderr);
		perror(s);
		exit(1);

	case sizeof (header):
		break;

	default:
		fprintf(stderr, "%s: %s is not an object module\n", my_name, s);
		exit(1);
	}

	if (strncmp(obj_header.hd_cmd, cmd, CMD_SZ) != 0)
	{
		fprintf(stderr, "%s: %s is not an object module\n", my_name, s);
		exit(1);
	}

	if (strncmp(obj_header.hd_mac, mac, MAC_SZ) != 0)
	{
		fprintf(stderr, "%s: version mismatch for %s (recompile)\n", my_name, s);
		exit(1);
	}

	if (obj_header.hd_type_size == 0)
	{
		fprintf(stderr, "%s: %s has compilation errors\n", my_name, s);
		exit(1);
	}

	sz = obj_header.hd_str_off + obj_header.hd_str_size - sizeof (header);
	data_base = salloc(sz);
	str_base = data_base + obj_header.hd_str_off - sizeof (header) - 1;

	if ((i = read(fd, data_base, (int)sz)) != sz)
	{
		if (i == SYSERROR)
		{
			fprintf(stderr, "%s: could not read ", my_name);
			fflush(stderr);
			perror(s);
		}
		else
			fprintf(stderr, "%s: %s is too small\n", my_name, s);

		exit(1);
	}

#if	DEBUG
	if (debug)
	{
		register char	*id;
		register long	l0;
		register long	l1;
		register long	l2;

		data_ptr = data_base;
		data_end = data_base + obj_header.hd_str_off - sizeof (header);
		type_index = 1;
		var_index = 1;
		str_num = 0;

		while (data_ptr < data_end)
		{
			i = getd();

			switch (obj_item(i))
			{
			case i_data:
				l0 = getv();
				l1 = getv();
				printf("data: var %ld (%s:%ld)\n", l0, &str_base[l1], getv());

				loop
				{
					i = getd();

					switch (obj_item(i))
					{
					case d_addr:
						printf("\taddr var %ld for %ld\n", get4(), obj_id(i));
						continue;

					case d_bytes:
						if (obj_id(i) == 0)
							l0 = getv();
						else
							l0 = obj_id(i);

						printf("\tbytes %ld\n", l0);
						data_ptr += l0;
						continue;

					case d_end:
						printf("\tend\n");
						break;

					case d_istring:
						if (obj_id(i) == 0)
							l0 = getv();
						else
							l0 = obj_id(i);

						data_ptr += l0;
						printf("\tstring %ld, length %ld\n", str_num++, l0);
						continue;

					case d_irstring:
						if (obj_id(i) == 0)
							l0 = getv();
						else
							l0 = obj_id(i);

						data_ptr += l0;
						printf("\tstring %ld + %ld, length %ld\n", str_num++, get4(), l0);
						continue;

					case d_space:
						printf("\tspace %ld\n", obj_id(i) == 0 ? getv() : obj_id(i));
						continue;

					case d_string:
						printf("\tstring %ld\n", getv());
						continue;

					case d_reloc:
						l0 = getv();
						printf("\treloc var %ld + %ld for %ld\n", l0, get4(), obj_id(i));
						continue;

					case d_rstring:
						l0 = getv();
						printf("\tstring %ld + %ld\n", l0, get4());
						continue;

					default:
						fprintf(stderr, "%s: unknown data id %d\n", my_name, i);
						exit(1);
					}

					break;
				}

				break;

			case i_lib:
				printf("lib: %s\n", &str_base[getv()]);
				break;

			case i_src:
				printf("src: %s\n", &str_base[getv()]);
				break;

			case i_string:
				if (obj_id(i) == 0)
					l0 = getv();
				else
					l0 = obj_id(i);

				printf("string %ld, length %ld\n", str_num++, l0);
				data_ptr += l0;
				break;

			case i_type:
				printf("type ");

				switch (obj_id(i))
				{
				case t_arrayof:
					l0 = getv();
					l1 = getv();
					printf("%ld: array %ld of %ld\n", type_index++, l0, l1);
					break;

				case t_basetype:
					printf("%ld: ", type_index++);
					print_basetype(getd() & 0xFF);
					printf("\n");
					break;

				case t_bitfield:
					l0 = getv();
					l1 = getv();
					printf("%ld: bitfield %ld of %ld\n", type_index++, l0, l1);
					break;

				case t_dimless:
					printf("%ld: array [] of %ld\n", type_index++, getv());
					break;

				case t_elaboration:
					l0 = getv();
					l1 = getv();
					printf("elab %ld: (%s:%ld) ", l0, &str_base[l1], getv());
					
					switch (i = obj_id(getd()))
					{
					case t_enum:
						printf("enum\n");
						l0 = getv();
						goto elab_enum;

					case t_structof:
						printf("struct\n");
						l0 = getv();

						do
						{
							l1 = getv();
							printf("\t%s = %ld @ %ld\n", &str_base[l0], l1, getv());
							l0 = getv();
						}
						while (l0 != 0);

						printf("\t(%ld)\n", getv());
						break;

					case t_unionof:
						printf("union\n");
						l0 = getv();

						do
						{
							l1 = getv();
							printf("\t%s = %ld\n", &str_base[l0], l1);
							l0 = getv();
						}
						while (l0 != 0);

						printf("\t(%ld)\n", getv());
						break;

					default:
						fprintf(stderr, "%s: unknown elaboration id %d\n", my_name, i);
						exit(1);
					}

					break;

				case t_enum:
					printf("%ld: ", type_index++);

					if ((l0 = getv()) == 0)
						id = "<anon>";
					else
						id = &str_base[l0];

					l0 = getv();
					l1 = getv();

					printf("enum %s (%s:%ld)\n", id, &str_base[l0], l1);

					if ((l0 = getv()) == 0)
					{
						printf("\tforward\n");
						break;
					}

				elab_enum:
					do
					{
						l1 = getu();
						printf("\t%s = %ld\n", &str_base[l0], l1);
						l0 = getv();
					}
					while (l0 != 0);

					l0 = getu();
					l1 = getu();

					printf("\t(%ld, %ld)\n", l0, l1);
					break;

				case t_ftnreturning:
					printf("%ld: function returning %ld\n", type_index++, getv());
					break;

				case t_ptrto:
					printf("%ld: pointer to %ld\n", type_index++, getv());
					break;

				case t_structof:
					printf("%ld: ", type_index++);

					if ((l0 = getv()) == 0)
						id = "<anon>";
					else
						id = &str_base[l0];

					l0 = getv();
					l1 = getv();

					printf("struct %s (%s:%ld)\n", id, &str_base[l0], l1);
					break;

				case t_unionof:
					printf("%ld: ", type_index++);

					if ((l0 = getv()) == 0)
						id = "<anon>";
					else
						id = &str_base[l0];

					l0 = getv();
					l1 = getv();

					printf("union %s (%s:%ld)\n", id, &str_base[l0], l1);
					break;

				default:
					fprintf(stderr, "%s: unknown type id %d\n", my_name, obj_id(i));
					exit(1);
				}

				break;

			case i_var:
				printf("var ");

				switch (obj_id(i))
				{
				case v_arglist:
					l0 = getv();
					l1 = getv();
					printf("arglist: %ld (%s:%ld)\n", l0, &str_base[l1], getv());

					while ((l0 = getv()) != 0)
					{
						l1 = getv();
						l2 = getv();
						printf("\t%ld: %s type %ld (%s:%ld)\n", var_index++, &str_base[l0], l1, &str_base[l2], getv());
					}

					break;

				case v_array_size:
					l0 = getv();
					printf("array %ld new type %ld\n", l0, getv());
					break;

				case v_auto:
					id = "auto";
					goto get_var;

				case v_block_static:
					id = "block static";
					goto get_var;

				case v_call:
					l0 = getv();
					l1 = getv();
					printf("call: var %ld (%s:%ld)\n", l0, &str_base[l1], getv());

					while ((l0 = getv()) != 0)
						printf("\ttype %ld\n", l0);

					break;

				case v_global:
					id = "global";
					goto get_var;

				case v_implicit_function:
					id = "implicit()";
					goto get_var;

				case v_static:
					id = "static";
					goto get_var;

				get_var:
					l0 = getv();
					l1 = getv();
					l2 = getv();
					printf("%ld: %s %s type %ld (%s:%ld)\n", var_index++, id, &str_base[l0], l1, &str_base[l2], getv());
					break;

				case v_varargs:
					l0 = getv();
					printf("%ld: varargs %ld\n", l0, getv());
					break;

				default:
					fprintf(stderr, "%s: unknown var id %d\n", my_name, obj_id(i));
					exit(1);
				}

				break;

			default:
				fprintf(stderr, "%s: unknown obj_item %d\n", my_name, obj_item(i));
				exit(1);
			}
		}

		fflush(stdout);
	}
#endif	DEBUG

	file_errors = 0;
	install_strings(str_base + 1, obj_header.hd_str_size);

	type_index = 1;
	data_ptr = data_base;
	data_end = data_base + obj_header.hd_str_off - sizeof (header);
	enter_types(obj_header.hd_type_size);

	str_num = 0;
	var_index = 1;
	data_ptr = data_base;
	data_end = data_base + obj_header.hd_str_off - sizeof (header);
	enter_vars(obj_header.hd_var_size);

	free((char *)str_trans);
	free((char *)type_trans);
	free((char *)var_trans);
	free(data_base);
	src_file = NULL;

	if (file_errors)
		fflush(stdout);
}

/*
 *	Open a library and load it.
 */
void
load_lib(l)
char	*l;
{
	register int	fd;
	register char	*s;
	extern int	errno;
	extern char	*strcat();
	extern char	*strcpy();

	s = salloc(strlen(LIB_PATH) + strlen(l) + 1L);
	(void)strcat(strcpy(s, LIB_PATH), l);

	if ((fd = open(s, 0)) == SYSERROR)
	{
		if (errno == ENOENT)
			fprintf(stderr, "%s: no library '-l%s'\n", my_name, l);
		else
		{
			fprintf(stderr, "%s: could not open ", my_name);
			fflush(stderr);
			perror(s);
		}

		exit(1);
	}

	read_obj(s, fd);
	free(s);
	(void)close(fd);
}

/*
 *	Open an object file and load it.
 */
void
load_obj(s)
register char	*s;
{
	register int	fd;

	if ((fd = open(s, 0)) == SYSERROR)
	{
		fprintf(stderr, "%s: could not open ", my_name);
		fflush(stderr);
		perror(s);
		exit(1);
	}

	read_obj(s, fd);
	(void)close(fd);
}

#define	STD_OBJ	1
#include	"stdobj.h"
#include	"types.h"
#include	"cem.h"
#include	"symbol.h"
#include	"type.h"

/*
 *	Routines for generating diagnostics.
 */

#define	err(c)		putchar(c)
#define	err_str(s)	printf(str_fmt, (s))
#define	err_num(l)	printf(num_fmt, (l))

static char	*num_fmt	= "%ld";
static char	*str_fmt	= "%s";
static int	time_forced;

extern char	*strchr();

/*
 *	File names match and so do modtimes.
 */
int
match_times(s, t)
symbol	*s;
symbol	*t;
{
	return s == t && strchr(s->sy_name, TIME_SEP) != NULL;
}

/*
 *	Print the name of a basetype.
 */
void
print_basetype(my_type)
register int	my_type;
{
	register int	bit;
	register int	spoken;

	static char		*type_names[] =
	{
		"unsigned",
		"long",
		"char",
		"short",
		"int",
		"float",
		"void",
		"<bad type>",
	};

	if (!pedantic)
	{
		if ((my_type & (LONG | FLOAT)) == (LONG | FLOAT))
		{
			err_str("double");
			return;
		}
		else if ((my_type & (LONG | SHORT)) != 0)
			my_type &= ~INT;
	}

	spoken = 0;
	
	for (bit = 0; my_type != 0; bit++, my_type >>= 1)
	{
		if ((my_type & 1) != 0)
		{
			if (spoken)
				err(' ');

			spoken++;
			err_str(type_names[bit]);
		}
	}
}

/*
 *	Print a type as one might say it.
 */
static void
put_verbose_type(t)
register type	*t;
{
	register int	spoken;
	register int	plural;

	spoken = 0;
	plural = 0;

	while (t != NULL)
	{
		if (spoken)
			err(' ');

		switch (t->t_type)
		{
		case t_arrayof:
			printf("array[%ld]", t->d.dim);

			if (plural)
				err('s');

			err_str(" of");
			plural = 1;
			break;

		case t_basetype:
			print_basetype(t->d.mask);

			if (plural)
				err('s');

			return;

		case t_bitfield:
			err_str("bitfield");

			if (plural)
				err('s');

			err_str(" of");
			break;

		case t_dimless:
			err_str("array[]");

			if (plural)
				err('s');

			err_str(" of");
			plural = 1;
			break;

		case t_ftnreturning:
			err_str("function");

			if (plural)
				err('s');

			err_str(" returning");
			break;

		case t_ptrto:
			err_str("pointer");

			if (plural)
				err('s');

			err_str(" to");
			break;

		case t_enum:
			err_str("enum");

			if (t->d.e->e_name != NULL)
				printf(" %s", t->d.e->e_name->sy_name);

			err_str(" (");
			put_file(t->d.e->e_file, t->d.e->e_line);
			err(')');
			return;

		case t_structof:
			err_str("struct");

			if (t->d.s->s_name != NULL)
				printf(" %s", t->d.s->s_name->sy_name);

			err_str(" (");
			put_file(t->d.s->s_file, t->d.s->s_line);
			err(')');
			return;

		case t_unionof:
			err_str("union");

			if (t->d.u->u_name != NULL)
				printf(" %s", t->d.u->u_name->sy_name);

			err_str(" (");
			put_file(t->d.u->u_file, t->d.u->u_line);
			err(')');
			return;

		default:
			err_str("unknown");

			if (plural)
				err('s');
		}

		t = t->t_subtype;
		spoken = 1;
	}
}

/*
 *	Stash routines for 'put_c_type'.  We need to be able
 *	to build up a type by 'outputing' characters before and
 *	after what we have already said.  The average type is
 *	expected to take up no more than TYPE_SIZE chars (not counting
 *	basetype or complex base which always comes first).  The
 *	amount of backup is expected not to exceed HEAD_SIZE chars.
 *	If these are exceeded we extend in the appropriate direction
 *	by TYPE_EXTEND chars.
 *
 *	str_buff:
 *
 *	0 - unused - str_head - stuff - str_tail - unused - str_limit
 */
static int	str_head;
static int	str_tail;
static int	str_start;
static int	str_limit;
static char	*str_buff;

#define	TYPE_SIZE	64
#define	TYPE_EXTEND	32
#define	HEAD_SIZE	16

static void
str_init()
{
	str_limit = TYPE_SIZE;
	str_start = HEAD_SIZE;
	str_buff = salloc((long)str_limit);
}

static void
str_extend(ext, head)
int	ext;
int	head;
{
	register char	*p;
	register char	*q;

	str_limit += ext;
	str_buff = srealloc(str_buff, (long)str_limit);

	if (head)
	{
		str_start += ext;
		str_head += ext;
		str_tail += ext;
		p = str_buff + str_limit;
		q = p - ext;

		while (q > str_buff)
			*--p = *--q;
	}
}

static void
head_char(c)
int	c;
{
	if (--str_head < 0)
		str_extend(HEAD_SIZE, 1);

	str_buff[str_head] = c;
}

static void
tail_char(c)
int	c;
{
	if (str_tail == str_limit)
		str_extend(TYPE_EXTEND, 0);

	str_buff[str_tail++] = c;
}

static void
tail_str(s)
register char	*s;
{
	while (*s != '\0')
		tail_char(*s++);
}

/*
 *	Print a type as it may have been declared.
 */
static void
put_c_type(ct)
type	*ct;
{
	register type	*t;
	char		buff[16];

	for (t = ct; t != NULL; t = t->t_subtype)
	{
		switch (t->t_type)
		{
		case t_arrayof:
			sprintf(buff, "[%ld]", t->d.dim);
			tail_str(buff);
			break;

		case t_basetype:
			print_basetype(t->d.mask);
			goto finish;

		case t_bitfield:
			sprintf(buff, ":%ld", t->d.size);
			tail_str(buff);
			break;

		case t_dimless:
			tail_str("[]");
			break;

		case t_ftnreturning:
			if (t != ct)
			{
				head_char('(');
				tail_char(')');
			}

			tail_str("()");
			break;

		case t_ptrto:
			head_char('*');
			break;

		case t_enum:
			err_str("enum");

			if (t->d.e->e_name != NULL)
				printf(" %s", t->d.e->e_name->sy_name);

			err_str(" (");
			put_file(t->d.e->e_file, t->d.e->e_line);
			err(')');
			goto finish;

		case t_structof:
			err_str("struct");

			if (t->d.s->s_name != NULL)
				printf(" %s", t->d.s->s_name->sy_name);

			err_str(" (");
			put_file(t->d.s->s_file, t->d.s->s_line);
			err(')');
			goto finish;

		case t_unionof:
			err_str("union");

			if (t->d.u->u_name != NULL)
				printf(" %s", t->d.u->u_name->sy_name);

			err_str(" (");
			put_file(t->d.u->u_file, t->d.u->u_line);
			err(')');
			goto finish;

		default:
			err_str("unknown");

		finish:
			if (t != ct)
				err(' ');

			return;
		}
	}
}

/*
 *	Print a type, choose between the two formats.
 */
void
put_type(t, force)
register type	*t;
int		force;
{
	time_forced = force;

	if (verbose)
		put_verbose_type(t);
	else
	{
		if (str_buff == NULL)
			str_init();

		str_head = str_start;
		str_tail = str_start;
		put_c_type(t);
		tail_char('\0');
		err_str(&str_buff[str_head]);
	}

	time_forced = 0;
}

/*
 *	Print a filename.  Strip leading occurences of "./".
 *	Perhaps also print the mod time.
 */
static void
format_filename(s)
register char	*s;
{
	register char	*p;
	register long	l;
	extern char	*strrchr();
	extern long	atol();

	while (s[0] == '.' && s[1] == '/' && s[2] != '\0')
		s += 2;

	if ((p = strrchr(s, TIME_SEP)) == NULL)
		printf("%s", s);
	else
	{
		*p++ = '\0';
		err_str(s);

		if ((time_forced || tell_times) && (l = atol(p)) != 0)
			printf(" [%s]", stime(l));

		*--p = TIME_SEP;
	}
}

/*
 *	Print the current filename and note that this file has errors.
 *	Used for file-static errors.
 */
void
say_file()
{
	format_filename(src_file->sy_name);
	file_errors = 1;
}

/*
 *	Print a filename and line number.  Omit filename if it is
 *	the current file.
 */

void
put_file(f, l)
symbol	*f;
long	l;
{
	if (f == src_file)
		printf("%ld", l);
	else
	{
		format_filename(f->sy_name);

		if (l != 0)
			printf(": %ld", l);
	}	
}

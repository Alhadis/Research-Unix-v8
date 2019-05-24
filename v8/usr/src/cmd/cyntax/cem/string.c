#include	"cem.h"

/*
 *	String table management routines.
 *
 *	str_pages is a vector of pointers to pages which hold the
 *	the string table.  str_ptr points into the current page
 *	and str_end is the end of the current page.  str_limit
 *	is how many pages str_pages can hold.  str_count is the
 *	count of pages.  str_index is the index into the resultant
 *	string table (offset by 1 so that 0 means "no string").
 */

static char	*str_ptr;
static char	*str_end;
static char	**str_pages;
static int	str_count;
static int	str_limit;
static long	str_index	= 1;

/*
 *	Dump the string table to the output file and return the size.
 */
long
dump_strings()
{
	register int	i;
	register int	j;

	for (i = 0, j = str_count - 1; i < j; i++)
	{
		if (write(out_fid, str_pages[i], OUTZ) == SYSERROR)
		{
			fprintf(stderr, "%s: ", my_name);
			perror("could not write output");
			exit(1);
		}
	}

	if (str_ptr != str_end && write(out_fid, str_pages[i], str_ptr - str_pages[i]) == SYSERROR)
	{
		fprintf(stderr, "%s: ", my_name);
		perror("could not write output");
		exit(1);
	}

	return str_index - 1;
}

/*
 *	Add a string to the string table.  Return its index (via p)
 *	and a pointer to a printable version of it.  Normally this
 *	pointer points into the string table but if the string
 *	straddles pages we allocate and copy.  The length is passed
 *	as a courtesy.
 */
char	*
str_alloc(s, len, p)
register char	*s;
register int	len;
long		*p;
{
	register char	*q;
	register char	*r;

	*p = str_index;
	str_index += len;
	q = str_ptr;

	if (q + len > str_end)
	{
		register char	*e;
		register char	*t;

		/*
		 *	String straddles page boundary.
		 */
		r = alloc(len);
		t = r;
		e = str_end;

		while (--len >= 0)
		{
			if (q == e)
			{
				/*
				 *	Allocate a new page.
				 */
				q = salloc((long)OUTZ);

				if (str_count == str_limit)
				{
					/*
					 *	Extend page pointer vector.
					 */
					str_limit += STR_INC;
					str_pages = vector(str_pages, str_limit, char *);
				}

				e = &q[OUTZ];
				str_pages[str_count++] = q;
			}

			*q++ = *s;
			*t++ = *s++;
		}

		str_end = e;
		str_ptr = q;
	}
	else
	{
		r = q;

		while (--len >= 0)
			*q++ = *s++;

		str_ptr = q;
	}

	return r;
}

/*
 *	Install an input string table into the symbol table and
 *	output string table.  Make a table which translates old
 *	string table indexes into symbol table pointers.
 */
void
install_strings(p, n)
register char	*p;
register long	n;
{
	register symbol	**v;
	extern symbol	*find_symbol();

	/*
	 *	n is the size of the string table.  allocate
	 *	a slot for 0 ("no string") and put a NULL in it.
	 */
	v = (symbol **)salloc((n + 1) * sizeof (symbol *));
	str_trans = v;
	*v++ = NULL;

	while (n > 0)
	{
		*v++ = find_symbol(p);
		n--;

		while (*p++ != '\0')
		{
			*v++ = NULL;
			n--;
		}
	}
}

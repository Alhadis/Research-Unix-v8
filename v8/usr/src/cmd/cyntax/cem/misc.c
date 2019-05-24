#include	"cem.h"

/*
 *	Allocate some memory.
 */
char	*
salloc(n)
long	n;
{
	register char	*p;
	extern char	*malloc();

	if ((p = malloc((unsigned int)n)) == NULL)
	{
		fprintf(stderr, "%s: ran out of memory\n", my_name);
		exit(1);
	}

	return p;
}
/*
 *	Reallocate some memory.
 */

char	*
srealloc(p, n)
register char	*p;
long		n;
{
	extern char	*realloc();

	if (p == NULL)
		return salloc(n);
	else if ((p = realloc(p, (unsigned int)n)) == NULL)
	{
		fprintf(stderr, "%s: ran out of memory\n", my_name);
		exit(1);
	}

	return p;
}

/*
 *	Refill the alloc buffer.
 */
char	*
alloc_fill(want)
long	want;
{
	register long	sz;

	if (want < ALLOC_SIZE)
		sz = ALLOC_SIZE;
	else
		sz = want;

	alloc_ptr = salloc(sz);
	alloc_end = alloc_ptr + sz;
	alloc_ptr += want;
	return alloc_ptr - want;
}

/*
 *	Fetch 'u' datum.
 *
 *	B0 = 11xxxxxx
 *
 *		B0 5:0 sext to 31:24
 *		B1 7:0      as 23:16
 *		B2 7:0      as 15: 8
 *		B3 7:0      as  7: 0
 *
 *	B0 = 10xxxxxx
 *
 *		B0 5:0 sext to 31: 8
 *		B1 7:0      as  7: 0
 *
 *	B0 = 0xxxxxxx
 *
 *		B0 6:0 sext to 31: 0
 */
long
getu()
{
	register int	i;
	register long	j;

	switch ((i = getd()) & 0xC0)
	{
	case 0xC0:
		j = ((i & 0x3F) << 8) | (getd() & 0xFF);
		j = (j << 8) | (getd() & 0xFF);
		j = (j << 8) | (getd() & 0xFF);

		if ((i & 0x20) == 0)
			return j;
		else
			return j | 0xC0000000L;

	case 0x80:
		j = ((i & 0x3F) << 8) | (getd() & 0xFF);

		if ((i & 0x20) == 0)
			return j;
		else
			return j | 0xFFFFC000L;

	case 0x40:
		return (i & 0x3F) | 0xFFFFFF80L;
	}

	return i;
}

/*
 *	Fetch 'v' datum.  Format as 'u' but zero extend.
 */
long
getv()
{
	register int	i;
	register long	j;

	switch ((i = getd()) & 0xC0)
	{
	case 0xC0:
		j = ((i & 0x3F) << 8) | (getd() & 0xFF);
		j = (j << 8) | (getd() & 0xFF);
		return (j << 8) | (getd() & 0xFF);

	case 0x80:
		return ((i & 0x3F) << 8) | (getd() & 0xFF);
	}

	return i & 0xFF;
}

/*
 *	Fetch '4' datum.
 */
long
get4()
{
	register long	j;

	j = getd() & 0xFF;
	j =  (j << 8) | (getd() & 0xFF);
	j =  (j << 8) | (getd() & 0xFF);
	return  (j << 8) | (getd() & 0xFF);
}

/*
 *	Skip 'u' or 'v' datum.
 */
void
skip()
{
	switch (getd() & 0xC0)
	{
	case 0xC0:
		getd();
		getd();
		getd();
		break;

	case 0x80:
		getd();
	}
}

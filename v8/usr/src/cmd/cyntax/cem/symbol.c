#include	"cem.h"
#include	"symbol.h"

/*
 *	stab is a hash table of pointers to binary trees.
 */
static symbol	*stab[STABZ];

/*
 *	Map the string 's' to its string table entry, creating a new one
 *	if required.
 */
symbol	*
find_symbol(s)
register char	*s;
{
	register int	i;
	register symbol	**n;
	register char	*p;
	register int	length;
	extern char	*str_alloc();

	for (i = 0, length = 1, p = s; *p != '\0'; i += i ^ *p++)
		length++;

	if (i < 0)
		i = -i;

	n = &stab[i % STABZ];

	while (*n != NULL)
	{
		register char	*q;

		p = s;
		q = (*n)->sy_name;

		while ((i = *p ^ *q) == 0 && *p++ != '\0' && *q++ != '\0')
			;

		if (i == 0)
			return *n;

		n = i & 1 ? &((*n)->sy_left) : &((*n)->sy_right);
	}

	*n = talloc(symbol);
	(*n)->sy_name = str_alloc(s, length, &((*n)->sy_index));
	(*n)->sy_left = NULL;
	(*n)->sy_right = NULL;
	(*n)->sy_inst = NULL;

	return *n;
}

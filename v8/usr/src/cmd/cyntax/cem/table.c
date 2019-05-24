#include	"cem.h"
#define	STD_OBJ	1
#include	"stdobj.h"
#include	"symbol.h"
#include	"type.h"

/*
 *	Type table routines.
 */

extern char	*strchr();

/*
 *	Comparison routines.  "Identical type" equivalence.
 *	We believe file modtimes unless we are told to doubt.
 */

static int
cmp_enum(p, q)
type	*p;
type	*q;
{
	register e_data	*e;
	register e_data	*f;
	register e_elt	*g;
	register e_elt	*h;
	register int	i;

	e = p->d.e;
	f = q->d.e;

	if (modtimes && e->e_line == f->e_line && match_times(e->e_file, f->e_file))
		return 0;

	if (e->e_name != f->e_name)
	{
		if (e->e_name == NULL)
			return -f->e_name->sy_index;
		else if (f->e_name == NULL)
			return e->e_name->sy_index;
		else
			return e->e_name->sy_index - f->e_name->sy_index;
	}

	if ((i = e->e_low - f->e_low) != 0 || (i = e->e_high - f->e_high) != 0)
		return i;

	g = e->e_list;
	h = f->e_list;

	while (g != NULL)
	{
		if (h == NULL)
			return -1;

		if (g->e_name != h->e_name)
			return g->e_name->sy_index - h->e_name->sy_index;

		if ((i = g->e_value - h->e_value) != 0)
			return i;

		g = g->e_next;
		h = h->e_next;
	}

	if (h != NULL)
		return 1;

	return 0;
}

static int
cmp_struct(p, q)
type	*p;
type	*q;
{
	register s_data	*e;
	register s_data	*f;
	register s_elt	*g;
	register s_elt	*h;
	register int	i;

	e = p->d.s;
	f = q->d.s;

	if (modtimes && e->s_line == f->s_line && match_times(e->s_file, f->s_file))
		return 0;

	if (e->s_name != f->s_name)
	{
		if (e->s_name == NULL)
			return -f->s_name->sy_index;
		else if (f->s_name == NULL)
			return e->s_name->sy_index;
		else
			return e->s_name->sy_index - f->s_name->sy_index;
	}

	if ((i = e->s_size - f->s_size) != 0)
		return i;

	g = e->s_list;
	h = f->s_list;

	while (g != NULL)
	{
		if (h == NULL)
			return -1;

		if (g->s_name != h->s_name)
			return g->s_name->sy_index - h->s_name->sy_index;

		if (g->s_type != h->s_type)
			return g->s_type->t_index - h->s_type->t_index;

		g = g->s_next;
		h = h->s_next;
	}

	if (h != NULL)
		return 1;

	return 0;
}

static int
cmp_union(p, q)
type	*p;
type	*q;
{
	register u_data	*e;
	register u_data	*f;
	register u_elt	*g;
	register u_elt	*h;
	register int	i;

	e = p->d.u;
	f = q->d.u;

	if (modtimes && e->u_line == f->u_line && match_times(e->u_file, f->u_file))
		return 0;

	if (e->u_name != f->u_name)
	{
		if (e->u_name == NULL)
			return -f->u_name->sy_index;
		else if (f->u_name == NULL)
			return e->u_name->sy_index;
		else
			return e->u_name->sy_index - f->u_name->sy_index;
	}

	if ((i = e->u_size - f->u_size) != 0)
		return i;

	g = e->u_list;
	h = f->u_list;

	while (g != NULL)
	{
		if (h == NULL)
			return -1;

		if (g->u_name != h->u_name)
			return g->u_name->sy_index - h->u_name->sy_index;

		if (g->u_type != h->u_type)
			return g->u_type->t_index - h->u_type->t_index;

		g = g->u_next;
		h = h->u_next;
	}

	if (h != NULL)
		return 1;

	return 0;
}

/*
 *	We win a lot by trusting the file/modtime linenumber
 *	of a struct/unions declaration.
 */
#define	THASHSZ	37
#define	TPRIME0	7
#define	TPRIME1	31

/*
 *	Type hashing by filename/modtime and line number.
 */
static type	*
type_hash(h0, h1, t)
long	h0;
long	h1;
type	*t;
{
	typedef struct thash	thash;

	struct thash
	{
		long	h0;
		long	h1;
		type	*t;
		thash	*next;
	};

	register long	i;
	register thash	*p;
	static thash	*thasht[THASHSZ];

	i = TPRIME0 * h0 + TPRIME1 * h1;

	if (i < 0)
		i = -i;

	i %= THASHSZ;

	for (p = thasht[i]; p != NULL; p = p->next)
	{
		if (p->h0 == h0 && p->h1 == h1)
			return p->t;

	}

	if (t == NULL)
		return NULL;

	p = talloc(thash);
	p->h0 = h0;
	p->h1 = h1;
	p->t = t;
	p->next = thasht[i];
	thasht[i] = p;
	return t;
}

/*
 *	Lookup a type hashing by filename/modtime and line number.
 */
type	*
lookup_hash(s, l)
symbol	*s;
long	l;
{
	if (modtimes && strchr(s->sy_name, TIME_SEP) != NULL)
		return type_hash(s->sy_index, l, (type *)NULL);
	else
		return NULL;
}

/*
 *	Enter a type hashing by filename/modtime and line number.
 */
void
enter_hash(s, l, t)
symbol	*s;
long	l;
type	*t;
{
	if (modtimes && strchr(s->sy_name, TIME_SEP) != NULL)
		type_hash(s->sy_index, l, t);
}

/*
 *	Type table manager.  Pass it a type and it will return you the
 *	(strongly) equivalent type that characterises it.  i.e. it
 *	'interns' a type.  Type comparison is then pointer comparison.
 */
type	*
find_type(t)
register type	*t;
{
	typedef struct ttnode	ttnode;

	struct ttnode
	{
		type	*tt_type;
		ttnode	*tt_left;
		ttnode	*tt_right;
	};

	register type	*p;
	register type	*q;
	register int	i;
	register ttnode	**n;
	register ttnode	*tt;

	static ttnode	*type_table[(int)t_types];

	n = &type_table[(int)t->t_type];

	while (*n != NULL)
	{
		p = t;
		q = (*n)->tt_type;

		if ((i = ((int)p->t_type) - ((int)q->t_type)) == 0)
		{
			switch (p->t_type)
			{
			case t_arrayof:
				if ((i = p->d.dim - q->d.dim) == 0)
					i = p->t_subtype->t_index - q->t_subtype->t_index;

				break;

			case t_basetype:
				i = p->d.mask - q->d.mask;
				break;

			case t_bitfield:
				if ((i = p->d.size - q->d.size) == 0)
					i = p->t_subtype->t_index - q->t_subtype->t_index;

				break;

			case t_dimless:
			case t_ftnreturning:
			case t_ptrto:
				i = p->t_subtype->t_index - q->t_subtype->t_index;
				break;

			case t_enum:
				i = cmp_enum(p, q);
				break;

			case t_structof:
				i = cmp_struct(p, q);
				break;

			case t_unionof:
				i = cmp_union(p, q);
				break;

			default:
				fprintf(stderr, "%s: find_type - bad type\n", my_name);
				exit(1);
			}

			if (i == 0)
			{
				delete_type(t);
				return (*n)->tt_type;
			}
		}

		n = i < 0 ? &((*n)->tt_left) : &((*n)->tt_right);
	}

	switch (t->t_type)
	{
	case t_structof:
		enter_hash(t->d.s->s_file, t->d.s->s_line, t);
		break;

	case t_unionof:
		enter_hash(t->d.u->u_file, t->d.u->u_line, t);
	}

	tt = talloc(ttnode);
	*n = tt;
	tt->tt_type = t;
	tt->tt_left = NULL;
	tt->tt_right = NULL;
	t->t_index = new_type_index++;
	return t;
}

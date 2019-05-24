#include	"cem.h"
#define	STD_OBJ	1
#include	"stdobj.h"
#include	"types.h"
#include	"symbol.h"
#include	"type.h"
#include	"var.h"

/*
 *	Type equivalence routines.
 */

/*
 *	Does p differ from q because of a file modtime difference?
 */
int
different_vintage(p, q)
register type	*p;
register type	*q;
{
	register char	*s;
	register char	*t;

	loop
	{
		if (p->t_type != q->t_type)
			return 0;

		switch (p->t_type)
		{
		case t_arrayof:
			if (p->d.size != q->d.size)
				return 0;

		case t_dimless:
		case t_ftnreturning:
		case t_ptrto:
			p = p->t_subtype;
			q = q->t_subtype;
			continue;

		case t_basetype:
		case t_bitfield:
			return 0;

		case t_enum:
			s = p->d.e->e_file->sy_name;
			t = q->d.e->e_file->sy_name;
			break;

		case t_structof:
			s = p->d.s->s_file->sy_name;
			t = q->d.s->s_file->sy_name;
			break;

		case t_unionof:
			s = p->d.u->u_file->sy_name;
			t = q->d.u->u_file->sy_name;
			break;

		default:
			fprintf(stderr, "%s: bad type in different_vintage\n", my_name);
			exit(1);
		}

		if (s == t)
			return 0;

		while (*s++ == *t)
		{
			if (*t++ == TIME_SEP)
				return 1;
		}

		return 0;
	}
}

/*
 *	Is p an elaboration of q (complex type as base).
 */
int
complex_elaboration(p, q)
register type	*p;
register type	*q;
{
	register symbol	*s;
	register symbol	*t;

	loop
	{
		if (p->t_type != q->t_type)
			return 0;

		switch (p->t_type)
		{
		case t_arrayof:
			if (p->d.size != q->d.size)
				return 0;

		case t_dimless:
		case t_ftnreturning:
		case t_ptrto:
			p = p->t_subtype;
			q = q->t_subtype;
			continue;

		case t_basetype:
		case t_bitfield:
			return 0;

		case t_enum:
			if (p->d.e->e_list == NULL || q->d.e->e_list != NULL)
				return 0;

			s = p->d.e->e_name;
			t = q->d.e->e_name;
			break;

		case t_structof:
			if (p->d.s->s_list == NULL || q->d.s->s_list != NULL)
				return 0;

			s = p->d.s->s_name;
			t = q->d.s->s_name;
			break;

		case t_unionof:
			if (p->d.u->u_list == NULL || q->d.u->u_list != NULL)
				return 0;

			s = p->d.u->u_name;
			t = q->d.u->u_name;
			break;

		default:
			fprintf(stderr, "%s: bad type in complex_elaboration\n", my_name);
			exit(1);
		}

		return s == t || s == NULL;
	}
}

/*
 *	Is p an elaboration of q (a dimensionless array).
 */
int
array_elaboration(p, q)
register type	*p;
register type	*q;
{
	return p->t_type == t_arrayof && q->t_type == t_dimless && p->t_subtype == q->t_subtype;
}

/*
 *	Can t be coerced (silently) to u (reflexive).
 */
int
coercible(t, u)
type	*t;
type	*u;
{
	register int	e;
	register int	b;

	if (pedantic)
		return 0;

	switch (t->t_type)
	{
	case t_basetype:
		if ((t->d.mask & FLOAT) != 0)
			return 0;

		b = 1;
		e = 0;
		break;

	case t_enum:
		b = 0;
		e = 1;
		break;

	default:
		return 0;
	}

	switch (u->t_type)
	{
	case t_basetype:
		if ((u->d.mask & FLOAT) != 0)
			return 0;

		b++;
		break;

	case t_enum:
		e++;
		break;

	default:
		return 0;
	}

	if (b == 2)
		return (t->d.mask & ~UNSIGNED) == (u->d.mask & ~UNSIGNED);

	return e != 2;
}

static int	really_compatible();

/*
 *	Struct/union member compatible.
 */
static int
member_compatible(p, q)
type	*p;
type	*q;
{
	if (p == q)
		return 1;

	return really_compatible(p, q);
}

/*
 *	Is p compatible with q (reflexive).
 */
static int
really_compatible(p, q)
register type	*p;
register type	*q;
{
	typedef struct busy	busy;

	struct busy
	{
		type	*b_p;
		type	*b_q;
		busy	*b_next;
	};

	busy		this;
	static busy	*busy_list;

	loop
	{
		if (p->t_type != q->t_type)
		{
			if (p->t_type == t_arrayof || p->t_type == t_dimless)
				p = p->t_subtype;
			else
				return 0;

			if (q->t_type == t_arrayof || q->t_type == t_dimless)
				q = q->t_subtype;
			else
				return 0;

			/*
			 *	If the types are now equal one is an array
			 *	elaboration of the other.
			 */
			if (p == q)
				return 1;

			continue;
		}

		switch (p->t_type)
		{
		case t_arrayof:
			if (p->d.size != q->d.size)
				return 0;

		case t_dimless:
		case t_ftnreturning:
		case t_ptrto:
			p = p->t_subtype;
			q = q->t_subtype;
			break;

		case t_basetype:
		case t_bitfield:
			return 0;

		case t_enum:
			if
			(
				p->d.e->e_name != q->d.e->e_name
				&&
				p->d.e->e_name != NULL
				&&
				q->d.e->e_name != NULL
			)
				return 0;

			if (p->d.e->e_list == NULL || q->d.e->e_list == NULL)
					return 1;

			return 0;

		case t_structof:
			if
			(
				p->d.s->s_name != q->d.s->s_name
				&&
				p->d.s->s_name != NULL
				&&
				q->d.s->s_name != NULL
			)
				return 0;

			if (p->d.s->s_list == NULL || q->d.s->s_list == NULL)
					return 1;

			if (p->d.s->s_size != q->d.s->s_size)
				return 0;

			{
				register busy	*sb;

				for (sb = busy_list; sb != NULL; sb = sb->b_next)
				{
					if
					(
						sb->b_p == p && sb->b_q == q
						||
						sb->b_p == q && sb->b_q == p
					)
						return 1;
				}
			}

			if (modtimes && p->d.s->s_line == q->d.s->s_line && match_times(p->d.s->s_file, q->d.s->s_file))
				return 1;

			this.b_p = p;
			this.b_q = q;
			this.b_next = busy_list;
			busy_list = &this;

			{
				register s_elt	*a;
				register s_elt	*b;

				a = p->d.s->s_list;
				b = q->d.s->s_list;

				while (a != NULL)
				{
					if (b == NULL || !member_compatible(a->s_type, b->s_type))
					{
						busy_list = this.b_next;
						return 0;
					}

					a = a->s_next;
					b = b->s_next;
				}

				busy_list = this.b_next;
				return b == NULL;
			}

		case t_unionof:
			if
			(
				p->d.u->u_name != q->d.u->u_name
				&&
				p->d.u->u_name != NULL
				&&
				q->d.u->u_name != NULL
			)
				return 0;

			if (p->d.u->u_list == NULL || q->d.u->u_list == NULL)
					return 1;

			if (p->d.u->u_size != q->d.u->u_size)
				return 0;

			{
				register busy	*ub;

				for (ub = busy_list; ub != NULL; ub = ub->b_next)
				{
					if
					(
						ub->b_p == p && ub->b_q == q
						||
						ub->b_p == q && ub->b_q == p
					)
						return 1;
				}
			}

			if (modtimes && p->d.u->u_line == q->d.u->u_line && match_times(p->d.u->u_file, q->d.u->u_file))
				return 1;

			this.b_p = p;
			this.b_q = q;
			this.b_next = busy_list;
			busy_list = &this;

			{
				register u_elt	*a;
				register u_elt	*b;

				a = p->d.u->u_list;
				b = q->d.u->u_list;

				while (a != NULL)
				{
					if (b == NULL || !member_compatible(a->u_type, b->u_type))
					{
						busy_list = this.b_next;
						return 0;
					}

					a = a->u_next;
					b = b->u_next;
				}

				busy_list = this.b_next;
				return b == NULL;
			}

		default:
			fprintf(stderr, "%s: bad type in really_compatible\n", my_name);
			exit(1);
		}
	}
}

int
compatible_arglist(a, b, v)
register arg	*a;
register arg	*b;
int		v;
{
	register args	*p;
	register args	*q;
	register int	n;

	if (v < 0)
	{
		if (a->a_count != b->a_count)
			return 0;

		n = a->a_count;
	}
	else
	{
		if (b->a_count < v)
			return 0;

		n = v;
	}

	p = a->a_head;
	q = b->a_head;

	while (--n >= 0)
	{
		if (p->a_type != q->a_type && !compatible(p->a_type, q->a_type))
			return 0;

		p = p->a_next;
		q = q->a_next;
	}

	return 1;
}

int
coercible_arglist(a, b, v)
register arg	*a;
register arg	*b;
int		v;
{
	register args	*p;
	register args	*q;
	register int	n;

	if (v < 0)
	{
		if (a->a_count != b->a_count)
			return 0;

		n = a->a_count;
	}
	else
	{
		if (b->a_count < v)
			return 0;

		n = v;
	}

	p = a->a_head;
	q = b->a_head;

	while (--n >= 0)
	{
		if (p->a_type != q->a_type && !coercible(p->a_type, q->a_type))
			return 0;

		p = p->a_next;
		q = q->a_next;
	}

	return 1;
}

/*
 *	We win a lot by hashing previous results.
 */
#define	CHASHSZ	37
#define	CPRIME0	7
#define	CPRIME1	31

/*
 *	'compatible' hashing by type indices.
 */
static int
hash_compatible(h0, h1, r)
long	h0;
long	h1;
int	r;
{
	typedef struct chash	chash;

	struct chash
	{
		long	h0;
		long	h1;
		int	r;
		chash	*left;
		chash	*right;
	};

	register long	i;
	register long	j;
	register chash	**n;
	static chash	*chasht[CHASHSZ];

	i = CPRIME0 * h0 + CPRIME1 * h1;

	if (i < 0)
		j = -i % CHASHSZ;
	else
		j = i % CHASHSZ;

	n = &chasht[j];

	while (*n != NULL)
	{
		j = CPRIME0 * (*n)->h0 + CPRIME1 * (*n)->h1 - i;

		if (j == 0 && (*n)->h0 == h0 && (*n)->h1 == h1)
			return (*n)->r;

		n = j > 0 ? &(*n)->left : &(*n)->right;
	}

	if (r >= 0)
	{
		*n = talloc(chash);
		(*n)->h0 = h0;
		(*n)->h1 = h1;
		(*n)->r = r;
		(*n)->left = NULL;
		(*n)->right = NULL;
	}

	return r;
}

/*
 *	Is p compatible with q (reflexive).  This includes coercible.
 */
int
compatible(p, q)
type	*p;
type	*q;
{
	register int	ret;

	if (coercible(p, q))
		return 1;

	if
	(
		(ret = hash_compatible(p->t_index, q->t_index, -1)) < 0
		&&
		(ret = hash_compatible(q->t_index, p->t_index, -1)) < 0
	)
	{
		ret = really_compatible(p, q);
		hash_compatible(p->t_index, q->t_index, ret);
	}

	return ret;
}

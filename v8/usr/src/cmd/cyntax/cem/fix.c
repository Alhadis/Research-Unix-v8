#include	"cem.h"
#define	STD_OBJ	1
#include	"stdobj.h"
#include	"symbol.h"
#include	"type.h"

/*
 *	Type fixing routines.
 */

static char	*interned;

/*
 *	Fix up a type.  First pass.  Intern all basetypes and enums
 *	and modifications of these.
 */
static void
fix_pure_simple(n, x)
type	**n;
int	x;
{
	register type	*p;

	p = *n;

	switch (p->t_type)
	{
	case t_arrayof:
	case t_bitfield:
	case t_dimless:
	case t_ftnreturning:
	case t_ptrto:
		if (!interned[p->t_sub.index])
			return;

		p->t_subtype = type_trans[p->t_sub.index];
		break;

	case t_basetype:
	case t_enum:
		break;

	case t_structof:
		if ((p = lookup_hash(p->d.s->s_file, p->d.s->s_line)) != NULL)
		{
			interned[x] = 1;
			delete_type(*n);
			*n = p;
		}

		return;

	case t_unionof:
		if ((p = lookup_hash(p->d.u->u_file, p->d.u->u_line)) != NULL)
		{
			interned[x] = 1;
			delete_type(*n);
			*n = p;
		}

		return;

	default:
		fprintf(stderr, "%s: unknown type in fix_pure_simple\n", my_name);
		exit(1);
	}

	interned[x] = 1;
	*n = find_type(p);
}

/*
 *	Fix up a type.  Second pass.  Intern structs and unions with
 *	exclusively pure components.
 */
static int
fix_pure_complex(n, x)
type	**n;
int	x;
{
	register s_elt	*s;
	register u_elt	*u;
	register type	*p;

	if (interned[x])
		return 0;

	p = *n;

	switch (p->t_type)
	{
	case t_arrayof:
	case t_bitfield:
	case t_dimless:
	case t_ftnreturning:
	case t_ptrto:
		if (!interned[p->t_sub.index])
			return 0;

		p->t_subtype = type_trans[p->t_sub.index];
		break;

	case t_basetype:
	case t_enum:
		return 0;

	case t_structof:
		for (s = p->d.s->s_list; s != NULL; s = s->s_next)
		{
			if (!interned[s->s_ptype.index])
				return 0;
		}

		for (s = p->d.s->s_list; s != NULL; s = s->s_next)
			s->s_type = type_trans[s->s_ptype.index];

		break;

	case t_unionof:
		for (u = p->d.u->u_list; u != NULL; u = u->u_next)
		{
			if (!interned[u->u_ptype.index])
				return 0;
		}

		for (u = p->d.u->u_list; u != NULL; u = u->u_next)
			u->u_type = type_trans[u->u_ptype.index];

		break;

	default:
		fprintf(stderr, "%s: unknown type in fix_pure_complex\n", my_name);
		exit(1);
	}

	interned[x] = 1;
	*n = find_type(p);
	return 1;
}

/*
 *	Fix up a type.  Final pass.  Fill in subtypes.
 */
static void
elab_complex(p, x)
register type	*p;
int		x;
{
	register s_elt	*s;
	register u_elt	*u;

	if (interned[x])
		return;

	p->t_index = new_type_index++;

	switch (p->t_type)
	{
	case t_arrayof:
	case t_bitfield:
	case t_dimless:
	case t_ftnreturning:
	case t_ptrto:
		p->t_subtype = type_trans[p->t_sub.index];
		break;

	case t_basetype:
	case t_enum:
		break;

	case t_structof:
		for (s = p->d.s->s_list; s != NULL; s = s->s_next)
			s->s_type = type_trans[s->s_ptype.index];

		break;

	case t_unionof:
		for (u = p->d.u->u_list; u != NULL; u = u->u_next)
			u->u_type = type_trans[u->u_ptype.index];

		break;

	default:
		fprintf(stderr, "%s: unknown type in elab_complex\n", my_name);
		exit(1);
	}
}

/*
 *	Fix the types in the type translation table.  If they are 'pure'
 *	intern them else elaborate them.
 */
void
fix_types(n)
register long	n;
{
	register char	*p;
	register type	**v;
	register int	have_fixed;
	register int	i;

	v = type_trans;
	p = (char *)salloc(n * sizeof (char));
	interned = p;
	i = n;

	while (--i >= 0)
		*p++ = 0;

	for (i = 1; i < n; i++)
		fix_pure_simple(&v[i], i);

	do
	{
		have_fixed = 0;

		for (i = 1; i < n; i++)
		{
			if (fix_pure_complex(&v[i], i))
				have_fixed = 1;
		}
	}
	while (have_fixed);

	for (i = 1; i < n; i++)
		elab_complex(v[i], i);

	free(interned);
}

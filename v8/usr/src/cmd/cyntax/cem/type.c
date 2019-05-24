#include	"cem.h"
#define	STD_OBJ	1
#include	"stdobj.h"
#include	"symbol.h"
#include	"type.h"

/*
 *	Type creation routines.
 */

/*
 *	We keep a cache of data objects rather than allowing malloc
 *	to inefficiently recycle them.
 */
static e_elt	*e_elt_free;
static s_elt	*s_elt_free;
static u_elt	*u_elt_free;
static e_data	*e_data_free;
static s_data	*s_data_free;
static u_data	*u_data_free;
static type	*type_free;

/*
 *	Allocation routines.
 */
static type	*
new_type()
{
	register type	*p;

	if (type_free == NULL)
		return talloc(type);

	p = type_free;
	type_free = p->t_subtype;
	return p;
}

static void
free_e_elt(p)
e_elt	*p;
{
	p->e_next = e_elt_free;
	e_elt_free = p;
}

static void
free_s_elt(p)
s_elt	*p;
{
	p->s_next = s_elt_free;
	s_elt_free = p;
}

static void
free_u_elt(p)
u_elt	*p;
{
	p->u_next = u_elt_free;
	u_elt_free = p;
}

static void
free_type(p)
type	*p;
{
	p->t_subtype = type_free;
	type_free = p;
}

static void
free_e_data(p)
e_data	*p;
{
	p->e_u.e_n = e_data_free;
	e_data_free = p;
}

static void
free_s_data(p)
s_data	*p;
{
	p->s_u.s_n = s_data_free;
	s_data_free = p;
}

static void
free_u_data(p)
u_data	*p;
{
	p->u_u.u_n = u_data_free;
	u_data_free = p;
}

static void
free_elist(p)
register e_elt	*p;
{
	register e_elt	*q;

	while (p != NULL)
	{
		q = p->e_next;
		p->e_next = e_elt_free;
		e_elt_free = p;
		p = q;
	}
}

static void
free_slist(p)
register s_elt	*p;
{
	register s_elt	*q;

	while (p != NULL)
	{
		q = p->s_next;
		p->s_next = s_elt_free;
		s_elt_free = p;
		p = q;
	}
}

static void
free_ulist(p)
register u_elt	*p;
{
	register u_elt	*q;

	while (p != NULL)
	{
		q = p->u_next;
		p->u_next = u_elt_free;
		u_elt_free = p;
		p = q;
	}
}

void
delete_type(p)
register type	*p;
{
	switch (p->t_type)
	{
	case t_enum:
		free_elist(p->d.e->e_list);
		p->d.e->e_u.e_n = e_data_free;
		e_data_free = p->d.e;
		break;

	case t_structof:
		free_slist(p->d.s->s_list);
		p->d.s->s_u.s_n = s_data_free;
		s_data_free = p->d.s;
		break;

	case t_unionof:
		free_ulist(p->d.u->u_list);
		p->d.u->u_u.u_n = u_data_free;
		u_data_free = p->d.u;
	}

	p->t_subtype = type_free;
	type_free = p;
}

/*
 *	Constructors.
 */
static type	*
make_enum(n, f, l)
symbol	*n;
symbol	*f;
long	l;
{
	register type	*p;
	register e_data	*e;

	if (e_data_free == NULL)
		e = talloc(e_data);
	else
	{
		e = e_data_free;
		e_data_free = e->e_u.e_n;
	}

	p = new_type();
	p->t_type = t_enum;
	p->d.e = e;
	e->e_file = f;
	e->e_line = l;
	e->e_name = n;
	e->e_list = NULL;
	return p;
}

static type	*
make_struct(n, f, l)
symbol	*n;
symbol	*f;
long	l;
{
	register type	*p;
	register s_data	*s;

	if (s_data_free == NULL)
		s = talloc(s_data);
	else
	{
		s = s_data_free;
		s_data_free = s->s_u.s_n;
	}

	p = new_type();
	p->t_type = t_structof;
	p->d.s = s;
	s->s_file = f;
	s->s_line = l;
	s->s_name = n;
	s->s_list = NULL;
	return p;
}

static type	*
make_union(n, f, l)
symbol	*n;
symbol	*f;
long	l;
{
	register type	*p;
	register u_data	*u;

	if (u_data_free == NULL)
		u = talloc(u_data);
	else
	{
		u = u_data_free;
		u_data_free = u->u_u.u_n;
	}

	p = new_type();
	p->t_type = t_unionof;
	p->d.u = u;
	u->u_file = f;
	u->u_line = l;
	u->u_name = n;
	u->u_list = NULL;
	return p;
}

/*
 *	Elaborators.
 */
static void
elab_enum(p, f, l, m)
type	*p;
symbol	*f;
long	l;
long	m;
{
	register e_data	*e;
	register e_elt	**n;
	register long	l0;

	e = p->d.e;
	e->e_file = f;
	e->e_line = l;
	n = &e->e_list;
	l0 = m;

	do
	{
		if (e_elt_free == NULL)
			*n = talloc(e_elt);
		else
		{
			*n = e_elt_free;
			e_elt_free = (*n)->e_next;
		}

		(*n)->e_name = str_trans[l0];
		(*n)->e_value = getv();
		n = &(*n)->e_next;
	}
	while ((l0 = getv()) != 0);

	*n = NULL;
	e->e_low = getu();
	e->e_high = getu();
}

static void
elab_struct(p, f, l)
type	*p;
symbol	*f;
long	l;
{
	register s_data	*s;
	register s_elt	**n;
	register long	l0;

	s = p->d.s;
	s->s_file = f;
	s->s_line = l;
	n = &s->s_list;

	l0 = getv();

	do
	{
		if (s_elt_free == NULL)
			*n = talloc(s_elt);
		else
		{
			*n = s_elt_free;
			s_elt_free = (*n)->s_next;
		}

		(*n)->s_name = str_trans[l0];
		(*n)->s_ptype.index = getv();
		(*n)->s_offset = getv();
		n = &(*n)->s_next;
	}
	while ((l0 = getv()) != 0);

	*n = NULL;
	s->s_size = getv();
}

static void
elab_union(p, f, l)
type	*p;
symbol	*f;
long	l;
{
	register u_data	*u;
	register u_elt	**n;
	register long	l0;

	u = p->d.u;
	u->u_file = f;
	u->u_line = l;
	n = &u->u_list;

	l0 = getv();

	do
	{
		if (u_elt_free == NULL)
			*n = talloc(u_elt);
		else
		{
			*n = u_elt_free;
			u_elt_free = (*n)->u_next;
		}

		(*n)->u_name = str_trans[l0];
		(*n)->u_ptype.index = getv();
		n = &(*n)->u_next;
	}
	while ((l0 = getv()) != 0);

	*n = NULL;
	u->u_size = getv();
}

/*
 *	Enter types from input file.  Construct type translation table.
 */
void
enter_types(n)
register long	n;
{
	register int	i;
	register type	*p;
	register type	**v;

	v = (type **)salloc(n * sizeof (type *));
	type_trans = v;

	while (data_ptr < data_end)
	{
		register long	l0;
		register long	l1;
		register long	l2;

		i = getd();

		switch (obj_item(i))
		{
		case i_data:
			skip();
			skip();
			skip();

			loop
			{
				i = getd();

				switch (obj_item(i))
				{
				case d_addr:
					skip4();
					continue;

				case d_bytes:
					if (obj_id(i) == 0)
						l0 = getv();
					else
						l0 = obj_id(i);

					data_ptr += l0;
					continue;

				case d_end:
					break;

				case d_istring:
					if (obj_id(i) == 0)
						l0 = getv();
					else
						l0 = obj_id(i);

					str_num++;
					data_ptr += l0;
					continue;

				case d_irstring:
					if (obj_id(i) == 0)
						l0 = getv();
					else
						l0 = obj_id(i);

					str_num++;
					data_ptr += l0;
					skip4();
					continue;

				case d_space:
					if (obj_id(i) == 0)
						skip();

					continue;

				case d_string:
					skip();
					continue;

				case d_reloc:
				case d_rstring:
					skip();
					skip4();
					continue;

				default:
					fprintf(stderr, "%s: unknown data id %d\n", my_name, i);
					exit(1);
				}

				break;
			}

			break;

		case i_lib:
			in_lib = 1;
			src_file = str_trans[getv()];
			break;

		case i_src:
			in_lib = 0;
			src_file = str_trans[getv()];
			break;

		case i_string:
			if (obj_id(i) == 0)
				l0 = getv();
			else
				l0 = obj_id(i);

			data_ptr += l0;
			break;

		case i_type:
			switch (obj_id(i))
			{
			case t_arrayof:
				p = talloc(type);
				p->t_type = t_arrayof;
				p->d.dim = getv();
				p->t_subindex = getv();
				v[type_index++] = p;
				break;

			case t_basetype:
				p = talloc(type);
				p->t_type = t_basetype;
				p->d.mask = getd();
				v[type_index++] = p;
				break;

			case t_bitfield:
				p = talloc(type);
				p->t_type = t_bitfield;
				p->d.size = getv();
				p->t_subindex = getv();
				v[type_index++] = p;
				break;

			case t_dimless:
				p = talloc(type);
				p->t_type = t_dimless;
				p->t_subindex = getv();
				v[type_index++] = p;
				break;

			case t_elaboration:
				l0 = getv();
				l1 = getv();
				l2 = getv();

				switch (i = obj_id(getd()))
				{
				case t_enum:
					elab_enum(v[l0], str_trans[l1], l2, getv());
					break;

				case t_structof:
					elab_struct(v[l0], str_trans[l1], l2);
					break;

				case t_unionof:
					elab_union(v[l0], str_trans[l1], l2);
					break;

				default:
					fprintf(stderr, "%s: unknown elaboration id %d\n", my_name, i);
					exit(1);
				}

				break;

			case t_enum:
				l0 = getv();
				l1 = getv();
				l2 = getv();
				v[type_index] = make_enum(str_trans[l0], str_trans[l1], l2);
				l0 = getv();

				if (l0 != 0)
					elab_enum(v[type_index], str_trans[l1], l2, l0);
				type_index++;
				break;

			case t_ftnreturning:
				p = talloc(type);
				p->t_type = t_ftnreturning;
				p->t_subindex = getv();
				v[type_index++] = p;
				break;

			case t_ptrto:
				p = talloc(type);
				p->t_type = t_ptrto;
				p->t_subindex = getv();
				v[type_index++] = p;
				break;

			case t_structof:
				l0 = getv();
				l1 = getv();
				l2 = getv();
				v[type_index++] = make_struct(str_trans[l0], str_trans[l1], l2);
				break;

			case t_unionof:
				l0 = getv();
				l1 = getv();
				l2 = getv();
				v[type_index++] = make_union(str_trans[l0], str_trans[l1], l2);
				break;

			default:
				fprintf(stderr, "%s: unknown type id %d\n", my_name, obj_id(i));
				exit(1);
			}

			break;

		case i_var:
			switch (obj_id(i))
			{
			case v_arglist:
				skip();
				skip();
				skip();

				while (getv() != 0)
				{
					skip();
					skip();
					skip();
				}

				break;

			case v_array_size:
				skip();
				skip();
				break;

			case v_call:
				skip();
				skip();
				skip();

				while (getv() != 0)
					;

				break;

			case v_auto:
			case v_block_static:
			case v_global:
			case v_implicit_function:
			case v_static:
				skip();
				skip();
				skip();
				skip();
				break;

			case v_varargs:
				skip();
				skip();
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

	fix_types(n);
}

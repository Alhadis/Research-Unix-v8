#include	"cem.h"
#define	STD_OBJ	1
#include	"stdobj.h"
#include	"types.h"
#include	"equiv.h"
#include	"symbol.h"
#include	"type.h"
#include	"var.h"

/*
 *	Variable routines.
 */

static long	new_var_index	= 1;

/*
 *	We keep a cache of data objects rather than allowing malloc
 *	to inefficiently recycle them.
 */
static arg	*arg_free;
static args	*args_free;
static var	*var_free;

/*
 *	Allocation routines.
 */
static var	*
new_var()
{
	register var	*p;

	if (var_free == NULL)
		return talloc(var);

	p = var_free;
	var_free = p->v_next;
	return p;
}

static void
free_arglist(a)
register arg	*a;
{
	register args	*p;
	register args	*q;

	for (p = a->a_head; p != NULL; p = q)
	{
		q = p->a_next;
		p->a_next = args_free;
		args_free = p;
	}

	a->a_next = arg_free;
	arg_free = a;
}

static arg	*
new_arg()
{
	register arg	*p;

	if ((p = arg_free) == NULL)
		p = talloc(arg);
	else
		arg_free = p->a_next;

	p->a_head = NULL;
	p->a_tail = &p->a_head;
	p->a_count = 0;
	p->a_next = NULL;
	return p;
}

static void
free_arguments(a)
register arg	*a;
{
	register arg	*b;

	while (a != NULL)
	{
		b = a->a_next;
		free_arglist(a);
		a = b;
	}
}

static void
add_argument(a, t)
register arg	*a;
type		*t;
{
	register args	*p;

	if ((p = args_free) == NULL)
		p = talloc(args);
	else
		args_free = p->a_next;

	p->a_type = t;
	p->a_next = NULL;
	*a->a_tail = p;
	a->a_tail = &p->a_next;
	a->a_count++;
}

/*
 *	Make a declaration.
 */
static var	*
declare(v, n, t, f, l)
obj_vars	v;
symbol		*n;
type		*t;
symbol		*f;
long		l;
{
	register var	*p;

	p = new_var();
	p->v_what = v;
	p->v_name = n;
	p->v_type = t;
	p->v_file = f;
	p->v_line = l;
	p->v_ifile = NULL;
	p->v_varargs = -1;
	p->v_argdefn = NULL;
	p->v_args = NULL;
	p->v_tail = &p->v_args;
	p->v_next = NULL;
	return p;
}

/*
 *	Set array size.
 */
static void
array_size(p, t)
var	*p;
type	*t;
{
	p->v_type = t;
}

/*
 *	Note initialisation.
 */
static void
initialisation(p, f, l)
var	*p;
symbol	*f;
long	l;
{
	p->v_ifile = f;
	p->v_iline = l;
}

/*
 *	Note varargs count.
 */
static void
set_varargs(p, l)
var	*p;
long	l;
{
	p->v_varargs = l;
}

/*
 *	Add an instance of a variable to the symbol table instance list.
 */
static void
add_instance(v)
register var	*v;
{
	register inst	*p;
	register defn	*d;
	register init	*i;

	/*
	 *	Don't let library definitions override.
	 */
	if (in_lib && v->v_name->sy_inst != NULL && v->v_name->sy_inst->i_init)
		return;

	if ((p = v->v_name->sy_inst) == NULL)
	{
		p = talloc(inst);
		v->v_name->sy_inst = p;
		p->i_argdefn = v->v_argdefn;

		if ((p->i_args = v->v_args) == NULL)
			p->i_tail = &p->i_args;
		else
			p->i_tail = v->v_tail;

		d = talloc(defn);
		p->i_defn = d;
		p->i_init = NULL;
		p->i_name = v->v_name;
		d->d_what = v->v_what;
		d->d_type = v->v_type;

		if (in_lib)
		{
			d->d_file = src_file;
			d->d_line = 0;
		}
		else
		{
			d->d_file = v->v_file;
			d->d_line = v->v_line;
		}

		d->d_next = NULL;
		p->i_next = global_list;
		global_list = p;
	}
	else
	{
		if (v->v_argdefn != NULL)
			p->i_argdefn = v->v_argdefn;

		if ((*p->i_tail = v->v_args) != NULL)
			p->i_tail = v->v_tail;

		d = p->i_defn;

		loop
		{
			if (d == NULL)
			{
				d = talloc(defn);
				d->d_next = p->i_defn;
				p->i_defn = d;

			elab:
				d->d_what = v->v_what;
				d->d_type = v->v_type;

				if (in_lib)
				{
					d->d_file = src_file;
					d->d_line = 0;
				}
				else
				{
					d->d_file = v->v_file;
					d->d_line = v->v_line;
				}

				break;
			}

			if (d->d_type == v->v_type)
				break;

			if (complex_elaboration(v->v_type, d->d_type))
				goto elab;

			if (array_elaboration(v->v_type, d->d_type))
				goto elab;

			if (compatible(d->d_type, v->v_type))
				break;

			d = d->d_next;
		}
	}

	if (v->v_ifile != NULL)
	{
		i = talloc(init);

		if (in_lib)
		{
			i->i_file = src_file;
			i->i_line = 0;
		}
		else
		{
			i->i_file = v->v_ifile;
			i->i_line = v->v_iline;
		}

		i->i_varargs = v->v_varargs;
		i->i_next = p->i_init;
		p->i_init = i;
	}
}

/*
 *	Add an arglist (call) to args list.  If it is ceorcible to
 *	the definition don't bother.
 */
static void
add_arglist(v, a)
register var	*v;
register arg	*a;
{
	if (v->v_argdefn != NULL && coercible_arglist(v->v_argdefn, a, -1))
	{
		free_arglist(a);
		return;
	}

	*v->v_tail = a;
	v->v_tail = &a->a_next;
	a->a_next = NULL;
}

/*
 *	Complain about improper calls.
 */
static void
put_improper_calls(d, u, v, af, tf)
register arg	*d;
register arg	*u;
int		v;
int		(*af)();
int		(*tf)();
{
	register args	*p;
	register args	*q;
	register char	*sep;
	register int	n;
	int		i;
	int		force;

	while (u != NULL)
	{
		if (!(*af)(d, u, v))
		{
			putchar('\t');
			put_file(u->a_file, u->a_line);

			if (v >= 0 && u->a_count < v)
				printf(", expected at least %d arg%s, found %d\n", v, v == 1 ? "" : "s", u->a_count);
			else if (d->a_count != u->a_count)
				printf(", expected %d arg%s, found %d\n", d->a_count, d->a_count == 1 ? "" : "s", u->a_count);
			else
			{
				if (v < 0)
					n = d->a_count;
				else
					n = v;

				i = 1;
				sep = ", ";
				p = d->a_head;
				q = u->a_head;

				while (--n >= 0)
				{
					if (p->a_type != q->a_type && !(*tf)(p->a_type, q->a_type))
					{
						force = different_vintage(p->a_type, q->a_type);
						printf("%sarg %d expected ", sep, i);
						put_type(p->a_type, force);
						printf(", found ");
						put_type(q->a_type, force);
						sep = "\n\t\t";
					}

					i++;
					p = p->a_next;
					q = q->a_next;
				}

				putchar('\n');
			}
		}

		u = u->a_next;
	}
}

/*
 *	Enter vars from input file.  Check statics and enter instances.
 */
void
enter_vars(n)
register long	n;
{
	register int	i;
	register long	j;
	register var	*p;
	register var	**v;

	v = (var **)salloc(n * sizeof (var *));
	var_trans = v;

	for (j = 0; j < n; j++)
		*v++ = NULL;

	v = var_trans;

	while (data_ptr < data_end)
	{
		register long	l0;
		register long	l1;
		register long	l2;
		arg		*ap;

		i = getd();

		switch (obj_item(i))
		{
		case i_data:
			l0 = getv();
			l1 = getv();
			initialisation(var_trans[l0], str_trans[l1], getv());

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
		case i_src:
			skip();
			break;

		case i_string:
			if (obj_id(i) == 0)
				l0 = getv();
			else
				l0 = obj_id(i);

			str_num++;
			data_ptr += l0;
			break;

		case i_type:
			switch (obj_id(i))
			{
			case t_arrayof:
			case t_bitfield:
				skip();
				skip();
				break;

			case t_basetype:
				(void)getd();
				break;

			case t_dimless:
			case t_ftnreturning:
			case t_ptrto:
				skip();
				break;

			case t_elaboration:
				skip();
				skip();
				skip();
				
				switch (i = obj_id(getd()))
				{
				case t_enum:
					skip();
					goto elab_enum;

				case t_structof:
					skip();

					do
					{
						skip();
						skip();
					}
					while (getv() != 0);

					skip();
					break;

				case t_unionof:
					skip();

					do
						skip();
					while (getv() != 0);

					skip();
					break;

				default:
					fprintf(stderr, "%s: unknown elaboration id %d\n", my_name, i);
					exit(1);
				}

				break;

			case t_enum:
				skip();
				skip();
				skip();

				if (getv() == 0)
					break;

			elab_enum:
				do
					skip();
				while (getv() != 0);

				skip();
				skip();
				break;

			case t_structof:
			case t_unionof:
				skip();
				skip();
				skip();
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
				l0 = getv();
				l1 = getv();
				initialisation(var_trans[l0], str_trans[l1], getv());
				ap = new_arg();

				while (getv() != 0)
				{
					add_argument(ap, type_trans[getv()]);
					skip();
					skip();
					var_index++;
				}

				var_trans[l0]->v_argdefn = ap;
				break;

			case v_array_size:
				l0 = getv();
				array_size(var_trans[l0], type_trans[getv()]);
				break;

			case v_auto:
				var_index++;
				skip();
				skip();
				skip();
				skip();
				break;

			case v_call:
				l0 = getv();
				ap = new_arg();
				ap->a_file = str_trans[getv()];
				ap->a_line = getv();

				while ((l1 = getv()) != 0)
					add_argument(ap, type_trans[l1]);

				add_arglist(var_trans[l0], ap);
				break;

			case v_block_static:
			case v_global:
			case v_implicit_function:
			case v_static:
				l0 = getv();
				l1 = getv();
				l2 = getv();
				var_trans[var_index++] = declare(obj_id(i), str_trans[l0], type_trans[l1], str_trans[l2], getv());
				break;

			case v_varargs:
				l0 = getv();
				set_varargs(var_trans[l0], getv());
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

	for (j = 1; j < n; j++)
	{
		register arg	*a;

		if ((p = v[j]) == NULL)
			continue;

		switch (p->v_what)
		{
		case v_block_static:
			break;

		case v_global:
		case v_implicit_function:
			add_instance(p);
			break;

		case v_static:
			switch (p->v_type->t_type)
			{
				char	*diag;

			error:
				say_file();
				printf(", static %s %s, ", diag, p->v_name->sy_name);
				put_file(p->v_file, p->v_line);
				printf(", never defined\n");
				errors++;
				break;

			case t_dimless:
				diag = "array[]";
				goto error;

			case t_ftnreturning:
				if (p->v_ifile == NULL)
				{
					diag = "function";
					goto error;
				}

				for (a = p->v_args; a != NULL; a = a->a_next)
				{
					if (!coercible_arglist(p->v_argdefn, a, p->v_varargs))
					{
						say_file();
						printf(": static function %s: ", p->v_name->sy_name);
						put_file(p->v_file, p->v_line);
						printf("\n");
						put_improper_calls(p->v_argdefn, a, p->v_varargs, coercible_arglist, coercible);
						errors++;
						break;
					}
				}

				free_arguments(p->v_args);
				free_arguments(p->v_argdefn);
			}

			break;

		default:
			fprintf(stderr, "%s: bad var type\n", my_name);
			exit(1);
		}

		p->v_next = var_free;
		var_free = p;
	}
}

/*
 *	Check for mutliple declarations and definitions.
 *	Also check arglist compatibility.
 */
static void
check_instance(p)
register inst	*p;
{
	register defn	*d;
	register init	*i;
	register arg	*a;

	if (p->i_defn->d_next != NULL)
	{
		printf("%s multiply declared:\n", p->i_name->sy_name);

		for (d = p->i_defn; d != NULL; d = d->d_next)
		{
			putchar('\t');
			put_file(d->d_file, d->d_line);

			if (d->d_what == v_implicit_function)
				printf(" implicitly");

			printf(" as ");
			put_type(d->d_type, 0);
			putchar('\n');
		}

		errors++;
	}

	if (p->i_init != NULL)
	{
		if (p->i_init->i_next != NULL)
		{
			printf("%s multiply defined:\n", p->i_name->sy_name);

			for (i = p->i_init; i != NULL; i = i->i_next)
			{
				putchar('\t');
				put_file(i->i_file, i->i_line);
				putchar('\n');
			}

			errors++;
			return;
		}

		if (p->i_argdefn != NULL)
		{
			for (a = p->i_args; a != NULL; a = a->a_next)
			{
				if (!compatible_arglist(p->i_argdefn, a, p->i_init->i_varargs))
				{
					printf("function %s: ", p->i_name->sy_name);
					put_file(p->i_init->i_file, p->i_init->i_line);
					printf("\n");
					put_improper_calls(p->i_argdefn, a, p->i_init->i_varargs, compatible_arglist, compatible);
					errors++;
					break;
				}
			}
		}
	}
}

void
check_externs()
{
	register inst	*p;

	for (p = global_list; p != NULL; p = p->i_next)
		check_instance(p);
}

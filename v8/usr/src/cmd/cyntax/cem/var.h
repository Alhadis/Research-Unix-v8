/*
 *	Structures for variables, initialisations
 *	and argument lists.
 */
typedef struct arg	arg;
typedef struct args	args;
typedef struct defn	defn;
typedef struct init	init;

struct arg
{
	args	*a_head;
	args	**a_tail;
	int	a_count;
	symbol	*a_file;
	long	a_line;
	arg	*a_next;
};

struct args
{
	type	*a_type;
	args	*a_next;
};

struct defn
{
	obj_vars d_what;
	type	*d_type;
	symbol	*d_file;
	long	d_line;
	defn	*d_next;
};

struct init
{
	symbol	*i_file;
	long	i_line;
	int	i_varargs;
	init	*i_next;
};

struct inst
{
	symbol	*i_name;
	arg	*i_argdefn;
	arg	*i_args;
	arg	**i_tail;
	defn	*i_defn;
	init	*i_init;
	inst	*i_next;
};

struct var
{
	obj_vars v_what;
	symbol	*v_name;
	type	*v_type;
	symbol	*v_file;
	long	v_line;
	symbol	*v_ifile;
	long	v_iline;
	long	v_index;
	int	v_varargs;
	arg	*v_argdefn;
	arg	*v_args;
	arg	**v_tail;
	var	*v_next;
};

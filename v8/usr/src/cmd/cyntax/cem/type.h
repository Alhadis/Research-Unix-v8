/*
 *	Type structures.
 */

typedef struct e_elt	e_elt;
typedef struct s_elt	s_elt;
typedef struct u_elt	u_elt;
typedef struct e_data	e_data;
typedef struct s_data	s_data;
typedef struct u_data	u_data;

typedef union
{
	long	index;
	type	*ptr;
}
	ptype;

struct e_elt
{
	long	e_value;
	symbol	*e_name;
	e_elt	*e_next;
};

struct s_elt
{
	ptype	s_ptype;
	long	s_offset;
	symbol	*s_name;
	s_elt	*s_next;
};

#define	s_type	s_ptype.ptr

struct u_elt
{
	ptype	u_ptype;
	long	u_offset;
	symbol	*u_name;
	u_elt	*u_next;
};

#define	u_type	u_ptype.ptr

struct e_data
{
	symbol	*e_file;
	long	e_line;
	symbol	*e_name;
	long	e_low;
	long	e_high;

	union
	{
		e_elt	*e_l;
		e_data	*e_n;
	}
		e_u;
};

#define	e_list	e_u.e_l

struct s_data
{
	symbol	*s_file;
	long	s_line;
	symbol	*s_name;
	long	s_size;

	union
	{
		s_elt	*s_l;
		s_data	*s_n;
	}
		s_u;
};

#define	s_list	s_u.s_l

struct u_data
{
	symbol	*u_file;
	long	u_line;
	symbol	*u_name;
	long	u_size;

	union
	{
		u_elt	*u_l;
		u_data	*u_n;
	}
		u_u;
};

#define	u_list	u_u.u_l

struct type
{
	obj_types	t_type;
	ptype		t_sub;

	union
	{
		long	dim;
		int	mask;
		int	size;
		e_data	*e;
		s_data	*s;
		u_data	*u;
	}
		d;

	long	t_index;
};

#define	t_subindex	t_sub.index
#define	t_subtype	t_sub.ptr

extern void	delete_type();
extern void	enter_hash();
extern void	fix_types();
extern type	*find_type();
extern type	*lookup_hash();

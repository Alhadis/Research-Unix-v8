#include	<stdio.h>

/*
 *	Cemantics.  Intelligent C loader and binder.
 */

#ifndef	DEBUG
#define	DEBUG		1
#endif	DEBUG

/*
 *	LIB_PATH is prefix for library pathnames.
 *	-lxxx maps to LIB_PATHxxx
 */
#ifndef	LIB_PATH
#define	LIB_PATH	"/usr/lib/cyntax/lib"
#endif	LIB_PATH

/*
 *	ALLOC_SIZE	Memory allocation chunk for alloc() style macros.
 *	OUTZ		Output buffer size.
 *	STABZ		Symbol table size.
 *	STR_INC		String allocation increment (page pointers).
 */
#define	ALLOC_SIZE	(8*1024)
#define	OUTZ		(4*1024)
#define	STABZ		251
#define	STR_INC		16

typedef struct inst	inst;
typedef struct symbol	symbol;
typedef struct type	type;
typedef struct var	var;

extern char	*alloc_end;
extern char	*alloc_ptr;
extern char	*my_name;
extern char	*load_out;
extern int	debug;
extern int	errors;
extern int	file_errors;
extern int	in_lib;
extern int	modtimes;
extern int	out_fid;
extern int	pedantic;
extern int	tell_times;
extern int	verbose;
extern char	*data_base;
extern char	*str_base;
extern char	*data_end;
extern char	*data_ptr;
extern long	new_type_index;
extern long	str_num;
extern long	type_index;
extern long	var_index;
extern inst	*global_list;
extern symbol	*src_file;
extern symbol	**str_trans;
extern type	**type_trans;
extern var	**var_trans;

extern char	*alloc_fill();
extern char	*salloc();
extern char	*srealloc();
extern char	*stime();
extern long	getu();
extern long	getv();
extern long	get4();
extern void	print_basetype();
extern void	put_file();
extern void	put_type();
extern void	say_file();
extern void	skip();

#define	SYSERROR	(-1)

#define	loop		for (;;)
#define	getd()		(*data_ptr++)
#define	skip4()		(data_ptr += 4)
#define talloc(t)	(t *)alloc(sizeof (t))
#define	alloc(n)	((alloc_ptr += (n)) > alloc_end ? alloc_fill((long)n) : (alloc_ptr - (n)))
#define	vector(p, n, t)	(t *)srealloc((char *)p, (long)(n) * sizeof (t))

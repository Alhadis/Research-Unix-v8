#include <sys/types.h>
#include <sys/stat.h>
#include "a.out.h"
#include "mcc.h"
#include "sdb.h"
#include "names.h"

struct ramnl {
	char		n_name[8];	/* identical to struct nlist */
	char		n_type;
	char		n_other;
	unsigned short 	n_desc;
	long		n_value;

	long		n_extra;	/* additional fields in ram for joff*/
	struct ramnl	*n_thread;
};

struct stable{
	char		*name;		/* Should never change */
	long		magic;		/* Expected magic - need not be */
	MLONG		rom;

	char		path[128];	/* Changes */
	struct ramnl	*base;
	int		size;
	struct ramnl	*threader[0x100];
	struct exec	header;
	MLONG		text;
	time_t		st_mtime;
	int		fd;
};

#define MAGREL 0406
#define MAGABS 0407

#define II( a,b )			( a | (  b << 8 ) )
#define III( a,b,c )			( a | ( II( b,c ) << 8 ) )
#define IV( a,b,c,d )			( a | ( III( b,c,d ) << 8 ) )

#define S_shift( x ) ( S_MAX + (x) )

#define S_ETEXT S_shift( N_TEXT|N_EXT )
#define S_EDATA S_shift( N_DATA|N_EXT )
#define S_EBSS  S_shift( N_BSS|N_EXT )
#define S_EABS  S_shift( N_ABS|N_EXT )


#define S_TEXT S_shift( N_TEXT )
#define S_DATA S_shift( N_DATA )
#define S_BSS  S_shift( N_BSS )

#define LDE IV( S_EBSS, S_ETEXT, S_EDATA, S_EABS )
#define LDL III( S_TEXT, S_DATA, S_BSS )

#define USER        1
#define MPX         2
#define USER_MPX    II( USER, MPX )

#define INTERACT 1

struct stable *stabtab[4];

struct stat *fstat();

int helpmode;


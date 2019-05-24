#include "common.h"
#include "user.h"

static struct flist {
	char		f_id[9];
	struct flist	*f_next;
};

static struct mlist {
	char		m_id[9];
	unsigned short	m_desc;
	struct flist	*m_list;
	struct mlist	*m_next;
};

static struct mlist *map;

struct ramnl *singlearg( bfun )			/* if the function at bfun has */
struct ramnl *bfun;				/* only a SINGLE arg return it */
{
	struct ramnl *s, *psym = 0;

	for( s = bfun; s->n_other != S_EFUN; ++s )
		if( s->n_other == S_PSYM ){
			if( psym ) return 0;
			psym = s;
		}
	return psym;
}

struct mlist *mfind( n, d )			/* find the record for named */
char *n;					/* structure and type	     */
unsigned short d;
{
	struct mlist *m;

	for( m = map; m; m = m->m_next )
		if( idmatch( n, m->m_id ) && d == m->m_desc ) return m;
	return 0;
}

makemap()
{
	struct ramnl *bfun, *psym, *tyid;
	struct mlist *m;
	struct flist *f;

	map = 0;
	for( bfun = lookup(USER, S_BFUN, "*", 0); bfun; bfun = bfun->n_thread ){
		if( !(psym = singlearg(bfun)) ) continue;
		if( psym->n_desc!=STRTY && psym->n_desc!=INCREF(STRTY) ) continue;
		if( (tyid = psym+1)->n_other != S_TYID ) continue;
		if( !(m = mfind( tyid->n_name, psym->n_desc ))){
			m = (struct mlist *) calloc( 1, sizeof *m );
			strncpy( m->m_id, tyid->n_name, 8 );
			m->m_desc = psym->n_desc;
			m->m_next = map;
			map = m;
		}
		f =  (struct flist *) calloc( 1, sizeof *f );
		strncpy( f->f_id, bfun->n_name, 8 );
		f->f_next = m->m_list;
		m->m_list = f;
	}
}

dumpmap()
{
	struct mlist *m;
	struct flist *f;

	for( m = map; m; m = m->m_next ){
		printf( "struct %s%c : ", m->m_id, m->m_desc==STRTY?' ':'*' );
		for( f = m->m_list; f; f = f->f_next )
			printf( "%s ", f->f_id );
		putchar( '\n' );
	}
}

mapscrapp( n, d )
char *n;
unsigned short d;
{
	struct mlist *m;
	struct flist *f;

	if( m = mfind( n, d ) ){
		for( f = m->m_list; f; f = f->f_next )
			scrapp( fmt("%s(~)", f->f_id), - (long) f->f_id );
	}
}

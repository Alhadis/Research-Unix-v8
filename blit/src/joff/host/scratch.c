#include "common.h"
#include "../menu.h"

extern int currmenu;

extern NewMenu menus[];

static struct slist {
	char		*s_string;
	long		s_key;
	struct slist	*s_next;
};

struct slist *scrlist, *scrbase;
int scroff, moreabove, morebelow, suggested;


scratch()
{
	scrlist = 0;
	scroff = 0;
}
	
scrapp( s, k )
char *s;
long k;
{
	struct slist *p = scrlist, *n = (struct slist *) talloc( sizeof *n );

	n->s_string = s;
	n->s_key = k;
	if( p == 0 ) scrlist = n;
	else {
		while( p->s_next ) p = p->s_next;
		p->s_next = n;
	}
}

scrcmp( a, b )
char *a, *b;
{
	if( a[0] == '*' ) return scrcmp( a+1, b );
	if( b[0] == '*' ) return scrcmp( a, b+1 );
	return strcmp( a, b );
}

scrsort( s, k )
char *s;
long k;
{
	int i;
	struct slist **p, *n = (struct slist *) talloc( sizeof *n );

	n->s_string = s;
	n->s_key = k;
	if( scrlist == 0 ) scrlist = n;
	for( p = &scrlist; *p; p = &((*p)->s_next) )
		if( (i = scrcmp( n->s_string, (*p)->s_string )) < 0 ) break;
		else if( i == 0 ) return;
	n->s_next = *p;
	*p = n;
}

scrins( s, k )
char *s;
long k;
{
	struct slist *p = (struct slist *) talloc( sizeof *p );

	p->s_string = s;
	p->s_key = k;
	p->s_next = scrlist;
	scrlist = p;
}

scrdel( k )
long k;
{
	struct slist *p;

	if( scrlist->s_key == k ){
		scrlist = scrlist->s_next;
		return;
	}
	for( p = scrlist; p->s_next; p = p->s_next ) if( p->s_next->s_key == k ){
		p->s_next = p->s_next->s_next;
		return;
	}
}

scrlen( p )
struct slist *p;
{
	int l = 0;

	for( ; p; p = p->s_next ) ++l;
	return l;
}

screfresh()
{
	struct slist *p;
	int o, item = 0, adjust;

	morebelow = moreabove = 0;
	suggested = scroff;
	if( (adjust = scroff - (scrlen(scrlist)-M_ITEMS+1)) > 0 )
		scroff -= adjust;
	if( scroff < 2 || scrlen(scrlist) <= M_ITEMS ) scroff = 0;
	if( suggested > scroff ) suggested -= scroff;
	if( scroff > 0 ){
		m_item( item++, "more..." );
		moreabove = 1;
	}
	for( o = scroff, p = scrlist; o--; p = p->s_next ) assert(p);
	scrbase = p;
	m_null( M_ITEMS );
	while( p && item < M_ITEMS ){
		if( p->s_next && item == M_ITEMS-1 ){
			m_item( item++, "more..." );
			morebelow = 1;
		} else {
			m_item( item++, p->s_string );
			p = p->s_next;
		}
	}
	m_null( item );
	addr_desc( suggested, M_SUGGEST );
}

long scrhit( keyorindex )
long keyorindex;
{
	int i;
	struct slist *p;

	if( !scrlist ) return -1;
	scroff = -1;
	for( p = scrlist, i = 0; p; p = p->s_next, ++i )
		if( p->s_key == keyorindex ){
			scroff = i;
			break;
		}
	if( scroff == -1 ) scroff = keyorindex;
	m_menu( M_SCRATCH, 0 );
	for( ;; ){
		screfresh();
		i = menuforce( M_SCRATCH );
		if( i == -1 ) return -1;
		if( i == 0 && moreabove ){
			scroff -= M_ITEMS-2;
			continue;
		}
		if( i == M_ITEMS-1 && morebelow ){
			scroff += M_ITEMS-2;
			continue;
		}
		if( moreabove ) i -= 1;
		for( p = scrbase; i > 0; --i ) p = p->s_next;
		return p->s_key;
	}
}

			
	
		












	

	

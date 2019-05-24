#ifdef JOURNAL
#include "term.h"

#define JOURNALSIZE 25
char *J_name[JOURNALSIZE+1];
char J_state[JOURNALSIZE];
char *J_hist = "<none>";
char J_init = 1;

int J_search(t)
register char *t;
{
	register i;
	for( i = 0; i < JOURNALSIZE && J_name[i]; ++i )
		if( *t==*J_name[i] && !strcmp(t, J_name[i]) )
			return i;
	if( i >= JOURNALSIZE ) return -1;
	J_name[i] = t;
	J_state[i] = J_init;
	return i;
}

void journal( n, f, a1, a2, a3, a4, a5, a6 )
register char *n, *f;
{
	register i;
	J_hist = n;
	if( !J_init && !J_name[0] ) return;
	i = J_search(n);
	if( i == -1 || !J_state[i] ) return;
	PutTextf( "%s:\t", n);
	PutTextf( f, a1, a2, a3, a4, a5, a6 );
	PutText('\n');
}


#endif JOURNAL

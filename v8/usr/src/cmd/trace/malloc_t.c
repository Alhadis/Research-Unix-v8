#include <stdio.h>
#include "trace.h"

#define A_LARGE 64
#define A_USER 0x55000000

#define POOL	0
#define ALLOC	1
#define FREE	2
#define NREVENT	3

 union M {
	long size;
	union M *link;
 };

 union M *freelist[A_LARGE];
 long	 req[A_LARGE];

char *
talloc(u)
	long u;
{ union M *m;
  register r;
  char *Smalloc();

	u = ((u-1)/sizeof(union M) + 2);

	if( u >= A_LARGE )
	{	m = (union M *) malloc( u * sizeof(union M) );
		if (m == NULL)
			whoops("malloc fault");
	} else
	{	if( freelist[u] == NULL )
		{	r = req[u] += req[u] ? req[u] : 1;
			if (r > NOTOOBIG)
				r = req[u] = NOTOOBIG+1;
			freelist[u] = (union M *) Smalloc( r*u*sizeof(union M) );
			if (freelist[u] == NULL)
				whoops("malloc fault");

			(freelist[u] + u*(r-1))->link = 0;
			for (m = freelist[u] + u*(r-2); m >= freelist[u]; m -= u)
					m->link = m+u;
		}

		m = freelist[u];
		freelist[u] = m->link;
	}
	m->size = u | A_USER;

	return (char *) (m+1);
}

tfree(v)
	char *v;
{ register union M *m = (union M *) v;
  register long u;

	--m;
	if ( (m->size&0xFF000000) != A_USER)
	{	fprintf(stderr, "releasing a free block\n");
		kill(getpid(), 3);
	}

	u = (m->size &= 0xFFFFFF);
	if ( u >= A_LARGE )
		free(m);
	else
	{	m->link = freelist[u];
		freelist[u] = m;
	}
}

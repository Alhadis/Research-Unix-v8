#include <stdio.h>
#include "trace.h"
#include "trace.d"

 extern int  *processes, *state, nrprocs, nrtbl;
 extern char mask[MAXPROC], althash;
 extern struct TBL *tbl;

 short *Factor;
 short maxr = 0;

inihash()
{ register int i;
  char *Smalloc();

	for (i = 0; i < nrtbl; i++)
		if (tbl[i].nrrows > maxr)
			maxr = tbl[i].nrrows;

	Factor = (short *)
		Smalloc(maxr * sizeof(short));

	for (i = 0; i < maxr; i++)
		Factor[i] = rand()%NOTOOBIG;	/* number between 0 and 16k */
}

hashvalue()
{ register int i, h;

	for (i = h = 0; i < nrprocs; i++)
	{	if (mask[i])
			continue;
		h = ((h << 2) | (h >> 13));	/* rotate */
		h ^= Factor[state[i]] ^ Factor[processes[i]];
	}

	return (h & NOTOOBIG);	/* return value will be stored in a short */
}

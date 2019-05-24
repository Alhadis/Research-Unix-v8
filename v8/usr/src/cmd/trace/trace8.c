#include "stdio.h"
#include "trace.h"
#include "trace.d"

 extern int level, maxlevel, maxreached;
 struct Wheel **charriot;

struct Spoke *
getspoke(avoid)
	struct STATE *avoid;
{ register int i, j;

	for (i = maxreached; i >= 0; i--)
	for (j = (charriot[i]->n)-1; j >= 0; j--)
		if (charriot[i]->spoke[j].st != avoid)
		{	charriot[i]->n = j;
			return &(charriot[i]->spoke[j]);
		}
	fprintf(stderr, "level %3d, unlikely event - getspoke\n", level);
	return NULL;
}

addspoke(this, that)
	struct STATE *this;
	struct VISIT *that;
{ int i = charriot[level]->n;

	if (i < NSPOKES)
	{	charriot[level]->spoke[i].st = this;
		charriot[level]->spoke[i].vi = that;
		charriot[level]->n++;
	}
}

inilookup()
{ register int i;
  char *Smalloc();
	charriot = (struct Wheel **)
			Smalloc(maxlevel * sizeof(struct Wheel *));
	for (i = 0; i < maxlevel; i++)
	{	charriot[i] = (struct Wheel *)
			Smalloc(sizeof(struct Wheel));
		charriot[i]->n = 0;
	}
}

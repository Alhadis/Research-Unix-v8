#include "stdio.h"
#include "trace.h"
#include "trace.d"

 extern struct QUEUE **head, **tail;
 extern int *qsize;
 extern int nrqs, level, maxreached;
 extern double iseen, ireseen, zapper;
 char *Realloc(), *Emalloc(), *Smalloc(), *emalloc();

 struct HTABLE {
	struct STATE **index;		/* index [h] [ibound]    */
	short ibound;			/* nr of available slots */
	short nr;			/* nr of occupied  slots */
 } oldstates[NOTOOBIG+1];		/* index of hash values  */

 int hbound = 0, hlast = 0;
 short igrowth  = 8;

growindex(h)
{ int nsz = (int) oldstates[h].ibound + igrowth;

	if (nsz == igrowth)
		oldstates[h].index = (struct STATE **)
			Emalloc(nsz * sizeof(struct STATE *));
	else
		oldstates[h].index = (struct STATE **)
			Realloc(oldstates[h].index, nsz * sizeof(struct STATE *));
	
	oldstates[h].ibound = (short) nsz;

}

initable()
{ register int i;

	for (i = 0; i < NOTOOBIG+1; i++)
	{	oldstates[i].nr = 0;
		oldstates[i].ibound = 0;
	}
	hbound = NOTOOBIG+1;
}

insert(h, pnt)		/* enter state pointer into the table at hash value h */
	struct STATE *pnt;
{ short cin;

	if (h >= hbound)
	{	fprintf(stderr, "h %d, hbound %d, NOTOOBIG %d\n",h,hbound,NOTOOBIG);
		whoops("cannot happen - insert");
	}
	cin = oldstates[h].nr++;

	iseen += (long) 1;

	if (cin >= oldstates[h].ibound)
		growindex(h);

	oldstates[h].index[cin] = pnt;
}

mark(stt, vis)
	struct STATE *stt;
	struct VISIT *vis;
{ int h = stt->hash;

	if (h >= hbound)
		whoops("cannot happen - mark");
	efree(vis->prop.h);
	vis->analyzed = 1;

}

relink(vis)
	struct VISIT *vis;
{ struct CONTS *inqtable();
  register int i, j, k;

	for (i = j = 0; i < nrqs ; i++)
		if (qsize[i] > 0)
			j++;

	vis->howmany = (char) j;

	if (zapper == 0)	/* grow without bound */
		vis->prop.c = (struct CONTS **)
			Smalloc(j * sizeof(struct CONTS *));
	else
		vis->prop.c = (struct CONTS **)
			emalloc(j * sizeof(struct CONTS *));

	for (i = k = 0; i < nrqs; i++)
	{	if ((j = qsize[i]) == 0)
			continue;

		vis->prop.c[k++] = inqtable(i);
	}
}

member(h) { return (h < hbound) ? oldstates[h].nr : 0; }

struct STATE *
giveme(h, n)
{ int m = n - 1;
	if (h >= hbound || oldstates[h].nr <= m || oldstates[h].index[m] == NULL)
		whoops("cannot happen - giveme");

	return oldstates[h].index[m];
}

struct VISIT *
findany(avoid)
	struct STATE *avoid;
{ register int i, j;
  struct VISIT *try = NULL;
  struct VISIT *pickstate();

	for (i = (hlast+1)%hbound; i != hlast; i++, i %= hbound)
	for (j = 0; j < oldstates[i].nr; j++)
		if (oldstates[i].index[j]->nrvisits > 0
		&&  oldstates[i].index[j] != avoid
		&&  (try = pickstate(oldstates[i].index[j])) != NULL)
		{	hlast = i;
			return try;
		}
	return (struct VISIT *) Smalloc(sizeof(struct VISIT));
}

struct VISIT *
findastate(avoid)
	struct STATE *avoid;
{ struct VISIT *findany(), *picknown(), *try;
  struct Spoke   *getspoke(),  *trs2;
  struct Swiffle *unswiffle(), *trs1;

	if (ireseen < zapper || zapper == 0)
		return (struct VISIT *) Smalloc(sizeof(struct VISIT));

	if ((trs1 = unswiffle(avoid)) != NULL
	&&  (try = picknown(trs1->st, trs1->vi)) != NULL)
		return try;

/*	if ((trs2 = getspoke(avoid)) != NULL
	&&  (try = picknown(trs2->st, trs2->vi)) != NULL)
		return try;
*/
	return findany(avoid);	/* last resort */
}

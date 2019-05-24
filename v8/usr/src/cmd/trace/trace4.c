#include "stdio.h"
#include "trace.h"
#include "trace.d"

 extern char lockplus, prefix, ignvars, ignques;
 extern char mask[MAXPROC];

 extern struct QUEUE *s_last;
 extern int *processes, *globvars, *state;
 extern int nrprocs, nrrefs, nrvars, level;
 extern double zapper;
 extern long loopsf, zapped;
 extern struct PROCSTACK **procstack;
 extern struct VARPARS *procpars;

 double iseen = 0;
 double ireseen = 0;

 struct VISIT *lastvisit;
 struct STATE *giveme(), *setstate();
 char *Smalloc(), *emalloc();

struct STATE *
inloop()
{ struct STATE *tmp;
  register struct VISIT *hook;
  register int i, x; char aa;
  int h = hashvalue();

	for (x = member(h); x > 0; x--)
	{	tmp = giveme(h, x);
		hook = tmp->next;
		if (samestate(tmp))
		{	for (i = tmp->nrvisits; i >= 1; i--, hook = hook->next)
			{
				if (hook->analyzed)
				{	if (ignques || Queuesmatch(hook))
					{	if (prefix)
							output("prefix: ", 0);
						return NULL;
					}
				} else
				{	if (Queuesmatch(hook))
					{	loopsf++;
						aa = assertholds();
						if (aa == 0 || lockplus)
							putloop(hook->prop.h, aa);
						return NULL;
					}
			}	}
			return setstate(tmp, h);
		}
	}
	return setstate((struct STATE *) NULL, h);
}

cmplvars(one, two)
	struct LOCVARS *one;
	struct VARPARS *two;
{ int i;

	if (one->nrlvars != two->nrlvars)
		whoops("cannot happen - cmplocal");

	for (i = 0; i < one->nrlvars; i++)
		if (one->lvarvals[i] != two->lvarvals[i])
			return 0;

	return 1;
}

cmplocals(one, two)
	struct VARPARS *one, *two;
{ int i;

	if (one->nrlvars != two->nrlvars)
		whoops("cannot happen - cmplocal");

	for (i = 0; i < one->nrlvars; i++)
		if (one->lvarvals[i] != two->lvarvals[i])
			return 0;

	return 1;
}

cmparams(one, two)
	struct VARPARS *one, *two;
{ register int i;

	if (one->nrms != two->nrms || one->nrvs != two->nrvs)
		whoops("cannot happen - cmparams");

	for (i = 0; i < one->nrms; i++)
		if (one->ms[i] != two->ms[i])
			return 0;
	for (i = 0; i < one->nrvs; i++)
		if (one->vs[i] != two->vs[i])
			return 0;

	return 1;
}

cmpstacks(older, newer)
	struct PROCSTACK *older, *newer;
{ struct PROCSTACK *tmp1 = older;
  struct PROCSTACK *tmp2 = newer;

	while (tmp2 != NULL)
	{	
		if (tmp1->uptable  != tmp2->uptable
		||  tmp1->uptransf != tmp2->uptransf)
			return 0;

		if (cmplocals(tmp1->varparsaved, tmp2->varparsaved) == 0
		||  cmparams (tmp1->varparsaved, tmp2->varparsaved) == 0)
			return 0;

		tmp1 = tmp1->follow;
		tmp2 = tmp2->follow;
	}
	return 1;
}

samestate(at)
	struct STATE *at;
{ register int i;

	for (i = 0; i < nrprocs; i++)
	{	if (mask[i])
			continue;

		if (at->trip[i].prev != (short) state[i]
		||  at->trip[i].pmap != (short) processes[i])
			return 0;
		if (ignvars == 0
		&&  (cmplvars(at->l_vars[i], &(procpars[i])) == 0 ||
		     (nrrefs > 0 && cmpstacks(at->traceback[i], procstack[i]) == 0)
		   ))
			return 0;
	}

	if (!ignvars)
	for (i = 0; i < nrvars; i++)
		if (at->g_vars[i] != (short) globvars[i])
			return 0;

	return 1;
}

cpylvars(into, from)
	struct LOCVARS  *into;
	struct VARPARS *from;
{ register int i;

	into->nrlvars = from->nrlvars;
	into->lvarvals = (short *)
		Smalloc(from->nrlvars * sizeof(short));
	for (i = 0; i < from->nrlvars; i++)
		into->lvarvals[i] = from->lvarvals[i];
}

cpylocals(into, from)
	struct VARPARS *into, *from;
{ register int i;

	into->nrlvars = from->nrlvars;
	into->lvarvals = (short *)
		Smalloc(from->nrlvars * sizeof(short));
	for (i = 0; i < from->nrlvars; i++)
		into->lvarvals[i] = from->lvarvals[i];
}

cpyparams(into, from)
	struct VARPARS *into, *from;
{ int i;

	into->nrms = from->nrms;
	into->ms = (short *)
		Smalloc(from->nrms * sizeof(short));
	for (i = 0; i < from->nrms; i++)
		into->ms[i] = from->ms[i];

	into->nrvs = from->nrvs;
	into->vs = (short *)
		Smalloc(from->nrvs * sizeof(short));
	for (i = 0; i < from->nrvs; i++)
		into->vs[i] = from->vs[i];
}

cpystacks(left, right)
	struct PROCSTACK *left, *right;
{ struct PROCSTACK *into = left;
  struct PROCSTACK *from = right;

	while (from != NULL)
	{	into->varparsaved = (struct VARPARS *)
			Smalloc(sizeof(struct VARPARS));

		cpylocals(into->varparsaved, from->varparsaved);
		cpyparams(into->varparsaved, from->varparsaved);

		into->uptable  = from->uptable;
		into->uptransf = from->uptransf;

		if ((from = from->follow) != NULL)
		{	into->follow = (struct PROCSTACK *)
				Smalloc(sizeof(struct PROCSTACK));
			into = into->follow;
	}	}
}

struct STATE *
newstate(pha)
	int pha;
{ struct STATE *hook;
  int i;
  struct VISIT *findastate();

	hook = (struct STATE *)
		Smalloc(sizeof(struct STATE));
	hook->trip = (struct TUPLE *)
		Smalloc(nrprocs * sizeof(struct TUPLE));

	if (!ignvars)
	{	hook->l_vars = (struct LOCVARS **)
			Smalloc(nrprocs * sizeof(struct LOCVARS *));

		hook->g_vars = (short *)
			Smalloc(nrvars * sizeof(short));
		for (i = 0; i < nrvars; i++)
			hook->g_vars[i] = (short) globvars[i];

		if (nrrefs > 0)
		hook->traceback = (struct PROCSTACK **)
			Smalloc(nrprocs * sizeof(struct PROCSTACK *));
	}

	for (i = 0; i < nrprocs; i++)
	{	hook->trip[i].prev = (short) state[i];
		hook->trip[i].pmap = (short) processes[i];

		if (!ignvars)
		{	hook->l_vars[i] = (struct LOCVARS *)
				Smalloc(sizeof(struct LOCVARS));

			cpylvars(hook->l_vars[i], &(procpars[i]));

			if (nrrefs > 0)
			{	if (procstack[i] != NULL)
				{	hook->traceback[i] = (struct PROCSTACK *)
						Smalloc(sizeof(struct PROCSTACK));

					cpystacks(hook->traceback[i], procstack[i]);
				} else
					hook->traceback[i] = NULL;
	}	}	}

	hook->hash = (short) pha;
	hook->nrvisits = 1;
	hook->next = findastate(hook);

	insert(pha, hook);	/* make index in hash table */

	return hook;
}

struct VISIT *
oldstate(where)
	struct STATE *where;
{ struct VISIT *tmp;
  struct VISIT *findastate();
  int i;

	if (where->nrvisits > 0)
	{	tmp = where->next;
		for (i = where->nrvisits; i > 1; i--)
			tmp = tmp->next;

		if (tmp == NULL)
			whoops("cannot happen - oldstate");

		tmp->next = findastate(where);
		where->nrvisits += 1;
		ireseen += (double)1;

		return tmp->next;
	} else
	{	where->next = findastate(where);
		where->nrvisits = 1;
		ireseen += (double)1;

		return where->next;
	}
}

struct STATE *
setstate(where, ha)
	struct STATE *where;
{ struct STATE *tmp;
  struct VISIT *work;

	if (where == NULL)
	{	tmp = newstate(ha);
		work = tmp->next;
	} else
	{	tmp = where;
		work = oldstate(where);
	}

	work->prop.h = (struct STUFF *) emalloc( sizeof(struct STUFF) );
	work->prop.h->s = s_last;
	work->analyzed = 0;

	lastvisit = work;
	relink(work);

	return tmp;
}

struct VISIT *
pickstate(at)
	struct STATE *at;
{ struct VISIT *latter = NULL;
  register struct VISIT *hook = at->next;
  register int i;

	for (i = at->nrvisits; i >= 1; i--, hook = hook->next)
	{	if (hook->analyzed == 1)
		{
			if (latter == NULL)
				at->next = hook->next;
			else
				latter->next = hook->next;

			efree(hook->prop.c);

			at->nrvisits -= 1;
			zapped++;
			return hook;
		}
		latter = hook;
	}
	return NULL;
}

struct VISIT *
picknown(at, want)
	struct STATE *at;
	struct VISIT *want;
{ struct VISIT *latter = NULL;
  register struct VISIT *hook = at->next;
  register int i, j = at->nrvisits;

	for (i = 0; i < j; i++, hook = hook->next)
	{	if (hook == want)
			break;
		latter = hook;
	}
	if (i == j)
		whoops("cannot happen - picknown");

	if (latter == NULL)
		at->next = hook->next;
	else
		latter->next = hook->next;

	efree(hook->prop.c);
	at->nrvisits -= 1;
	zapped++;

	return hook;
}

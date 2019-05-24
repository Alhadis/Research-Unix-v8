#include "beauty.h"

#define	INFINITY	1.0e12
#define	EPSILON	1.0e-6
#define MOV_THR 0.2

#define	abs(x)	(((x)>0)?(x):-(x))
typedef	int	boolean;
#define	TRUE	1
#define	FALSE	0

static void errmsg (s, a, b, c, d)
char *s, *a, *b, *c, *d;
{
	fprintf (fq, s, a, b, c, d);
}

static void die (s, a, b, c, d)
char *s, *a, *b, *c, *d;
{
	fprintf (fq, s, a, b, c, d);
	abort ();
}

typedef struct _deplist {
	struct _deplist *next;
	double coeff;
	struct _variable *var;
} deplist;

#define	NEXT(p)	(p->next)
#define	COEFF(p)	(p->coeff)
#define	VAR(p)	(p->var)

typedef struct _variable {
	deplist *rep, *nrep;
	struct _variable *nextdep;
} variable;

static variable vartab[2*SIDE_BUF];
static int nvartab;

#define	VARTAB(i,c)	((c == 'x')?\
						&vartab[2*(i)]:\
					(c == 'y')?\
						&vartab[2*(i)+1]:\
					NULL)

#define	ROW(i)	(((i)%2 == 0)?\
					row[(i)/2].x:\
					row[(i-1)/2].y)

#define	ROWP(v)	ROW(v - vartab);

static variable *depvarlist;

#define	DEPHUNKSIZE	128

typedef struct _deparena {
	struct _deparena *next;
	deplist arena[DEPHUNKSIZE];
} deparena;

static deparena *toparena = NULL;

static deplist *newdeplist ()
{
	static int nextdep;
	deparena *temp;
	if (toparena == NULL) {
		toparena = (deparena *) calloc (1, sizeof(deparena));
		nextdep = DEPHUNKSIZE - 1;
	} else if (nextdep < 0) {
		temp = toparena;
		toparena = (deparena *) calloc (1, sizeof(deparena));
		toparena->next = temp;
		nextdep = DEPHUNKSIZE - 1;
	}
	return &(toparena->arena[nextdep--]);
}

static void depfree (p)
deplist *p;
{
}

static void depclean ()
{
	deparena *temp;
	while (toparena) {
		temp = toparena->next;
		free (toparena);
		toparena = temp;
	}
}

static deplist *depgen (c, x)
double c;
variable *x;
{
	register deplist *dummy;
	dummy = newdeplist ();
	COEFF(dummy) = c;
	VAR(dummy) = x;
	return dummy;
}

static double epsilon;

varinit (k)
register int k;
{
	/* Make all variables independent.
	*/
	register variable *v;
/*
/*	double xmin, xmax, ymin, ymax;
/*
/*	xmin = INFINITY;
/*	xmax = -INFINITY;
/*	ymin = INFINITY;
/*	ymax = -INFINITY;
*/
	epsilon = EPSILON;
	nvartab = 2*k;
	for (; --k >= 0;) {
/*
/*		if (row[k].x < xmin)
/*			xmin = row[k].x;
/*		if (xmax < row[k].x)
/*			xmax = row[k].x;
*/
		v = VARTAB(k,'x');
		v->rep = v->nrep = depgen (1.0, v);
/*
/*		if (row[k].y < ymin)
/*			ymin = row[k].y;
/*		if (ymax < row[k].y)
/*			ymax = row[k].y;
*/
		v = VARTAB(k,'y');
		v->rep = v->nrep = depgen (1.0, v);
	}
	epsilon = MOV_THR*smalls;
	depvarlist = NULL;
}

extern deplist *depadd ();
extern deplist *depsubst ();

#define	morevert(x)	(QPI < x && x < TPI)
#define	cot(x)	tan(HPI - x)

static deplist *side_dx (s)
sides *s;
{
	return depadd (1.0, VARTAB(s->p2 - row, 'x')->rep,
		-1.0, VARTAB(s->p1 - row, 'x')->rep);
}

static deplist *side_dy (s)
sides *s;
{
	return depadd (1.0, VARTAB(s->p2 - row, 'y')->rep,
		-1.0, VARTAB(s->p1 - row, 'y')->rep);
}

static int sign_dx (s)
sides *s;
{
	if (s->p2->x < s->p1->x)
		return -1;
	else
		return 1;
}

static int sign_dy (s)
sides *s;
{
	if (s->p2->y < s->p1->y)
		return -1;
	else
		return 1;
}

#define	depeq(c1,d1,c2,d2)	(depadd(c1,d1,-c2,d2))

static deplist *makedep (e)
register eqnS *e;
{
	/* Use equation e to create a dependency list
	/* that is formally equal to zero.
	/* Note that an equation's "s" is an *angle*; thus slope = tan s.
	/* We must decide which of dy = (tan s)*dx and dx = (cot s)*dy to use.
	/* The relevant identity is tan(PI/2 - x) = cot(x).
	*/
	register deplist *result;

	if(detail)
	fprintf(fq,"side=%d sl1=%g side=%d sl2=%g type=%d\n",
		e->s1-stt,e->s1->s,e->s2-stt,e->s2->s,e->u);

	switch (e->u) {
		case SLOPE:
			{
			if (morevert(e->s1->s))
				result = depeq(1.0, side_dx(e->s1),
					cot(e->s1->s), side_dy(e->s1));
			else
				result = depeq(1.0, side_dy(e->s1),
					tan(e->s1->s), side_dx(e->s1));
			}
			break;
		case EXTENSION:
			{
			register deplist *dx, *dy;
			/* Note: this code could be smarter about collinearities
			/* at a point, by choosing a maximally distant pair;
			/* but it's not.
			*/
			if (abs(e->s1->s - e->s2->s) > EPSILON)
				errmsg ("extension of lines of slopes %g and %g\n",
					e->s1->s, e->s2->s);
			dx = depadd (1.0, VARTAB(e->s2->p1 - row, 'x')->rep,
				-1.0, VARTAB(e->s1->p1 - row, 'x')->rep);
			dy = depadd (1.0, VARTAB(e->s2->p1 - row, 'y')->rep,
				-1.0, VARTAB(e->s1->p1 - row, 'y')->rep);
			if (morevert(e->s1->s))
				result = depeq(1.0, dx,
					cot(e->s1->s), dy);
			else
				result = depeq(1.0, dy,
					tan(e->s1->s), dx);
			}
			break;
		case EQUAL:
			{
			register deplist *lhd, *rhd; /* differences */
			double lhq, rhq;	/* quantities; coeff = sqrt (1 + q*q) */
			int lhsign, rhsign;
			if (morevert(e->s1->s)) {
				lhq = cot(e->s1->s);
				lhd = side_dy(e->s1);
				lhsign = sign_dy(e->s1);
			} else {
				lhq = tan(e->s1->s);
				lhd = side_dx(e->s1);
				lhsign = sign_dx(e->s1);
			}
			if (morevert(e->s2->s)) {
				rhq = cot(e->s2->s);
				rhd = side_dy(e->s2);
				rhsign = sign_dy(e->s2);
			} else {
				rhq = tan(e->s2->s);
				rhd = side_dx(e->s2);
				rhsign = sign_dx(e->s2);
			}
			result = depeq(lhsign*sqrt (1 + lhq*lhq), lhd,
				rhsign*sqrt (1 + rhq*rhq), rhd);
			}
			break;
		case SAMEX:
			result = depeq(1.0, VARTAB(e->s1->p1 - row, 'x')->rep,
				1.0, VARTAB(e->s2->p1 - row, 'x')->rep);
			break;
		case SAMEY:
			result = depeq(1.0, VARTAB(e->s1->p1 - row, 'y')->rep,
				1.0, VARTAB(e->s2->p1 - row, 'y')->rep);
			break;
		default:
			die ("unknown equation type: %d\n", e->u);
	}
	return result;
}

#define	ROWDEX(v)	((v - vartab)/2)
#define	VARCHAR(v)	(((v - vartab)%2 == 0)?'x':'y')
extern char *varnameprint();

static variable *equatetozero (p)
deplist *p;
{
	/* Dependency list p is formally equal to zero.
	/* Make one of the variables it contains
	/* depend on the others.
	*/
	if (!VAR(p)) {
		if (abs(COEFF(p)) > EPSILON)
			die ("inconsistent equation\n");
		return NULL;
	} else {
		double curmax;
		deplist *q, *r;
		curmax = 0;
		for (q = p; q; q = NEXT(q))
			if (VAR(q) && curmax < abs(COEFF(q))) {
				curmax = abs(COEFF(q));
				r = q;
			}
		/* solve p = 0 for r */
		VAR(r)->nrep = depadd (1.0, VAR(r)->rep, -1.0/COEFF(r), p);
		if(detail) {
			errmsg ("%s: ", varnameprint (VAR(r)));
			depprint (VAR(r)->nrep);
		}
		return VAR(r);
	}
}

static double depeval (p)
register deplist *p;
{
	double value;
	value = 0.0;
	for (; p && VAR(p); p = NEXT(p))
		value += COEFF(p)*ROWP(VAR(p));
	if (p && !VAR(p))
		value += COEFF(p);
	return value;
}

static boolean depok (v)
variable *v;
{
	double evalvalue, origvalue;
	/* v->nrep is a dependency list representing constraints on v.
	/* Does plugging in the original values of the variables on which
	/* v depends cause v to move too far?
	*/
	evalvalue = depeval(v->nrep);
	origvalue = ROWP(v);
	if(detail)
	errmsg ("%s originally = %g; evaluates to %g\n",
		varnameprint (v), origvalue, evalvalue);
	return (abs(origvalue - evalvalue) < epsilon);
}

static void ped_out(v)
variable *v;
{
	register j, k;
	j = ROWDEX(v);
	if((v - vartab)%2) row[j].y = depeval(v->nrep);
	else row[j].x = depeval(v->nrep);
}

static void alldepeval ()
{
	for (; --nvartab >= 0;)
		ped_out (&vartab[nvartab]);
}

static void removedeps (v)
variable *v;
{
	/* Remove all explicit dependencies on v.
	*/
	variable *p;
	v->nextdep = depvarlist;
	depvarlist = v;
	for (p = depvarlist; p; p = p->nextdep)
		p->nrep = depsubst (p->rep, v->nrep, v);
}

static void resetreps ()
{
	/* Accept the new dependencies.
	*/
	variable *p;
	for (p = depvarlist; p; p = p->nextdep)
		p->rep = p->nrep;
}

ignoreeq (v, e)
variable *v;
eqnS *e;
{
	/* Equation e would make v dependent,
	/* but we will ignore it instead.
	/* First, throw away the proposed new representation.
	*/
	v->nrep = v->rep;
	/* Now, print an informative message.
	*/
	fprintf (fq, "equation %d [", e - eq);
	switch (e->u) {
		case SLOPE:
			fprintf (fq, "SLOPE of side %d",
				e->s1 - stt);
			break;
		case EXTENSION:
			fprintf (fq, "EXTENSION of sides %d and %d",
				e->s1 - stt, e->s2 - stt);
			break;
		case EQUAL:
			fprintf (fq, "EQUAL of sides %d and %d",
				e->s1 - stt, e->s2 - stt);
			break;
		case SAMEX:
			fprintf (fq, "SAMEX of row[%d].x",
				e->s1->p1 - row);
			fprintf (fq, " and row[%d].x",
				e->s2->p1 - row);
			break;
		case SAMEY:
			fprintf (fq, "SAMEY of row[%d].y",
				e->s1->p1 - row);
			fprintf (fq, " and row[%d].y",
				e->s2->p1 - row);
			break;
	}
	fprintf (fq, "] ignored\n");
}

solve (eq, neq)
eqnS *eq;
{
	eqnS *p, *q;
	deplist *d;
	variable *v;
	errmsg ("entering solve()\n");
	q = eq + neq;
	for (p = eq; p < q; p ++) {
		d = makedep (p);
		v = equatetozero (d);
		if (!v) /* redundant equation */
			continue;
		if (depok (v)) {
			removedeps (v);
			resetreps ();
		} else {
			ignoreeq (v, p);
		}
	}
	alldepeval ();
	depclean ();
}

deplist *depadd(coeffone, dlistone, coefftwo, dlisttwo)
float coeffone;
deplist *dlistone;
float coefftwo;
deplist *dlisttwo;
{
	/* produce a dependency list = coeffone*dlistone + coefftwo*dlisttwo */
	register deplist *onewalk, *twowalk, *newhead, *newwalk, *prevnew;
	deplist nuhead;
	prevnew = &nuhead;
	prevnew->next = NULL;
	onewalk = dlistone;
	twowalk = dlisttwo;
	while ((onewalk != NULL) || (twowalk != NULL)) {
		if (onewalk != NULL)
			if (twowalk != NULL)
				if (VAR(onewalk) > VAR(twowalk )) {
					newwalk = depgen (
					    coeffone*COEFF(onewalk),
					    VAR(onewalk)
					    );
					onewalk = NEXT(onewalk);
				}
				else
					if (VAR(onewalk) == VAR(twowalk)) {
						newwalk = depgen (
						    coeffone*COEFF(onewalk)
						    + coefftwo*COEFF(twowalk),
						    VAR(onewalk)
						    );
						onewalk = NEXT(onewalk);
						twowalk = NEXT(twowalk);
					}
					else {
						newwalk = depgen (
						    coefftwo*COEFF(twowalk),
						    VAR(twowalk)
						    );
						twowalk = NEXT(twowalk);
					}
			else {
				newwalk = depgen (
				    coeffone*COEFF(onewalk),
				    VAR(onewalk)
				    );
				onewalk = NEXT(onewalk);
			}
		else {
			newwalk = depgen (
			    coefftwo*COEFF(twowalk),
			    VAR(twowalk)
			    );
			twowalk = NEXT(twowalk);
		}
		if ((fabs(COEFF(newwalk))) > EPSILON) {
			NEXT(prevnew) = newwalk;
			prevnew = newwalk;
		} else
			depfree (newwalk);
	}
	newhead = nuhead.next;
	if (newhead != NULL)
		return(newhead);
	else
		return (depgen (0.0, (variable *) NULL));
}

deplist *depsubst(depinto, depfrom, depwho)
/* substitutes depfrom for depwho in depinto
	/* WARNING:  if depinto actually contains depfrom,
	/* depinto is replaced */
deplist *depinto, *depfrom;
variable *depwho;
{
	deplist *intowalker, *intoparent, *temp;
	for (intowalker = depinto;
	    intowalker != NULL;
	    intowalker = NEXT(intowalker))
		if (VAR (intowalker) == depwho)
			break;
		else
			intoparent = intowalker;
	if (intowalker == NULL)
		return(depinto);
	if (intowalker == depinto)
		depinto = NEXT(depinto);
	else
		NEXT(intoparent) = NEXT(intowalker);
	temp = depadd(1.0, depinto, COEFF(intowalker), depfrom);
/*
	depfree (depinto);
	tryfree(intowalker);
*/
	depinto = temp;
	return (temp);
}

depprint (p)
register deplist *p;
{
	while (p) {
		fprintf (fq, "%f", COEFF(p));
		if (VAR(p))
			fprintf (fq, "* %s + ", varnameprint (VAR(p)));
		p = NEXT(p);
	}
	fprintf (fq, "\n");
}

char *varnameprint (v)
register variable *v;
{
	char buff[100];
	sprintf (buff, "row[%d].%c", ROWDEX(v), VARCHAR(v));
	return buff;
}

#include "a.out.h"
#include "mld.h"
#include "stdio.h"

stab *htab[HLEN];
extern char *malloc();
extern char flg[];

/* names are <= NAMESZ long or \0 terminated */

stab *
lookup(s)
char *s;
{	int n;
	stab *p;
	n = hash(s);
	for(p = htab[n]; p; p = p->next)
		if(strncmp(s, p->ref->n_name, NAMESZ) == 0)
			return(p);
	return(0);
}

insert(n)
struct nlist *n;
{	int i;
	stab *p;
	i = hash(n->n_name);
	p = (stab *)malloc(sizeof(stab));
	p->next = htab[i];
	p->ref = n;
	htab[i] = p;
}

merge(p)
struct nlist *p;
{	stab *s;
	struct nlist *old;
	if((s = lookup(p->n_name)) == 0) {
		verbose("new\n");
		insert(p);
		return;
	}
	old = s->ref;
	switch(type(old)) {
	default:
		fatal("unk type in merge\n");
	case N_BSS:
	case N_DATA:
		if(type(p) != N_UNDF)
			error("%8.8s initialized twice\n", p->n_name);
		verbose("undefined, bss or data already found\n");
		break;
	case N_TEXT:
		if(type(p) == N_UNDF) {
			verbose("undefined, text already found\n");
			break;
		}
		if(type(p) == N_TEXT)
			error("%8.8s multiply defined\n", p->n_name);
		else
			error("%8.8s redefined\n", p->n_name);
		break;
	case N_ABS:
		if(type(p) == N_UNDF)
			break;
		if(type(p) != N_ABS || p->n_value != old->n_value)
			error("%8.8s redefined\n", p->n_name);
		break;
	case N_UNDF:
		verbose("was previously undefined\n");
		if(old->n_value >= p->n_value && type(p) == N_UNDF)
			break;
		s->ref = p;
		return;
	}
}

resolves(p)	/* policy of what gets loaded out of libs */
struct nlist *p;
{	stab *s;
	struct nlist *old;
	if((s = lookup(p->n_name)) == 0)
		return(0);
	old = s->ref;
	if(type(old) != N_UNDF)
		return(0);
	if(type(p) == N_UNDF && p->n_value == 0)
		return(0);
	if(old->n_value == 0)
		return(1);
	if(type(p) == N_BSS || type(p) == N_DATA)
		return(1);
	return(0);
}

hash(s)
char *s;
{	unsigned int i, n;
	for(i = n = 0; i < NAMESZ && *s; i++)
		n += *s++ ^ i;
	return(n % HLEN);
}

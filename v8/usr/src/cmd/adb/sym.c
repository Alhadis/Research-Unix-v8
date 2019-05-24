/*
 * adb - symbol table routines
 */
#include "defs.h"
#include <a.out.h>
#include <stab.h>
#include <sys/types.h>

/*
 * Lookup a symbol by name
 * leave a pointer to the symbol in cursym
 */
struct nlist *
lookup(symstr)
	register char *symstr;
{
	register struct nlist *sp;

	cursym = 0;
	if (symtab)
	for (sp = symtab; sp < esymtab; sp++)
		switch (sp->n_type &~ N_EXT) {
		case N_FUN:
		case N_STSYM:
		case N_TEXT:
		case N_DATA:
		case N_BSS:
		case N_ABS:
			if (eqsym(sp->n_un.n_name, symstr, '_'))
				return(cursym = sp);
		}
	return (0);
}

/*
 * find the symbols for a routine
 */

int
findrtn(rtn)
char *rtn;
{
	register struct nlist *sp;

	cursym = 0;
	if (symtab)
	for (sp = symtab; sp < esymtab; sp++)
		switch (sp->n_type &~ N_EXT) {
		case N_FUN:
		case N_TEXT:
			if (eqsym(sp->n_un.n_name, rtn, '_')) {
				cursym = sp;
				return (1);
			}
		}
	return (0);
}

/*
 * Find the closest symbol to val, and return
 * the difference between val and the symbol found.
 * Leave a pointer to the symbol found as cursym.
 */
WORD
findsym(val, type)
	register WORD val;
	int type;
{
	register WORD diff;
	register struct nlist *sp;

	cursym = 0;
	diff = HUGE;
	if ((type & SPTYPE) == NOSP || symtab == 0)
		return (diff);
	for (sp = symtab; sp < esymtab; sp++)
		switch (sp->n_type) {
		case N_TEXT|N_EXT:
		case N_DATA|N_EXT:
		case N_BSS|N_EXT:
		case N_ABS|N_EXT:
		case N_FUN:
		case N_STSYM:
		/* data objects? */
		/* should look at type here */
			if (val - sp->n_value < diff && val >= sp->n_value) {
				diff = val - sp->n_value;
				cursym = sp;
				if (diff == 0)
					break;
			}
		}
	if (cursym && cursym->n_value == 0 && diff != 0)
		return (HUGE);
	return (diff);
}

/*
 * advance cursym to the next local variable
 * return 0 if the next variable is a global
 */
int
localsym()
{
	register struct nlist *sp;

	if (cursym == NULL)
		return (0);
	for (sp = cursym; ++sp < esymtab; )
		switch (sp->n_type) {
		case N_TEXT:
		case N_DATA:
			if (sp->n_un.n_name[0] == '_')	/* C global */
				return (0);
			continue;
		case N_LSYM:
		case N_PSYM:
		case N_RSYM:
		case N_STSYM:
			cursym = sp;
			return (1);

		case N_FUN:
		case N_EFUN:
			return (0);
		}
	cursym = 0;
	return (0);
}

/*
 * Print value v and then the string s.
 * If v is not zero, then we look for a nearby symbol
 * and print name+offset if we find a symbol for which
 * offset is small enough.
 */
psymoff(v, type, s)
	WORD v;
	int type;
	char *s;
{
	WORD w;
	int r;

	if ((type & SPTYPE) == REGSP) {
		printf("%%%R", v);
		printf(s);
		return;
	}
	if (v)
		w = findsym(v, type);
	if (v==0 || w >= maxoff)
		printf("%R", v);
	else {
		printf("%s", cursym->n_un.n_name);
		if (w)
			printf("+%R", w);
	}
	printf(s);
}

/*
 * Print value v symbolically if it has a reasonable
 * interpretation as name+offset.  If not, print nothing.
 * Used in printing out registers $r.
 */
valpr(v, idsp)
	WORD v;
{
	WORD d;

	d = findsym(v, idsp);
	if (d >= maxoff)
		return;
	printf("%s", cursym->n_un.n_name);
	if (d)
		printf("+%R", d);
}

eqsym(s1, s2, c)
register char *s1, *s2;
{

	if (!strcmp(s1,s2))
		return (1);
	if (*s1 == c && !strcmp(s1+1, s2))
		return (1);
	return (0);
}

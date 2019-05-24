/*	@(#)hash.c	1.1	*/
#include "hash.h"

#define LOCHWIDTH 3
#define HICHWIDTH 3
#define CHARWIDTH (LOCHWIDTH+HICHWIDTH)
#define LOCHMASK ((1<<LOCHWIDTH)-1)

/* The hash function is first a substitution cipher into 6-bit
 * characters.  Then the string of 6-bit bytes is interpreted
 * as a huge integer, and taken mod a big prime (hashsize).
 * Unhashable trash produces an otherwise impossible value; it
 * would be prudent not to have that value in the dictionary.
*/
/* if HASHWIDTH + CHARWIDTH < bitsizeof(long)
 * one could make LOCHWIDTH=6 and HICHWIDTH=0
 * and simplify accordingly; the hanky-panky
 * is to avoid overflow in long multiplication
 */
#define NC 30

long hashsize = HASHSIZE;
long pow2[NC*2];

static char hashtab[] = {
-1,	-1,	-1,	-1,	-1,	-1,	0,	31,	/*  &' */
-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,
2,	25,	20,	35,	54,	61,	40,	39,	/* 0-7 */
42,	33,	-1,	-1,	-1,	-1,	-1,	-1,
-1,	60,	43,	30,	5,	16,	47,	18,	/* A-G */
41,	36,	51,	6,	13,	56,	55,	58,
49,	12,	59,	46,	21,	32,	63,	34,
57,	52,	3,	-1,	-1,	-1,	-1,	-1,
-1,	22,	29,	8,	7,	10,	1,	28,	/* a-g */
11,	62,	37,	48,	15,	50,	9,	4,
19,	38,	45,	24,	23,	26,	17,	44,
27,	14,	53,	-1,	-1,	-1,	-1,	-1
};

long hash(s)
char *s;
{
	register c;
	register long *lp;
	long h = 0;
	for(lp=pow2; c = *s++&0177; ) {
		if((c-=' ') >= 0 && (c=hashtab[c]) >= 0
		   && lp < pow2+(sizeof pow2/sizeof *pow2)) {
			h += (c&LOCHMASK) * *lp++;
			h += (c>>LOCHWIDTH) * *lp++;
			h %= hashsize;
		} else
			return (1L<<HASHWIDTH) - 1;	/*trash value*/
	}
	return(h);
}

hashinit()
{
	register i;
	if(1L<<(HASHWIDTH+LOCHWIDTH)<=0
	   || 1L<<(HASHWIDTH+HICHWIDTH)<=0)
		abort();	/* overflow is imminent */
	pow2[0] = 1L<<(HASHWIDTH-CHARWIDTH-2);
	for(i=0; i<2*NC-3; i+=2) {
		pow2[i+1] = (pow2[i]<<LOCHWIDTH) % hashsize;
		pow2[i+2] = (pow2[i+1]<<HICHWIDTH) % hashsize;
	}
}

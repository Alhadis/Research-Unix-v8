/*
 *	unsigned integer to float
 */

#include "fp.h"

fp
uitof (x)
	register unsigned x;
{
	fp result;
	long frac;
	register int exp;
	
	/* converting zero? */
	if (x == 0)
		return zero;
	
	/* create an unnormalized fraction and exponent */
	frac = x;
	exp = EXPOFFSET + FRACSIZE + 1;

	/* normalize */
	while ((frac & HIDDENBIT) == 0) {
		frac <<= 1;
		exp--;
	}

	/* store the result */
	SETSIGN (result, 0);
	SETEXP (result, exp);
	SETFRAC (result, frac & ~HIDDENBIT);

	return result;
}

/*
 *	floating-point divide
 */

#include "fp.h"

fp
fldiv (x, y)
	fp x, y;
{
	fp result;
	int exp;
	register i;
	long z, q, r, xfrac, yfrac;

	/* if divisor is zero, return infinity with proper sign */
	if (EXP (y) == 0) {
		result = infinity;
		SETSIGN (result, SIGN (x));
		return result;
	}

	/* if dividend is zero, return zero */
	if (EXP (x) == 0)
		return zero;
	
	/* calculate result exponent */
	exp = EXP (x) - EXP (y) + EXPOFFSET + FRACSIZE + 2;

	/* extract the true fractions */
	xfrac = FRAC (x) | HIDDENBIT;
	yfrac = FRAC (y) | HIDDENBIT;

	/* divide, by repeated subtraction (ugh) */
	q = 0;
	do {
		q <<= 1;
		if (xfrac >= yfrac) {
			q++;
			xfrac -= yfrac;
		}
		xfrac <<= 1;
		exp--;
	} while ((q & HIDDENBIT) == 0);

	/* round, perhaps renormalize */
	if (xfrac >= yfrac) {
		q++;
		if ((q & HIDDENBIT) == 0) {
			q >>= 1;
			exp++;
		}
	}

	/* underflow? */
	if (exp < 1)
		return zero;

	/* store result or overflow indication */
	if (exp > MAXEXP)
		result = infinity;
	else
		SETEF (result, exp, q);

	/* result sign */
	SETSIGN (result, SIGN (x) != SIGN (y));

	return result;
}

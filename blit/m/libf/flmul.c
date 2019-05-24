/*
 *	floating-point multiply
 */

#include "fp.h"

fp
flmul (x, y)
	fp x, y;
{
	fp result;
	int exp;
	long zh, zm, zl, xfrac, yfrac;
	short xh, xl, yh, yl;

	/* if either operand is zero, return zero */
	if (EXP (x) == 0 || EXP (y) == 0)
		return zero;
	
	/* calculate result exponent */
	exp = EXP (x) + EXP (y) - EXPOFFSET;

	/* extract the true fractions */
	xfrac = FRAC (x) | HIDDENBIT;
	yfrac = FRAC (y) | HIDDENBIT;

	/* split the fractions into pieces */
	xh = hi (xfrac);
	xl = lo (xfrac);
	yh = hi (yfrac);
	yl = lo (yfrac);

	/* multiply the pieces, but don't merge carries yet */
	zh = lmul (xh, yh);
	zm = lmul (xh, yl) + lmul (xl, yh);
	zl = lmul (xl, yl);

	/* propagate carries */
	zm += hi (zl);
	zh += hi (zm);

	/* normalize if necessary */
	while ((zh & HIDDENBIT) == 0) {
		zh = (zh << 1) | hibit (zm);
		zm <<= 1;
		exp--;
	}

	/* round, perhaps renormalize */
	if (hibit (zm)) {
		zh++;
		if ((zh & HIDDENBIT) == 0) {
			zh >>= 1;
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
		SETEF (result, exp, zh);

	/* result sign */
	SETSIGN (result, SIGN (x) != SIGN (y));

	return result;
}

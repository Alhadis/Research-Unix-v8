/*
 *	add magnitudes of values addressed by x and y.
 *	sign of y is ignored, result sign = sign of x.
 */

#include "fp.h"

fp
fp_addmag (x, y)
	register fp x, y;
{
	fp result;
	register int lexp, sexp;
	register long lfrac, sfrac, rfrac;
	register int shift;

	/* force sign(result) = sign(x) */
	result = x;

	/*
	 *	put large exponent, fraction into lexp, lfrac
	 *	and small exponent, fraction into sexp, sfrac
	 */
	if (EXP (x) > EXP (y)) {
		lexp = EXP (x);
		lfrac = FRAC (x);
		sexp = EXP (y);
		sfrac = FRAC (y);
	} else {
		lexp = EXP (y);
		lfrac = FRAC (y);
		sexp = EXP (x);
		sfrac = FRAC (x);
	}

	/* install hidden bit in both fractions and create guard bits */
	lfrac = (lfrac | HIDDENBIT) << GBITS;
	sfrac = (sfrac | HIDDENBIT) << GBITS;

	/* difference between exponents is how many bits to shift */
	shift = lexp - sexp;

	/*
	 *	if the smaller operand is zero
	 *	or the exponent difference is too large,
	 *	return the larger operand
	 */
	if (shift > FRACSIZE + 2 || sexp == 0) {
		SETEF (result, lexp, lfrac >> GBITS);
		return result;
	}

	/* the actual addition */
	rfrac = lfrac + (sfrac >> shift);

	/* check for a carry during addition */
	if (rfrac & CARRYBIT) {
		lexp++;
		rfrac >>= 1;
	}

	/* rounding */
	if (rfrac & (1 << (GBITS - 1))) {
		rfrac += 1 << GBITS;
		if (rfrac & CARRYBIT) {
			lexp++;
			rfrac >>= 1;
		}
	}

	/* overflow check */
	if (lexp > MAXEXP) {
		lexp = EXP (infinity);
		lfrac = FRAC (infinity) << GBITS;
	}

	/* store final result */
	SETEF (result, lexp, rfrac >> GBITS);

	return result;
}

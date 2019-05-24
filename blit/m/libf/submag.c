/*
 *	subtract magnitude of y from magnitude of x.
 *	result has sign of x if |y| <= |x|, opposite
 *	sign otherwise.  sign of y is ignored.
 */

#include "fp.h"

fp
fp_submag (x, y)
	fp x, y;
{
	fp result;
	register int lexp, sexp;
	register long lfrac, sfrac, rfrac;
	register int shift;
	register long t;

	/* force sign(result) = sign(x) */
	result = x;

	/*
	 *	put exponent, fraction of large operand into lexp, lfrac
	 *	and exponent, fraction of small operand into sexp, sfrac
	 *	flip result sign if necessary
	 */
	if (SEXP (x) > SEXP (y) || SEXP (x) == SEXP (y) && FRAC (x) >= FRAC (y)) {
		lexp = EXP (x);
		lfrac = FRAC (x);
		sexp = EXP (y);
		sfrac = FRAC (y);
	} else {
		lexp = EXP (y);
		lfrac = FRAC (y);
		sexp = EXP (x);
		sfrac = FRAC (x);
		SETSIGN (result, SIGN (result) == 0);
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

	/* the actual subtraction, with sticky right shift */
	t = sfrac >> shift;
	rfrac = lfrac - t;
	if (sfrac != t << shift)
		rfrac--;

	/* if the result is zero, return true zero */
	if (rfrac == 0)
		return zero;
	
	/* result nonzero, normalize */
	while ((rfrac & NORMMASK) == 0) {
		lexp--;
		rfrac <<= 1;
	}

	/* round */
	if (rfrac & (1 << (GBITS - 1))) {
		rfrac += 1 << GBITS;
	
		/* we may need to renormalize (no more than once) */
		if (rfrac & CARRYBIT) {
			lexp++;
			rfrac >>= 1;
		}
	}

	/* check for underflow */
	if (lexp <= 0)
		return zero;

	/* store final result */
	SETEF (result, lexp, rfrac >> GBITS);

	return result;
}

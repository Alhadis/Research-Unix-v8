/*
 *	floating-point to long conversion
 */

#include "fp.h"

long
ftol (x)
	fp x;
{
	unsigned long l;
	register int exp, shift;


	/* extract exponent, test for zero */
	exp = EXP (x);
	if (exp == 0)
		return 0L;
	
	/* extract fraction, restore hidden bit */
	l = FRAC (x) | HIDDENBIT;

	/* calculate how far to shift */
	shift = exp - EXPOFFSET - FRACSIZE - 1;

	/* shift in the proper direction */
	if (shift > 0)
		l <<= shift;
	else
		l >>= -shift;

	/* compensate for sign */
	if (SIGN (x))
		l = -l;
	
	return l;
}

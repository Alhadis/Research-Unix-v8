/*
 *	ldexp - combine a fraction and exponent
 *	returns value * 2**exp
 */

#include "fp.h"

fp
ldexp (value, exp)
	fp value;
	int exp;
{
	/* check for zero argument */
	if (EXP (value) == 0)
		return zero;
	
	exp += EXP (value);

	/* range check */
	if (exp > MAXEXP) {
		SETEXP (value, EXP (infinity));
		SETFRAC (value, FRAC (infinity));
	} else if (exp <= 0)
		return zero;
	
	SETEXP (value, exp);

	return value;
}

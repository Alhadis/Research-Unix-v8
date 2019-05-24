/*
 *	frexp - split a float into a fraction and exponent
 */

#include "fp.h"

fp
frexp (value, eptr)
	fp value;
	int *eptr;
{
	register exp;

	exp = EXP (value);

	if (exp == 0) {
		*eptr = 0;
		return zero;
	}

	*eptr = exp - EXPOFFSET;
	SETEXP (value, EXPOFFSET);
	return value;
}

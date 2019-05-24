/* change the sign of a floating-point number */

#include "fp.h"

fp
flneg (x)
	fp x;
{
	if (SIGN (x))
		SETSIGN (x, 0);
	else if (EXP (x))
		SETSIGN (x, 1);
	return x;
}

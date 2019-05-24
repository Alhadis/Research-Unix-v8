/* add two floating-point numbers */

#include "fp.h"

fp
fladd (x, y)
	fp x, y;
{
	if (SIGN (x) == SIGN (y))
		return fp_addmag (x, y);
	else
		return fp_submag (x, y);
}

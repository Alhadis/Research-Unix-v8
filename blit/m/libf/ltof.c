/*
 *	long signed integer to float
 */

#include "fp.h"

fp
ltof (x)
	register long x;
{
	fp result;

	if (x >= 0)
		return ultof ((unsigned long) x);
	
	result = ultof ((unsigned long) -x);
	SETSIGN (result, 1);

	return result;
}

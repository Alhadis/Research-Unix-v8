/*
 *	signed integer to float
 */

#include "fp.h"

fp
itof (x)
	register int x;
{
	fp result;

	if (x >= 0)
		return uitof ((unsigned) x);
	
	result = uitof ((unsigned) -x);
	SETSIGN (result, 1);

	return result;
}

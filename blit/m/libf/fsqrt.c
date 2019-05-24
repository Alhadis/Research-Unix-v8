/*
 *	fsqrt returns the square root of its floating
 *	point argument. Newton's method.
 *
 *	calls frexp, ldexp
 */

#include <errno.h>

double frexp(), ldexp();

double
fsqrt(arg)
double arg;
{
	double x, temp;
	int exp;

	if(arg <= 0)
		return 0;

	x = frexp (arg, &exp);

	if(exp & 1) {
		x *= 2;
		exp--;
	}

	/*
	 *	arg = x * 2**exp
	 *	0.5 <= x < 2
	 *
	 *	compute initial guess
	 */
	temp = (x + 1.0) * 0.5;

	/* Newton iteration */
	temp = (temp + x / temp) * 0.5;
	temp = (temp + x / temp) * 0.5;
	temp = (temp + x / temp) * 0.5;

	return ldexp (temp, exp >> 1);
}

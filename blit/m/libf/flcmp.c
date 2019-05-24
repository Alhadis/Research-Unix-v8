/*
 *	floating-point comparison -- specific to 68000
 */

int
flcmp (x, y)
	long x, y;
{
	register int r;

	r = 0;
	if (x < y)
		r = -1;
	else if (x > y)
		r = 1;

	if (x < 0 && y < 0)
		r = -r;

	return r;
}

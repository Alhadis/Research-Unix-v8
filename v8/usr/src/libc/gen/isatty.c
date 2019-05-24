/*
 * Returns 1 iff file is a tty
 */

#include <sgtty.h>

isatty(f)
{
	extern errno;
	struct sgttyb ttyb;

	if (gtty(f, &ttyb) < 0) {
		errno = 0;
		return(0);
	}
	return(1);
}

/*
 *	fd = ptopen (buf)
 *
 *	Attempt to open an odd pt - if successful, return the
 *	file descriptor in fd and place the name of the corresponding
 *	even pt in buf.  Buf must be large enough.
 *
 *	We assume that the digits 0-9 are consecutive in the
 *	machine's character set.
 */

#include <errno.h>

#define PTNAME "/dev/pt/pt01"

extern int errno;

int
ptopen (name)
	char *name;
{
	register int fd;
	register char *p, *q;

	/*
	 *	copy the initial name to user's buffer
	 *	leave p pointing at the last character
	 */
	p = name;
	q = PTNAME;
	while ((*p++ = *q++) != '\0')
		;
	p -= 2;
	
	/* try to open pt files until success or we run out */
	for (;;) {
		fd = open (name, 2);

		/* check for success */
		if (fd >= 0) {
			--*p;
			return fd;
		}

		/* check for run-out */
		if (errno != ENXIO)
			return -1;

		/* this pt is busy, try the next */
		if (*p != '9')
			*p += 2;
		else {
			/* propagate carry from low-order digit */
			*p = '1';
			q = p - 1;
			while (*q == '9')
				*q-- = '0';
			if (*q >= '0' && *q <= '9')
				++*q;
			else {
				/* insert a leading digit */
				register char *r = ++p;
				while (r > q) {
					r[1] = r[0];
					--r;
				}
				q[1] = '1';
			}
		}
	}
}

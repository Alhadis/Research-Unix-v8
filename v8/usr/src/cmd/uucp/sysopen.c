/*
 * open the next Systems file in sequence
 * all occurrences of `fopen(SYSFILE)' should be turned into
 * loops that come here
 * currently these exist in conn.c uuname.c versys.c
 * the names are also known by uucheck.c
 */

#include "uucp.h"

#ifdef MANYSYS

/*
 * suffixes to be tacked onto SYSFILE
 */

char *Sysnames[] = {
	".local",
	".dk",
	"",		/* just plain Systems */
	"1",
	".gen",
	NULL
};

int Nextsys;

sysrewind()
{

	Nextsys = 0;
}

FILE *
sysopen(mode)
char *mode;
{
	FILE *fp;
	char buf[MAXFULLNAME];

	while (Sysnames[Nextsys] != NULL) {
		sprintf(buf, "%s%s", SYSFILE, Sysnames[Nextsys]);
		Nextsys++;
		CDEBUG(4, "try %s\n", buf);
		if ((fp = fopen(buf, mode)) != NULL)
			return (fp);
	}
	return (NULL);
}

#endif

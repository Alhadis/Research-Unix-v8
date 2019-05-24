#include <stdio.h>
#define GFLIST "forwardlist"
#define LFLIST "forwardlist.lo"

/* imported */
extern char *upaspath();

static checkfile();

/*
 *	Return nonzero if forwarding is allowed to the system
 *	named in the string at "cp".  If we can't read the file
 *	which contains the systems, assume blanket forwarding.
 */
okrmt(cp)
char *cp;
{
	/* always accept the null string */
	if (*cp == '\0')
		return 1;

	/* try both general and local forwardlists */
	switch (checkfile(cp, GFLIST)) {
	case -1:
		return checkfile(cp, LFLIST);
	case 0:
		return checkfile(cp, LFLIST) == 1;
	}
	return 1;
}

static
checkfile(cp, file)
char *cp;
char *file;
{
	register FILE *fp;
	char buf[20];

	/* try to open the file; allow forwarding on failure */
	fp = fopen (upaspath(file), "r");
	if (fp == NULL)
		return -1;

	/* one iteration per system name in the file */
	while (fgets (buf, sizeof buf, fp) != NULL) {
		buf[strlen(buf)-1] = '\0';
		if (strcmp (buf, cp) == 0) {
			/* found it, allow forwarding */
			fclose (fp);
			return 1;
		}
	}

	/* didn't find it, prohibit forwarding */
	fclose (fp);
	return 0;
}

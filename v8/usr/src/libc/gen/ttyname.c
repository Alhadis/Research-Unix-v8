/*
 * on success:
 *	returns the pathname ("/dev/...") of the terminal
 *	with file descriptor "fd".
 *	bug: returns pointer to static area.
 * on failure:
 *	returns 0.
 */

char	*_ttyname();
static char ttybuf[32];

char *
ttyname(fd)
	int fd;
{
	return(_ttyname(&ttybuf[0], fd));
}

/*
 * on success:
 *	stores at "s" the pathname of the terminal with file descriptor fd,
 *	and returns "s".
 * on failure:
 *	leaves "s" unchanged,
 *	and returns 0.
 */

#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>

static char *dirlist[] = {
	"/dev/",
	"/dev/dk/",
	"/dev/pt/",
	0
};

char	*strcpy();
char	*strncat();

char *
_ttyname(s, fd)
	char *s;
	register int fd;
{
	register char **dpp, *dp;
	struct stat fstb, tsb;
	struct direct db;
	char tmps[32];

	if (fstat(fd, &fstb) < 0)
		return(0);
	for (dpp = dirlist; dp = *dpp++;) {
		if ((fd = open(dp, 0)) < 0)
			continue;
		while (read(fd, (char *) &db, sizeof(db)) == sizeof(db)) {
			if (db.d_ino == 0 || db.d_ino != fstb.st_ino)
				continue;
			strcpy(tmps, dp);
			strncat(tmps, db.d_name, sizeof(db.d_name));
			if (stat(tmps, &tsb) < 0)
				continue;
			if (tsb.st_dev != fstb.st_dev || tsb.st_ino != fstb.st_ino)
				continue;
			close(fd);
			strcpy(s, tmps);
			return(s);
		}
		close(fd);
	}
	return(0);
}

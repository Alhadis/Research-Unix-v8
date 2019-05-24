#include <stdio.h>
#include <sys/types.h>
#include <dir.h>
#include <sys/stat.h>
#include "string.h"

/* imports */
extern char *mktemp();
extern long time();
extern unsigned int sleep();

#define LOCKPREFIX "/tmp/L."
static char    lockname[DIRSIZ+sizeof(LOCKPREFIX)] = { 0 };

/* break an old lock */
static void
breaklock(file, lock)
	char *file, *lock;
{
	FILE *fp;

	fprintf(stderr, "mail: breaking lock\n");
	if ((fp=fopen("/usr/spool/mail/mail.log", "a")) != NULL) {
		fprintf(fp, "lock-err file=%s lock=%s\n", file, lock);
		fclose(fp);
	}
	unlink(lock);
}

/* Lock the given file.  The parameter "file" must contain at least one '/'. */
extern void
lock(file)
char *file;
{
#	define TMPLNAME "/tmp/mlXXXXX"
#	define LOCKPREFIX "/tmp/L."
	char tmplname[sizeof(TMPLNAME)];
	struct stat stbuf;
	int fd;

	/* return if we are already in the middle of a lock */
	if (*lockname != '\0') {
		fprintf(stderr, "mail: lockfile botch\n");
		exit(1);
	}

	/* create a temporary file */
	(void)strcpy(tmplname, TMPLNAME);
	(void)mktemp(tmplname);
	if ((fd=creat(tmplname, 0444))<0)
		return;
	close(fd);

	/* Make a link to it with the lock file name.  This will fail only
	 * if it already exists.
	 */
	(void)strcpy(lockname, LOCKPREFIX);
	(void)strcat(lockname, strrchr(file, '/')+1);
	lockname[DIRSIZ+sizeof("/tmp/")-1] = '\0';
	while (link(tmplname, lockname) < 0) {
		long now;

		/* File is already locked.  Break it if the lock is old. */
		sleep(2);
		now = time((long *)0);
		if (stat(lockname, &stbuf)==0 && stbuf.st_ctime+60 < now) {
			if (stat(file, &stbuf)==0 && stbuf.st_mtime+180 >= now)
				continue;
			breaklock(file, lockname);
		}
	}
	unlink(tmplname);
	return;
}

extern void
unlock()
{
	if (*lockname != '\0') {
		unlink(lockname);
		*lockname = '\0';
	}
}

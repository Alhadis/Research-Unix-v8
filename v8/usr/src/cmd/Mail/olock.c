#

/*
 * A mailing program.
 *
 * Stuff to do version 7 style locking.
 */

#include "rcv.h"
#include <sys/stat.h>

static char *SccsId = "@(#)lock.c	2.3 12/17/82";

static	int		locked;			/* To note that we locked it */
static 	char 		lockfile[64];		/* Last locked file */

/*
 * Lock the specified mail file by setting the execute bit on mailfile.
 * We must, of course, be careful to remove the lock by a call
 * to unlock before we stop.  The algorithm used here is to see if
 * the lock exists, and if it does, to check its modify time.  If it
 * is older than 5 minutes, we assume error and set our own lock.
 * Otherwise, we wait for 5 seconds and try again.
 */

lock(file)
char *file;
{
	register int f;
	struct stat sbuf;
	long curtime;

	if (file == NOSTR) {
		printf("Locked = %d\n", locked);
		return(0);
	}
	if (locked)
		return(0);
	strcpy(lockfile, file);
	for (;;) {
		f = lock1(file);
		if (f == 0) {
			locked = 1;
			return(0);
		}
		if (stat(file, &sbuf) < 0)
			return(0);
		time(&curtime);
		if (curtime < sbuf.st_ctime + 300) {
			sleep(5);
			continue;
		}
		unlock();
	}
}

/*
 * Remove the mail lock, and note that we no longer
 * have it locked.
 */

unlock()
{
	struct stat sbuf;

	if (stat(lockfile, &sbuf) < 0)
		return;
	if (chmod(lockfile, sbuf.st_mode & ~01) < 0)
		return;
	locked = 0;
}

/*
 * Attempt to set the lock by setting the execute bit.
 * If it fails, return -1 else 0
 */

lock1(file)
char *file;
{
	struct stat sbuf;

	if (stat(file, &sbuf) < 0)
		return(-1);
	if (sbuf.st_mode & 01)
		return(-1);
	if (chmod(file, sbuf.st_mode | 01) < 0)
		return(-1);
	return(0);
}

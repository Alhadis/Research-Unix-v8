/*
 *	notify user on mail arrival
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utmp.h>
#include "string.h"

/* imports */
extern char *thissys;
extern unsigned int alarm();

extern void
setnotify()
{
	struct stat s;
	char *ttyname();
	char *p;

	fstat(2, &s);
	setgid(getgid());
	setuid(getuid());
	if ((p = ttyname(2)) && *p)
		chmod(p, s.st_mode ^ S_IEXEC);
}


static int
blurbtime()
{
}

static void
blurb(tty, sender, maxsize)
char *tty, *sender;
{
	FILE *f = NULL;
	struct stat s;
	char devtty[32];

	strcpy(devtty, "/dev/");
	strncat(devtty, tty, maxsize);
	signal(SIGALRM, blurbtime);
	alarm(30);
	stat(devtty, &s);
	if (s.st_mode & S_IEXEC)	/* notify only if enabled */
	if ((f = fopen(devtty, "w")) != NULL) {
		fprintf(f, "\r\n[%s: mail from %s]\r\n\7", thissys,
		  sender);
	}
	if (f)
		fclose(f);
	alarm(0);
}

extern int
notify(name, sender)
char *name, *sender;
{
	struct utmp entry;
	FILE *f;

	if ((f = fopen("/etc/utmp", "r")) == NULL)
		return;
	while (fread((char *)&entry, sizeof(entry), 1, f) == 1) {
		if (strncmp(name, entry.ut_name, sizeof(entry.ut_name)) == 0)
			blurb(entry.ut_line, sender, sizeof(entry.ut_line));
	}
	fclose(f);
}

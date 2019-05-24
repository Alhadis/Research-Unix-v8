#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mail.h"
#include "string.h"

/* imported */
extern char *getenv();
extern int getpid();
extern char *ttyname();

/* global to the module */
static char semaphore[PATHSIZE];
static char tty[64];

extern void
V()
{
	unlink(semaphore);
}

/* return name of tty if file is already being read, NULL otherwise */
extern char *
P()
{
	char file[PATHSIZE];
	struct stat sbuf;
	FILE *fp;
	int pid;
	char *home = getenv("HOME");

	if (home == NULL)
		return NULL;
	(void)strcpy(file, home);
	(void)strcat(file, "/.Maillock");
	if (stat(file, &sbuf) >= 0) {

		/* lock file exists */
		fp = fopen(file, "r");
		if (fp == NULL)
			return "another tty";
		fscanf(fp, "%d %s", &pid, tty);
		fclose(fp);
		if (kill(pid, 0) == 0)
			return tty;
	}

	/* create a semaphore file */
	strcpy(semaphore, file);
	V();
	fp = fopen(semaphore, "w");
	if (fp == NULL)
		return 0;		/* nothing else we can do */
	fprintf(fp, "%d %s\n", getpid(), ttyname(2));
	fclose(fp);
	return 0;
}


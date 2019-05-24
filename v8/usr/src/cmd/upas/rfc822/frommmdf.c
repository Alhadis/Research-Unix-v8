#include <stdio.h>
#include "mail.h"

/* imports */
extern char *strcpy();
extern char *convertaddr();
extern FILE *popen();

/* predeclared */
char *lowercase();

main(ac, av)
int ac;
char *av[];
{
	char addr[FROMLINESIZE];
	char cmd[CMDSIZE];
	char buf[FROMLINESIZE];
	int status;
	FILE *fp;

	if (av[2][0] != 0)
		sprintf(addr, "%s.%s", av[1], av[2]);
	else 
		strcpy(addr, av[1]);

	/* start up the mailer and pipe mail into it */
	sprintf(cmd, "/bin/rmail %s", convertaddr(lowercase(addr)));
	fp = (FILE *)popen(cmd, getuid(), getgid());
	if (fp == NULL)
		exit(1);
	(void)from822("csnet", fp);	

	/* return any errors */
	status = pclose(fp, buf, sizeof(buf));
	if (status)
		write(2, buf, strlen(buf));
	return status;
}

/*
 *	Convert a string to lower case.
 */
char *
lowercase(sp)
char *sp;
{
	register char *lp = sp;

	while(*lp) {
		*lp = tolower(*lp);
		lp++;
	}
	return sp;
}


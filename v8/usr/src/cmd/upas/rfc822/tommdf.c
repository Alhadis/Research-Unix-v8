#include <stdio.h>
#include "mail.h"

int rmail = 0;
int onatty = 0;

/* imports */
extern FILE *popen();
extern char *convertaddr();
extern void to822();

main(ac, av)
int ac;
char *av[];
{
	char buf[FROMLINESIZE];
	int status;
	FILE *fp;

	if (ac != 3)
		exit(1);
	/* start up the mailer and pipe mail into it */
	fp = (FILE *)popen("/usr/mmdf/lib/submit -stmlrxto", getuid(), getgid());
	if (fp == NULL)
		exit(1);
	fprintf(fp, "To: %s\n", convertaddr(av[1]));
	(void)to822(fp, av[2]);	

	/* return any errors */
	status = pclose(fp, buf, sizeof(buf));
	if (status)
		write(2, buf, strlen(buf));
	return status;
}

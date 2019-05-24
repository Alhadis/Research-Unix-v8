#include "mail.h"

/* global to this module */
static char rmtlist[ADDRSIZE];

/*  Parse a UNIX from line.
 *
 *  Returns: -1 if not a valid from line
 *	      0 otherwise
 */
extern int
parsefrom(line, sender, date)
	char *line;	/* line to parse */
	char *sender;	/* filled by parseline */
	char *date;	/* filled by parseline */
{
	char *rp, *dp, *sp;

	if ((sp = stringin(FROM, line))==NULL
	    && (sp = stringin(ALTFROM, line))==NULL) {
	    

		/* tack sender's name and date onto the list of remote machines */
		(void)strcat(rmtlist, sender);
		(void)strcpy(sender, rmtlist);
		return 0;
	} else if ((rp = stringin(REMFROM, line))==NULL) {

		/* parse normal from line */
		if ((dp = strchr(sp, ' ')) != NULL) {
			strcpy(date, dp+1);
			*dp = '\0';
		}
		(void)strcat(rmtlist, sp);
		(void)strcpy(sender, rmtlist);
		return 0;
	} else {

		/* parse remote from line */
		line[strlen(line)-1] = '\0';
		*(rp - (sizeof(REMFROM)-1)) = '\0';
		if ((dp = strchr(sp, ' ')) != NULL) {
			strcpy(date, dp+1);
			*dp = '\0';
		}
		(void)strcpy(sender, sp);
		(void)strcat(rmtlist, rp);
		(void)strcat(rmtlist, "!");
		return -1;
	}
}


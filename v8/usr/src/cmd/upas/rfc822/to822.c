#include <stdio.h>
#include <ctype.h>
#include "mail.h"
#include "header.h"
#include "string.h"

/* imports */
header hdrs[] = {
	HEADER("Date:"),
	HEADER("From:"),
	HEADER("To:"),
	HEADER("Received:"),
	HEADER("Message-Id:"),
	HEADER("Status:"),
	HEADER("")
};
#define datehdr hdrs[0]
#define fromhdr hdrs[1]
#define tohdr hdrs[2]
#define receivedhdr hdrs[3]
#define idhdr hdrs[4]
#define statushdr hdrs[5]

/* imported */
extern void getheader();
extern void printheaders();
extern void addheader();
extern char *fgets();
extern int getunix();
extern void initgetunix();

/* predeclared */
char *gets();
static char *convertdate();
extern char *convertaddr();

extern void
to822(fp, hostname)
FILE *fp;		/* file to output to */
char *hostname;		/* name of our host */
{
	char buf[FROMLINESIZE];
	char from[FROMLINESIZE];
	char date[DATESIZE];

	/* get UNIX from line */
	if (fgets(buf, sizeof buf, stdin) == NULL)
		exit(1);

	/* first line had better be a from */
	if (getunix(buf, from, date) == 0)
		exit(1);

	/* get any pre-existing RFC822 header lines */
	getheader();

	/* output new message */
	fprintf(fp, "%s %s.%s\n", fromhdr.name, convertaddr(from), hostname);
	fprintf(fp, "%s %s\n\n", datehdr.name, convertdate(date));
	printheaders(fp);
	while (gets(buf) != NULL)
		fprintf(fp, "%s\n", buf);
}

/* juggle date fields */
static char *
convertdate(date)
char *date;
{
	static char ndate[DATESIZE];
	char *field[6];
	char *sp;
	int i;

	sp = date;
	/* parse the date into fields */
	for (i = 0; i < 6; i++) {		
		while (isspace(*sp) || *sp == ',' || *sp == '-')
			*sp++ = '\0';
		field[i] = sp;
		while (!isspace(*sp) && *sp != ',' && *sp != '-' && *sp != '\0')
			sp++;
	}
	*sp = '\0';

	/* shuffle the fields into internet format */
	sprintf (ndate, "%s, %s %s %s %s %s", field[0], field[2], field[1],
		 field[4], field[5], field[3]);
	return ndate;
}

/*
 *	Convert from `bang' to `rfc822' format.
 */
extern char *
convertaddr(addr)
char *addr;
{
	static char buf[FROMLINESIZE];
	register int i=0;
	register char *sp;
	char *field[128];

	sp = field[i] = addr;
	while (*sp) {
		if (*sp == '!') {
			*sp = '\0';
			if (strcmp(field[i], "uucp") == 0) {
				/* the token 'uucp' stops the parsing */
				field[i] = ++sp;
				break;
			}
			field[++i] = ++sp;
		} else
			++sp;
	}
	strcpy(buf, field[i--]);
	if (i >= 0) {
		strcat(buf, "@");
		strcat(buf, field[i--]);
	}
	while (i >= 0) {
		strcat(buf, ".");
		strcat(buf, field[i--]);
	}
		
	return buf;
}

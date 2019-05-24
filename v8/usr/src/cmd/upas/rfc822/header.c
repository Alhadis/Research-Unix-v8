#include <stdio.h>
#include <ctype.h>
#include "mail.h"
#include "header.h"
#include "string.h"

/* imported */
extern char *malloc();
extern char *strcpy();
extern char *strchr();
extern int strncmp();
extern char *fgets();

/* predeclared */
extern void addheader();
static char *newstring();

/*
 *	Input header from standard input.  Actually two extra lines are also
 *	read, but this isn't a problem.  Save some header lines in the header
 *	array.
 */
extern void
getheader()
{
	char buf[2*FROMLINESIZE];
	char line[2*FROMLINESIZE];
	header *hp;
 
	if (fgets(buf, sizeof buf, stdin) == NULL)
		buf[0] = '\0';
	while (buf[0] != '\n' && buf[0] != '\0') {
		/* gather a multiple line header field */
		line[0] = '\0';
		do {
			(void)strcat(line, buf);
			if (fgets(buf, sizeof buf, stdin) == NULL)
				buf[0] = '\0';
		} while (buf[0] == ' ' || buf[0] == '\t');

		/* header lines must contain `:' */
		if (strchr(line, ':') == NULL) {
			addheader(line);
			break;
		}

		/* look for `important' headers */
		for (hp = hdrs; *(hp->name) != '\0'; hp++) {
			if (STRCMP(line, hp) == 0) {
				hp->line = newstring(line+(hp->size));
				break;
			};
		}
		if (*(hp->name) == '\0')
			addheader(line);
	}
	/* at this point, buf contains the first line of the body */
	addheader(buf);
}

static char*
newstring(line)
char *line;
{
	char *rv;

	rv = malloc(strlen(line)+1);
	if (rv == NULL) {
		perror("reading header");
		exit(1);
	}
	return strcpy(rv, line);
}

/*
 *	Keep a list of header lines to output.
 */
typedef struct quux{
	struct quux *next;
	char line[1];
} hlist;
static hlist dummy = { &dummy, '\0' };
static hlist *list = &dummy;

/*
 *	Add to list of header lines.
 */
extern void
addheader(line)
char *line;
{
	hlist *thing;

	thing = (hlist *)malloc(strlen(line) + sizeof(hlist));
	if (thing == NULL) {
		perror("reading header");
		exit(1);
	}
	strcpy(thing->line, line);
	thing->next = list->next;
	list->next = thing;
	list = thing;
}

/*
 *	Print list of headers.
 */
extern void
printheaders(fp)
FILE *fp;
{
	hlist *thing;

	for(thing = list->next->next; thing != &dummy; thing = thing->next)
		fprintf(fp, "%s", thing->line);
}

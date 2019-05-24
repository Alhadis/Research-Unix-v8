#include <stdio.h>
#include <ctype.h>
#include "mail.h"
#include "header.h"
#include "string.h"

/*
 *	Convert the rfc822 message on standard input into `UNIX' format
 *	and write it onto the passed FILE. 
 */

/* header tags */
header hdrs[] = {
	HEADER("Date:"),
	HEADER("From:"),
	HEADER("Sender:"),
	HEADER("Received:"),
	HEADER("Message-Id:"),
	HEADER("Status:"),
	HEADER("")
};
#define datehdr hdrs[0]
#define fromhdr hdrs[1]
#define senderhdr hdrs[2]
#define receivedhdr hdrs[3]
#define idhdr hdrs[4]
#define statushdr hdrs[5]
#define hideoushdr hdrs[6]

#ifdef SILLY
/* silly fields */
header silly[] = {
#include "silly.name"
};
#define NSILLY sizeof(silly)/sizeof(header)
#endif SILLY

/* imported */
extern void getheader();
extern void printheaders();
extern void addheader();
extern char *gets();
extern void putrfunix();

/* predeclared */
static int getfrom();
static void getdate();
extern char *convertaddr();

from822(netname, fp)
char *netname;
FILE *fp;
{
	register int rv;
	char from[FROMLINESIZE];
	char date[FROMLINESIZE];
	char line[2*FROMLINESIZE];

	getheader();
	from[0] = date[0] = '\0';

	/*  Get sender's address.  If anything else is on the from line,
	 *  keep it under another name.  If no `From:' lineis found, use
	 *  `Sender:' line.
	 */
	if (fromhdr.line != NULL) {
		rv = getfrom(fromhdr.line, from);
	} else if (senderhdr.line != NULL)
		getfrom(senderhdr.line, from);
	
	/*  Get date line */
	if (datehdr.line != NULL)
		getdate(datehdr.line, date);

	/* output UNIX header */
	if (*from != '\0' && *date != '\0')
		putrfunix(convertaddr(from), date, netname, fp);

	/* output the rest */
	if (rv > 0) {
#ifdef SILLY
		srand(time((long *)0));
		fprintf(fp, "%s%s", silly[nrand(NSILLY)].name, fromhdr.line);
#else
		fprintf(fp, "%s%s", "Original-From: ", fromhdr.line);
#endif
	}
	if (senderhdr.line != NULL)
		fprintf(fp, "%s%s", senderhdr.name, senderhdr.line);
	printheaders(fp);
	while (gets(line) != NULL)
		fprintf(fp, "%s\n", line);
}

/*
 *  The sender is either the next first whitespace delimited token or
 *  the first thing enclosed in "<" ">".
 *
 *  Returns:  0		if a from line with only the address
 *	     >0		if a from line with other cruft in it
 */
static int
getfrom(line, sender)
char *line, *sender;
{
	register char *lp, *sp;
	register int comment = 0;
	register int anticomment = 0;
	register int inquote = 0;
	int rv = 0;

	lp = line;
	for (sp = sender; *lp; lp++) {
		if (comment) {
			if (*lp==')')
				comment = 0;
			continue;
		}
		if (anticomment) {
			if (*lp=='>')
				break;
		}
		if (inquote) {
			if (*lp=='"')
				inquote = 0;
			*sp++ = *lp;
			continue;
		}
		switch (*lp) {
		case '\t':
		case '\n':
			break;
		case ' ':
			if (strncmp(lp, " at ", sizeof(" at ")-1)==0) {
				*sp++ = '@';
				lp += sizeof(" at ")-2;
			}
			break;
		case '<':
			rv++;
			anticomment = 1;
			sp = sender;
			break;
		case '(':
			rv++;
			comment = 1;
			break;
		case ',':
			sp = sender;
			break;
		case '"':
			inquote = 1;
			/* fall through */
		default:
			*sp++ = *lp;
			break;
		}
	}
	*sp = '\0';
	return rv;
}

/*
 *  Get a date line.  Convert to `UNIX' format.
 *
 */
static void
getdate(line, date)
char *line, *date;
{
	register char *sp;

	sp = line + datehdr.size;
	while (isspace(*sp) || *sp == ',' || *sp == '-')
		sp++;
	while (*sp != '\0' && *sp != '\n')
		*date++ = *sp++;
	*date = '\0';
}

/*
 *	Convert from to `bang' format. 
 */
extern char *
convertaddr(from)
char *from;
{
	static char buf[FROMLINESIZE];
	char *sp;

	buf[0] = '\0';
	for (sp = from + strlen(from); sp >= from; sp--) {
		if (*sp == '.') {
			strcat(buf, sp+1);
			strcat(buf, "!");
			*sp = '\0';
		} else if (*sp == '@' || *sp == '%') {
			strcat(buf, sp+1);
			strcat(buf, "!");
			*sp = '\0';
			break;
		}
	}
	if (strchr(from, '!') != NULL)
		strcat(buf, "uucp!");
	strcat(buf, from);

	return buf;
}

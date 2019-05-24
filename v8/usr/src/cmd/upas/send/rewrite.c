#include <stdio.h>
#include <ctype.h>
#include <regexp.h>
#include "mail.h"
#include "string.h"

/* configuration */
#define RULEFILE "rewrite"
#define BUFFERSIZE 512 

/* 
 *	Routines for dealing with the rewrite rules.
 */

/* imports */
extern char *malloc();
extern int strlen();
extern regexp *regcomp();
extern int regexec();
extern void regsub();
extern char *upaspath();
extern char *thissys;

/* globals */
typedef struct rule rule;

struct rule {
	char *matchre;		/* address match */
	char *cmdre;		/* command used to send message */
	char *machre;		/* next hop */
	char *restre;		/* address after the next hop */
	regexp *program;
	rule *next;
};
static rule *rulep;
static rule alwayslocal = {
	".*",
	"",
	"",
	NULL
};
static int eof = 0, eol = 0;


/* predeclared */
static char *getstring();
static char *newstring();
static void substitute();
static int geteol();

extern int
getrules()
{
	FILE	*rfp;
	rule	*rp, *rlastp=NULL;
	char	*bp;

	eof = eol = 0;
	rfp = fopen(upaspath(RULEFILE), "r");
	if (rfp == NULL) {
		rulep = &alwayslocal;
		return -1;
	}
	do {
		/* ignore comments */
		bp = getstring(rfp);
		if (*bp=='#' || *bp=='\0')
			continue;

		/* get a rule */
		rp = (rule *)malloc(sizeof(rule));
		if (rp == NULL)
			return -1;
		if (rulep == NULL)
			rulep = rlastp = rp;
		else
			rlastp = rlastp->next = rp;
		rp->next = NULL;
		rp->matchre = newstring(bp);
		rp->cmdre = newstring(getstring(rfp));
		rp->machre = newstring(getstring(rfp));
		rp->restre = newstring(getstring(rfp));
		rp->program = NULL;
	} while (geteol(rfp));
	fclose(rfp);
#ifdef DEBUG
	dumprules();
#endif DEBUG
	return 0;
}

static char *
getstring(fp)
	FILE *fp;
{
	static char buf[BUFFERSIZE];
	int c, term1=' ', term2='\t';
	char *bp=buf;

	if (!eol) {
		/* skip whitespace */
		do {
			c = getc(fp);

			/* hack to recognize escaped newline */
			if (c == '\\') {
				c = getc(fp);
				if (c == '\n')
					c = ' ';
				else
					*bp++ = '\\';
			}
		} while (c == ' ' || c == '\t');

		/* possible quoted string */
		if (c == '\'' || c == '"') {
			term1 = term2 = c;
			c = getc(fp);
		}

		while (c != term1 && c != term2 && !eol) {
			switch (c) {
			case EOF:
				eof = 1;
				/* fall through */
			case '\n':
				eol = 1;
				break;
			case '\\':
				c = getc(fp);
				/* hack to recognize escaped newline */
				if (c == '\n') {
					c = term1;
					break;
				} else {
					*bp++ = '\\';
					/* fall through */
				}
			default:
				*bp++ = c;
				c = getc(fp);
				break;
			}
		}

	}
	*bp = 0;

	return buf;
}

static int
geteol(fp)
	FILE *fp;
{
	while(!eol)
		getstring(fp);
	eol = 0;
	return !eof;
}

static char *
newstring(sp)
	char *sp;
{
	char *np;

	np = malloc(strlen(sp)+1);
	if (np == NULL) {
		fprintf(stderr, "mail: out of memory\n");
		exit(1);
	}
	strcpy(np, sp);
	return np;
}

static rule *
findrule(addrp)
	char *addrp;
{
	rule *rp;

	for (rp = rulep; rp != NULL; rp = rp->next) {
		if (rp->program == NULL)
			rp->program = regcomp(rp->matchre);
		if (regexec(rp->program, addrp))
			return rp;
	}
	return NULL;
}

/*  Transforms the address into a command.
 *  Returns:	-1 if address not matched by reules
 *		 0 if address matched and ok to forward
 *		 1 if address matched and not ok to forward
 */
extern int
rewrite(addrp, s, chkfl, cmdp)
	char *addrp;		/* address to rewrite */
	char *s;		/* string that matches \s */
	int chkfl;		/* non-zero if we must check forwarding */
	char *cmdp;		/* (returned) substituted strings */
{
	rule *rp;
	char dest[ADDRSIZE];	/* next hop for message */
	char rest[ADDRSIZE];	/* rest of address after hop */
	char addr[ADDRSIZE];	/* rest of address after hop */

	/* rewrite the address */
	rp = findrule(addrp);
	if (rp == NULL)
		return -1;
	if (cmdp != NULL)
		substitute(rp->program, rp->cmdre, cmdp, s);

	/* check for legal forwarding */
	if (!chkfl)
		return 0;
	while (1) {
		substitute(rp->program, rp->restre, rest, s);
		substitute(rp->program, rp->machre, dest, s);
#ifdef DEBUG
		printf("	%s %s\n", dest, rest);
#endif DEBUG
		if (dest[0] == '\0')
			return 0;
		if (!okrmt(dest))
			return 1;
		strcpy(addr, rest);
		rp = findrule(addr);
		if (rp == NULL)
			return 1;
		
	}
}

static void
substitute(progp, sp, dp, s)
	regexp *progp;	/* context of substitution */
	char *sp;	/* source string */
	char *dp;	/* destination string */
	char *s;	/* strig to substitute for \s */
{
	char *ssp;

	while (*sp != '\0') {
		if (*sp == '\\') {
			switch (*++sp) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if (progp->startp[*sp-'0'] != NULL)
					for (ssp = progp->startp[*sp-'0'];
					     ssp < progp->endp[*sp-'0'];
					     ssp++)
						*dp++ = *ssp;
				break;
			case '\\':
				*dp++ = '\\';
				break;
			case '\0':
				sp--;
				break;
			case 's':
				for(ssp = s; *ssp; ssp++)
					*dp++ = *ssp;
				break;
			default:
				*dp++ = *sp;
				break;
			}
		} else if (*sp == '&') {				
			if (progp->startp[0] != NULL)
				for (ssp = progp->startp[0];
				     ssp < progp->endp[0]; ssp++)
					*dp++ = *ssp;
		} else
			*dp++ = *sp;
		sp++;
	}
	*dp = '\0';
}

#ifdef DEBUG
regdump(subp)
	regexp *subp;
{
	int i;
	char *cp;

	printf ("matches are:\n");
	for (i = 0; i < NSUBEXP; i++) {
		if (subp->startp[i] != NULL) {
			printf("\t(%d) ", i);
			for (cp = subp->startp[i]; cp < subp->endp[i]; cp++)
				putchar(*cp);
			putchar('\n');
		}
	}
}
#endif DEBUG

#ifdef DEBUG
dumprules()
{
	rule *rp;

	for (rp = rulep; rp != NULL; rp = rp->next) {
		fprintf (stderr, "matchre: %s\n", rp->matchre);
		fprintf (stderr, "cmdre: %s\n", rp->cmdre);
		fprintf (stderr, "machre: %s\n", rp->machre);
		fprintf (stderr, "restre: %s\n", rp->restre);
	}
}
#endif DEBUG

regerror(s)
char* s;
{
	fprintf(stderr, "rewrite: %s\n", s);
	exit(1);
}

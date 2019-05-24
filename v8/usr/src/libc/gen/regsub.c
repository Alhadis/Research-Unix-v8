#include "regprog.h"
#define NULL 0

/* substitute into one string using the matches from the last regexec() */
extern void
regsub (progp, sp, dp)
	Prog *progp;	/* context of substitution */
	char *sp;	/* source string */
	char *dp;	/* destination string */
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
				if (progp->se.startp[*sp-'0'] != NULL)
					for (ssp = progp->se.startp[*sp-'0'];
					     ssp < progp->se.endp[*sp-'0'];
					     ssp++)
						*dp++ = *ssp;
				break;
			case '\\':
				*dp++ = '\\';
				break;
			case '\0':
				sp--;
				break;
			default:
				*dp++ = *sp;
				break;
			}
		} else if (*sp == '&') {				
			if (progp->se.startp[0] != NULL)
				for (ssp = progp->se.startp[0];
				     ssp < progp->se.endp[0]; ssp++)
					*dp++ = *ssp;
		} else
			*dp++ = *sp;
		sp++;
	}
	*dp = '\0';
}

#ifdef DEBUG
regdump(subp)
	Subexp *subp;
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

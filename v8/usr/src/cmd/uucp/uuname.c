/*	/sccs/src/cmd/uucp/s.uuname.c
	uuname.c	1.1	8/30/84 17:38:10
*/
#include "uucp.h"
VERSION(@(#)uuname.c	1.1);
 
/*
 * returns a list of all remote systems.
 * option:
 *	-l	-> returns only the local system name.
 */
main(argc,argv, envp)
int argc;
char **argv, **envp;
{
	FILE *np;
	register short lflg = 0;
	char s[BUFSIZ], prev[BUFSIZ], name[BUFSIZ];
#ifdef MANYSYS
	int nsys = 0;
	FILE *sysopen();
#endif

	while (*(++argv) && *argv[0] == '-')
		switch(argv[0][1]) {
		case 'l':
			lflg++;
			break;
		default:
			(void) fprintf(stderr, "usage: uuname [-l]\n");
			exit(1);
		}
 
	if (lflg) {
		uucpname(name);

		/* initialize to null string */
		(void) printf("%s",name);
		(void) printf("\n");
		exit(0);
	}
#ifndef MANYSYS
	if ((np=fopen(SYSFILE, "r")) == NULL) {
		(void) fprintf(stderr, "File \" %s \" is protected\n", SYSFILE);
		exit(1);
	}
#else
	sysrewind();
	while ((np = sysopen("r")) != NULL) {
		nsys++;
#endif
	while (fgets(s, BUFSIZ, np) != NULL) {
		if((s[0] == '#') || (s[0] == ' ') || (s[0] == '\t') || 
		    (s[0] == '\n'))
			continue;
		(void) sscanf(s, "%s", name);
		if (EQUALS(name, prev))
		    continue;
		(void) printf("%s", name);
		(void) printf("\n");
		(void) strcpy(prev, name);
	}
#ifdef MANYSYS
	}
	if (nsys == 0) {
		fprintf(stderr, "cannot open any System files\n");
		exit(1);
}
#endif
	exit(0);
}

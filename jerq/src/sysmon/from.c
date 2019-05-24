#include <stdio.h>
#include <ctype.h>
#include <regexp.h>

/* imports */
extern int strlen();
extern char *strrchr();

extern int
from(addrp, src, sender)
	char *addrp;		/* address to rewrite */
	char *src;		/* where to source machine name */
	char *sender;		/* where to put sender name */
{
	char *nsender, *nsrc, *cp;

	/* very cruddy algorithm */
	nsender = strrchr(addrp, '!');
	if (nsender == NULL) {
		/* local mail */
		nsender = addrp;
		nsrc = "";
	} else {
		/* get last uucp hop */
		*nsender = 0;
		nsender++;
		nsrc = strrchr(addrp, '!');
		if (nsrc == NULL)
			nsrc = addrp;
		else
			nsrc++;
		/* look for arpa style address */
		for (cp = nsender; *cp; cp++) {
			if (*cp == '@' || *cp == '%' || *cp == '.') {
				*cp = '\0';
				nsrc = ++cp;
				break;
			}
		}
		for (; *cp; cp++) {
			if (*cp == '@' || *cp == '%' || *cp == '.') {
				*cp = '\0';
				break;
			}
		}
	}
	(void)strcpy(sender, nsender);
	(void)strcpy(src, nsrc);
	return 0;
}

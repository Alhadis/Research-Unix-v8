/*
 * chown uid[,gid] file ...
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

struct	passwd	*getpwnam();
struct	group	*getgrnam();

main (argc, argv)
	char *argv[];
{
	register c;
	register char *cuid, *cgid;
	int uid, gid, status;
	struct stat stbuf;
	struct passwd *pwd;
	struct group *grp;
	
	if (argc < 3) {
		printf("usage: chown uid[,gid] file ...\n");
		exit(4);
	}

	cuid = cgid = argv[1];

	/* split the first argument into uid and gid parts */
	while (*cgid != '\0' && *cgid != ',')
		cgid++;
	if (*cgid == ',')
		*cgid++ = '\0';
	else
		cgid = NULL;

	if (isnumber(cuid))
		uid = atoi(cuid);
	else {
		if ((pwd = getpwnam (cuid)) == NULL) {
			printf ("unknown user id: %s\n", cuid);
			exit (4);
		}
		uid = pwd->pw_uid;
	}

	if (cgid) {
		if (isnumber (cgid))
			gid = atoi (cgid);
		else {
			if ((grp = getgrnam (cgid)) == NULL) {
				printf ("unknown group id: %s\n", cgid);
				exit (4);
			}
			gid = grp->gr_gid;
		}
	}

	for(c=2; c<argc; c++) {
		stat(argv[c], &stbuf);
		if(chown(argv[c], uid, cgid? gid: stbuf.st_gid) < 0) {
			perror(argv[c]);
			status = 1;
		}
	}
	exit(status);
}

isnumber(s)
char *s;
{
	register c;

	while(c = *s++)
		if(!isdigit(c))
			return(0);
	return(1);
}

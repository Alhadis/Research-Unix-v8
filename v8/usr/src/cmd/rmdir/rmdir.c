/*
 * Remove directory
 */

#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <stdio.h>

int	Errors = 0;
char	*rindex();
char	*strcat();
char	*strcpy();
char	*cmdname;

main(argc,argv)
int argc;
char **argv;
{

	cmdname = argv[0];
	if(argc < 2) {
		fprintf(stderr, "%s: arg count\n", cmdname);
		exit(1);
	}
	while(--argc)
		rmdir(*++argv);
	exit(Errors!=0);
}

rmdir(d)
char *d;
{
	int	fd;
	char	*np, name[500];
	struct	stat	st, cst;
	struct	direct	dir;

	strcpy(name, d);
	if((np = rindex(name, '/')) == NULL)
		np = name;
	if(stat(name,&st) < 0) {
		fprintf(stderr, "%s: %s non-existent\n", cmdname, name);
		++Errors;
		return;
	}
	if (stat("", &cst) < 0) {
		fprintf(stderr, "%s: cannot stat \", cmdname\"");
		++Errors;
		exit(1);
	}
	if((st.st_mode & S_IFMT) != S_IFDIR) {
		fprintf(stderr, "%s: %s not a directory\n", cmdname, name);
		++Errors;
		return;
	}
	if(st.st_ino==cst.st_ino &&st.st_dev==cst.st_dev) {
		fprintf(stderr, "%s: cannot remove current directory\n", cmdname);
		++Errors;
		return;
	}
	if((fd = open(name,0)) < 0) {
		fprintf(stderr, "%s: %s unreadable\n", cmdname, name);
		++Errors;
		return;
	}
	while(read(fd, (char *)&dir, sizeof dir) == sizeof dir) {
		if(dir.d_ino == 0) continue;
		if(!strcmp(dir.d_name, ".") || !strcmp(dir.d_name, ".."))
			continue;
		fprintf(stderr, "%s: %s not empty\n", cmdname, name);
		++Errors;
		close(fd);
		return;
	}
	close(fd);
	if(!strcmp(np, ".") || !strcmp(np, "..")) {
		fprintf(stderr, "%s: cannot remove . or ..\n", cmdname);
		++Errors;
		return;
	}
	strcat(name, "/.");
	if((access(name, 0)) < 0) {		/* name/. non-existent */
		strcat(name, ".");
		goto unl;
	}
	strcat(name, ".");
	if((access(name, 0)) < 0)		/* name/.. non-existent */
		goto unl2;
	if(access(name, 02)) {
		name[strlen(name)-3] = '\0';
		fprintf(stderr, "%s: %s: no permission\n", cmdname, name);
		++Errors;
		return;
	}
unl:
	unlink(name);	/* unlink name/.. */
unl2:
	name[strlen(name)-1] = '\0';
	unlink(name);	/* unlink name/.  */
	name[strlen(name)-2] = '\0';
	if (unlink(name) < 0) {
		fprintf(stderr, "%s: %s not removed\n", cmdname, name);
		++Errors;
	}
}

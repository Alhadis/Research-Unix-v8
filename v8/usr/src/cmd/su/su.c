#include <stdio.h>
#include <pwd.h>
#include <time.h>
#include <signal.h>
#define SULOG "/dev/console"

struct	passwd *pwd,*getpwnam();
char	*crypt();
char	*getpass();
char	**environ;

main(argc,argv)
int	argc;
char	**argv;
{
	register char **p;
	char *nptr;
	char *password;
	int badsw = 0;
	int newgid, newuid;
	char *shell = "/bin/sh";


	if(argv[0][0] != '/'){
		fprintf(stderr, "su must be invoked with a full path\n");
		exit(1);
	}

	if(argc > 1)
		nptr = argv[1];
	else
		nptr = "root";
	if((pwd=getpwnam(nptr)) == NULL) {
		printf("Unknown id: %s\n",nptr);
		exit(1);
	}
	if(pwd->pw_passwd[0] == '\0' || getuid() == 0)
		goto ok;
	password = getpass("Password:");
	if(badsw || (strcmp(pwd->pw_passwd, crypt(password, pwd->pw_passwd)) != 0)) {
		log(SULOG,nptr,0);
		printf("Sorry\n");
		exit(2);
	}

ok:
	newgid = pwd->pw_gid;
	newuid = pwd->pw_uid;
	log(SULOG,nptr,1);
	endpwent();
	setgid(newgid);
	setuid(newuid);
	if (pwd->pw_shell && *pwd->pw_shell)
		shell = pwd->pw_shell;
	if (newuid == 0)
		for (p=environ; *p; p++) {
			if (strncmp("PS1=", *p, 4) == 0)
				*p = "PS1=# ";
			else if (strncmp("PATH=", *p, 5) == 0)
				*p = "PATH=/bin:/usr/bin:/etc";
		}
	execl(shell, "su", "-p", 0);
	printf("No shell\n");
	exit(3);
}
log(where, towho, how)
char *where, *towho;
int how;
{
	int catch();
	FILE *logf;
	long now, time();
	char *cuserid(), *strrchr();
	char *ttyn, *ttyname();
	struct tm *tmp, *localtime();

	if((ttyn=ttyname(0))==NULL)
		if((ttyn=ttyname(1))==NULL)
			if((ttyn=ttyname(2))==NULL)
				if((ttyn=ttyname(3))==NULL)
					ttyn="/dev/tty??";
	now = time((long *)0);
	tmp = localtime(&now);
	signal(SIGALRM, catch);
	alarm(5);
	if((logf=fopen(where,"a")) == NULL) return;
	fprintf(logf,"\r\nSU %.2d/%.2d %.2d:%.2d %c %s %s-%s\r\n",
		tmp->tm_mon+1,tmp->tm_mday,tmp->tm_hour,tmp->tm_min,
		how?'+':'-',(strrchr(ttyn,'/')+1),cuserid((char *)0),towho);
	fclose(logf);
	alarm(0);
}
char *cuserid(x)
char *x;
{
	struct passwd *getpwuid();

	return getpwuid(getuid())->pw_name;
}
catch(){}

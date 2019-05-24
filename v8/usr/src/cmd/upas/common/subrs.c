/*
 *	subroutines that weren't worth their own files
 */
#include <stdio.h>
#include <signal.h>
#include <sys/param.h>
#include "mail.h"
#include "string.h"

/* configuration */
#define FRWRD	"Forward to "
#define PIPE	"Pipe to "
#define WHOAMI "/etc/whoami"		/* file containing system name */
#define MAILROOT "/usr/spool/mail/"	/* root of mail system */
#define UPASROOT "/usr/lib/upas/"	/* root of upas system */

/* imported */
int rmail;		/* true if started as rmail (init if undefined) */
int onatty;		/* true if on a tty (init if undefined) */
extern unsigned int alarm();

/* return a pointer to last element of a pathname */
extern char *
basename(file)
char *file;
{
	char *base;
	char *strrchr();

	if ((base = strrchr(file, '/')) != 0)
		base++;		/* past '/' */
	else
		base = file;
	return (base);
}

/* get the next whitespace delimited token from a string */
extern char *
getarg(s, p)      /* copy p... into s, update p */
register char *s, *p;
{
	while (*p == ' ' || *p == '\t')
		p++;
	if (*p == '\n' || *p == '\0')
		return(NULL);
	while (*p != ' ' && *p != '\t' && *p != '\n' && *p != '\0')
		*s++ = *p++;
	*s = '\0';
	return(p);
}

/* a path to a mail file */
extern char *
mailpath(relp)
	char *relp;		/* relative path name */
{
	static char pathname[128];

	if (*relp == '/')
		*pathname = '\0';
	else
		(void)strcpy(pathname, MAILROOT);
	(void)strcat(pathname, relp);
	return pathname;
}

/* a path to a special upas file */
extern char *
upaspath(relp)
	char *relp;		/* relative path name */
{
	static char pathname[128];

	if (*relp == '/')
		*pathname = '\0';
	else
		(void)strcpy(pathname, UPASROOT);
	(void)strcat(pathname, relp);
	return pathname;
}

/* get the system's name */
extern char *
getsysname()
{
	int f;
	char *strchr();
	static char wsysname[33];

	f = open(WHOAMI, 0);
	if (f>=0) {
		wsysname[0] = '\0';
		read(f, wsysname, 32);
		if (strchr(wsysname, '\n')) {
			*strchr(wsysname, '\n') = '\0';
		}
		close(f);
	}
	return wsysname;
}

/*
 *	Return:
 *		0 if no special action is requested
 *		1 if fowarding (new address in param)
 *		2 if mail is to be piped to a comand (command in param)
 */
extern int
delivery_status(s, param)
	char *s, *param;
{
	FILE *fp;
	char fbuf[CMDSIZE], *p, *sp;
	int rv = MF_NORMAL;
	extern char *stringin();


	fp = fopen(s, "r");
	if(fp == NULL)
		return MF_NORMAL;
	fbuf[0] = '\0';
	fgets(fbuf, sizeof(fbuf), fp);
	if (p = stringin(FRWRD, fbuf))
		rv = MF_FORWARD;
	else if (p = stringin(PIPE, fbuf))
		rv = MF_PIPE;
	if (rv != 0) {
		while (*p == ' ')
			p++;
		sp = param;
		while (*p && *p!='\n')
			*sp++ = *p++;
		*sp = '\0';
		if (fgets(fbuf, sizeof(fbuf), fp) != NULL)
			rv |= MFEXTRA;
	}
	fclose(fp);
	return (*param != 0) ? rv : MF_NORMAL;
}

/*
 *	Find one string within another.
 *	Return:	 NULL if line doesn't contain str
 *		 a pointer to the first byte following str in line otherwise
 */
extern char *
stringin(str, line)
char *str, *line;
{
	register char *sp, *lp;

	while (*line) {
		sp = str;
		lp = line++;
		for (;;) {
			if (*sp != *lp) {
				if (*sp=='\0')
					return(lp);
				break;
			}
			sp++;
			lp++;
		}
	}
	return(NULL);
}

/* print error and core dump */
extern int
panic(s)
	char *s;
{
	fprintf(stderr, "mail: %s\n", s);
	signal(SIGQUIT, SIG_DFL);
	kill(getpid(), SIGQUIT);
	exit(-1);
}

/* global to from parsing */
static char rmtlist[ADDRSIZE];

/* output a unix header with remote from blurb */
extern void
putrfunix(sender, date, sys, fp)
	char *date, *sys, *sender;
	FILE *fp;
{
	fprintf(fp, "From %s %s%s%s\n", sender, date, REMFROM, sys);
}

/* output a unix header */
extern void
putunix(sender, date, fp)
	char *sender, *date;
	FILE *fp;
{
	fprintf(fp, "From %s %s\n", sender, date);
}

/* start the unix parse */
extern void
initgetunix()
{
	rmtlist[0] = '\0';
}

/* get/parse unix from line */
extern int
getunix(line, sender, date)
	char *line;	/* line to parse */
	char *sender;	/* filled by parseline */
	char *date;	/* filled by parseline */
{
	char *rp, *dp, *sp;

	/* look for a from */
	if ((sp = stringin(FROM, line))!=NULL
	    || (sp = stringin(ALTFROM, line))!=NULL) {

		/* do we have a remote from? */
		if ((rp = stringin(REMFROM, line))!=NULL) {

			/* add to remote list */
			line[strlen(line)-1] = '\0';
			*(rp - (sizeof(REMFROM)-1)) = '\0';
			if ((dp = strchr(sp, ' ')) != NULL) {
				strcpy(date, dp+1);
				*dp = '\0';
			}
			(void)strcpy(sender, sp);
			(void)strcat(rmtlist, rp);
			(void)strcat(rmtlist, "!");
			return -1;
		}
	}

	/* Not a from line */
	/* Use what we've found so far */
	(void)strcat(rmtlist, sender);
	(void)strcpy(sender, rmtlist);
	return 0;
}

/*
 *	a paranoid's version of popen 
 */
#define	RDR	0
#define	WTR	1

static int ppid;
static int perrfd;
static char *envp[] = {
	"IFS= 	\n",
	"PATH=/bin:/usr/bin:/usr/lib/upas",
	0
};

static char perrbuf[CMDSIZE];	/* where to put standard error */
static int forked;		/* true if we've already forked */

/* here if popen times out */
static
ptimeout()
{
	fprintf(stderr, "time out - possbile mail problem\n");
	exit(1);
}

extern FILE *
popen(cmd, uid, gid)
char	*cmd;
int	uid, gid;
{
	register i;
	int p[2], perr[2];

	/* do mail in background to keep user from getting bored */
	if (onatty && !rmail && !forked) {
		switch(fork()) {
		case 0:
			signal(SIGHUP, SIG_IGN);
			signal(SIGINT, SIG_IGN);
			signal(SIGQUIT, SIG_IGN);
			signal(SIGTERM, SIG_IGN);
			forked = 1;
			break;
		case -1:
			break;
		default:
			exit(0);
		}
	}

	if(pipe(p) < 0)
		return NULL;
	if(pipe(perr) < 0) {
		close(p[RDR]);
		close(p[WTR]);
		return NULL;
	}
	if((ppid = fork()) == 0) {
		(void)setgid(gid);
		(void)setuid(uid);
		(void)dup2(p[RDR], 0);
		(void)dup2(perr[WTR], 2);
		(void)close(1);
		for (i=3; i<NOFILE; i++)
			close(i);
		execle("/bin/sh", "sh", "-c", cmd, 0, envp);
		_exit(1);
	}
	if(ppid == -1)
		return NULL;

	signal(SIGALRM, ptimeout);
	signal(SIGPIPE, SIG_IGN);
	alarm(1200);	/* in case of deadly embrace twixt stdin and stderr */

	close(perr[WTR]);
	perrfd = perr[RDR];
	close(p[RDR]);
	return(fdopen(p[WTR], "w"));
}

extern int
pclose(ptr, perrbuf, perrlen)
FILE *ptr;
char *perrbuf;
unsigned int perrlen;
{
	register r, (*hstat)(), (*istat)(), (*qstat)();
	int status, len, i;

	fclose(ptr);
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	hstat = signal(SIGHUP, SIG_IGN);
	if (perrlen > 0) {
		for (i = 0; i < perrlen; i += len) {
			len = read(perrfd, perrbuf+i, perrlen-i);
			if (len <= 0)
				break;
		}
		perrbuf[i] = '\0';
	}
	close(perrfd);
	alarm(0);	/* see alarm in popen */
	signal(SIGALRM, SIG_IGN);
	while((r = wait(&status)) != ppid && r != -1)
		;
	if(r == -1)
		status = 0;
	signal(SIGINT, istat);
	signal(SIGQUIT, qstat);
	signal(SIGHUP, hstat);
	return(status);
}

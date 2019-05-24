#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <pwd.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/timeb.h>
#include "letter.h"
#include "mail.h"
#include "string.h"


/* configuration */
#define DEAD "dead.letter"
#define MAILMODE (~0644)		/* mode of created mail */
#define MAXFORWARD 32
#define MAXHOP 5

/* imports */
extern char *basename();
extern int (*signal())();
extern char *getsysname();
extern char *stringin();
extern char *getarg();
extern char *getenv();
extern char *mailpath();
extern char *upaspath();
extern void lock();
extern void unlock();
extern int rewrite();
extern int getrules();
extern long getunix();
extern void putunix();
extern void putrfunix();
extern FILE *popen();
extern char *getlogin();
extern char *asctime();
extern struct tm *localtime();
extern char *timezone();

/* exported */
char *thissys;		/* name of this system */
char *network = "uucp";	/* name of network mail is coming from */
int onatty;
int rmail;		/* true if not executed as 'mail' */

/* global to this module */
#define LETTERNAME 0
#define DEVTTY NOFILE-1
static int error;
static char *myname;		/* name of this program */

/* predeclared */
static void saveletter();
static int saveonint(), done();
static int send();
static int append_to_file();
static int pipe_to_cmd();
static long getfrom();

extern void
sendmail(ac, av)
	int ac;
	char *av[];
{
	int chkfl;		/* check forward list */
	int ind=1;
	int (*rc)();
	char sender[ADDRSIZE];
	char date[DATESIZE];
	long bodyoffset;
	letter *lp;

	/* avoid hanging dk lines (HACK!!!!!!!!) */
	close(DEVTTY);

	myname = basename(av[0]);
	rmail = chkfl = strcmp(myname, "mail") != 0;
	onatty = isatty(0);
	thissys = getsysname();

	/* make temporary */
	rc = signal(SIGINT, done);
	inittmp();

	/* get network (default uucp) */
	if (av[1][0] == '-' && av[1][1] == 'N') {
		if (av[1][2] == '\0') {
			network = av[2];
			ind++;
		} else {
			network = &(av[1][2]);
		}
		ind++;
	}

	/* read letter */
	(void)signal(SIGINT, saveonint);
	lp = lopen(LETTERNAME, "w");
	copyto(stdin, lp, onatty, 1);
	if (ltell(lp) == 0L)
		done();
		
	/* send */
	if (!error) {
		(void)getrules();
		lp = lopen(LETTERNAME, "r");
		bodyoffset = getfrom(lp, sender, date);
		(void)signal(SIGINT, rc);
		for (; ind<ac; ind++) {
			letseek(lp, bodyoffset);
			error += send(lp, sender, date, av[ind], chkfl, 0);
		}
	}
	if (error && onatty)
		saveletter(lp);
	(void)done();
}

/* come here on interrupt while reading in letter */
static int
saveonint() {
	(void)signal(SIGINT, saveonint);
	error++;
}

/* come here to finish up */
static int
done() {
	unlock();
	releasetmp();
	exit(error);
}

/* save the letter in dead.letter */
static void
saveletter(lp)
	letter *lp;
{
	FILE *malf;
	char where[PATHSIZE];
	char *home = getenv("HOME");

	(void)setgid(getgid());
	(void)setuid(getuid());
	if (home != NULL) {
		strcpy(where, home);
		strcat(where, "/");
	} else
		*where = '\0';
	strcat(where, DEAD);
	malf = fopen(where, "w");
	if (malf == NULL) {
		fprintf(stderr, "mail: cannot open %s\n", where);
		return;
	}
	letseek(lp, (long)0);
	copyfrom(lp, malf);
	fclose(malf);
	printf("Mail saved in %s\n", where);
}

/*
 *	Send a letter.  Return 0 if all went well.
 */
static int
send(lp, sender, date, rcvr, chkfl, hop)
	letter *lp;
	char *sender, *date, *rcvr;
	int chkfl;
	int hop;
{
	int rv, notinfl;
	char dest[ADDRSIZE], rest[ADDRSIZE], cmd[CMDSIZE], format[32], sysname[64];

	if ((notinfl = rewrite(rcvr, sender, chkfl, cmd)) < 0) {
		fprintf(stderr, "mail: can't understand address %s\n", dest);
		return 1;
	}
	if (cmd[0]=='\0')
		rv = append_to_file(lp, sender, date, rcvr, chkfl, hop);
	else
		rv = pipe_to_cmd(lp, sender, date, rcvr, cmd,
			chkfl && notinfl);
	return rv;
}

/*
 *	Put mail into a mail file in the standard place.
 */
static int
append_to_file(lp, sender, date, rcvr, chkfl, hop)
	letter *lp;
	char *sender, *date, *rcvr;
	int hop;
{
	register mask;
	struct passwd *pw, *getpwnam();
	struct stat stbuf;
	char file[PATHSIZE];
	char sendto[CMDSIZE];
	int exists;
	FILE *malf;
	char *p;
	int i;

	if (strcmp(rcvr, "-") == 0)
		return(1);
	if (strchr(rcvr, '/') != NULL) {
		fprintf(stderr, "mail: illegal name %s\n", rcvr);
		return(0);
	}
	strcpy(file, mailpath(rcvr));
	switch (delivery_status(file, sendto) & MFTYPE) {
	case MF_NORMAL:
		/* normal message */
		break;
	case MF_FORWARD:
		/* look for possible forwarding loop */
		for (i = 0, p = sender-1; p != NULL; p = strchr(p+1, '!'))
			if (strncmp(thissys, p+1, strlen(thissys))==0)
				i++;
		if (i > MAXHOP) {
			logsend(rcvr, sender, date, "remote-mail-loop-err");
			fprintf(stderr, "mail: remote mail loop\n");
			break;
		}

		/* forward message */
		return forward_to(lp, rcvr, sender, date, sendto, hop);
	case MF_PIPE:
		/* pipe to a command */
		(void)stat(file, &stbuf);
		return pipe_to(lp, rcvr, sender, date, sendto, stbuf.st_uid,
			       stbuf.st_gid);
	}
	exists = stat(file, &stbuf);
	if ((pw = getpwnam(rcvr)) == NULL) {
		if (exists < 0) {
			if (chkfl) {
				refuse(lp,sender, date, rcvr, "addressee unknown.");
				return(0);
			}
			fprintf(stderr, "mail: can't send to %s\n", rcvr);
			return 1;
		}
	} else {
		stbuf.st_uid = pw->pw_uid;
		stbuf.st_gid = pw->pw_gid;
	}
	mask = umask(MAILMODE);
	lock(file);
	malf = fopen(file, "a");
	umask(mask);
	if (malf == NULL) {
		fprintf(stderr, "mail: cannot append to %s\n", file);
		logsend(rcvr, sender, date, "append-err");
		return 1;
	}
	if (exists < 0)
		chown(file, stbuf.st_uid, stbuf.st_gid);
	fprintf(malf, "%s%s %s\n", FROM, sender, date);
	copyfrom(lp, malf);
	fputs("\n", malf);
	fclose(malf);
	unlock();
	logsend(rcvr, sender, date, "delivered");
	notify(rcvr, sender);	/* let him know it right away! */
	return 0;
}

/*
 *	Pipe the mail into a command.  This is either for remote
 *	mail or specially handled mail.  Remote mail is distinguished
 *	by a non-null string as the destination.
 */
static int
pipe_to_cmd(lp, sender, date, rcvr, cmd, chkfl)
	letter *lp;
	char *sender, *date, *rcvr, *cmd;
	int chkfl;
{
	FILE *fp;
	char errbuf[CMDSIZE];
	char reason[2*CMDSIZE];
	long pos, status;

	/* check forwarding (what anti-social crap!) */
	if (chkfl) {
		if (rewrite(sender, "", chkfl, (char *)NULL) != 0) {
			refuse(lp, sender, date, rcvr,
				"forwarding to this system disallowed.");
			return(0);
		}
	}

	/* do it */
	if ((fp=popen(cmd, getuid(), getgid())) == NULL) {
		fprintf(stderr, "mail: can't pipe to network mailer\n");
		exit(1);
	}
	pos = ltell(lp);
	putrfunix(sender, date, thissys, fp);
	copyfrom(lp, fp);
	status = pclose(fp, errbuf, CMDSIZE);
	if (status==0) {
		logsend(rcvr, sender, date, "remote");
		return 0;
	} else {
		logsend(rcvr, sender, date, "remote-err");
		logsend("", cmd, "", "remote-err");
		sprintf(reason, "\"%s\"\nwas unable to send your mail.  It returned the status 0x%x.\nStderr was:\n%s",
			cmd, status, errbuf);
		(void)letseek(lp, pos);
		refuse(lp, sender, date, rcvr, reason);
		return 0;
	}
}


/* return to sender, address unknown, no such number, no such phone... */
refuse(lp, sender, date, rcvr, reason)
	letter *lp;
	char *sender, *date, *rcvr, *reason;
{
	FILE *pf;
	char cbuf[CMDSIZE];

	sprintf(cbuf, "PATH=/bin:/usr/bin mail %s", sender);
	if ((pf = popen(cbuf, getuid(), getgid())) == NULL)
		return;
	fprintf(pf, "Mail to %s failed. %s\nMessage was:\n", rcvr, reason);
	fprintf(pf, "%s%s %s\n", FROM, sender, date);
	copyfrom(lp, pf);
	pclose(pf, 0, 0);
	logsend(rcvr, sender, date, "refused");
}

/* forward mail to a list of people */
forward_to(lp, rcvr, sender, date, list, hop)
	letter *lp;
	char *rcvr, *sender, *date, *list;
	int hop;
{
	char dest[ADDRSIZE];
	char *p;
	int rv=0;

	if (hop > MAXFORWARD) {
		fprintf(stderr, "mail: forwarding loop!!\n");
		logsend(rcvr, sender, date, "forward-loop-err");
		return 1;
	}
	for (p = list; (p = getarg(dest, p)) != NULL; ) {
		(void)letseek(lp, (long)0);
		rv += send(lp, sender, date, dest, 0, ++hop);
	}
	return rv;
}

#define SU 0	/* super-duper user uid */

/* change uid to the receiver and perform the command */
pipe_to(lp, rcvr, sender, date, cmd, uid, gid)
	letter *lp;
	char *rcvr, *sender, *date, *cmd;
{
	FILE *fp;
	int i;

	/* do it */
	if ((fp=popen(cmd, uid, gid)) == NULL) {
		fprintf(stderr, "mail: can't pipe to network mailer\n");
		return 1;
	}
	putunix(sender, date, fp);
	copyfrom(lp, fp);
	if ((i=pclose(fp, 0, 0))==0) {
		logsend(rcvr, sender, date, "pipe_to");
		return 0;
	} else {
		logsend(rcvr, sender, date, "pipe_to_err");
		logsend("", cmd, "", "pipe_to_err");
		fprintf(stderr, "mail: Pipe to \"%s\" returns error status %d\n",
			cmd, i);
		return 1;
	}
}

/* default sender */
static void
defaultsender(sender, date)
	char *sender;	/* filled by defaultsender */
	char *date;	/* filled by defaultsender */
{
	char *tp, *dp;
	struct timeb timbuf;
	struct tm *bp;
	long thetime;

	(void)strcpy(sender, getlogin());
	ftime(&timbuf);
	thetime = timbuf.time;
	bp = localtime(&thetime);
	dp = asctime(bp);
	tp = timezone(timbuf.timezone, bp->tm_isdst);
	sprintf(date, "%.16s %.3s %.4s", dp, tp, dp+20);
}

/* Get/parse a unix header. */
static long
getfrom(lp, sender, date)
	letter *lp;
	char *sender, *date;
{
	char line[FROMLINESIZE];
	long n=0;

	defaultsender(sender, date);
	initgetunix();
	while (lgets(line, sizeof(line), lp) != NULL) {
		if (getunix(line, sender, date) == 0)
			break;
		n = ltell(lp);
	}

	return n;
}

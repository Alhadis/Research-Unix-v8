/*
 *	print mail
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <ctype.h>
#include "mail.h"
#include "letter.h"
#include "string.h"

/* globals */
static int flgp;
static int manual;
static int forward;
static int nlet;
static int error;
static int otherfile;
static jmp_buf	sjbuf;
#define seq(i) (forward ? i-1 : nlet-i)
static int changed;		/* true if mail file has been changed */

/* exported */
char *thissys;

/* imported */
extern void lock();
extern void unlock();
extern char *getlogin();
extern char *getenv();
extern char *getarg();
extern FILE *popen();
extern char *mailpath();
extern long getunix();
extern char *getsysname();
extern char *P();
extern void V();

/* predefined */
static char *doargs();
static long copymt();
static void domanual();
static void doauto();
static int docommand();
static int copyback();
static int done();
static int special_mailfile();
static int doprint();
static void dohelp();
static void print_from();
static int reply();
static int remail();
static int squirrel_away();

main(argc, argv)
char **argv;
{
	struct stat stbuf;
	char *mp, *tp;
	long mailsize;
	FILE *malf;

	mp = doargs(argc, argv);
	if (stat(mp, &stbuf) < 0 || stbuf.st_size == 0L) {
		printf("No mail.\n");
		return 0;
	}
	thissys = getsysname();
	(void)signal(SIGINT, done);
	(void)signal(SIGHUP, done);
	lock(mp);
	tp = otherfile ? NULL : P();
	if (tp != NULL)
		fprintf(stderr, "You are already reading mail on %s!\n\n", tp);
	malf = fopen(mp, "r");
	if (malf == NULL) {
		fprintf(stderr, "mail: cannot read %s.\n", mp);
		error++;
		(void)done();
	}

	/* see if we're piping or forwarding */
	if(special_mailfile(mp))
		(void)done();

	/* make temporary */
	inittmp();

	/* read mbox */
	mailsize = copymt(malf);
	fclose(malf);
	unlock();

	/* print the mail */
	if (flgp)
		printall();
	else {
		if (manual)
			domanual();
		else
			doauto();
	}
	if (changed)
		copyback(mailsize, mp);
	(void)done();

	return 0;
}

/*	Output all messages */
printall()
{
	int i, current;

	for (i = 0; i < nlet; i++) {
		current = forward ? i : nlet - i - 1;
		doprint(current);
	}
}

/* remember an interrupt happened */
rememberint()
{
	signal(SIGINT, rememberint);
	clearerr(stdin);
	clearerr(stdout);
	longjmp(sjbuf, 1);
}

#define eatwhite(p) while(*p==' '||*p=='\t') p++

/*
 *	Return a letter number
 */
static int
lnumber(pp, dot, def)
	char **pp;
{
	eatwhite(*pp);
	switch(**pp) {
	case '$':
		(*pp)++;
		dot = nlet;
		break;
	case 'n':
	case '+':
		(*pp)++;
		dot = dot + 1;
		break;
	case '^':
	case '-':
		(*pp)++;
		dot = dot - 1;
		break;
	case '.':
		(*pp)++;
		break;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		dot = 0;
		while (isdigit(**pp))
			dot = dot*10 + *(*pp)++ - '0';
		break;		/* first letter is 1 */
	default:
		dot = def;
		break;
	}
	if (dot <= 0)
		dot = 1;
	return dot;
}

/*
 *	Read and parse a command line.  A command consists of an optional
 *	range, a single letter command, and optional arguments.
 */
static char *
parse(cmdp, firstp, lastp, dot)
	char *cmdp;		/* the 1 letter command */
	int *firstp;		/* first command in a range */
	int *lastp;		/* last command in a range */
	int dot;		/* current letter */
{
	static char resp[CMDSIZE];
	char *p = resp;

	/* get command and dump trailing newline */
	printf("? ");
	fflush(stdout);
	if (fgets(resp, CMDSIZE, stdin) == NULL)
		return NULL;
	p[strlen(p) - 1] = '\0';

	/* get range */
	eatwhite(p);
	if (*p != '\0') {
		if (*p == ',')
			*firstp = 1;
		else
			*firstp = lnumber(&p, dot, dot);
		eatwhite(p);
		if (*p != ',')
			*lastp = *firstp;
		else {
			p++;
			*lastp = lnumber(&p, dot, nlet);
		}
	} else
		*firstp = *lastp = dot+1;
	
	/* get command */
	eatwhite(p);
	if (*p != '\0')
		*cmdp = *p++;
	else
		*cmdp = 'p';

	/* compound command */
	if (*cmdp == 'd' && (*p == 'q' || *p == 'p'))
		*(cmdp+1) = *p++;
	else
		*(cmdp+1) = '\0';
	eatwhite(p);
	return p;
}

/*	Process user commands in the new style.
 *	All actions are prompted for.
 */
static void
domanual()
{
	int dot=1, print, i;
	int first, last;
	char cmd[2], *args;

	printf("%d letters\n", nlet);
	while (1) {
		/* in case of interrupt, come back here but don't print */
		if (setjmp(sjbuf))	/* after long jump */
			printf("\n");
		signal(SIGINT, rememberint);
		args = parse(cmd, &first, &last, dot);
		if (args == NULL)
			break;
		for (i = first; i <= last && i <= nlet; i++) {
			dot = i;	/* must be here in case of interrupt */
			if (docommand(cmd[0], args, i, &print) < 0)
				return;
		}
		switch (cmd[1]) {
		case 'q':
			return;
		case 'p':
			doprint(seq(++dot));
			break;
		}
	}
}

/*	Process user commands in the old style.
 *	This implies printing the next message 
 *	without prompting for it
 */
static void
doauto()
{
	int print=1, dot=1, i;
	int first, last;
	char cmd[2], *args;

	while (dot <= nlet && dot >= 0) {
		/* in case of interrupt, come back here but don't print */
		if (setjmp(sjbuf))	/* after long jump */
			printf("\n");
		else {		/* here normally */
			signal(SIGINT, rememberint);
			if (print)
				doprint(seq(dot));
		}
		print = 0;
		args = parse(cmd, &first, &last, dot);
		if (args == NULL)
			break;
		for (i = first; i <= last && i <= nlet; i++) {
			dot = i;	/* in case of interrupt while printing */
			dot = docommand(cmd[0], args, i, &print);
		}
		if (dot > 0 && dot < last)
			dot = last;
		switch (cmd[1]) {
		case 'q':
			return;
		case 'p':
			print++;
			break;
		}
	}
}

/*
 *	Perform a command.
 */
static int
docommand(cmd, args, dot, pflag)
	char cmd;		/* command to perform */
	char *args;		/* arguments to the command */
	int dot;		/* the message to apply it to */
	int *pflag;		/* (returned) true if next message to be printed */
{
	switch (cmd) {
	default:
		printf("usage\n");
	case '?':
		dohelp();
		break;
	case 'h':
		print_from(seq(dot));
		break;
	case '=':
		printf("at letter %d\n", dot);	
		break;
	case 'p':
		doprint(seq(dot));
		break;
	case 'x':
		changed = 0;
		dot = -1;
		break;
	case 'y':
	case 'w':
	case 's':
		if (squirrel_away(seq(dot), args) == 0) {
			(void)ldelete(seq(dot));
			changed++;
			(*pflag)++;
			dot++;
		}
		break;
	case 'm':
		if (remail(seq(dot), args) == 0) {
			(void)ldelete(seq(dot));
			changed++;
			(*pflag)++;
			dot++;
		}
		break;
	case 'r':
		if (*args != '\0') {
			fprintf(stderr, "invalid: r takes no arguments\n");
			break;
		}
		(void)reply(seq(dot));
		printf("!\n");
		break;
	case '!':
		system(args);
		printf("!\n");
		break;
	case 'd':
		if (*args != '\0') {
			fprintf(stderr,"invalid: d takes no arguments\n");
			break;
		}
		(void)ldelete(seq(dot));
		changed++;
		(*pflag)++;
		dot++;
		break;
	case 'u':
		if (*args != '\0') {
			fprintf(stderr,"invalid: u takes no arguments\n");
			break;
		}
		(void)lundelete(seq(dot));
		(*pflag)++;
		dot++;
		break;
	case 'q':
		dot = -1;
		break;
	}
	return dot;
}

/* copy mail to temp file, returns size of mail file */
static long
copymt(mfp)
	FILE *mfp;
{
	letter *lp=NULL;
	char line[FROMLINESIZE];

	/* read in letters */
	while (fgets(line, FROMLINESIZE, mfp) != NULL) {
		if (strncmp(line, FROM, FSIZE)==0 || lp==NULL) {
			lp = lopen(nlet++, "w");
		}
		lputs(line, lp);
	}

	/* remember where end of file is */
	return ftell(mfp);
}

/* copy temp or whatever back to /usr/spool/mail */
static int
copyback(oldsize, mp)
	long oldsize;
	char *mp;
{
	register i;
	register int new = 0;
	struct stat stbuf;
	letter *lp;
	FILE *malf;

	lock(mp);

	/* read new mail that arrived while we were printing */
	stat(mp, &stbuf);
	if (stbuf.st_size != oldsize) {   /* new mail has arrived */
		malf = fopen(mp, "r");
		if (malf == NULL) {
			fprintf(stderr, "mail: can't re-read %s\n", mp);
			(void)done();
		}
		fseek(malf, oldsize, 0);
		(void)copymt(malf);
		fclose(malf);
		new++;
	}

	/* turn off interrupts while writing back */
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	/* write the sucker back */
	malf = fopen(mp, "w");
	if (malf == NULL) {
		fprintf(stderr, "mail: can't rewrite %s\n", mp);
		(void)done();
	}
	for (i = 0; i < nlet; i++)
		if ((lp=lopen(i, "ru")) != NULL) {
			copyfrom(lp, malf);
		}
	fclose(malf);
	unlock();
	if (new)
		printf("new mail arrived\n");
}

/* come here to finish up */
static int
done()
{
	unlock();
	V();
	releasetmp();
	exit(error);
	return 0;
}

static char *
doargs(argc, argv)
	int argc;
	char *argv[];
{
	static char mailfile[256];

	(void)strcpy(mailfile, mailpath(getlogin()));

	for (; argc>1; argv++, argc--) {
		if (argv[1][0]=='-') {
			if (argv[1][1]=='p') {
				flgp++;
			} else if (argv[1][1]=='f') {
				if (argc>=3) {
					strcpy(mailfile, argv[2]);
					argv++;
					argc--;
					otherfile = 1;
				}
			} else if (argv[1][1]=='r') {
				forward = 1;
			} else if (argv[1][1]=='m') {
				manual = 1;
			} else {
				fprintf(stderr, "mail: unknown option %c\n", argv[1][1]);
				done();
			}
		} else
			break;
	}
	return mailfile;
}

/* return non-zero if this is not a normal mail file */
static int
special_mailfile(mp)
	char *mp;		/* mail file */
{
	char fbuf[256];
	char *sp1, *sp2;
	int stat;

	sp2 = "";

	/* if something is being done to the mail, go away */
	stat = delivery_status(mp, fbuf);
	switch(stat & MFTYPE) {
	case MF_FORWARD:
		sp1 = "Your mail is being forwarded to";
		break;
	case MF_PIPE:
		sp1 = "Your mail is being piped to";
		break;
	default:
		return 0;
	}

	if (stat & MFEXTRA)
		sp2 = "and your mailbox contains extra stuff\n";
	printf("%s %s %s\n", sp1, fbuf, sp2);
	return stat & MFTYPE;
}

static void
dohelp()
{
	printf("Commands are of the form '[range] command [args]'.\n");
	printf("The command can be:\n");
	printf("d\tdelete\n");
	printf("u\tundelete\n");
	printf("h\tprint from lines of all messages\n");
	printf("m user\tmail to user\n");
	printf("p\tprint\n");
	printf("r\treply to last message\n");
	printf("q\tquit\n");
	printf("s[file]\tsave (default mbox)\n");
	printf("x\texit without changing mail\n");
	printf("=\tprint current message number\n");
	printf("! cmd\texecute cmd\n");
}

/* print the from line (and status) of each message */
static void
print_from(let)
	int let;
{
	int deleted;
	char from[FROMLINESIZE];
	letter *lp;

	lp = lopen(let, "ru");
	if (lp == NULL) {
		deleted = 1;
		lp = lopen(let, "r");
	} else
		deleted = 0;
	lgets(from, FROMLINESIZE, lp);
	printf("%3d %c %4d  %s", seq(let), deleted?'d':' ',
		lsize(lp), from+FSIZE);
}

/* save a letter in a file, return 0 if successful */
static int
squirrel_away(let, resp)
	int let;
	char *resp;
{
	char *p;
	letter *lp;
	char path[PATHSIZE];
	int problems=0;
	FILE *malf;

	if (*resp == '\0') {
		p = getenv("HOME");
		if(p != 0) {
			strcpy(resp, p);
			strcat(resp, "/mbox");
		} else
			strcpy(resp, "mbox");
	}
	for (p = resp; (p = getarg(path, p)) != NULL; ) {
		malf = fopen(path, "a");
		if (malf == NULL) {
			fprintf(stderr, "mail: cannot append to %s\n", path);
			problems++;
			continue;
		}
		lp = lopen(let, "r");
		copyfrom(lp, malf);
		fclose(malf);
	}
	return problems;
}

/* reply to mail, return 0 if no problems */
static char replyline[FROMLINESIZE];
static char replycmd[CMDSIZE];
static int
reply(let)
	int let;
{
	char *sp;
	letter *lp;

	lp = lopen(let, "r");
	lgets(replyline, FROMLINESIZE, lp);
	sp = strchr(replyline+FSIZE, ' ');
	if (sp != NULL)
		*sp = '\0';
	strcpy(replycmd, "/bin/mail ");
	strcat(replycmd, replyline+FSIZE);
	printf("!%s\n", replycmd);
	system(replycmd);
	return 0;
}

/* pass mail onto someone else, return 0 if no problems */
static int
remail(let, resp)
	int let;
	char *resp;
{
	char cmd[CMDSIZE];
	FILE *pipf;
	letter *lp;

	if (*resp == '\0') {
		fprintf(stderr, "invalid: m requires a destination\n");
		return -1;
	}
	strcpy(cmd, "mail ");
	strcat(cmd, resp);

	/* fork so that mail becomes super-user again */
	pipf = popen(cmd, "w");
	if(pipf == NULL) {
		fprintf(stderr, "mail: can't fork to remail\n");
		return -1;
	}
	lp = lopen(let, "r");
	copyfrom(lp, pipf);
	return pclose(pipf);
}

/* output the letter */
static int
doprint(let)
	int let;
{
	letter *lp;

	lp = lopen(let, "r");
	if (lp == NULL)
		return -1;
	copyfrom(lp, stdout);
	return 0;
}

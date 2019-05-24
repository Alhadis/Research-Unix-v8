#include <stdio.h>
#include <pwd.h>
#include <utmp.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/timeb.h>
#include <setjmp.h>
#include <ctype.h>
#include <dir.h>


#define SMTP "/usr/lib/uucp/smtp"

/*copylet flags */
	/*remote mail, add rmtmsg */
#define REMOTE  1
#define INTERNET  4
	/* zap header and trailing empty line */
#define ZAP	3
#define ORDINARY 2
#define SENDING 5
#define LSIZE   256
#define MAXLET  1000     /* maximum number of letters */
#define MAILMODE (~0644)		/* mode of created mail */
#define FLIST "/etc/forwardlist"
#define TMPNAME "/tmp/maXXXXX"

/* definitions for inet gateways */
struct igateway {
	char	*id;		/* network identifier */
	char	*addr;		/* address translation */
	char	*via;		/* machine that talks to gateway */
	char	*cmd;		/* command for gateway */
};
struct igateway igates[] = {
	"arpa_or_csnet", ".csnet-relay", "research", "/usr/mmdf/lib/mail",
	"csnet", ".csnet.csnet-relay", "research", "/usr/mmdf/lib/mail",
	"arpa", ".arpa.csnet-relay", "research", "/usr/mmdf/lib/mail",
	"acsnet", "", "research", "/usr/ACSnet/bin/inject",
};
#define NIGATES 4
unsigned int igate;		/* offset into igates (csnet by default) */


char    line[LSIZE];
char	resp[LSIZE];
struct let {
	long    adr;
	char    change;
} let[MAXLET];
int	nlet    = 0;
char	lfil[50];
char	*stringin();
FILE	*popen();
char	*getenv();
char    tmpname[] = TMPNAME;
char	lettmp[sizeof(TMPNAME)];
char	maildir[] = "/usr/spool/mail/";
char	mailfile[] = "/usr/spool/mail/xxxxxxxxxxxxxxxxxxxxxxx";
char	dead[] = "dead.letter";
char	whoami[] = "/etc/whoami";
#define FRWRD	"Forward to "
char	*thissys = "";
FILE	*tmpf;
FILE	*malf;
char	*my_name;
char	real_name[512];
char	*getlogin();
struct	passwd  *getpwuid();
char	*strchr();
int	error;
int	changed;
int	forward;
int	onatty;
char	from[] = "From ";
long	ftell();
int	delete();
int	savdead();
int	(*saveint)();
int	(*setsig())();
char	*strcpy();
char	*strcat();
char	*mktemp();
char	*ctime();
long	time();
int	flgf;
int	flgp;
int	delflg = 1;
int	rflg;
int	frominet;
int	normtf;
jmp_buf	sjbuf;
char	dev[] = "/dev/";

void lock(), unlock();

main(argc, argv)
char **argv;
{
	static char sobuf[BUFSIZ];
	char wsysname[32];
	int f;
	char *prog;
	char *basename();

	if (argc == 2 && (argv[1][0] == '-' && argv[1][1]=='n')) {
		struct stat s;
		char *ttyname();
		char *p;

		fstat(2,&s);
		setgid(getgid());
		setuid(getuid());
		if ((p = ttyname(2)) && *p)
			chmod(p, s.st_mode ^ S_IEXEC);
		exit(0);
	}
	prog = basename(argv[0]);
	normtf = prog[0]=='r';
	frominet = prog[0]=='c';
	rflg =  prog[0]=='r' || frominet;
	setbuf(stdout, sobuf);
	strcpy(lettmp, tmpname);
	mktemp(lettmp);
	unlink(lettmp);
	my_name = getlogin();
	if (my_name==NULL)
		my_name = "daemon";
	f = open(whoami, 0);
	if (f>=0) {
		wsysname[0] = '\0';
		read(f, wsysname, 32);
		if (strchr(wsysname, '\n')) {
			*strchr(wsysname, '\n') = '\0';
			thissys = wsysname;
		}
		close(f);
	}
	if(*thissys == 0)
		fprintf(stderr, "mail: bad data from %s\n", whoami);
	if(setjmp(sjbuf))
		done();
	setsig(SIGINT, delete);
	setsig(SIGHUP, delete);
	setsig(SIGKILL, delete);
	tmpf = fopen(lettmp, "w");
	if (tmpf == NULL) {
		fprintf(stderr, "mail: cannot open %s for writing\n", lettmp);
		done();
	}
	chmod(lettmp, 0600);
	chown(lettmp, getuid(), getgid());
	if( !rflg &&                    /* no favors for rmail*/
	   (argc == 1 || (argv[1][0] == '-' && argv[1][1]!='R')))
		printmail(argc, argv);
	else
		sendmail(argc, argv);
	done();
}

char *
basename(file)
char *file;
{
	char *base;
	char *strrchr();

	if ((base = strrchr(file, '/')) != NULL)
		base++;		/* past '/' */
	else
		base = file;
	return (base);
}

int (*setsig(i, f))()
int i;
int (*f)();
{
	register int (*rc)();

	if ((rc = signal(i, SIG_IGN))!=SIG_IGN)
		signal(i, f);
	return(rc);
}

sig_ign()
{
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
}

printmail(argc, argv)
char **argv;
{
	int flg, i, j, print;
	struct stat stbuf;
	char *p, *getarg();
	char fbuf[256];
	FILE *pipf;

	setgid(getgid());
	setuid(getuid());
	cat(mailfile, maildir, my_name);
	for (; argc>1; argv++, argc--) {
		if (argv[1][0]=='-') {
			if (argv[1][1]=='p') {
				flgp++;
				delflg = 0;
			} else if (argv[1][1]=='f') {
				if (argc>=3) {
					strcpy(mailfile, argv[2]);
					argv++;
					argc--;
				}
			} else if (argv[1][1]=='r') {
				forward = 1;
			} else {
				fprintf(stderr, "mail: unknown option %c\n", argv[1][1]);
				done();
			}
		} else
			break;
	}
	if (stat(mailfile, &stbuf) < 0 || stbuf.st_size == 0L) {
		printf("No mail.\n");
		return;
	}
	malf = fopen(mailfile, "r");
	if (malf == NULL) {
		fprintf(stderr, "mail: cannot read %s.\n", mailfile);
		return;
	}
	lock(mailfile);
	if(areforwarding(mailfile, fbuf)) {
		printf("Your mail is being forwarded to ");
		fseek(malf, (long)(sizeof(FRWRD) - 1), 0);
		fgets(fbuf, sizeof(fbuf), malf);
		printf("%s", fbuf);
		if(getc(malf) != EOF)
			printf("and your mailbox contains extra stuff\n");
		unlock();
		return;
	}
	copymt(malf, tmpf);
	fclose(malf);
	fclose(tmpf);
	unlock();
	tmpf = fopen(lettmp, "r");

	changed = 0;
	print = 1;
	for (i = 0; i < nlet; ) {
		j = forward ? i : nlet - i - 1;
		if(setjmp(sjbuf) == 0) {
			if (print)
				copylet(j, stdout, ORDINARY, (char *)NULL);
			print = 1;
		}
		if (flgp) {
			i++;
			continue;
		}
		setjmp(sjbuf);
		printf("? ");
		fflush(stdout);
		if (fgets(resp, LSIZE, stdin) == NULL)
			break;
		switch (resp[0]) {

		default:
			printf("usage\n");
		case '?':
			print = 0;
			printf("q\tquit\n");
			printf("x\texit without changing mail\n");
			printf("p\tprint\n");
			printf("s[file]\tsave (default mbox)\n");
			printf("-\tprint previous\n");
			printf("d\tdelete\n");
			printf("+\tnext (no delete)\n");
			printf("m user\tmail to user\n");
			printf("! cmd\texecute cmd\n");
			break;

		case '+':
		case 'n':
		case '\n':
			i++;
			break;
		case 'x':
			changed = 0;
		case 'q':
			goto donep;
		case 'p':
			break;
		case '^':
		case '-':
			if (--i < 0)
				i = 0;
			break;
		case 'y':
		case 'w':
		case 's':
			flg = 0;
			if (resp[1] != '\n' && resp[1] != ' ') {
				printf("invalid\n");
				flg++;
				print = 0;
				continue;
			}
			if (resp[1] == '\n' || resp[1] == '\0') {
				p = getenv("HOME");
				if(p != 0)
					cat(resp+1, p, "/mbox");
				else
					cat(resp+1, "", "mbox");
			}
			for (p = resp+1; (p = getarg(lfil, p)) != NULL; ) {
				malf = fopen(lfil, "a");
				if (malf == NULL) {
					fprintf(stderr, "mail: cannot append to %s\n", lfil);
					flg++;
					continue;
				}
				copylet(j, malf, resp[0]=='w'? ZAP: ORDINARY,
				    (char *)NULL);
				fclose(malf);
			}
			if (flg)
				print = 0;
			else {
				let[j].change = 'd';
				changed++;
				i++;
			}
			break;
		case 'm':
			flg = 0;
			if (resp[1] == '\n' || resp[1] == '\0') {
				i++;
				continue;
			}
			if (resp[1] != ' ') {
				printf("invalid\n");
				flg++;
				print = 0;
				continue;
			}
			strcpy(fbuf, "mail ");
			strcat(fbuf, resp+1);
			/* fork so that mail becomes super-user again */
			pipf = popen(fbuf, "w");
			if(pipf == NULL) {
				fprintf(stderr, "mail: can't fork to remail\n");
				flg++;
				continue;
			}
			copylet(j, pipf, SENDING, (char *)NULL);
			flg |= pclose(pipf);
			if (flg)
				print = 0;
			else {
				let[j].change = 'd';
				changed++;
				i++;
			}
			break;
		case '!':
			system(resp+1);
			printf("!\n");
			print = 0;
			break;
		case 'd':
			let[j].change = 'd';
			changed++;
			i++;
			if (resp[1] == 'q')
				goto donep;
			break;
		}
	}
   donep:
	if (changed)
		copyback();
}

copyback()      /* copy temp or whatever back to /usr/spool/mail */
{
	register i, c;
	register int new = 0;
	struct stat stbuf;

	sig_ign();
	lock(mailfile);
	stat(mailfile, &stbuf);
	if (stbuf.st_size != let[nlet].adr) {   /* new mail has arrived */
		malf = fopen(mailfile, "r");
		if (malf == NULL) {
			fprintf(stderr, "mail: can't re-read %s\n", mailfile);
			done();
		}
		fseek(malf, let[nlet].adr, 0);
		fclose(tmpf);
		tmpf = fopen(lettmp, "a");
		fseek(tmpf, let[nlet].adr, 0);
		while ((c = fgetc(malf)) != EOF)
			fputc(c, tmpf);
		fclose(malf);
		fclose(tmpf);
		tmpf = fopen(lettmp, "r");
		let[++nlet].adr = stbuf.st_size;
		new = 1;
	}
	malf = fopen(mailfile, "w");
	if (malf == NULL) {
		fprintf(stderr, "mail: can't rewrite %s\n", mailfile);
		done();
	}
	for (i = 0; i < nlet; i++)
		if (let[i].change != 'd') {
			copylet(i, malf, ORDINARY, (char *)NULL);
		}
	fclose(malf);
	if (new)
		printf("new mail arrived\n");
	unlock();
}

copymt(f1, f2)  /* copy mail (f1) to temp (f2) */
FILE *f1, *f2;
{
	long nextadr;

	nlet = nextadr = 0;
	let[0].adr = 0;
	while (fgets(line, LSIZE, f1) != NULL) {
		if (isfrom(line)) {
			let[nlet++].adr = nextadr;
			if (nlet > MAXLET) {
				fprintf (stderr, "mail: too many letters\n");
				unlock();
				exit(1);
			}
		}
		nextadr += strlen(line);
		fputs(line, f2);
	}
	let[nlet].adr = nextadr;        /* last plus 1 */
}

copylet(n, f, type, postmark)
FILE *f;
char *postmark;
{
	long k;
	register ch;
	char *inetheader();

	fseek(tmpf, let[n].adr, 0);
	k = let[n+1].adr - let[n].adr;
	if (postmark && type != ZAP) {
		if (type == INTERNET)
			postmark = inetheader (postmark);
		fputs(postmark, f);
		if (type==REMOTE)
			fprintf(f, " remote from %s", thissys);
		fputc('\n', f);
	}
	while (k>0) {
		ch = fgetc(tmpf);
		if (ch == EOF)
			break;
		fputc(ch, f);
		k--;
	}
}

areforwarding(s, sendto)
char *s, *sendto;
{
       FILE *fd;
	char fbuf[64], *p, *sp;

	fd = fopen(s, "r");
	if(fd == NULL)
		return(0);
	fbuf[0] = '\0';
	fgets(fbuf, sizeof(fbuf), fd);
	if (p = stringin(FRWRD, fbuf)) {
		while (*p == ' ')
			p++;
		sp = sendto;
		while (*p && *p!='\n')
			*sp++ = *p++;
		*sp = '\0';
		fclose(fd);
		return(*sendto != 0);
	}
	fclose(fd);
	return(0);
}

isfrom(lp)
register char *lp;
{
	return(strncmp(lp, from, 5)==0);
}

sendmail(argc, argv)
char **argv;
{
	char *asctime();
	struct timeb timbuf;
	struct tm *bp, *localtime();
	char *tp, *zp, postmark[512];
	char *timezone();
	int rrrflg=0;
	long newpost();

	while (argv[1][0]=='-') {
		switch(argv[1][1]) {
		case 'R': /* return receipt */
			rrrflg = 1;
		}
		argc--; argv++;
	}
	ftime(&timbuf);
	bp = localtime(&timbuf.time);
	tp = asctime(bp);
	zp = timezone(timbuf.timezone, bp->tm_isdst);
	onatty = isatty(0);
	sprintf(postmark, "%s%s %.16s %.3s %.4s%s",from, my_name, tp, zp, tp+20,
	    rrrflg?" (RRR)":"");
	flgf = 1;
	saveint = setsig(SIGINT, savdead);
	while (fgets(line, LSIZE, stdin) != NULL) {
		if (line[0] == '.' && line[1] == '\n' && onatty)
			break;
		if (isfrom(line))
			fputs(">", tmpf);
		fputs(line, tmpf);
		flgf = 0;
	}
	setsig(SIGINT, saveint);
	if (line[0]!='\n' && error==0)
		fputs("\n", tmpf);
	nlet = 1;
	let[0].adr = 0;
	let[1].adr = ftell(tmpf);
	fclose(tmpf);
	if (flgf)
		return;
	tmpf = fopen(lettmp, "r");
	if (tmpf == NULL) {
		fprintf(stderr, "mail: cannot reopen %s for reading\n", lettmp);
		return;
	}
	let[0].adr = newpost(postmark);
	if (error == 0)
		while (--argc > 0) {
			if (!send(0, *++argv, 0, postmark))
				error++;
		}
	if (error && onatty) {
		setgid(getgid());
		setuid(getuid());
		malf = fopen(dead, "w");
		if (malf == NULL) {
			fprintf(stderr, "mail: cannot open %s\n", dead);
			fclose(tmpf);
			return;
		}
		copylet(0, malf, ZAP, (char *)NULL);
		fclose(malf);
		printf("Mail saved in %s\n", dead);
	}
	fclose(tmpf);
}

long
newpost(pm)
char *pm;
{
	long inetpost(), unixpost();
	if (frominet)
		return inetpost(pm);
	else
		return unixpost(pm);
}

long
inetpost(pm)
char *pm;
{
	char line[512], date[64], *lp;

	/* get default source  and date */
	lp = strchr(pm+5, ' ');
	strcpy(date, lp);

	/* RFC 822 (or similar) */
	/* construct the "From " line from the "From: " line */
	while (fgets(line, sizeof(line), tmpf) != NULL) {
		line[strlen(line)-1] = '\0';

		/* pick up from field */
		if (strncmp("From: ", line, 6) == 0) {
			lp = strchr(line+6, ' ');
			if (lp)
				*lp = '\0'; 
			sprintf(pm, "From %s", line+6);
		}

		/* pick up date field */
		if (strncmp("Date: ", line, 6) == 0)
			strcpy(date, line+5);

		/* stop if we're out of header */
		if (strchr(line, ':') == NULL && line[0] != ' '
		    && line[0] != '\t')
			break;
	}
	strcpy(real_name, pm+5);
	strcat(pm, date);
	return(0);
}

long
unixpost(pm)
char *pm;
{
	long n = 0;
	char line[512], *cp, *lp;
	char rmtlist[512];

	/* UNIX mail */
	/* construct the "From " line from the existing ones */
	rmtlist[0] = '\0';
	while (fgets(line, sizeof(line), tmpf) != NULL) {

		/* stop if we run out of header */
		if (stringin("From ", line)==NULL
		    && stringin(">From ", line)==NULL)
			break;
		if ((cp = stringin(" remote from ", line))==NULL)
			break;

		/* get name of remote system and add to rmtlist */
		line[strlen(line)-1] = '\0';
		if (okrmt(cp) && n==0)
			normtf = 0;
		*(cp - sizeof(" remote from ") + 1) = '\0';
		strcpy(pm, line[0] == '>' ? line + 1 : line);
		strcat(rmtlist, cp);
		strcat(rmtlist, "!");
		n = ftell(tmpf);
	}

	/* tack sender's name and date onto the list of remote machines */
	strcpy(line, pm);
	sprintf(pm, "From %s%s", rmtlist, line+5);
	if ((cp = strchr(line+5, ' ')) != NULL)
		*cp = '\0';
	sprintf(real_name, "%s%s", rmtlist, line+5);
	return(n);
}

char *
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

/*
 *	Return nonzero if forwarding is allowed to the system
 *	named in the string at "cp".  If we can't read the file
 *	which contains the systems, assume blanket forwarding.
 */
okrmt(cp)
char *cp;
{
	register FILE *fp;
	char buf[20];

	/* try to open the file; allow forwarding on failure */
	fp = fopen (FLIST, "r");
	if (fp == NULL)
		return 1;

	/* one iteration per system name in the file */
	while (fgets (buf, sizeof buf, fp) != NULL) {
		buf[strlen(buf)-1] = '\0';
		if (strcmp (buf, cp) == 0) {
			/* found it, allow forwarding */
			fclose (fp);
			return 1;
		}
	}

	/* didn't find it, prohibit forwarding */
	fclose (fp);
	return 0;
}

savdead()
{
	setsig(SIGINT, saveint);
	error++;
}

sendrmt(n, name, postmark)
char *name, *postmark;
{
	FILE *rmf;
	register char *p, *q, *rmtid;
	char cmd[512], buf[512];
	static forked;

	strcpy(buf, name);
	rmtid = strchr(name, '!');
	if (rmtid == NULL || rmtid[1]=='\0') {
		fprintf(stderr, "mail: bad remote name\n");
		return(0);
	}
	*rmtid++ = '\0';
	if (normtf)
		for (p = q = &buf[0]; (p = strchr(q, '!')) && p[1] != '\0';
		    q = p + 1) {
			*p = '\0';
			if (!okrmt(q)) {
				*--rmtid = '!';
				refuse(n, name, postmark,
				   "forwarding to this system disallowed");
				return(1);
			}
		}
	if(onatty & !forked) {	/* fork off remote send to avoid user tedium */
		int p = fork();
		if(p == 0) {
			sig_ign();
			forked = 1;
		} else if(p != -1)
			exit(0);
	}
	if(access(SMTP, 0) != -1){
		sprintf(cmd, "exec %s %s %s", SMTP, name, rmtid);
		if((rmf = popen(cmd, "w")) != NULL){
			copylet(n, rmf, REMOTE, postmark);
			if(pclose(rmf) == 0) return(1);
		}
	}
	sprintf(cmd, "exec /usr/bin/uux - -a %s %s!rmail \\(%s\\)",
			real_name, name, rmtid);
	if ((rmf=popen(cmd, "w")) == NULL) {
		fprintf(stderr, "mail: can't pipe to uux\n");
		exit(1);
	}
	copylet(n, rmf, REMOTE, postmark);
	*--rmtid = '!';
	logsend(name, postmark, "remote");
	return(pclose(rmf)==0);
}



send(n, name, level, postmark)	/* send letter n to name */
int n;
char *name, *postmark;
{
	char file[512];
	register mask;
	struct passwd *pw, *getpwnam();
	struct stat stbuf;
	char sendto[256];
	char inetaddr[256];
	int exists;


	if(level > 20) {
		fprintf(stderr, "mail: unbounded forwarding\n");
		return(0);
	}
	if (strcmp(name, "-") == 0)
		return(1);
	if (strchr(name, '!'))
		return(sendrmt(n, name, postmark));
	if (strchr(name, '/')) {
		fprintf(stderr, "mail: illegal name %s\n", name);
		return(0);
	}
	cat(file, maildir, name);
	if(areforwarding(file, sendto)) {
		int rf, sval;
		rf = normtf;
		normtf = 0;	/* to permit transshipping */
		sval = send(n, sendto, level+1, postmark);
		normtf = rf;
		return(sval);
	}
	exists = stat(file, &stbuf);
	if ((pw = getpwnam(name)) == NULL) {
		if (exists < 0) {
			(void)strcpy (inetaddr, name);
			if (isinet(inetaddr)) {
				if (strcmp (thissys, igates[igate].via) == 0) {
					(void)inetgateaddr(inetaddr);
					return(sendinet(n,inetaddr,postmark));
				} else {
					(void)strcpy (sendto, igates[igate].via);
					(void)strcat (sendto, "!");
					(void)strcat (sendto, name);
					return(sendrmt(n, sendto, postmark));
				}
			}
			if (rflg) {
				refuse(n, name, postmark,
				      "addressee unknown");
				return(1);
			}
			fprintf(stderr, "mail: can't send to %s\n", name);
			return(0);
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
		return(0);
	}
	if (exists < 0)
		chown(file, stbuf.st_uid, stbuf.st_gid);
	copylet(n, malf, SENDING, postmark);
	fclose(malf);
	unlock();
	logsend(name, postmark, "delivered");
	notify(name);	/* let him know it right away! */
	return(1);
}

notify(name)
char *name;
{
	struct utmp entry;
	FILE *f;

	if ((f = fopen("/etc/utmp", "r")) == NULL)
		return;
	while (fread((char *)&entry, sizeof(entry), 1, f) == 1) {
		if (strncmp(name, entry.ut_name, sizeof(entry.ut_name)) == 0)
			blurb(entry.ut_line);
	}
	fclose(f);
}

blurb(tty)
char *tty;
{
	FILE *f = NULL;
	struct stat s;
	int blurbtime();
	char devtty[32];

	cat(devtty, dev, tty);
	signal(SIGALRM, blurbtime);
	alarm(30);
	stat(devtty, &s);
	if (s.st_mode & S_IEXEC)	/* notify only if enabled */
	if ((f = fopen(devtty, "w")) != NULL) {
		fprintf(f, "\r\n[%s: mail from %s]\r\n\7", thissys,
		  real_name[0]?real_name: my_name);
	}
	if (f)
		fclose(f);
	alarm(0);
}

blurbtime()
{
}

refuse(n, name, postmark, reason)
char *name, *postmark, *reason;
{
	FILE *pf;
	char cbuf[256], *cp;

	cp = strchr(postmark+5, ' ');
	if (cp == NULL)
		return;
	*cp = '\0';
	sprintf(cbuf, "PATH=/bin:/usr/bin mail %s", postmark+5);
	*cp = ' ';
	if ((pf = popen(cbuf, "w")) == NULL)
		return;
	fprintf(pf, "Mail to %s failed: %s.  Message was:\n", name, reason);
	copylet(n, pf, SENDING, postmark);
	pclose(pf);
}

delete(i)
{
	setsig(i, delete);
	fprintf(stderr, "\n");
	if(delflg)
		longjmp(sjbuf, 1);
	done();
}

done()
{
	unlock();
	unlink(lettmp);
	exit(error);
}


#define LOCKPREFIX "/tmp/L."
char    lockname[DIRSIZ+sizeof(LOCKPREFIX)];

/* Lock the given file.  The parameter "file" must contain at least one '/'. */
void
lock(file)
char *file;
{
#	define TMPLNAME "/tmp/mlXXXXX"
#	define LOCKPREFIX "/tmp/L."
	char tmplname[sizeof(TMPLNAME)];
	struct stat stbuf;
	int fd;

	/* return if we are already in the middle of a lock */
	if (*lockname != '\0' || flgf)
		return;

	/* create a temporary file */
	strcpy(tmplname, TMPLNAME);
	mktemp(tmplname);
	if ((fd=creat(tmplname, 0444))<0)
		return;
	close(fd);

	/* Make a link to it with the lock file name.  This will fail only
	 * if it already exists.
	 */
	strcpy(lockname, LOCKPREFIX);
	strcat(lockname, strrchr(file, '/')+1);
	lockname[DIRSIZ+sizeof("/tmp/")-1] = '\0';
	while (link(tmplname, lockname) < 0) {
		long now;

		/* File is already locked.  Break it if the lock is old. */
		sleep(2);
		now = time((long *)0);
		if (stat(lockname, &stbuf)==0 && stbuf.st_ctime+60 < now) {
			if (stat(file, &stbuf)==0 && stbuf.st_mtime+60 >= now)
				continue;
			fprintf(stderr, "mail: breaking lock\n");
			unlink(lockname);
		}
	}
	unlink(tmplname);
	return;
}

void
unlock()
{
	if (*lockname != '\0')
		unlink(lockname);
	*lockname = '\0';
}

cat(to, from1, from2)
char *to, *from1, *from2;
{
	strcpy(to, from1);
	strcat(to, from2);
}

char *getarg(s, p)      /* copy p... into s, update p */
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


/* is this an internet address (characterized by "@" or ".") */
isinet(string)
char *string;
{
	char *rindex();

	return rindex(string, '@') != 0 || rindex(string, '.') != 0;
}

/* add any gateway info to the address */
inetgateaddr(string)
char *string;
{
	char *at, *dot;
	char *rindex();

	/* find last "." or "@" */
	at = rindex (string, '@');
	if (at != 0)
		dot = rindex (at + 1, '.');
	else
		dot = rindex (string, '.');
	if (dot != 0)
		at = dot;

	for (igate=0; igate<NIGATES; igate++) {
		if (istrcmp(at+1, igates[igate].id) == 0) {
			strcpy(at, igates[igate].addr);
			break;
		}
	}
	if (igate == NIGATES) {
		igate = 0;	/* csnet or arpa, we really don't know */
		strcat(string, igates[igate].addr);
	}
}

/* case insensitive strcmp */
istrcmp(p1, p2)
char *p1, *p2;
{
	do {
		if (tolower(*p1) != tolower(*p2))
			return *p1 - *p2;
		p2++;
	} while (*p1++ != 0);

	return 0;
}

/* log a mail transmission */
logsend (to, pm, tag)
	char *to;	/* receiver */
	char *pm;	/* the postmark */
	char *tag;	/* type of mail */
{
#	include <grp.h>
#	define LOGFILE "/usr/spool/mail/mail.log"
#	define LOGTEMP "/usr/spool/mail/mail.tmp"
	char buf[BUFSIZ];
	int out, in;
	long len;
	struct stat sbuf;
	struct group sysgroup;
	struct group *getgrnam();

	lock(LOGFILE);

	/* append to log */
	if (stat(LOGFILE, &sbuf) < 0) {
		out = creat(LOGFILE, 0660);
	} else if (sbuf.st_size < 32000) {
		out = open(LOGFILE, 2);
	} else {
		out = creat(LOGTEMP, 0660);
		in = open(LOGFILE, 0);
		if (out >= 0 && in >= 0 && lseek(in, -4000L, 2) >= 0) {
			while ((len = read(in, buf, BUFSIZ)) > 0)
				(void)write(out, buf, len);
			close(in);
			close(out);
			unlink(LOGFILE);
			link(LOGTEMP, LOGFILE);
			unlink(LOGTEMP);
		} else {
			close(in);
			close(out);
		}
			
		out = open(LOGFILE, 2);
	}
	sprintf (buf, "%s %s %s\n", tag, to, pm);
	(void)lseek(out, 0, 2);
	(void)write(out, buf, strlen(buf));
	close(out);

	unlock ();
}

/*
 *	Stuff for INTERNET headers
 */

char *
inetheader (pm) 
char *pm;
{
	static char buf[256];
	char *sender, *date;
	char *field[6];
	int i;

	/* skip the old from field */
	pm += strlen (from);

	/* skip over the sender */
	for(sender=pm; !isspace(*sender) && *sender!=NULL; sender++)
		;
	date = *sender == '\0' ? sender : sender+1;

	/* reverse the path and replace '!'s with '.'s */
	(void)strcpy (buf, "From: ");
	while (sender >= pm) {
		for (*sender = NULL; sender >= pm && *sender != '!'; sender--)
			;
		(void)strcat (buf, sender + 1);
		(void)strcat (buf, ".");
	}
	(void)strcat (buf, "btl.");

	/* change an asctime date to an INTERNET style one (yech!) */
	/* parse the old date */
	for (i = 0; i < 6; i++) {		
		for (; isspace(*date) || *date==',' || *date=='-'; date++)
			;
		field[i] = date;
		for (; !isspace(*date) && *date!=',' && *date!='-' && *date!='\0'; date++)
			;
		if (*date != '\0')
			*date++ = '\0';
	}

	/* output the arpa type format */
	pm = buf + strlen (buf) - 1;
	sprintf (pm, "\nDate: %s %s %s %s %s\n", field[2], field[1],
		 field[5], field[3], field[4]);
	return buf;
}

sendinet(n, name, postmark)
char *name, *postmark;
{
	FILE *rmf;
	char cmd[512], buf[512];
	static forked;

	if (normtf) {
		refuse(n, name, postmark, "forwarding to this system disallowed");
		return(1);
	}
	if(onatty & !forked) {
		int p = fork();
		if(p == 0) {
			sig_ign();
			forked = 1;
		} else if(p != -1)
			exit(0);
	}
#ifndef DEBUG
	sprintf(cmd, "exec %s %s", igates[igate].cmd, name);
#else
	sprintf(cmd, "echo %s %s; cat", igates[igate].cmd, name);
#endif DEBUG
	if ((rmf=popen(cmd, "w")) == NULL) {
		fprintf(stderr, "mail: can't pipe to %s\n", igates[igate].cmd);
		exit(1);
	}
	copylet(n, rmf, INTERNET, postmark);
	logsend(name, postmark, igates[igate].id);
	return(pclose(rmf)==0);
}

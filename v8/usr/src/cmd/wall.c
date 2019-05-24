static char *sccsid = "@(#)wall.c	4.5 (Berkeley) 81/06/12";
/*
 *
 * This program is not related to David Wall, whose Stanford Ph.D. thesis
 * is entitled "Mechanisms for Broadcast and Selective Broadcast".
 */

#include <stdio.h>
#include <utmp.h>
#include <time.h>
#include <sys/utsname.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#define	USERS	128

char	mesg[3000];
int	msize,sline;
struct	utmp utmp[USERS];
char	*strcpy(), *strncpy();
char	*strcat(), *strncat();
char who[9] = "greg";
long	clock, time();
struct tm *localtime();
struct tm *localclock;
struct utsname ourname;

main(argc, argv)
char *argv[];
{
	register i;
	register char c;
	register struct utmp *p;
	FILE *f;
	FILE *mf;

	if((f = fopen("/etc/utmp", "r")) == NULL) {
		fprintf(stderr, "Cannot open /etc/utmp\n");
		exit(1);
	}
	clock = time( 0 );
	localclock = localtime( &clock );
	mf = stdin;
	if(argc >= 2) {
		/* take message from unix file instead of standard input */
		if((mf = fopen(argv[1], "r")) == NULL) {
			fprintf(stderr,"Cannot open %s\n", argv[1]);
			exit(1);
		}
	}
	while((i = getc(mf)) != EOF) {
		if (msize >= sizeof mesg) {
			fprintf(stderr, "Message too long\n");
			exit(1);
		}
		mesg[msize++] = i;
	}
	fclose(mf);
	sline = ttyslot(2); /* 'utmp' slot no. of sender */
	fread((char *)utmp, sizeof(struct utmp), USERS, f);
	fclose(f);
	if (sline)
		strncpy(who, utmp[sline].ut_name, sizeof(utmp[sline].ut_name));
	for(i=0; i<USERS; i++) {
		register int j;
		p = &utmp[i];
		if (p->ut_name[0] == 0)
			continue;
		/* reject people not really logged in */
		j = 0;
		while (j < sizeof (p->ut_name) && p->ut_name[j] != '*')
			j++;
		if (j < sizeof (p->ut_name))
			continue;
		sendmes(p->ut_line, sizeof(p->ut_line));
	}
	exit(0);
}

sendmes(tty, len)
char *tty;
{
	register i;
	char t[50], buf[BUFSIZ];
	register char *cp;
	register int c, ch;
	FILE *f;

	while ((i = fork()) == -1)
		if (wait((int *)0) == -1) {
			fprintf(stderr, "Try again\n");
			return;
		}
	if(i)
		return;
	strcpy(t, "/dev/");
	strncat(t, tty, len);
	t[sizeof("/dev/")+len] = 0;

	if (ckmode(t) == 0)
		exit(0);
	signal(SIGALRM, SIG_DFL);	/* blow away if open hangs */
	alarm(10);

	if((f = fopen(t, "w")) == NULL) {
		fprintf(stderr,"cannot open %s\n", t);
		exit(1);
	}
	setbuf(f, buf);
	uname (&ourname);
	fprintf(f,
	    "\nBroadcast Message from %s!%s (%.*s) at %d:%02d ...\r\n\n"
		, ourname.nodename
		, who
		, sizeof(utmp[sline].ut_line)
		, utmp[sline].ut_line
		, localclock -> tm_hour
		, localclock -> tm_min
	);
	/* fwrite(mesg, msize, 1, f); */
	for (cp = mesg, c = msize; c-- > 0; cp++) {
		ch = *cp;
		if (ch == '\n')
			putc('\r', f);
		putc(ch, f);
	}

	/*
	 * Bitchin'.
	 */

	exit(0);
}

/*
 * don't write on ttys whose mode is 0
 * hack to prevent hanging ndcon
 */

ckmode(tty)
char *tty;
{
	struct stat sb;

	if (stat(tty, &sb) < 0)
		return (1);	/* eh? */
	if (sb.st_mode & ~S_IFMT)
		return (1);
	return (0);
}

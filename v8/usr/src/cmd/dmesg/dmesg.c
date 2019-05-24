static char *sccsid = "@(#)dmesg.c	4.3 (Berkeley) 2/28/81";
/*
 *	Suck up system messages
 *	dmesg
 *		print current buffer
 *	dmesg -
 *		print and update incremental history
 */

#include <stdio.h>
#include <sys/param.h>
#include <nlist.h>
#include <signal.h>
#include <sys/vm.h>
#include <sys/msgbuf.h>

struct	msgbuf msgbuf;
char	*msgbufp;
int	sflg;
int	wflg;

struct	msgbuf omesg;
struct	nlist nl[2] = {
	{ "_msgbuf" },
	{ 0 }
};

#define	BUFFER	"/usr/adm/msgbuf"

main(argc, argv)
char **argv;
{
	int mem;
	register char *mp, *omp, *mstart;
	int timeout();
	int samef;

	signal(SIGALRM, timeout);
	alarm(60);
	if (argc>1 && argv[1][0] == '-') {
		sflg++;
		switch (argv[1][1]) {
		default:		/* ugh */
			wflg++;
			break;

		case 'i':
			break;
		}
		argc--;
		argv++;
	}
	if (sflg)
		openbuf();
	sflg = 0;
	timeout("can't read namelist\n");
	nlist(argc>2? argv[2]:"/unix", nl);
	if (nl[0].n_type==0)
		done("No namelist\n");
	if ((mem = open((argc>1? argv[1]: "/dev/kmem"), 0)) < 0)
		done("No /dev/kmem\n");
	lseek(mem, (long)nl[0].n_value, 0);
	read(mem, &msgbuf, sizeof (msgbuf));
	if (msgbuf.msg_magic != MSG_MAGIC)
		done("Magic number wrong (namelist mismatch?)\n");
	mstart = &msgbuf.msg_bufc[omesg.msg_bufx];
	omp = &omesg.msg_bufc[msgbuf.msg_bufx];
	mp = msgbufp = &msgbuf.msg_bufc[msgbuf.msg_bufx];
	samef = 1;
	do {
		if (*mp++ != *omp++) {
			mstart = msgbufp;
			samef = 0;
			pdate();
			printf("...\n");
			break;
		}
		if (mp == &msgbuf.msg_bufc[MSG_BSIZE])
			mp = msgbuf.msg_bufc;
		if (omp == &omesg.msg_bufc[MSG_BSIZE])
			omp = omesg.msg_bufc;
	} while (mp != mstart);
	if (samef && omesg.msg_bufx == msgbuf.msg_bufx)
		exit(0);
	mp = mstart;
	do {
		pdate();
		if (*mp && (*mp & 0200) == 0)
			putchar(*mp);
		mp++;
		if (mp == &msgbuf.msg_bufc[MSG_BSIZE])
			mp = msgbuf.msg_bufc;
	} while (mp != msgbufp);
	done((char *)NULL);
}

done(s)
char *s;
{
	register char *p, *q;

	if (s && s!=(char *)omesg.msg_magic && sflg==0) {
		pdate();
		printf(s);
	}
	if (wflg)
		writebuf();
	exit(s!=NULL);
}

openbuf()
{
	int f;

	timeout("can't read buffer file\n");
	if ((f = open(BUFFER, 0)) < 0)
		return;
	if (read(f, (char *)&omesg, sizeof(omesg)) != sizeof(omesg)
	||  omesg.msg_magic != MSG_MAGIC)
		zero((char *)&omesg, sizeof(omesg));
	close(f);
}

writebuf()
{
	int f;

	timeout("can't write buffer file\n");
	if ((f = open(BUFFER, 1)) < 0)
		done("can't open buffer\n");
	if (write(f, (char *)&msgbuf, sizeof(msgbuf)) != sizeof(msgbuf))
		done("error writing buffer\n");
	close(f);
}

pdate()
{
	extern char *ctime();
	static firstime;
	time_t tbuf;

	if (firstime==0) {
		firstime++;
		time(&tbuf);
		printf("\n%.12s\n", ctime(&tbuf)+4);
	}
}

zero(b, n)
register char *b;
register int n;
{

	while (--n >= 0)
		*b++ = 0;
}

char *terr;

timeout(s)
char *s;
{
	extern ttrap();

	terr = s;
	signal(SIGALRM, ttrap);
	alarm(60);
}

ttrap()
{
	char buf[100];

	sprintf(buf, "timeout: %s", terr);
	done(buf);
}

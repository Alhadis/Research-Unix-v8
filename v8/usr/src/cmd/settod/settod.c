#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <sys/clock.h>
#include <utmp.h>
#include <signal.h>

#define CLOCK "mh/astro/clock"
/* buffer to read time into and print it from */
char buf[24]; /* HH:MM:SS.S     DD/MM/YY */

typedef unsigned long tod_t;

tod_t settod(), readnet();
long labs();
int sflag, vflag;

static	char	dmsize[12] =
{
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

main (argc, argv)
	int argc;
	char **argv;
{
	tod_t ourtime, wwvtime;
	long delta;
	int c;

	while ((c = getopt (argc, argv, "vs")) != EOF) {
		switch (c) {
		case 's':
			sflag++;
			break;
		case 'v':
			vflag++;
			break;
		case '?':
			return 1;
		}
	}

	if (!sflag)
		vflag++;

	wwvtime = readnet();
	if (wwvtime) {
		ourtime = settod(0L);
		delta = wwvtime - ourtime;

		if (sflag) {
			if (settod (wwvtime) == -1)
				printf ("not super-user, hence not set\n");
		}

		if (vflag) {
			printf("%.23s ", buf);
			if (delta == 0)
				printf ("exact\n");
			else
				printf ("%s %ld.%.2ld\n",
				    delta < 0? "retard": "advance",
				    labs (delta) / 100, labs (delta) % 100);
		}
	}
	return 0;

}

tod_t
readnet()
{
	int i, j, year, mon;
	int day, hour, mins, secs, tenth;
	int f;
	int alcatch();
	tod_t timbuf;
	extern errno, sys_nerr;
	extern char *sys_errlist[];
	extern dkp_ld;
	int retry = 30;		/* max number of retries */

	do {
		int err;
		signal(SIGALRM, alcatch);
		alarm(15);
		f = tdkdial(CLOCK, 2);
		if (f < 0) {
			alarm(0);
			printf("settod: can't open clock\n");
			return(0);
		}
		if (dkproto(f, dkp_ld) < 0) {
			alarm(0);
			printf("settod: can't turn on DK proto\n");
			return(0);
		}
		do {
			if ((j = read(f, buf, 1)) <= 0)
				goto readerr;
		} while ((buf[0]&0177) != '\r');
		for (i=0; i<24; i += j) {
			if ((j = read(f, buf+i, 24-i)) <= 0)
				goto readerr;
		}
		alarm(0);
		for (i=0; i<24; i++)
			buf[i] &= 0177;
		/* accept '?' for tenths */
		if (buf[9]=='?')
			buf[9] = '0';
		if (sscanf(buf, "%d:%d:%d.%d    %d/%d/%d\r", &hour, &mins, &secs,
		   &tenth, &mon, &day, &year) != 7)
			goto formerr;
		if( mon<1 || mon>12 ||
		    day<0 || day>31 ||	/* June 30, 1984 == July 0, 1984 !!?? */
		    mins<0 || mins>59 ||
		    secs<0 || secs>59 ||
		    tenth<0 || tenth>9 ||
		    hour<0 || hour>23)
			goto formerr;
		/* leap year */
		timbuf = 0;
		if (year % 4 == 0 && mon >= 3)
			timbuf++;
		while(--mon)
			timbuf += dmsize[mon-1];
		timbuf += day-1;
		timbuf = 24*timbuf + hour;
		timbuf = 60*timbuf + mins;
		timbuf = 60*timbuf + secs;
		timbuf = 10*timbuf + tenth;
		timbuf = timbuf * 10 + TODRZERO;
		return timbuf;
	formerr:
		printf("settod: clock format error: %.24s\n", buf);
		return(0);
	readerr:
		err = errno;
		close(f);
		alarm(0);
		if (j < 0) {
			printf("settod: clock read ");

			if (err == EINTR) {
				printf ("timeout\n");
				return 0;
			} else if (err < sys_nerr)
				printf("error: %s\n", sys_errlist[err]);
			else
				printf("error: %d\n", err);
		}
		sleep (1 + (int)((getpid() + time ((time_t) 0)) % 7));
	} while (--retry >= 0);
	return 0;
}

alcatch() {}

long
labs (n)
	long n;
{
	if (n >= 0)
		return n;
	return -n;
}

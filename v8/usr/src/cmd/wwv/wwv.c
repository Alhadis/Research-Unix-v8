#include <stdio.h>
/*
 * get date from wwv network clock
 */
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <utmp.h>
#include <signal.h>

int	uflag;
int	sflag;
int	bflag;
int	fflag;
char	*timezone();
static	int	dmsize[12] =
{
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

char	clock[] = "mh/astro/clock";

struct utmp wtmp[2] = { {"|", "", 0}, {"{", "", 0}};

char	*ctime();
char	*asctime();
struct	tm *localtime();
struct	tm *gmtime();

main(argc, argv)
char *argv[];
{
	int wf, rc;
	time_t wwvtime, readnet(), labs();

	rc = 0;
	for(;;) {
		switch(getopt(argc, argv, "usbf")) {

		case 's':
			sflag++;
			continue;

		case 'u':
			uflag++;
			continue;

		case 'b':
			bflag++;
			continue;

		case 'f':
			fflag++;
			continue;

		case '?':
			exit(1);

		case EOF:
			goto OK;
		}
	}
OK:
	wwvtime = readnet();
	if (wwvtime==0)
		exit(1);
	if (sflag) {
		time_t nowtime = time((time_t)0);
		if (fflag || labs(nowtime-wwvtime) < 20*60) {
			wtmp[0].ut_time = nowtime;
			wtmp[1].ut_time = wwvtime;
			if(stime(&wwvtime) < 0) {
				rc++;
				printf("wwv: no permission\n");
			} else {
				if (wwvtime >= nowtime)
					printf("advanced %ld sec\n",
						wwvtime-nowtime);
				else
					printf("retarded %ld sec\n",
						nowtime-wwvtime);
				if ((wf = open("/usr/adm/wtmp", 1)) >= 0) {
					time(&wtmp[1].ut_time);
					lseek(wf, 0L, 2);
					write(wf, (char *)wtmp, sizeof(wtmp));
					close(wf);
				}
			}
		} else {
			printf("wwv: >20min difference; force with  wwv -sf\n");
			rc = 1;
			bflag++;
		}
	}
	if (bflag) {
		printf("WWV: "); prt(wwvtime);
		printf("you: "); prt(time((time_t)0));
	} else
		prt(wwvtime);
	exit(rc);
}

prt(t)
time_t t;
{
	struct timeb info;
	char *ap, *tzn;

	if (uflag) {
		ap = asctime(gmtime(&t));
		tzn = "GMT";
	} else {
		struct tm *tp;
		ftime(&info.time);
		tp = localtime(&t);
		ap = asctime(tp);
		tzn = timezone(info.timezone, tp->tm_isdst);
	}
	printf("%.20s", ap);
	if (tzn)
		printf("%s", tzn);
	printf("%s", ap+19);
}

time_t
readnet()
{
	int i, j, year, mon;
	int day, hour, mins, secs, tenth;
	register struct tm *L;
	char buf[24]; /* HH:MM:SS.S     DD/MM/YY */
	int f;
	int alcatch();
	time_t nowtime, timbuf;
	extern errno, sys_nerr;
	extern char *sys_errlist[];
	extern dkp_ld;

	signal(SIGALRM, alcatch);
	alarm(15);
	f = tdkdial(clock, 2);
	if (f < 0) {
		alarm(0);
		printf("wwv: can't open clock\n");
		return(0);
	}
	if (dkproto(f, dkp_ld) < 0) {
		alarm(0);
		printf("wwv: can't turn on DK proto\n");
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
	if (year!= 84)
		goto formerr;
	if( mon<1 || mon>12 ||
	    day<0 || day>31 ||	/* June 30, 1984 == July 0, 1984 ??!!?? */
	    mins<0 || mins>59 ||
	    secs<0 || secs>59 ||
	    hour<0 || hour>23)
		goto formerr;
	time(&nowtime);
	L = gmtime(&nowtime);
	timbuf = 0;
	for(i=70; i<L->tm_year; i++)
		timbuf += dysize(i);
	/* always leap year */
	if (mon >= 3)
		timbuf++;
	while(--mon)
		timbuf += dmsize[mon-1];
	timbuf += day-1;
	timbuf = 24*timbuf + hour;
	timbuf = 60*timbuf + mins;
	timbuf = 60*timbuf + secs;
	return(timbuf);
formerr:
	printf("wwv: clock format error: %.24s\n", buf);
	return(0);
readerr:
	alarm(0);
	printf("wwv: clock read ");
	if (j < 0) {
		if (errno < sys_nerr)
			printf("error: %s\n", sys_errlist[errno]);
		else
			printf("error: %d\n", errno);
	} else
		printf("EOF\n");
	return(0);
}

alcatch()
{
	return;
}

time_t
labs(t)
{
	if (t < 0)
		return(-t);
	return(t);
}

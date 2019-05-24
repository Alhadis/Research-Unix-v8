#include <stdio.h>

#define NPIPE	4

#define PARGVAL	((*argv)[2] ? (*argv)+2 : --argc ? *++argv : (char *)0)

#define putch(c)	(cbuf=c, write(1,&cbuf,1))

FILE *filep, *popen(); unsigned char buffer[256], cbuf;

char *jpgm = { "/usr/blit/mbin/bbterm.m" }; int zflag = 0;

char *ppgm[NPIPE] = {
	"/usr/bin/bcan -", "/usr/bin/3bcan -",
	"/usr/bin/4bcan -", "/usr/bin/8bcan -"
};
int pflag = 0;
extern errno;

main(argc,argv)
int argc; char **argv;
{
	int i, rcvstat;
	while (--argc > 0) {
		if ((*++argv)[0] == '-') switch ((*argv)[1]) {
		case 'j':
			jpgm = PARGVAL; break;
		case 'z':
			zflag++; break;
		case 'p':
			i = atoi(PARGVAL);
			if (i<0 || i>=NPIPE) {
				fprintf(stderr,"index too big: %s\n",*argv);
				exit(1);
			}
			ppgm[i] = *++argv; --argc; break;
		default:
			fprintf(stderr,"unknown option %s\n",*argv);
			exit(1);
		} else {
			fprintf(stderr,"unknown argument %s\n",*argv);
			exit(1);
		}
	}

	if (jload(jpgm,zflag)) exit(1);
	rawtty(1);
	putch(0); putch(255);

	while ((rcvstat=recvbuf()) > 0) {
		if (buffer[1] != '\n') {
			filep=fopen(buffer+1,"w");
		} else if (buffer[2] > 0 && buffer[2] <= NPIPE) {
			filep=popen(ppgm[buffer[2]-1],"w"); pflag = 1;
		} else {
			filep = (FILE *)0;
		}

		if (filep != (FILE *)0) putch(0);
		else { putch(1); pflag = 0; continue; }

		while ((rcvstat=recvbuf()) > 0) fwrite(buffer+1,*buffer-1,1,filep);

		if (!pflag) {
			fclose(filep); putch(0);
		} else {
			putch(pclose(filep)); pflag = 0;
		}
		if (rcvstat < 0) break;
	}

	rawtty(0);
	if (rcvstat < 0) fprintf(stderr,"error %d reading from jerq\n",errno);
	exit(0);
}

recvbuf()
{
	register nr, nb = 0;
	putch(0);
	do {
		if ((nr = read(0,buffer+nb,(sizeof buffer)-nb)) <= 0) return -1;
		nb += nr;
	} while (nb < *buffer);
	return *buffer;
}

jload(prog,zflag)
char *prog; int zflag;
{
	static char *cmd[] = { "68ld", "-e", (char *)0, (char *)0, (char *)0 };
	if (zflag) { cmd[2] = "-z"; cmd[3] = prog; }
	else { cmd[2] = prog; cmd[3] = (char *)0; }
	if (systemv("/usr/blit/bin/68ld",cmd)) return 1;
	sleep(2);
	return 0;
}

systemv(name,argv)
char *name, **argv;
{
	int status, pid, l;
	if ((pid = fork()) == 0) { execv(name,argv); _exit(127); }
	while ((l = wait(&status)) != pid && l != -1);
	return (l == -1) ? -1 : status;
}

#include <sgtty.h>

rawtty(flag)
{
	static struct sgttyb ttyb; static int tty_flags, tty_raw = 0;
	extern int tty_ld;

	if (flag == tty_raw) return;

	if (flag) {
		ioctl(0, TIOCGETP, &ttyb);
		tty_flags=ttyb.sg_flags;
		ttyb.sg_flags |=  RAW;
		ttyb.sg_flags &= ~ECHO;
		ioctl(0, TIOCSETP, &ttyb);
		ioctl(0, FIOPOPLD, 0);
	} else {
		ioctl(0, FIOPUSHLD, &tty_ld);
		ttyb.sg_flags=tty_flags;
		ioctl(0, TIOCSETP, &ttyb);
	}
	tty_raw = flag;
}

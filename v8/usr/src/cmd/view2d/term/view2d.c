#include	<stdio.h>
#include	<signal.h>
#include	<sgtty.h>
#include	"/usr/jerq/include/jioctl.h"

#define		MAX		32767

struct sgttyb sttybuf, sttysave;
char *zflag = "";
int shade = 0;
int grid = 1;
typedef struct Point
{
	int x, y;
} Point;
struct Frame
{
	short z[4096];
	short time;
} frames[200];
int zmax = -1000000000, zmin = 1000000000;
int nsleep, ntime;
int nframes = 0;
int period = 5;
int floor = -32767;
char *progname;
short ts, te, timewarp;

main(argc, argv)
	char **argv;
{
	char buf[512];
	int zc, scale, m, i;
	short ragnorok, dim;
	short *nz, *p, *q;
	int fd;
	int cv, tmin, tmax;
	short nx, ny, fru, frv;
	char *s;
	char **av = argv;
	int ac = argc;
	extern char *getenv();
	int magic[4];

	timewarp = 0;
	progname = argv[0];
	cv = ((s = getenv("TERM")) == 0) || strcmp(s, "5620");
	ioctl(0, TIOCGETP, &sttysave);
	strcpy(buf, "level");
	for(argc--, argv++; *argv && (**argv == '-'); argv++)
		switch(argv[0][1])
		{
		case 'z':
			zflag = "-z";
			break;
		case 's':
			shade++;
			break;
		case 'g':
			grid = 0;
			break;
		case 'T':
			if(argv[0][2] == 'c') cv = 1;
			else if(argv[0][2] == '5') cv = 0;
			else if(argv[0][2] == 'p') cv = 2;
			else goto err;
			break;
		case 't':
			i = sscanf(&argv[0][2], "%hd, %hd", &ts, &te);
			if(i==0) error("bad TS,TE");
			timewarp = i;
			break;
		case 'p':
			sscanf(&argv[0][2], "%d", &period);
			break;
		case 'i':
			sscanf(&argv[0][2], "%d", &floor);
			break;
		case 'S':
		case 'b':
		case 'm':
		case 'w':
			/* these options only apply in color, so assume -Tc */
			cv = 1;
			break;
		case 'c':
		case 'v':
			break;
		default:
		err:
			fprintf(stderr, "Usage: view2d [-Tdev] [-ptime] [-cn] [-ms]\n");
			exit(1);
		}
	if((fd = open(*argv, 0)) == -1)
		quit("cannot open input");
	if(cv==1)
	{
		if(access("/usr/bin/level", 1) == -1)
		{
			fprintf(stderr, "go to kwee to use the frame buffer\n");
			exit(1);
		}
		dup2(fd,0);     /* close(0); dup(fd); close(fd); */
		strcpy(*av, "level");
		execvp(*av, av);
		perror("level");
		exit(1);
	} else if(cv==2) {
		if(access("/usr/bin/contour", 1) == -1)
		{
			fprintf(stderr, "can't find /usr/bin/contour\n");
			exit(1);
		}
		dup2(fd,0);     /* close(0); dup(fd); close(fd); */
		strcpy(*av, "contour");
		execvp(*av, av);
		perror("contour");
		exit(1);
	}
	sprintf(buf, "/usr/jerq/bin/32ld %s vwd.m", zflag);
	if(system(buf)) exit(1);
	sttybuf = sttysave;
	sttybuf.sg_flags |= RAW;
	sttybuf.sg_flags &= ~ECHO;
	ioctl(0, TIOCSETP, &sttybuf);
	nz = frames[0].z;
	rd2dh(fd, &nx, &ny);
	putchar('X'); sendn(nx);
	putchar('Y'); sendn(ny);
	if(grid) putchar('G');
	if(shade) putchar('S');
	putchar('P');
	sendn(period);
	m = 2*nx*ny;
	tmin = MAX;
	tmax = -MAX;
	while( rd2di( &frames[nframes].time, frames[nframes].z) )
	{
		if(frames[nframes].time < tmin) tmin = frames[nframes].time;
		if(frames[nframes].time > tmax) tmax = frames[nframes].time;
		for(nz = frames[nframes].z, i = nx*ny; i; i--)
		{
			if(*nz < zmin) zmin = *nz;
			if(*nz > zmax) zmax = *nz;
			nz++;
		}
		nframes++;
	}
	putchar('n');
	sendn(nframes);
	zc = (zmax+zmin)/2;
	scale = (zmax-zmin)/2+1;
#define	SCALE(x,m)	((int)((((long)x)*(m))/scale))
	putchar('I');
	sendn(floor,MAX);
	for(m = 0; m < nframes; m++)
	{
		putchar('F');
		sprintf(buf, "%d", frames[m].time);
		sends(buf);
		for(q = nx*ny + (p = frames[m].z); q != p; p++)
			sendn(SCALE(*p - zc, MAX-2));
	}

	putchar('Q');
	fflush(stdout);
	(void)getchar();
	quit(0);
}

quit(s)
	char *s;
{
	ioctl(0, JTERM, 0);
	ioctl(0, TIOCSETP, &sttysave);
	if(s)
		printf("view2d: %s\n", s);
	exit(0);
}

sendn(n)
{
	putchar(n>>8);
	putchar(n);
}

sends(s)
	char *s;
{
	for(;;)
	{
		putchar(*s);
		if(*s++ == 0) break;
	}
}

abs(n)
{
	return(n<0? -n:n);
}

sendframe(t, dim)
{
	struct Frame *f1, *f2;
	int t1, t2, m, tt;
	register short *a, *b, i;

	t1 = t2 = 32767;
	for(i = 0; i < nframes; i++)
	{
		tt = abs(frames[i].time - t);
		if(tt < t1)
		{
			f2 = f1; t2 = t1;
			f1 = &frames[i]; t1 = tt;
		}
		else if(tt < t2)
		{
			f2 = &frames[i]; t2 = tt;
		}
	}
	putchar('F');
	sendn(t);
	for(m = dim, a = f1->z, b = f2->z; m--; a++, b++)
		sendn(*a + t1*(*b-*a)/(t1+t2));
}

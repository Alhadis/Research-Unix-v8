#include "stdio.h"
#include "/usr/blit/include/jioctl.h"
#include "sgtty.h"
#include "signal.h"
struct sgttyb sttybuf, sttysave;
FILE *pf;
char *p;
extern FILE *popen();

done()
{
	ioctl(0, TIOCSETP, &sttysave);
	exit(0);
}

main(argc, argv)
	char *argv[];
{	int fd;
	char buf[64], *ttyname();
	char *maze="/usr/blit/mbin/maze.m";
	if(argc>1)
		maze=argv[1];
	p=ttyname(1);
	if(p==NULL){
		perror("not on a tty:");
		exit(2);
	}
	fd = open("/dev/pt/maze", 2);
	if(fd < 0) {
		perror("/dev/pt/ump");
		exit(1);
	}
	ioctl(0, TIOCGETP, &sttysave);
	sttybuf = sttysave;
	sttybuf.sg_flags |= CBREAK;
	sttybuf.sg_flags &= ~ECHO;
	ioctl(0, TIOCSETP, &sttybuf);
	sprintf(buf, "/usr/blit/bin/68ld %s", maze);
	if(system(buf)) {
		perror(maze);
		done();
	}
	signal(SIGINT, done);
	write(fd, p, strlen(p));
	pause();
}

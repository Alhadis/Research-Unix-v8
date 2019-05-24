#include "stdio.h"
#include "/usr/blit/include/jioctl.h"
#include "sgtty.h"
#include "signal.h"
struct sgttyb sttybuf, sttysave;

char *rmt = "/usr/blit/lib/rmtmaze";

done()
{
	ioctl(0, TIOCSETP, &sttysave);
	exit(0);
}

main()
{	int n;
	char buf[128];
	int fd;
	ioctl(0, TIOCGETP, &sttysave);
	sttybuf = sttysave;
	sttybuf.sg_flags |= CBREAK;
	sttybuf.sg_flags &= ~ECHO;
	ioctl(0, TIOCSETP, &sttybuf);
	system("68ld /usr/blit/lib/maze.m");
	fd = dkexec("ikeya", rmt);
	if(fd < 0) {
		perror(rmt);
		exit(1);
	}
	signal(SIGINT, done);
	if(fork()) {
		while((n = read(fd, buf, sizeof(buf))) >= 0)
			if(n > 0)
				write(1, buf, n);
		printf("dk stopped\n");
		exit(0);	/* reboot jerq */
	}
	else {
		while((n = read(0, buf, sizeof(buf))) > 0)
			if(write(fd, buf, n) < 0)
				exit(0);
	}
}

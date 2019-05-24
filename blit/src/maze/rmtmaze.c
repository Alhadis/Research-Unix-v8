#include "stdio.h"
#include "sgtty.h"

FILE *pf;
char *p;
extern FILE *popen();

main()
{	int fd;
	p = ttyname(1);
	if(p == 0) {
		perror("not a tty:");
		exit(2);
	}
	fd = open("/dev/pt/maze", 2);
	if(fd < 0) {
		perror("/dev/pt/ump");
		exit(1);
	}
	chmod(ttyname(0), 0666);
	ioctl(0,TIOCNXCL,0);
	write(fd, p, strlen(p));
	sleep(30);
}

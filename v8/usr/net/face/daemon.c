/* file service */
#include "fserv.h"
#include "errno.h"
#include "neta.h"

char cmdbuf[256];
struct rcva y, nilrcv;
struct stat rootstat;
char *mountpt;
char *dialstring;
int dev, myfd;

#define USAGE "usage: faced dialstring mount-pt mount-dev\n"

main(argc, argv)
char **argv;
{
	netf *p;

	if (argc != 4) {
		write(2, USAGE, strlen(USAGE));
		exit(1);
	}
	dialstring = argv[1];
	mountpt = argv[2];
	dev = atoi(argv[3])<<8;
	detach("/tmp/facedl");
	signals();
	while((myfd = domount()) < 0)
		sleep(60);
	while((p = newnetf("/")) == 0)
		sleep(60);
	rootstat = p->statb;
	work();
}

respond(n)
{
	y.errno = n;
	(void) write(myfd, (char *)&y, sizeof(y));
}

domount()
{
	int pfd[2];

	if (pipe(pfd) < 0) {
		perror("mount pipe");
		return -1;
	}
	gmount(1, dev, 1, 0, mountpt);
	if (gmount(1, dev, 0, pfd[0], mountpt) < 0) {
		perror("mounting");
		gmount(1, dev, 1, 0, mountpt);
		close(pfd[0]);
		close(pfd[1]);
		return -1;
	}
	close(pfd[0]);
	return(pfd[1]);
}

gmount(fstyp, dev, flag, fd, path)
	char *path;
{
	syscall(49, fstyp, dev, flag, fd, path);
}

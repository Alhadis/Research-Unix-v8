#include <stdio.h>

#define GMOUNT	49
#define MOUNT	0
#define UNMOUNT	1

extern int errno;

int mntfstyp = 2;
char mntdir[] = "/proc";

main(argc, argv)
char **argv;
{
	register flag, n;

	flag = (argc > 1) ? UNMOUNT : MOUNT;
	if (n = syscall(GMOUNT, mntfstyp, mntdir, flag)) {
		printf("gmount(%d, \"%s\", %d) returned %d, errno = %d\n",
			mntfstyp, mntdir, flag, n, errno);
		perror("gmount");
	} else {
		if (flag == MOUNT)
			printf("fstyp = %d mounted on %s\n", mntfstyp, mntdir);
		else
			printf("fstyp = %d unmounted from %s\n", mntfstyp, mntdir);
	}
	exit(0);
}

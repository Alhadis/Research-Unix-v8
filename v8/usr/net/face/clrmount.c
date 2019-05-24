#include <stdio.h>
main(ac, av)
	char **av;
{
	int n, fstyp;

	switch(ac) {
	case 2:
		fstyp = 1;
		break;
	case 3:
		fstyp = atoi(av[2]);
		break;
	default:
		fprintf(stderr, "usage: unmount major-dev-no [fs-type]\n");
		exit(1);
	}
 
	n = gmount(fstyp, (atoi(av[1]))<<8, 1, 0, 0);
	if (n<0) {
		perror("+ ");
	}
}


/*	ls.c	4.2	81/03/15	*/

#include <sys/param.h>
#include <sys/inode.h>
#include <sys/ino.h>
#include <sys/dir.h>
#include "saio.h"

char line[100];

main()
{
	int i;

	printf("ls\n");
	do  {
		printf(": "); gets(line);
		i = open(line, 0);
	} while (i < 0);

	ls(i);
}

ls(io)
register io;
{
	struct direct d;
	register i;

	while (read(io, (char *)&d, sizeof d) == sizeof d) {
		if (d.d_ino == 0)
			continue;
		printf("%d\t", d.d_ino);
		for (i=0; i<DIRSIZ; i++) {
			if (d.d_name[i] == 0)
				break;
			printf("%c", d.d_name[i]);
		}
		printf("\n");
	}
}

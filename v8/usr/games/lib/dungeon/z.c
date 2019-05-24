
main()
{

	register int *up;
	int fd3, fd4, fd5;


ok:
	/*
	 * open up files needed by program
	 * look in current directory first, then try default names
	 * The following files must be as follows:
	 * "dtext.dat" open read-only on fd 3
	 * "dindex.dat open read-only on fd 4 (maybe this file isn't used)
	 * "doverlay" open read-only on fd 5 (put this file on fast disk)
	 */
	close(3);
	close(4);
	close(5);
	if ((fd3 = open("dtext.dat", 0)) < 0)
		if ((fd3 = open("/usr/games/lib/dungeon/dtext.dat", 0)) < 0)
			error("Can't open dtext.dat\n");

	if ((fd4 = open("dindex.dat", 0)) < 0)
		if ((fd4 = open("/usr/games/lib/dungeon/dindex.dat", 0)) < 0)
			error("Can' open dindex.dat\n");

	if ((fd5 = open("doverlay", 0)) < 0)
		if ((fd5 = open("/tmp/nedtmp/doverlay", 0)) < 0)
			if ((fd5 = open("/usr/games/lib/dungeon/doverlay", 0)) < 0)
				error("Can't open doverlay\n");

	if (fd3 != 3 || fd4 != 4 || fd5 != 5)
		error("Files opened on wrong descriptors\n");

	signal(2,1);

	printf("You are in an open field west of a big white house with a boarded\n");
	printf("front door.\n");
	printf("There is a small mailbox here.\n>");
	execl("/usr/games/lib/dungeon/dung","DUNGEON", 0);
	printf("Can't start dungeons.\n");
	exit(1);
}
error(s)
char *s;
{
	printf("%s", s);
	exit(1);
}

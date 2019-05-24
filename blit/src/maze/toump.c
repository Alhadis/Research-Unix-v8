main(argc, argv)
char **argv;
{	int n, fd;
	char buf[32];
	if(argc != 3) {
		printf("even pt odd pt\n");
		exit(1);
	}
	fd = open(argv[2], 2);
	if(fd < 0) {
		perror(argv[2]);
		exit(0);
	}
	open(argv[1], 0);	/* who knows? */
	n = open("/dev/pt/maze", 1);
	write(n, argv[1], strlen(argv[1]));
	close(n);
	if(n < 0) {
		perror("pt18");
		exit(0);
	}
	for(;;) {
		sleep(10);
		printf("%s sending\n", argv[2]);
		write(fd, argv[2], strlen(argv[2]));
		while((n = read(fd, buf, 16)) > 0)
			printf("%s got %s\n", argv[2], buf);
	}
}

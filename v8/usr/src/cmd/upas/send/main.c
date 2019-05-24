/* configuration */
#define READMAIL "printmail"

/* import */
extern void readmail();
extern void sendmail();
extern void setnotify();
extern char *upaspath();
extern char *basename();

main(ac, av)
	int ac;
	char *av[];
{
	int i;
		
	/* skip flags */
	for (i = 1; i < ac; i++) {
		if (av[i][0] == '-') {
			switch (av[i][1]) {
			case 'n':
				setnotify();
				exit(0);
			case 'f':
				i++;
				break;
			case 'N':
				sendmail(ac, av);
				exit(0);
			}
		} else {
			sendmail(ac, av);
			exit(0);
		}
	}

	if (strcmp(basename(av[0]), "rmail") != 0) {
		(void)setgid(getgid());
		(void)setuid(getuid());
		execv(upaspath(READMAIL), av);
	}
}

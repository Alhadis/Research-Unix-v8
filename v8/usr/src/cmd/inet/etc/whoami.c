char *
whoami()
{
	static char name[128];
	int fd, n;
	char *cp;

	fd = open("/etc/whoami", 0);
	if (fd < 0)
		return ("Kremvax");
	n = read(fd, name, sizeof(name)-1);
	if (n <= 0)
		return ("Kremvax");
	name[n] = '\0';
	for (cp=name; *cp; cp++)
		if (*cp == '\n') {
			*cp = '\0';
			break;
		}
	return name;
}


/*
 * Return the number of the slot in the utmp file
 * corresponding to the current user.
 * Definition is the line number in the /etc/ttys file.
 */

char	*cttyname();
char	*strcpy();
char	*getttys();

ttyslot()
{
	register char *cp, *tp;
	register int s, tf;

	if ((cp = cttyname()) == 0)
		return(0);
	if (strncmp(cp, "/dev/", 5) == 0)
		cp += 5;
	if ((tf=open("/etc/ttys", 0)) < 0)
		return(0);
	s = 0;
	while (tp = getttys(tf)) {
		s++;
		if (strcmp(cp, tp)==0) {
			close(tf);
			return(s);
		}
	}
	close(tf);
	return(0);
}

static char *
getttys(f)
{
	static char line[32];
	register char *lp;

	lp = line;
	for (;;) {
		if (read(f, lp, 1) != 1)
			return(0);
		if (*lp =='\n') {
			*lp = '\0';
			return(line+2);
		}
		if (lp >= &line[32])
			return(line+2);
		lp++;
	}
}

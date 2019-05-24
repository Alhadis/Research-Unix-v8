/*
 * log a user into a remote system via datakit
 *
 *  first attempt at a valid userid came from tdkdial(),
 * who sent it as part of the setup request.  Remote
 * system responds to that with either "OK" or "NO".
 * If the latter, we must send a string containing
 * "userid,password<newline>" until either we receive
 * the OK response or our user here gives up.
 */

#include	<sgtty.h>
#include	<stdio.h>
extern	char *dkerror;

/* this better be larger than either login names or passwords */
#define PWDSIZE 16

char *_tdkgetpass();

tdklogin(fd)
{
#	define BFS 2*PWDSIZE + 3	/* <log>,<pswd>\n\0 */
	int	cc;
	struct sgttyb echo, noecho;
	char	buf[BFS];
	char	*rbuf;
	FILE	*tty;

	cc = read(fd, buf, 2) ;
	if (cc <= 0) {
		dkerror = "tdklogin can't read remote system";
		return(-1);
	}
	if (buf[0] == 'O' && buf[1] == 'K')
		return fd ;
	tty = fopen("/dev/tty", "r+");
	if (tty == NULL) {
		dkerror = "tdklogin can't open /dev/tty";
		return -1;
	}
	ioctl(fileno(tty), TIOCGETP, &echo);
	ioctl(fileno(tty), TIOCGETP, &noecho);
	noecho.sg_flags &= ~ECHO;
	write (fileno(tty), "please ", 7) ;
	while (1) {
		rbuf = _tdkgetpass (tty, "login: ");
		if (rbuf == 0) {
			cc = -1;
			break;
		}
		strcpy (buf, rbuf);
		strcat (buf, ",");

		ioctl(fileno(tty), TIOCSETP, &noecho) ;
		rbuf = _tdkgetpass (tty, "Password:");
		ioctl(fileno(tty), TIOCSETP, &echo) ;
		write(fileno(tty), "\n", 1);
		if (rbuf == 0) {
			cc = -2;
			break;
		}
		strcat (buf, rbuf);
		strcat (buf, "\n");

		write(fd, buf, strlen (buf)) ;
		cc = read(fd, buf, 2) ;
		if (cc <= 0)
			break ;
		if (buf[0] == 'O' && buf[1] == 'K')
			break ;
	}
	fclose(tty);
	if (cc <= 0) {
		close(fd);
		dkerror = "tdklogin can't log in";
		return -1;
	}
	return fd;
}


char *
_tdkgetpass(tty, prompt)
FILE *tty;
char * prompt;
{
	char c;
	char *cp;
	static char buf[PWDSIZE + 1];

	write (fileno(tty), prompt, strlen (prompt));

	cp = buf;
	while ((c = getc(tty)) != '\n' && c != EOF && c != '\04') {
		if (cp < &buf[PWDSIZE])
			*cp++ = c;
	}
	*cp = '\0';

	if (buf[0] == '\0' && (c == EOF || c== '\04'))
		return ((char *)0);
	if (buf[0] == '~' && buf[1] == '.')
		return ((char *)0);

	return (buf);
}

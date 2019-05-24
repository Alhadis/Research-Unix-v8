#include <sys/types.h>
#include <sys/stat.h>
#include "mail.h"

/* configuration */
#define LOGFILE "/usr/spool/mail/mail.log"
#define LOGTEMP "/usr/spool/mail/mail.tmp"

/* imports */
void lock();
void unlock();

/* log a mail transmission */
logsend (to, from, at, tag)
	char *to;	/* receiver */
	char *from;	/* the sender */
	char *at;	/* time sent */
	char *tag;	/* type of mail */
{
#	include <grp.h>
	char buf[FROMLINESIZE];
	int out, in;
	long len;
	long lseek();

	lock(LOGFILE);

	/* append to log */
	out = open(LOGFILE, 2);
	len = lseek(out, 0L, 2);
	if (len > 32000) {
		in = out;
		out = creat(LOGTEMP, 0660);
		if (out >= 0 && in >= 0 && lseek(in, -4000L, 2) >= 0) {
			while ((len = read(in, buf, FROMLINESIZE)) > 0)
				(void)write(out, buf, (unsigned int)len);
			close(in);
			close(out);
			unlink(LOGFILE);
			link(LOGTEMP, LOGFILE);
			unlink(LOGTEMP);
		} else {
			close(in);
			close(out);
		}
		out = open(LOGFILE, 2);
	}
	sprintf (buf, "%s %s From %s %s\n", tag, to, from, at);
	(void)write(out, buf, strlen(buf));
	close(out);
	chmod(LOGFILE, 0666);

	unlock ();

}

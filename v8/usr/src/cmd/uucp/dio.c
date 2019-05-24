/*	%W%
*/
#include "uucp.h"
VERSION(%W%);

#ifdef DATAKIT
#include <dk.h>

#define XBUFSIZ 1024
time_t time();
static jmp_buf Dfailbuf;

/*
 * Datakit protocol
 */
static dalarm() {longjmp(Dfailbuf);}
static int (*dsig)();
#ifndef V8
static short dkrmode[3] = { DKR_BLOCK, 0, 0 };
static short dkeof[3] = { 106, 0, 0 };	/* End of File signal */
#endif

/*
 * turn on protocol
 */
dturnon()
{

	dsig=signal(SIGALRM, dalarm);
#ifndef V8
	if(ioctl(Ofn, DIOCRMODE, dkrmode) < 0) {
	    int ret; extern int errno;
	    ret=ioctl(Ofn, DIOCRMODE, dkrmode);
	    DEBUG(4, "dturnon: ret=%d, ", ret);
	    DEBUG(4, "Ofn=%d, ", Ofn);
	    DEBUG(4, "errno=%d\n", errno);
	    return(-1);
	}
#endif !V8
	return(0);
}
dturnoff()
{
	(void) signal(SIGALRM, dsig);
	return(0);
}

/*
 * write message across Datakit link
 *	type	-> message type
 *	str	-> message body (ascii string)
 *	fn	-> Datakit file descriptor
 * return
 *	SUCCESS	-> message sent
 *	FAIL	-> write failed
 */
dwrmsg(type, str, fn)
register char *str;
int fn;
char type;
{
	register char *s;
	char bufr[XBUFSIZ];

	bufr[0] = type;
	s = &bufr[1];
	while (*str)
		*s++ = *str++;
	*s = '\0';
	if (*(--s) == '\n')
		*s = '\0';
	return(write(fn, bufr, (unsigned) strlen(bufr) + 1) < 0 ? FAIL : SUCCESS);
}

/*
 * read message from Datakit link
 *	str	-> message buffer
 *	fn	-> Datakit file descriptor
 * return
 *	FAIL	-> send timed out
 *	SUCCESS	-> ok message in str
 */
drdmsg(str, fn)
register char *str;
{

	register int len;

	if(setjmp(Dfailbuf))
		return(FAIL);

	(void) alarm(60);
	for (;;) {
		if( (len = read(fn, str, XBUFSIZ)) <= 0) {
			(void) alarm(0);
			return(FAIL);
		}
		str += len;
		if (*(str - 1) == '\0')
			break;
	}
	(void) alarm(0);
	return(SUCCESS);
}

/*
 * read data from file fp1 and write
 * on Datakit link
 *	fp1	-> file descriptor
 *	fn	-> Datakit descriptor
 * returns:
 *	FAIL	->failure in Datakit link
 *	SUCCESS	-> ok
 */
dwrdata(fp1, fn)
register FILE *fp1;
{
	register int len, ret;
	long bytes;
	char bufr[XBUFSIZ];
	char text[128];
	time_t	ticks;

	bytes = 0L;
	(void) millitick();	/* set msec timer */
	while ((len = fread(bufr, sizeof (char), XBUFSIZ, fp1)) > 0) {
		bytes += len;
		ret = write(fn, bufr, (unsigned) len);
		if (ret != len) {
			return(FAIL);
		}
		if (len != XBUFSIZ)
			break;
	}
#ifndef V8
	ioctl(fn, DIOCXCTL, dkeof);
#endif
	ret = write(fn, bufr, (unsigned) 0);
	ticks = millitick();
	(void) sprintf(text, "-> %ld / %ld.%.3d secs",
		bytes, ticks / 1000, ticks % 1000);
	DEBUG(4, "%s\n", text);
	syslog(text);
	return(SUCCESS);
}

/*
 * read data from Datakit link and
 * write into file
 *	fp2	-> file descriptor
 *	fn	-> Datakit descriptor
 * returns:
 *	SUCCESS	-> ok
 *	FAIL	-> failure on Datakit link
 */
drddata(fn, fp2)
register FILE *fp2;
{
	register int len;
	long bytes;
	char text[128];
	char bufr[XBUFSIZ];
	time_t ticks;

	bytes = 0L;
	(void) millitick();	/* set msec timer */
	for (;;) {
		len = drdblk(bufr, XBUFSIZ, fn);
		if (len < 0) {
			return(FAIL);
		}
		bytes += len;
		fwrite(bufr, sizeof (char), len, fp2);
		if (len < XBUFSIZ)
			break;
	}
	ticks = millitick();
	(void) sprintf(text, "<- %ld / %ld.%.3d secs",
		bytes, ticks / 1000, ticks % 1000);
	DEBUG(4, "%s\n", text);
	syslog(text);
	return(SUCCESS);
}

/*
 * read block from Datakit link
 * reads are timed
 *	blk	-> address of buffer
 *	len	-> size to read
 *	fn	-> Datakit descriptor
 * returns:
 *	FAIL	-> link error timeout on link
 *	i	-> # of bytes read
 */
drdblk(blk, len,  fn)
register char *blk;
{
	register int i, ret;

	if(setjmp(Dfailbuf))
		return(FAIL);

	for (i = 0; i < len; i += ret) {
		(void) alarm(60);
		if ((ret = read(fn, blk, (unsigned) len - i)) < 0) {
			(void) alarm(0);
			return(FAIL);
		}
		blk += ret;
		if (ret == 0)	/* zero length block contains only EOF signal */
			break;
	}
	(void) alarm(0);
	return(i);
}
#endif DATAKIT

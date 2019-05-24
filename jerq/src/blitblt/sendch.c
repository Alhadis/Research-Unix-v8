#include <jerq.h>

#define sendword(w)	(sendch(w&255),sendch((w>>8)&255))

#define NBUF	246
#define MBUF	256

static unsigned char buffer[MBUF], *bufp=buffer+1; static int nbuf=NBUF; 

sendch(c)
int c;
{
	*bufp++ = c;
	if (--nbuf <= 0) flushch();
}

sendnch(n,str)
register int n; register char *str;
{
	while (n-- > 0) {
		*bufp++ = *str++;
		if (--nbuf <= 0) flushch();
	}
}

flushch()
{
	register c = -1;
	if ((buffer[0]=NBUF+1-nbuf) > 1) {
		while ((c=rcvchar()) < 0) wait(RCV);
		sendnchars(buffer[0],buffer);
	}
	bufp=buffer+1; nbuf=NBUF;
	return c;
}

sendctl(c)
register c;
{
	buffer[0] = 0;
	buffer[1] = c;
	while ((c=rcvchar()) < 0) wait(RCV);
	sendnchars(2,buffer);
	return c;
}

recvch()
{
	register c;
	while ((c=rcvchar()) < 0) wait(RCV);
	return c;
}

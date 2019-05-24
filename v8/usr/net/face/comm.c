#include "fserv.h"
#include "proto.h"

/* communications fd */
int cfd = -1;

connect()
{
	int fd;
	extern int dkp_ld;
	extern char *dialstring;

	fd = tdkdial(dialstring, 0);
	if (fd >= 0) {
		if (dkproto(fd, dkp_ld) < 0) {
			close(fd);
			fd = -1;
		}
	}
	if (fd < 0)
		debug("problem dialing %s", dialstring);
	return (cfd = fd);
}

disconnect()
{
	if (cfd >= 0) {
		close(cfd);
		cfd = -1;
	}
}

statnetf(p)
	netf *p;
{
	return(statbyname(p->name, &p->statb));
}

statbyname(s, stp)
	char *s;
	struct stat *stp;
{
	strcpy(mb.path, s);
	mh.type = DOSTAT;
	mh.len = strlen(mb.path) + 1;
	remcall();
	*stp = mb.statb;
	return(mh.param1);
}

seeknetf(p, off)
	netf *p;
	long off;
{
	p->offset = off;
}

readnetf(p, bp, len)
	netf *p;
	char *bp;
{
	strcpy(mb.path, p->name);
	mh.type = DOREAD;
	mh.param1 = p->offset;
	mh.param2 = len;
	mh.len = strlen(mb.path) + 1;
	remcall();
	if (mh.param1 > 0)
		bcopy(mb.buf, mb.buf+mh.param1, bp);
	return(mh.param1);
}

writenetf(p, bp, len)
	netf *p;
	char *bp;
{
	return -1;
}

remcall()
{
	int len;

	if(cfd<0)
		if (connect()<0) {
			mh.param1 = -1;
			return -1;
		}
	if (write(cfd, &mh, sizeof(mh)) != sizeof(mh)) {
		close(cfd);
		mh.param1 = cfd = -1;
		return -1;
	}
	if (mh.len > 0 && write(cfd, &mb, mh.len) != mh.len) {
		close(cfd);
		mh.param1 = cfd = -1;
		return -1;
	}
	if ((len = read(cfd, &mh, sizeof(mh))) != sizeof(mh)) {
		close(cfd);
		mh.param1 = cfd = -1;
		return -1;
	}
	if (mh.len > 0 && read(cfd, &mb, mh.len) != mh.len) {
		close(cfd);
		mh.param1 = cfd = -1;
		return -1;
	}
	return (len);
}

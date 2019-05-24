#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <dkmgr.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdio.h>
#include "proto.h"

extern int errno;
extern int dkp_ld;
extern int dkmgropen;
extern int dkmgrreply;

fd_set cfds;
#define USAGE "usage: faces server-name"

main(ac, av)
	int ac;
	char *av[];
{
	if (ac != 2) {
		write(2, USAGE, strlen(USAGE));
		exit(1);
	}
	for(;;) {
		detach("/tmp/facesl");
		faceinit("/usr/net/face");
		listen(av[1]);
		debug("restarting");
	}
}

listen(dialstring)
	char *dialstring;
{
	struct mgrmsg *dkmgr(), *mp;
	fd_set rfds;
	int fds, fd;

	FD_ZERO(cfds);
	FD_ZERO(rfds);
	while (1) {

		/* only read if there is a request waiting */
		if (dkmgropen < 0 || FD_ISSET(dkmgropen, rfds)) {
			mp = dkmgr(dialstring, 1);
			if (mp != NULL)
				newclient (mp->m_chan, mp->m_source);
		}

		/* scan for request */
		rfds = cfds;
		if (dkmgropen >= 0)
			FD_SET(dkmgropen, rfds);
		if (select (NOFILE, &rfds, 0, 1000) < 0)
			return;	/* something pretty bad has happened */

		/* send information to clients (if any) */
		for (fd = 0; fd < NOFILE; fd++)
			if (FD_ISSET(fd, rfds) && FD_ISSET(fd, cfds))
				dorequest(fd);
	}
}

/* add client to the list */
newclient(chan, who)
int chan;		/* dkchannel */
char * who;		/* client */
{
	int fd, i, rv;
	char *devname, *dkfilename();

	/* open the line to the client */
	devname = dkfilename(chan);
	fd = open (devname, 2);
	if (fd < 0) {
		dkmgrnak (chan);
		return;
	}
	if (dkproto(fd, dkp_ld) < 0) {
		(void)close (fd);
		dkmgrnak (chan);
		return;
	}
	dkmgrack(chan);

	/* channel is open, add client to fdlist */
	FD_SET(fd, cfds);

	debug("new client %s on fd %d", who, fd);
}

dropclient (fd)
int fd;
{
	/* do our bookkeeping */
	FD_CLR(fd, cfds);
	close (fd);
	debug("drop client on fd %d", fd);
}

dorequest(fd)
{
	int sfd, n;
	struct stat statb;

	if (read(fd, &mh, sizeof(mh)) != sizeof(mh)) {
		debug("bad header on fd %d", fd);
		dropclient(fd);
		return;
	}
	if (mh.len > 0 && read(fd, &mb, mh.len) != mh.len) {
		debug("bad message on fd %d", fd);
		dropclient(fd);
		return;
	}
	switch(mh.type) {
	case DOSTAT:
		mh.param1 = simstat(mb.path, &statb);
		mb.statb = statb;
		mh.len = sizeof(mb.statb);
		reply(fd);
		break;
	case DOREAD:
		sfd = simopen(mb.path, 0);
		if (sfd < 0) {
			replyerror(fd, errno);
			break;
		}
		if (simlseek(sfd, mh.param1, 0) < 0) {
			simclose(sfd);
			replyerror(fd, errno);
			break;
		}
		if((n = simread(sfd, mb.buf, mh.param2)) < 0) {
			simclose(sfd);
			replyerror(fd, errno);
			break;
		}
		simclose(sfd);
		mh.len = n;
		mh.param1 = n;
		reply(fd);
		break;
	default:
		debug("bad request type %d on fd %d", mh.type, fd);
		dropclient(fd);
		break;
	}
		
}

reply(fd)
{
	if (write(fd, &mh, sizeof(mh)) != sizeof(mh)) {
		debug("write error on fd %d", fd);
		dropclient(fd);
		return;
	}
	if (mh.len > 0 && write(fd, &mb, mh.len) != mh.len) {
		debug("write error on fd %d", fd);
		dropclient(fd);
	}
}

replyerror(fd, no)
{
	mh.len = 0;
	mh.param1 = -no;
	reply(fd);
}

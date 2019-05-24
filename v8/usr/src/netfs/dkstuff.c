#include "fserv.h"
#include "dk.h"
#include "dkmgr.h"
#include "sys/ioctl.h"
#include "signal.h"
#include "errno.h"

extern int dkp_ld, rdk_ld;
extern int dkmgrreply;	/* global variable for dkmgrack */
extern int dkmgropen;
char mgrbuf[128];
struct mgrmsg mesg;
char buf[1024];

announce(s)	/* hello world, here we are */
char *s;
{	int i, fd;
persist:
	for(i = 0; i < 5; i++) {
		fd = tdkserv(s, 2);
		if(fd < 0) {
			debug("announce(%s) failed", s);
			sleep(5);
			continue;
		}
		dkmgropen = cntlfd = fd;
		debug("announce %d", cntlfd);
		dkproto(fd, dkp_ld);
		/*fd = open("/dev/dk/dk01", 2);*/
		fd = dkctlchan(2);
		if(fd < 0) {
			perror("dkctlchan");
			break;
		}
		dkmgrreply = ackfd = fd;
		debug("ackfd %d", ackfd);
		FD_SET(cntlfd, active);
		return;
	}
	debug("sleeping 100 and trying again");
	for(i = 4; i < NOFILE; i++)	/* smash some just in case */
		close(fd);
	sleep(100);
	goto persist;
}

newone()
{	int n, i, fd;
	char *p;
	n = read(cntlfd, mgrbuf, sizeof(mgrbuf));
	if(n < 0) {
		perror("select lied about control channel");
		exit(1);
	}
	if(n == 0) {
		debug("0 length read on control channel");
		debug("assuming datakit died, killing children");
		if(chdir("/usr/net") == -1)
			chdir("/etc/net");
		for(i = 0; i < NORMAN; i++)
			if(children[i].pid && children[i].flags == SPLIT)
				kill(children[i].pid, SIGKILL);
		close(cntlfd);
		cntlfd = -1;
		return;
	}
	if(!readmgr())
		return;
	if(!(n = isfriend(mesg.m_source))) {
		debug("not friend, nak");
nakit:
		dkmgrnak(mesg.m_chan, 4);
		return;
	}
	for(i = 0; i < NORMAN; i++)
		if(children[i].who && strcmp(children[i].who, mesg.m_source) == 0) {
			debug("already attached");
			free(children[i].who);
			children[i].who = 0;
			/* some more? */
			if (children[i].pid)
				kill(children[i].pid, SIGKILL);	/* who did we kill?*/
			break;
		}
	/*sprintf(buf, "/dev/dk/dk%02d", mesg.m_chan);*/
	p = (char *) dkfilename(mesg.m_chan);
	if((fd = open(p, 2)) < 0) {
		perror(p);
		goto nakit;
	}
	if (fd >= NORMAN) {
		close(fd);
		debug("too many connections, nak");
		goto nakit;
	}
	if(children[fd].flags) {
		debug("opened %s twice", buf);
		debug("pid %d fd %d flags %d %s\n", children[fd].pid,
			children[fd].fd, children[fd].flags,
			children[fd].who);
		/* who should we believe?  the new one */
		if(children[fd].pid != 0)
			kill(children[fd].pid, SIGKILL);
		/* did that pid exist, and was it us? */
	}
	children[fd].pid = 0;
	children[fd].fd = fd;
	children[fd].flags = UNINIT;
	children[fd].host = n;
	children[fd].lastheard = time(0);
	children[fd].who = malloc(strlen(mesg.m_source) + 1);
	strcpy(children[fd].who, mesg.m_source);
	debug("ok, %s (%d) on fd %d", mesg.m_source, n, fd);
	if((i = dkproto(fd, rdk_ld)) < 0) {
	/*if((i = ioctl(fd, FIOPUSHLD, &rdk_ld)) <0) {*/
		perror("rdk_ld");
		close(fd);
		goto nakit;
	}
	dkmgrack(mesg.m_chan);
	ioctl(fd, FIOPOPLD, 0);
	if((i = dkproto(fd, dkp_ld)) < 0) {
	/*if((i = ioctl(fd, FIOPUSHLD, &dkp_ld)) < 0) {*/
		perror("dkp_ld");
		close(fd);
		goto nakit;		/* clean up.  too late to nak */
	}
	FD_SET(fd, active);
}

#define skip	while(*p != '.' && *p && *p != '\n') p++
readmgr()
{	char *p;
	debug("mgrbuf: %s", mgrbuf);
	p = mgrbuf;
	mesg.m_chan = 0;
	while(*p >= '0' && *p <= '7')
		mesg.m_chan = mesg.m_chan * 8 + *p++ - '0';
	if(mesg.m_chan == 0) {
		debug("incall wanted channel 0");
		return(0);
	}
	while(*p && *p != '\n')
		p++;
	if(*p++ != '\n') {
		debug("bad incall message %s", mgrbuf);
		return(0);
	}
	mesg.m_dial = p;
	mesg.m_service = 0;
	skip;
	if(*p == '.') {
		*p++ = 0;
		mesg.m_service = p;
		while(*p != '\n' && *p)
			p++;
	}
	if(*p == '\n')
		*p++ = 0;
	mesg.m_param1 = 0;
	mesg.m_param2 = 0;
	mesg.m_uid = 0;
	mesg.m_source = 0;
	if(*p == 0) {
		dkmgrnak(mesg.m_chan, 4);
		debug("nak, no source");
		return(0);
	}
	/* two forms for source and uid are:
	 *	uid
	 *	source
	 * and
	 *	source.uid
	 */
	mesg.m_source = p;
	skip;
	if(*p == '.') {
		/* form 2 */
		*p++ = 0;
		mesg.m_uid = p;
	} else {
		/* form 1 */
		*p++ = 0;
		mesg.m_uid = mesg.m_source;
		mesg.m_source = p;
	}
	skip;
	*p++ = 0;
	/* ignore the rest */
	return(1);
}

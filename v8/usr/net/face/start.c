#include "fserv.h"
#include "neta.h"

#define TIMEOUT 2*60	/* 2 minutes */
char cmdbuf[256];
int alive, rdcnt, wrcnt;
long dtime;
struct senda y, nilrcv;
int host;

work()
{	int n;
	struct senda *x;
	fd_set fds;

	debugreset();
	FD_ZERO(fds);
	do {
		FD_SET(myfd, fds);
		if (select(NOFILE, &fds, 0, 10000) <= 0) {
			if (time((long*)0) - dtime > TIMEOUT)
				disconnect();
			continue;
		}
		dtime = time((long *)0);
		n = read(myfd, cmdbuf, sizeof(cmdbuf));
		if(n < 0) {
			debug("read -1 on %d", myfd);
			leave(2);
		}
		if(n != sizeof(struct senda)) {
			debug("read %d wanted %d", n, sizeof(struct senda));
			leave(3);
		}
		x = (struct senda *)cmdbuf;
		errno = 0;
		y = nilrcv;
		prcmd(x);
		switch(x->cmd) {
		default:
			debug("unk cmnd %d", n);
			leave(4);
		case NSTAT:
			dostat(x);
			break;
		case NWRT:
			dowrite(x);
			break;
		case NREAD:
			doread(x);
			break;
		case NFREE:
			dofree(x);
			break;
		case NTRUNC:
			dotrunc(x);
			break;
		case NUPDAT:
			doupdat(x);
			break;
		case NGET:
			doget(x);
			break;
		case NNAMI:
			donami(x);
			break;
		case NPUT:
			doput(x);
			break;
		}
	} while(alive > 0);
	leave(0);
}

leave(n)
{	int i;
	debug("leaving(%d)", n);
	for(i = 0; i < dptr; i++) {
		strcat(debugbuf[i], "\n");
		write(dbgfd, debugbuf[i], strlen(debugbuf[i]));
	}
	exit(n);
}

extern netf netftab[];
extern int nnetf;
dumpstate()
{	struct senda *x = (struct senda *)cmdbuf;
	netf *p;
	int i;
	debug("\tmesg: ver %d flags %d trannum %d\n\tuid %d gid %d dev 0x%x tag %d mode 0%o",
	x->version, x->flags, x->trannum, x->uid, x->gid, x->dev, x->tag, x->mode);
	debug("\t\tino %d count %d offset %d", x->ino, x->count, x->offset);
	prcmd(x);
	for(i = 0; i < ndev; i++)
		debug("\tdev: ours 0x%x his 0x%x", devtab[i].ours,
			devtab[i].his);
	for(p = netftab, i = 0; i < nnetf; i++, p++) {
		debug("\tnetf %d dev 0x%x ino %d %s",
			p->tag, p->dev, p->ino,
			p->name? p->name: "(null)");
		debug("\tstat dev 0x%x ino %d mode 0%o nlink %d size %d",
			p->statb.st_dev, p->statb.st_ino, p->statb.st_mode,
			p->statb.st_nlink, p->statb.st_size);
		debug("\t\tuid %d gid %d ctime %s\t\t", p->statb.st_uid,
			p->statb.st_gid, ctime(&p->statb.st_ctime));
	}
}

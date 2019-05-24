#include "fserv.h"
#include "sys/neta.h"

char cmdbuf[256];
int alive, rdcnt, wrcnt, dtime;
int rdnum;
struct senda y, nilrcv;
int host;

work()
{	int n;
	struct senda *x;
	dtime = children[myfd].dtime;
	host = children[myfd].host;
	silent = children[myfd].silent;
	debugreset();
	do {
		/* would select be better? */
		n = read(myfd, cmdbuf, sizeof(cmdbuf));
		if(n < 0) {
			debug("read -1 on %d for %s", myfd, children[myfd].who);
			leave(2);
		}
		if(n != sizeof(struct senda)) {
			debug("read %d wanted %d for %s", n, sizeof(struct senda),
				children[myfd].who);
			leave(3);
		}
		rdnum++;
		if(rdnum % 20 == 4)
			permredo();	/* every 10 minutes when idle */
		x = (struct senda *)cmdbuf;
		errno = 0;
		y = nilrcv;
		prcmd(x);
		switch(x->cmd) {
		default:
			debug("unk cmnd %d for %s", n, children[myfd].who);
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
	debug("\t%s: fl %d dtime %d dev 0x%x host %d",
		children[myfd].who, children[myfd].flags, children[myfd].dtime,
		children[myfd].dev, children[myfd].host);
	for(i = 0; i < ndev; i++)
		debug("\tdev: ours 0x%x his 0x%x", devtab[i].ours,
			devtab[i].his);
	for(p = netftab, i = 0; i < nnetf; i++, p++) {
		debug("\tnetf %d dev 0x%x ino %d how %d fd %d %s",
			p->tag, p->dev, p->ino, p->how, p->fd,
			p->name? p->name: "(null)");
		debug("\tstat dev 0x%x ino %d mode 0%o nlink %d size %d",
			p->statb.st_dev, p->statb.st_ino, p->statb.st_mode,
			p->statb.st_nlink, p->statb.st_size);
		debug("\t\tuid %d gid %d ctime %s\t\t", p->statb.st_uid,
			p->statb.st_gid, ctime(&p->statb.st_ctime));
	}
}

#include "fserv.h"
#include "errno.h"
#include "neta.h"
extern char *buf, *nbuf;
extern int myfd, wrcnt, rdcnt;
extern struct rcva y, nilrcv;

doput(x)
struct senda *x;
{
	y.trannum = x->trannum;
	if(x->tag == 0) {
		debug("put got 0");
		respond(EIO);
		dumpstate();
		return;
	}
	clrnetf(x->tag);
	respond(0);
}

doget(x)
struct senda *x;
{	netf *p;
	y.trannum = x->trannum;
	p = getnetf(x->dev, x->ino);
	if(p == 0) {
		respond(EIO);
		return;
	}
	debug2("\tget %d %d 0%o %d %d %s\n", p->dev, p->ino,
		p->statb.st_mode, p->statb.st_uid, p->statb.st_gid, p->name);
	y.mode = p->statb.st_mode;
	y.tag = p->tag;
	y.nlink = p->statb.st_nlink;
	y.uid = hostuid(&p->statb);
	y.gid = hostgid(&p->statb);
	y.size = p->statb.st_size;
	respond(0);
}

dofree(x)		/* means rmt thinks nlink == 0 */
struct senda *x;
{
	y.trannum = x->trannum;
	respond(0);
}

doupdat(x)
struct senda *x;
{	netf *p;
	int i;

	y.trannum = x->trannum;
	p = gettag(x->tag);
	if(p == 0) {
		respond(EIO);
		return;
	}
	debug2("\tupdat 0%o %d %d\n", p->statb.st_mode, p->statb.st_uid, p->statb.st_gid);
	debug2("\t\t0%o %d %d\n", x->mode, x->uid, x->gid);
	if(p->name == 0) {	/* of course this can't happen */
		debug("\tname 0 tag %d\n", p->tag);
		respond(EIO);
		dumpstate();
		return;
	}
	y.mode = p->statb.st_mode;
	y.nlink = p->statb.st_nlink;
	y.uid = hostuid(&p->statb);
	y.gid = hostgid(&p->statb);
	y.size =  p->statb.st_size;
	if(hisdev(p->statb.st_dev) != p->dev || p->ino != p->statb.st_ino) {
		debug("\tdev oops\n");
		dumpstate();
	}
	respond(0);
}

doread(x)
struct senda *x;
{	netf *p;
	int n, offset = 0;

	y.trannum = x->trannum;
	p = gettag(x->tag);
	if(p == 0) {
		respond(EIO);
		return;
	}
	if(seeknetf(p, x->offset) < 0)
		perror("\tread lseek");
	getbuf(x->count);
	n = readnetf(p, buf, x->count);
	debug1("\tread got %d wanted %d\n", n, x->count);
	if(n < 0) {
		y.count = n;
		respond(EIO);
		return;
	}
	y.count = n - offset;
	respond(0);
	(void) write(myfd, buf + offset, (unsigned) n - offset);
	wrcnt += n;
}

dowrite(x)
struct senda *x;
{	netf *p;
	int n;

	y.trannum = x->trannum;
	if(x->count == 0)
		respond(EACCES);
	getbuf(x->count);
	if((n = read(myfd, buf, x->count)) != x->count) {
		debug("\twrite expected %d got %d\n", x->count, n);
		rdcnt += n;
		respond(EIO);
		dumpstate();
		return;
	}
	respond(EACCES);
}

dotrunc(x)
struct senda *x;
{
	y.trannum = x->trannum;
	respond(0);
}

dostat(x)
struct senda *x;
{	netf *p;

	y.trannum = x->trannum;
	debug2("\tstat %d\n", x->tag);
	p = gettag(x->tag);
	if(p == 0) {
		respond(EIO);
		return;
	}
	debug2("\tlstat %s ", p->name);
	debug2("0%o %d %d\n", p->statb.st_mode, p->statb.st_uid, p->statb.st_gid);
	y.mode = p->statb.st_mode;
	y.nlink = p->statb.st_nlink;
	y.uid = hostuid(&p->statb);
	y.gid = hostgid(&p->statb);
	debug2("\thostuid(%d) = %d\n", p->statb.st_uid, y.uid);
	debug2("\tgid %d\n", y.gid);
	y.size = p->statb.st_size;
	y.tm[0] = p->statb.st_atime;
	y.tm[1] = p->statb.st_mtime;
	y.tm[2] = p->statb.st_ctime;
	if(hisdev(p->statb.st_dev) != p->dev || p->ino != p->statb.st_ino) {
		debug("\toops dev or ino\n");
		dumpstate();
	}
	respond(0);
}

donami(x)
struct senda *x;
{	netf *p, *q;
	int fd, i, nlink = 0;

	y.trannum = x->trannum;
	getbuf(x->count);
	if((i = read(myfd, buf, x->count)) != x->count) {
		rdcnt += i;
		respond(EIO);
		return;
	}
	rdcnt += i;
	p = gettag(x->tag);
	if(p == 0) {
		respond(EIO);
		return;
	}
	debug1("\tcdir %s 0%o %d %d\n", p->name, p->statb.st_mode,
		p->statb.st_uid, p->statb.st_gid);
	if(ftype(p) != S_IFDIR) {
		respond(ENOTDIR);
		return;
	}
	if(noaccess(x, &p->statb, S_IEXEC)) {
		respond(EACCES);
		return;
	}
	fixnbuf(p->name, x->count, buf, x->flags);
	debug2("\tnami %s (%d)\n", nbuf, x->flags);
	q = newnetf(nbuf);
	if(q != 0) {
		debug2("\tfound\n");
		errno = 0;
		y.ino = q->ino;
		y.tag = q->tag;
		y.dev = q->dev;
		y.mode = q->statb.st_mode;
		y.nlink = q->statb.st_nlink;
		y.uid = hostuid(&q->statb);
		y.gid = hostgid(&q->statb);
		y.size = q->statb.st_size;
		if(isroot(q) && strncmp(buf, ".", x->count)) {
			debug2("NROOT %s\n", buf);
			y.flags = NROOT;
		}
		if(x->flags == NDEL) {
			debug1("\tnodelete\n");
			respond(EPERM);
			return;
		}
		respond(0);
		return;
	} else {
		y.flags = NOMATCH;
		if(x->flags == NLINK) {
			debug1("\tnolink %d %d %d\n", x->uid,
				p->statb.st_uid, p->statb.st_gid);
			respond(EPERM);
			return;
		} else if(x->flags == NCREAT) {
			debug1("\tnami creat 0%o %s\n", x->mode, nbuf);
			debug1("\tnoaccess\n");
			respond(EACCES);
			return;
		}
		respond(ENOENT);
	}
}

#include "fserv.h"
#include "errno.h"
#include "sys/neta.h"
extern char *buf, *nbuf;
extern int dtime, myfd, wrcnt, rdcnt, lflag;
extern struct rcva y, nilrcv;

doput(x)
struct senda *x;
{
	y.trannum = x->trannum;
	if(x->tag == 0) {
		debug("\tput got 0");
		respond(EIO);
		dumpstate();
		leave(9);
	}
	clrnetf(x->tag);
	respond(errno);
}

doget(x)
struct senda *x;
{	netf *p;
	y.trannum = x->trannum;
	p = getnetf(x->dev, x->ino);
	if(p == 0) {
		debug("\tget didn't %d %d", x->dev, x->ino);
		respond(errno);
		dumpstate();
		return;
	}
	if(p->fd == -1 && xstat(p->name, &p->statb) < 0
		|| p->fd != -1 && fstat(p->fd, &p->statb) < 0) {
		respond(errno);
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
		respond(errno);
		return;
	}
	debug2("\tupdat 0%o %d %d\n", p->statb.st_mode, p->statb.st_uid, p->statb.st_gid);
	debug2("\t\t0%o %d %d\n", x->mode, x->uid, x->gid);
	x->ta += dtime;
	x->tm += dtime;
	if(p->name == 0) {	/* of course this can't happen */
		debug("\tname 0 tag %d\n", p->tag);
		respond(EIO);
		dumpstate();
		return;
	}
	debug2("\t%s\n", p->name);
	if(x->ta == dtime)
		x->ta = p->statb.st_atime;
	if(x->tm == dtime)
		x->tm = p->statb.st_mtime;
	utime(p->name, &x->ta);
	errno = 0;	/* p->name may be wrong, so times off [bug] */
	if(x->mode != p->statb.st_mode) {
		i = 1;
		if(isowner(x, &p->statb))
			fchmod(p->fd, x->mode);
		else
			errno = EPERM;
		perror("\tfchmod");
	}
	if(x->uid == 0 && (hostuid(&p->statb) != x->newuid
		|| hostgid(&p->statb) != x->newgid)) {
		i = 1;
		if(isowner(x, &p->statb))
			fchown(p->fd, myuid(x->newuid, x->newgid),
				mygid(x->newuid, x->newgid));
		else
			errno = EPERM;
		if(errno)
			perror("\tfchown");
		debug1("\tupdat %d %d\n", p->statb.st_uid, p->statb.st_gid);
	}
	if(i)
		fstat(p->fd, &p->statb);
	y.mode = p->statb.st_mode;
	y.nlink = p->statb.st_nlink;
	y.uid = hostuid(&p->statb);
	y.gid = hostgid(&p->statb);
	y.size =  p->statb.st_size;
	if(hisdev(p->statb.st_dev) != p->dev || p->ino != p->statb.st_ino) {
		debug("\tdev oops\n");
		dumpstate();
	}
	respond(errno);
}

doread(x)
struct senda *x;
{	netf *p;
	int n, offset = 0;
	y.trannum = x->trannum;
	p = gettag(x->tag);
	if(p == 0) {
		if(errno == 0)
			errno = EIO;
		respond(errno);
		return;
	}
	if(ftype(p) == S_IFLNK) {
		n = readlink(p->name, buf, x->count);
		offset = x->offset;
		debug2("readlink %s %d %d\n", p->name, n, x->count);
		offset = x->offset;
		if(n < offset)
			offset = n;
	}
	else {
		if(opennf(p, 0)) {
			respond(errno);
			return;
		}
		if(lseek(p->fd, x->offset, 0) < 0)
			perror("\tread lseek");
		getbuf(x->count);
		n = read(p->fd, buf, x->count);
	}
	debug1("\tread got %d wanted %d\n", n, x->count);
	if(n < 0) {
		y.errno = errno;
		y.count = n;
		respond(0);
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
		respond(0);
	getbuf(x->count);
	if((n = read(myfd, buf, x->count)) != x->count) {
		debug("\twrite expected %d got %d\n", x->count, n);
		rdcnt += n;
		if(errno == 0)
			errno = EIO;
		respond(errno);
		dumpstate();
		return;
	}
	wrcnt += n;
	debug1("\twrite %d\n", x->count);
	p = gettag(x->tag);
	if(p == 0) {
		respond(errno);
		return;
	}
	if(opennf(p, 1)) {
		respond(errno);
		return;
	}
	n = lseek(p->fd, x->offset, 0);
	if(n < 0) {
		debug("\twrite lseek %d\n", errno);
		dumpstate();
	}
	if((n = write(p->fd, buf, x->count)) != x->count) {
		debug("\twrite failed %d %d\n", n, p->fd);
		if(errno == 0)
			errno = EIO;
		dumpstate();
	}
	respond(errno);
}

dotrunc(x)
struct senda *x;
{
	y.trannum = x->trannum;
	truncnf(x->tag);
	respond(errno);
}

dostat(x)
struct senda *x;
{	netf *p;
	y.trannum = x->trannum;
	debug2("\tstat %d\n", x->tag);
	p = gettag(x->tag);
	if(p == 0) {
		respond(errno);
		return;
	}
	debug2("\tlstat %s ", p->name);
	if(p->fd == -1 && xstat(p->name, &p->statb) < 0
		|| p->fd != -1 && fstat(p->fd, &p->statb) < 0) {
		respond(errno);
		return;
	}
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
		if(errno == 0)
			errno = EIO;
		respond(errno);
		return;
	}
	rdcnt += i;
	p = gettag(x->tag);
	if(p == 0) {
		respond(errno);
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
again:
	q = 0;
	if(x->flags == NDEL)
		q = oldnetf(nbuf);
	if(q == 0)
		q = newnetf(nbuf, -1, 0);
	if(q != 0) {
		debug2("\tfound\n");
		errno = 0;
		if(lflag && ftype(q) == S_IFLNK)
			if(lcllink(q)) {
				if(nlink++ > 4) {
					errno = ELOOP;
					goto bad;
				}
				goto again;
			}
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
			if(noaccess(x, &p->statb, S_IWRITE)) {
				debug1("\tnodelete\n");
				respond(EPERM);
				return;
			}
			if(y.dev != x->dev)
				errno = EBUSY;
			if(y.flags == NROOT)
				errno = EPERM;
			(void) unlink(nbuf);
			debug1("\tunlink %s %d\n", nbuf, errno);
		}
		if(x->flags == NCREAT && noaccess(x, &q->statb, S_IWRITE)) {
			debug1("\tnocreat\n");
			respond(EPERM);
			return;
		}
		respond(errno);
		return;
	}
	else {
		y.flags = NOMATCH;
		if(x->flags == NLINK) {
			if(noaccess(x, &p->statb, S_IWRITE)) {
				debug1("\tnolink %d %d %d\n", x->uid,
					p->statb.st_uid, p->statb.st_gid);
				respond(EPERM);
				return;
			}
			q = gettag(x->tag);
			p = getnetf(x->dev, x->ino);
			if(q == 0 || p == 0) {
				debug1("\tno link p 0x%x q 0x%x", p, q);
				respond(EXDEV);
				return;
			}
			debug1("\tlink %d %d %d %s %s",
				x->dev, x->ino, q->dev, p->name, nbuf);
			if(q->dev != p->dev)
				errno = EXDEV;
			i = link(p->name, nbuf);
			if(i != -1)
				errno = 0;
			else
				debug("\tlink %d", errno);
		}
		else if(x->flags == NCREAT) {
			debug1("\tnami creat 0%o %s\n", x->mode, nbuf);
			if(noaccess(x, &p->statb, S_IWRITE)) {
				debug1("\tnoaccess\n");
				respond(EACCES);
				return;
			}
			switch(x->mode & S_IFMT) {
			case 0:
			case S_IFREG:
				fd = creat(nbuf, x->mode);
				if(fd != -1) {
					q = newnetf(nbuf, fd, 1);
					y.ino = q->ino;
					y.dev = q->dev;
					errno = 0;
					chown(nbuf, myuid(x->uid, x->gid),
						mygid(x->uid, x->gid));
					debug1("\tchown %d %d\n",
						myuid(x->uid, x->gid),
						mygid(x->uid, x->gid));
					(void) fstat(fd, &q->statb);
				}
				break;
			case S_IFDIR:
				if(x->uid != 0) {
					debug1("\tsuser %d\n", x->uid);
					respond(EPERM);
					return;
				}
				i = mknod(nbuf, x->mode, 0);
				if(i != -1) {
					q = newnetf(nbuf, -1, 0);
					y.ino = q->ino;
					y.dev = q->dev;
					errno = 0;
					chown(nbuf, myuid(x->uid, x->gid),
						mygid(x->uid, x->gid));
					(void) fstat(q->fd, &q->statb);
					debug1("\tmknod 0%o %d %d\n",
						q->statb.st_mode, q->statb.st_uid, q->statb.st_gid);
				}
				else
					debug1("\tmknod\n");
				break;
			case S_IFLNK:
				errno = 0;
				fd = creat(nbuf, 0777);
				if(fd < 0)
					break;
				chown(nbuf, myuid(x->uid, x->gid),
					mygid(x->uid, x->gid));
				i = fchmod(fd, x->mode | S_IFLNK);
				debug1("\tfchmod(%s) %d errno %d\n", nbuf, i, errno);
				if(errno == 0) {
					debug2("\tfd = %d\n", fd);
					q = newnetf(nbuf, fd, 1);
					y.ino = q->ino;
					y.dev = q->dev;
				}
				break;
			default:
				errno = EPERM;
				break;
			}
		}
	}
bad:
	y.tag = q->tag;
	y.nlink = q->statb.st_nlink;
	y.size = q->statb.st_size;
	y.uid = hostuid(&q->statb);
	y.gid = hostuid(&q->statb);
	respond(errno);
}

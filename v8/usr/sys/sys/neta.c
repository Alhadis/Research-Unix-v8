#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/inode.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/stat.h"
#include "../h/neta.h"
#include "../h/mount.h"
#include "../h/buf.h"
#include "../h/stream.h"
#include "../h/trace.h"
#include "../h/file.h"

extern struct stdata *stenter();
struct senda nilx;
#define ND 100
struct {
	char len, s, n;
	struct senda x[ND];
	struct rcva y[ND];
} netabuf = {ND};
#define addx(z) netabuf.x[netabuf.n] = *z; netabuf.s = 1;
#define addy(z) netabuf.y[netabuf.n++] = *z; if(netabuf.n >= ND) netabuf.n = 0; netabuf.s = 0;

namount()
{	struct a {
		int fstyp;
		int unqname;
		int flag;	/* 0 = mount, 1 = unmount */
		int cfd;
		char *freg;
	} *uap = (struct a *)u.u_ap;

	if(!suser())
		return;
	if(minor(uap->unqname)) {	/* minor device numbers used by server */
		u.u_error = ENXIO;
		return;
	}
	if (uap->flag)
		naunmount(uap->fstyp, uap->unqname);
	else
		nadomount(uap->fstyp, uap->unqname, uap->cfd, uap->freg);
}

nadomount(fstyp, dev, cfd, freg)
	dev_t dev;
	char *freg;
{
	struct inode *cip, *dip;
	struct mount *mp;
	struct file *fp;

	/* convert cfd to the inode with the stream */
	fp = getf(cfd);
	if(u.u_error)
		return;
	cip = fp->f_inode;
	if(cip->i_sptr == NULL) {
		u.u_error = ENXIO;
		return;
	}
	/* convert the mt point to an inode */
	u.u_dirp = (caddr_t)freg;
	dip = namei(uchar, 0, 1);
	if(dip == NULL) {
		u.u_error = EBUSY;
		return;
	}
	if((dip->i_mode & IFMT) != IFDIR) {
		iput(dip);
		u.u_error = EBUSY;
		return;
	}
	/* find a mount struct */
	mp = allocmount(fstyp, dev);
	if (mp == NULL) {
		iput(dip);
		u.u_error = EBUSY;
		return;
	}
	mp->m_inodp = dip;
	mp->m_idev = cip;
	dip->i_flag |= IMOUNT;
	cip->i_count++;	
	cip->i_un.i_key = 0;
	prele(cip);
	prele(dip);
}

naunmount(fstyp, dev)
	int fstyp;
	dev_t dev;
{
	struct inode *ip;
	struct mount *mp;		/* should we send unmount mesg? */

	mp = findmount(fstyp, dev);
	if (mp == NULL) {
		u.u_error = EINVAL;
		return;
	}
	xumount(dev);	/* shared text from remote root */
	for(ip = inode; ip < inodeNINODE; ip++)
		if(ip->i_number != 0 && ip->i_fstyp == fstyp
			&& major(ip->i_dev) == major(dev)) {
				xumount(ip->i_dev);	/* others, one at a time */
				u.u_error = EBUSY;
				return;
			}
	ip = mp->m_inodp;
	ip->i_flag &= ~IMOUNT;
	plock(ip);
	iput(ip);
	if(mp->m_idev)
		stclose(mp->m_idev, 1);
	iput(mp->m_idev);
	freemount(mp);
}

naput(ip)
struct inode *ip;
{	struct senda x;
	struct rcva y;

	trace(NPUT, ip->i_number, trannum);
	if(ip->i_flag & ICHG)
		naupdat(ip, &time, &time, 0);
	x = nilx;
	x.trannum = trannum;
	x.cmd = NPUT;
	x.uid = u.u_uid;
	x.tag = ip->i_un.i_tag;
	x.dev = ip->i_dev;
	x.ino = ip->i_number;
	send(ip->i_un.i_cip, &x, &y);
	if(y.errno)
		u.u_error = y.errno;
}

struct inode *
naget(fstyp, dev, ino, ip)
dev_t dev;
struct inode *ip;
{	struct mount *mp;
	struct senda x;
	struct rcva y;

	if(ino == 0) {	/* this must be a disaster of some sort */
		if(!u.u_error)
			u.u_error = EMFILE;	/* just in case for nanami */
		printf("naget ino 0 dev %d\n", ip->i_dev);
		iput(ip);
		return(0);
	}
	trace(NGET, ip->i_number, trannum);
	for(mp = mount; mp < mount + NMOUNT; mp++) {
		if(mp->m_fstyp != fstyp)
			continue;
		if(mp->m_dev != (dev & ~0xff))	/* minor devs used by server */
			continue;
		ip->i_un.i_cip = mp->m_idev;
		ip->i_dev = dev;
		goto found;
	}
	u.u_error = EMFILE;
	iput(ip);
	return(0);
found:
	x = nilx;
	x.trannum = trannum;
	x.cmd = NGET;
	x.uid = u.u_uid;
	x.dev = dev;
	x.ino = ino;
	send(ip->i_un.i_cip, &x, &y);
	if(y.errno == 0) {
		ip->i_mode = y.mode;
		ip->i_un.i_tag = y.tag;
		ip->i_nlink = y.nlink;
		ip->i_size = y.size;
		ip->i_uid = y.uid;
		ip->i_gid = y.gid;
		return(ip);
	}
	iput(ip);
	u.u_error = y.errno;
	return(0);
}

nafree(ip)
struct inode *ip;
{	struct senda x;
	struct rcva y;

	trace(NFREE, ip->i_number, trannum);
	x = nilx;
	x.trannum = trannum;
	x.cmd = NFREE;
	x.uid = u.u_uid;
	x.tag = ip->i_un.i_tag;
	x.dev = ip->i_dev;
	x.ino = ip->i_number;
	send(ip->i_un.i_cip, &x, &y);
	if(y.errno)
		u.u_error = y.errno;
}

/* this is used by CREAT */
naupdat(ip, ta, tm, waitfor)
struct inode *ip;
time_t *ta, *tm;
{	struct senda x;
	struct rcva y;

	trace(NUPDAT, ip->i_number, trannum);
	x = nilx;
	x.trannum = trannum;
	x.cmd = NUPDAT;
	x.uid = u.u_uid;
	x.gid = u.u_gid;
	x.newuid = ip->i_uid;
	x.newgid = ip->i_gid;
	x.mode = ip->i_mode;
	x.tag = ip->i_un.i_tag;
	x.dev = ip->i_dev;
	x.ino = ip->i_number;
	if(ip->i_flag & IACC)
		x.ta = *ta;
	else
		x.ta = 0;
	if(ip->i_flag & ICHG)
		x.tm = *tm;
	else
		x.tm = 0;
	send(ip->i_un.i_cip, &x, &y);
	if(y.errno) {
		u.u_error = y.errno;
		return;
	}
	ip->i_mode = y.mode;
	ip->i_nlink = y.nlink;
	ip->i_size = y.size;
	ip->i_uid = y.uid;
	ip->i_gid = y.gid;
	ip->i_flag &= ~(IUPD|IACC|ICHG);
}

naread(ip)
struct inode *ip;
{	struct senda x;
	struct rcva y;
	struct buf *bp;
	int n;

	trace(NREAD, ip->i_number, trannum);
	bp = geteblk();
	clrbuf(bp);	/* could use user's buffer */
	x = nilx;
	x.trannum = trannum;
	x.cmd = NREAD;
	x.uid = u.u_uid;
	x.tag = ip->i_un.i_tag;
	x.dev = ip->i_dev;
	x.ino = ip->i_number;
	x.buf = bp->b_un.b_addr;
	do {
		n = u.u_count;
		if(n > BUFSIZE)
			n = BUFSIZE;
		x.count = n;
		x.offset = u.u_offset;
		send(ip->i_un.i_cip, &x, &y);
		if((n = y.count) > 0) {
			if(u.u_segflg != 1) {
				if(copyout(bp->b_un.b_addr, u.u_base, n)) {
					u.u_error = EFAULT;
					break;
				}
			}
			else
				bcopy(bp->b_un.b_addr, u.u_base, n);
			u.u_base += n;
			u.u_offset += n;
			u.u_count -= n;
		}
		if(y.errno)
			u.u_error = y.errno;
	} while(u.u_error == 0 && u.u_count != 0 && n > 0);
	brelse(bp);
}
nawrite(ip)
struct inode *ip;
{	struct senda x;
	struct rcva y;
	struct buf *bp;
	int n;

	trace(NWRT, ip->i_number, trannum);
	bp = geteblk();
	x = nilx;
	x.trannum = trannum;
	x.cmd = NWRT;
	x.uid = u.u_uid;
	x.tag = ip->i_un.i_tag;
	x.dev = ip->i_dev;
	x.ino = ip->i_number;
	x.buf = bp->b_un.b_addr;
	do {
		n = u.u_count;
		if(n > BUFSIZE)		/* should be bufsiz, but ... */
			n = BUFSIZE;
		if(u.u_segflg != 1) {
			if(copyin(u.u_base, bp->b_un.b_addr, n)) {
				u.u_error = EFAULT;
				break;
			}
		}
		else
			bcopy(u.u_base, bp->b_un.b_addr, n);
		x.count = n;
		x.offset = u.u_offset;
		send(ip->i_un.i_cip, &x, &y);
		if(y.errno) {
			u.u_error = y.errno;
			break;
		}
		ip->i_flag |= IUPD|ICHG;
		u.u_count -= n;
		u.u_offset += n;
		u.u_base += n;
	} while(u.u_error == 0 && u.u_count != 0);
	brelse(bp);
}

natrunc(ip)
struct inode *ip;
{	struct senda x;
	struct rcva y;

	trace(NTRUNC, ip->i_number, trannum);
	x = nilx;
	x.trannum = trannum;
	x.cmd = NTRUNC;
	x.uid = u.u_uid;
	x.tag = ip->i_un.i_tag;
	x.dev = ip->i_dev;
	x.ino = ip->i_number;
	send(ip->i_un.i_cip, &x, &y);
	if(y.errno)
		u.u_error = y.errno;
}

nastat(ip, ub, adj)
struct inode *ip;
struct stat *ub;
off_t adj;
{	struct senda x;
	struct rcva y;
	struct stat ds;

	trace(NSTAT, ip->i_number, trannum);
	x = nilx;
	x.trannum = trannum;
	x.cmd = NSTAT;
	x.uid = u.u_uid;
	x.tag = ip->i_un.i_tag;
	x.dev = ip->i_dev;
	x.ino = ip->i_number;
	send(ip->i_un.i_cip, &x, &y);
	if(y.errno) {
		u.u_error = y.errno;
		return;
	}
	ds.st_dev = ip->i_dev;
	ds.st_ino = ip->i_number;
	ip->i_mode = ds.st_mode = y.mode;
	ip->i_nlink = ds.st_nlink = y.nlink;
	ip->i_uid = ds.st_uid = y.uid;
	ip->i_gid = ds.st_gid = y.gid;
	ds.st_size = y.size - adj;
	ds.st_atime = y.tm[0];
	ds.st_mtime = y.tm[1];
	ds.st_ctime = y.tm[2];
	if(copyout((caddr_t)&ds, (caddr_t)ub, sizeof(ds)) < 0)
		u.u_error = EFAULT;
}

/* a lot like fsnami */
nanami(p, pflagp, follow)
struct nx *p;
struct argnamei **pflagp;
{	register struct inode *dp, *dip;
	register char *cp;
	register struct buf *bp, *nbp;
	register struct direct *ep;
	int i, fstyp;
	dev_t d;
	ino_t ino;
	struct argnamei *flagp = *pflagp;
	struct senda x;
	struct rcva y;

	trace(NNAMI, 0, trannum);
	cp = p->cp;
	dp = p->dp;
	x = nilx;
	x.cmd = NNAMI;
	x.uid = u.u_uid;
	x.gid = u.u_gid;

	/*
	 * dp must be a directory and
	 * must have X permission.
	 * cp is a path name relative to that directory.
	 */

dirloop:
	if((dp->i_mode&IFMT) != IFDIR)
		u.u_error = ENOTDIR;
	(void) access(dp, IEXEC);
	for (i=0; *cp!='\0' && *cp!='/'; i++) {
		if (i >= DIRSIZ) {
			u.u_error = ENOENT;
			break;
		}
		u.u_dbuf[i] = *cp++;
	}
	if(u.u_error)
		goto out;
	while (i < DIRSIZ)
		u.u_dbuf[i++] = '\0';
	if (u.u_dbuf[0] == '\0') {		/* null name, e.g. "/" or "" */
		if (flagp) {
			u.u_error = ENOENT;
			goto out;
		}
		goto out1;
	}
	u.u_segflg = 1;

	/* send off the request, get back ino, dev, flagp stuff */
	x.trannum = trannum;
	x.tag = dp->i_un.i_tag;
	x.dev = dp->i_dev;
	x.ino = dp->i_number;
	x.count = DIRSIZ;
	x.buf = u.u_dbuf;
	while(*cp == '/')
		cp++;
	if(flagp) {
		if(*cp != 0)
			goto noflag;
		switch(flagp->flag) {
		case NI_DEL:
			x.flags = NDEL;
			break;
		case NI_LINK:
			x.flags = NLINK;
			x.dev = flagp->idev;
			x.ino = flagp->ino;
			break;
		case NI_CREAT:
		case NI_NXCREAT:
			x.flags = NCREAT;
			x.mode = flagp->mode & ~u.u_cmask;
			break;
		}
	}
noflag:
	send(dp->i_un.i_cip, &x, &y);
	if(y.errno) {
		u.u_error = y.errno;
		goto out;
	}
	if(y.flags == NOMATCH)
		goto nomatch;
	u.u_dent.d_ino = y.ino;
	d = y.dev;

	if(flagp && flagp->flag == NI_DEL && *cp == 0) {
		/* delete the entry, server did all but xrele */
		if(dp->i_number == u.u_dent.d_ino)	/* for '.' */
			goto out;
		else
			dip = ifind(d, u.u_dent.d_ino, dp->i_fstyp);
		if(dip == NULL)
			goto out;
		if(u.u_dent.d_ino == 0)
			printf("nanami ino = 0 before iget\n");
		else {
			dip = iget(d, u.u_dent.d_ino, dp->i_fstyp);	/* to lock it */
			if(dip->i_flag&ITEXT)
				xrele(dip);		/* free busy text */
			dip->i_nlink--;
			dip->i_flag |= ICHG;
			iput(dip);
		}
		goto out;
	}
	/*
	 * Special handling for ".."
	 */
	ino = dp->i_number;
	fstyp = dp->i_fstyp;

	if(y.flags == NROOT) {	/* popped out of net fs */
		if (dp == u.u_rdir)
			u.u_dent.d_ino = dp->i_number;
		else if (u.u_dent.d_ino==ROOTINO && dp->i_number == ROOTINO) {
			for(i=1; i<NMOUNT; i++)
				if(mount[i].m_fstyp == fstyp &&
				   mount[i].m_dev == dp->i_dev) {
					iput(dp);
					dp = mount[i].m_inodp;
					plock(dp);
					dp->i_count++;
					/* was there always .. there? */
					if(*cp && cp < p->nbp->b_un.b_addr + 3)
						panic("nanami");
					if(!*cp && cp < p->nbp->b_un.b_addr + 2)
						panic("nanami2");
					if(*cp)
						*--cp = '/';
					*--cp = '.';
					*--cp = '.';
					if(fstyp != dp->i_fstyp)
						goto more;
					goto dirloop;
				}
		}
	}
	if(ino != u.u_dent.d_ino || dp->i_dev != d) {	/* avoid extra put */
		iput(dp);
		if(u.u_dent.d_ino == 0) {
			printf("nanami ino is 0\n");
			dp = 0;
		}
		else 
			dp = iget(d, u.u_dent.d_ino, fstyp);
	}
	if(dp == NULL)
		goto out1;
	if(fstyp != dp->i_fstyp)
		goto more;
	/*
	 * Check for symbolic link
	 */
	if ((dp->i_mode&IFMT)==IFLNK && (follow || *cp)) {
		char *ocp;

		ocp = cp;
		while (*cp++)
			;
		if (dp->i_size + (cp-ocp) >= BSIZE(dp->i_dev)-1
		    || ++p->nlink>8) {
			u.u_error = ELOOP;
			goto out;
		}
		bcopy(ocp, p->nbp->b_un.b_addr+dp->i_size + 1, cp-ocp);
		*(p->nbp->b_un.b_addr + dp->i_size) = '/';
		u.u_base = p->nbp->b_un.b_addr;
		u.u_count = dp->i_size;
		u.u_offset = 0;
		readi(dp);
		if(u.u_error)
			goto out;
		cp = p->nbp->b_un.b_addr;
		iput(dp);
		if (*cp == '/') {
			while (*cp == '/')
				cp++;
			if ((dp = u.u_rdir) == NULL)
				dp = rootdir;
			plock(dp);
			dp->i_count++;
		} else {
			if(ino == 0) {
				printf("nanami ino 0 in dir\n");
				dp = 0;
			}
			else
				dp = iget(d, ino, fstyp); /* retrieve original directory */
			if (dp == NULL)	/* how can this happen? */
				goto out1;
		}
		if(fstyp != dp->i_fstyp)
			goto more;
		goto dirloop;
	}
	if(*cp)
		goto dirloop;
	goto out1;
nomatch:
	/*
	 * Search failed.
	 */
	if(flagp) {		/* probably creating a new file */
		switch(flagp->flag) {
		case NI_LINK:	/* make a link */
			goto out;
		case NI_CREAT:	/* create a new file */
		case NI_NXCREAT:
			if(y.errno == EACCES || y.ino == 0) {
				u.u_error = EACCES;
				goto out;
			}
			dip = iget(y.dev, y.ino, dp->i_fstyp);
			if(dip == NULL)
				goto out;
			iput(dp);
			dp = dip;
			flagp->ino = dip->i_number;
			*pflagp = 0;
			goto out1;
		}
	}
	u.u_error = ENOENT;
out:
	p->dp = dp;
	return(1);
out1:
	p->dp = dp;
	return(0);
more:
	p->dp = dp;
	p->cp = cp;
	return(2);
}

struct {
	int tran;
	short proc, dev;
} ntx[32];	/* more debuggery */
send(cip, x, y)
struct inode *cip;
struct senda *x;
struct rcva *y;
{	int n, tn, ix;
	x->version = NETVERSION;
	/* until demux works, use key as a lock */
	while(cip->i_un.i_key)
		sleep((caddr_t)cip, PZERO);
	cip->i_un.i_key = 1;
	trannum++;		/* uniqueness */
	tn = x->trannum;
	netabuf.s = 2;
	for(ix = 0; ntx[ix].tran && ix < 32; ix++) {
		ntx[ix].tran = tn;
		ntx[ix].proc = u.u_procp->p_pid;
		ntx[ix].dev = x->dev;
	}
	n = istwrite(cip, (char *)x, sizeof(*x));
	netabuf.s = 3;
	if(n == -1) {
		y->errno = EIO;
		goto bad;
	}
	netabuf.s = 4;
	if(x->count > 0 && x->buf && x->cmd != NREAD) {
		n = istwrite(cip, x->buf, x->count);
		if(n == -1)
			goto bad;
	}
readagain:
	netabuf.s = 5;
	n = istread(cip, (char *)y, sizeof(*y), 0);
	netabuf.s = 6;
	if(n != sizeof(*y))
		goto bad;
	if(y->errno == 0 && x->cmd == NREAD) {
		n = istread(cip, x->buf, y->count, 0);
		if(n != y->count) {
			printf("read %d expected %d\n", n, y->count);
			/* shut it down */
			istwrite(cip, (char *)x, 0);
			goto bad;
		}
	}
	if(y->errno == 0) {
		if(y->trannum != tn) {
			if(y->trannum < tn)	/* distant past */
				goto readagain;
			printf("sent %d got %d\n", tn, y->trannum);
			goto bad;
		}
		addx(x);
		addy(y);
		if(ix < 32)
			ntx[ix].tran = 0;
		cip->i_un.i_key = 0;
		wakeup((caddr_t)cip);
		return;
	}
bad:
	cip->i_un.i_key = 0;
	if(ix < 32)
		ntx[ix].tran = 0;
	for(ix = 0; ix < 32; ix++)
		if(ntx[ix].tran)
			printf("tran %d proc %d 0x%x\n", ntx[ix].tran, ntx[ix].proc, ntx[ix].dev);
	wakeup((caddr_t)cip);
	addx(x);
	addy(y);
	if(y->errno)
		return;
	y->errno = EIO;
}
static struct D { struct D *a; char *b;} VER = {&VER,"\n85/6/9:neta.c\n"};

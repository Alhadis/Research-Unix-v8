/*	sys3.c	4.9	81/03/08	*/

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/mount.h"
#include "../h/ino.h"
#include "../h/reg.h"
#include "../h/buf.h"
#include "../h/filsys.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/inode.h"
#include "../h/file.h"
#include "../h/conf.h"
#include "../h/stat.h"
#include "../h/inline.h"

/*
 * the fstat system call.
 */
fstat()
{
	register struct file *fp;
	register struct a {
		int	fdes;
		struct stat *sb;
	} *uap;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if(fp == NULL)
		return;
	stat1(fp->f_inode, uap->sb, 0);
}

/*
 * the stat system call.  This version follows links.
 */
stat()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		struct stat *sb;
	} *uap;

	uap = (struct a *)u.u_ap;
	ip = namei(uchar, 0, 1);
	if(ip == NULL)
		return;
	stat1(ip, uap->sb, (off_t)0);
	iput(ip);
}

/*
 * the lstat system call.  This version does not follow links.
 */
lstat()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		struct stat *sb;
	} *uap;

	uap = (struct a *)u.u_ap;
	ip = namei(uchar, 0, 0);
	if(ip == NULL)
		return;
	stat1(ip, uap->sb, (off_t)0);
	iput(ip);
}

/*
 * The basic routine for fstat and stat:
 * get the inode and pass appropriate parts back.
 */
stat1(ip, ub, pipeadj)
register struct inode *ip;
struct stat *ub;
off_t pipeadj;
{
	register struct dinode *dp;
	register struct buf *bp;
	struct stat ds;

	IUPDAT(ip, &time, &time, 0);
	if(ip->i_fstyp && fstypsw[ip->i_fstyp].t_stat) {
		(*fstypsw[ip->i_fstyp].t_stat)(ip, ub, pipeadj);
		return;
	}
	/*
	 * first copy from inode table
	 */
	ds.st_dev = ip->i_dev;
	ds.st_ino = ip->i_number;
	ds.st_mode = ip->i_mode;
	ds.st_nlink = ip->i_nlink;
	ds.st_uid = ip->i_uid;
	ds.st_gid = ip->i_gid;
	ds.st_rdev = (dev_t)ip->i_un.i_rdev;
	ds.st_size = ip->i_size - pipeadj;
	/*
	 * next the dates in the disk
	 */
	bp = bread(ip->i_dev, itod(ip->i_dev, ip->i_number));
	dp = bp->b_un.b_dino;
	dp += itoo(ip->i_dev, ip->i_number);
	ds.st_atime = dp->di_atime;
	ds.st_mtime = dp->di_mtime;
	ds.st_ctime = dp->di_ctime;
	brelse(bp);
	if (copyout((caddr_t)&ds, (caddr_t)ub, sizeof(ds)) < 0)
		u.u_error = EFAULT;
}

/*
 *  readlink -- return target name of a symbolic link
 */
readlink()
{
	register struct inode *ip;
	register struct a {
		char	*name;
		char	*buf;
		int	count;
	} *uap;

	ip = namei(uchar, 0, 0);
	if (ip == NULL)
		return;
	uap = (struct a *)u.u_ap;
	if ((ip->i_mode&IFMT) != IFLNK) {
		u.u_error = ENXIO;
		goto out;
	}
	u.u_offset = 0;
	u.u_base = uap->buf;
	u.u_count = uap->count;
	u.u_segflg = 0;
	readi(ip);
out:
	iput(ip);
	u.u_r.r_val1 = uap->count - u.u_count;
}

/*
 * symlink -- make a symbolic link
 */
symlink()
{
	register struct a {
		char	*target;
		char	*linkname;
	} *uap;
	register struct inode *ip;
	register char *tp;
	register c, nc;
	struct argnamei nmarg;

	uap = (struct a *)u.u_ap;
	tp = uap->target;
	nc = 0;
	while (c = fubyte(tp)) {
		if (c < 0) {
			u.u_error = EFAULT;
			return;
		}
		tp++;
		nc++;
	}
	u.u_dirp = uap->linkname;
	nmarg.flag = NI_NXCREAT;
	nmarg.mode = IFLNK|0777;
	ip = namei(uchar, &nmarg, 0);
	if (ip == NULL)
		return;
	u.u_base = uap->target;
	u.u_count = nc;
	u.u_offset = 0;
	u.u_segflg = 0;
	writei(ip);
	iput(ip);
}

/*
 * the dup system call.
 */
dup()
{
	register struct file *fp;
	register struct a {
		int	fdes;
		int	fdes2;
	} *uap;
	register i, m;

	uap = (struct a *)u.u_ap;
	m = uap->fdes & ~077;
	uap->fdes &= 077;
	fp = getf(uap->fdes);
	if(fp == NULL)
		return;
	if ((m&0100) == 0) {
		if ((i = ufalloc()) < 0)
			return;
	} else {
		i = uap->fdes2;
		if (i<0 || i>=NOFILE) {
			u.u_error = EBADF;
			return;
		}
		u.u_r.r_val1 = i;
	}
	if (i!=uap->fdes) {
		if (u.u_ofile[i]!=NULL)
			closef(u.u_ofile[i]);
		u.u_ofile[i] = fp;
		fp->f_count++;
	}
}

/* find the mount slot assigned a particular device */
struct mount *
findmount(fstyp, dev)
	int fstyp;
	dev_t dev;
{
	register struct mount *mp;

	for(mp = mount; mp < mount + NMOUNT; mp++)
		if(mp->m_flags&M_MOUNTED && mp->m_dev==dev && mp->m_fstyp==fstyp)
			return mp;
	return NULL;
}

/* get a free mount slot */
struct mount *
allocmount(fstyp, dev)
	int fstyp;
	dev_t dev;
{
	register struct mount *mp, *free;

	free = NULL;
	for(mp = mount; mp < mount + NMOUNT; mp++) {
		if(!mp->m_flags&M_MOUNTED) {
			if (free == NULL)
				free = mp;
		} else if(mp->m_dev == dev && mp->m_fstyp == fstyp)
			return NULL;	/* mounted twice */
	}
	if (free != NULL) {
		free->m_flags |= M_MOUNTED;
		free->m_dev = dev;
		free->m_fstyp = fstyp;
	}
	return free;
}

/* free up a mount structure */
freemount(mp)
	struct mount *mp;
{
	if (mp == NULL)
		panic("freemount");
	mp->m_flags &= ~M_MOUNTED;
}

/*
 * the mount system call.
 */
fsmount()
{	dev_t dev;
	register struct inode *ip;
	register struct mount *mp;
	register struct filsys *fp;
	struct buf *bp;
	register struct a {
		char	*fspec;
		char	*freg;
		int	ronly;
	} *uap;
	register char *cp;

	uap = (struct a *)u.u_ap;
	dev = getmdev();
	if(u.u_error)
		return;
	u.u_dirp = (caddr_t)uap->freg;
	ip = namei(uchar, 0, 1);
	if(ip == NULL)
		return;
	if(ip->i_count!=1 || (ip->i_mode&(IFBLK&IFCHR))!=0)	/* doubtful */
		goto out;
	mp = allocmount(0, dev);
	if (mp == NULL)
		goto out;
	(*bdevsw[major(dev)].d_open)(dev, !uap->ronly);
	if(u.u_error) {
		freemount(mp);
		goto out;
	}
	bp = bread(dev, SUPERB);
	if(u.u_error) {
		freemount(mp);
		brelse(bp);
		goto out1;
	}
	mp->m_inodp = ip;
	bp->b_flags |= B_LOCKED;
	mp->m_bufp = bp;
	fp = bp->b_un.b_filsys;
	fp->s_ilock = 0;
	fp->s_flock = 0;
	fp->s_ronly = uap->ronly & 1;
	fp->s_nbehind = 0;
	fp->s_lasti = 1;
	u.u_dirp = uap->freg;
	for (cp = fp->s_fsmnt; cp < &fp->s_fsmnt[sizeof (fp->s_fsmnt) - 1]; )
		if ((*cp++ = uchar()) == 0)
			u.u_dirp--;		/* get 0 again */
	*cp = 0;
	brelse(bp);
	if(BITFS(dev) && !fp->s_valid && !fp->s_ronly) {
		bp->b_flags &= ~B_LOCKED;
		freemount(mp);
		goto out;			/* NOT IMPLEMENTED */
	}
	ip->i_flag |= IMOUNT;
	prele(ip);
	return;

out:
	u.u_error = EBUSY;
out1:
	iput(ip);
}

/*
 * the umount system call.
 */
sumount()
{
	dev_t dev;
	register struct inode *ip;
	register struct mount *mp;
	struct buf *bp;
	int stillopen, flag;
	register struct a {
		char	*fspec;
	};

	dev = getmdev();
	if(u.u_error)
		return;
	xumount(dev);	/* remove unused sticky files from text table */
	update();
	mp = findmount(0, dev);
	if (mp == NULL) {
		u.u_error = EINVAL;
		return;
	}
	stillopen = 0;
	for(ip = inode; ip < inodeNINODE; ip++)
		if (ip->i_number != 0 && dev == ip->i_dev) {
			u.u_error = EBUSY;
			return;
		} else if (ip->i_number != 0 && (ip->i_mode&IFMT) == IFBLK &&
		    ip->i_un.i_rdev == dev)
			stillopen++;
	ip = mp->m_inodp;
	ip->i_flag &= ~IMOUNT;
	plock(ip);
	iput(ip);
	if ((bp = getblk(dev, SUPERB)) != mp->m_bufp)
		panic("umount");
	bp->b_un.b_filsys->s_valid = 1;
	bp->b_flags &= ~B_LOCKED;
	flag = !bp->b_un.b_filsys->s_ronly;
	if(BITFS(dev) && flag)
		bwrite(bp);
	else
		brelse(bp);
	mpurge(mp - &mount[0]);
	freemount(mp);
	if (!stillopen) {
		(*bdevsw[major(dev)].d_close)(dev, flag);
		binval(dev);
	}
}

/*
 * Common code for mount and umount.
 * Check that the user's argument is a reasonable
 * thing on which to mount, and return the device number if so.
 */
dev_t
getmdev()
{
	dev_t dev;
	register struct inode *ip;

	if (!suser())
		return(NODEV);
	ip = namei(uchar, 0, 1);
	if(ip == NULL)
		return(NODEV);
	if((ip->i_mode&IFMT) != IFBLK)
		u.u_error = ENOTBLK;
	dev = (dev_t)ip->i_un.i_rdev;
	if(major(dev) >= nblkdev)
		u.u_error = ENXIO;
	iput(ip);
	return(dev);
}

/* gmount(fstyp, ...) */
gmount()
{	struct a {
		unsigned int fstyp;
		char *restofit;
	} *uap = (struct a *)u.u_ap;

	if(uap->fstyp < nfstyp && fstypsw[uap->fstyp].t_mount)
		(*fstypsw[uap->fstyp].t_mount)();
	else
		u.u_error = EINVAL;
}

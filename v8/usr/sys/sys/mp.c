/*
 *	This file contains the routines which support mounted
 *	processes (file system type 3).
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/inode.h"
#include "../h/user.h"
#include "../h/mount.h"
#include "../h/file.h"

/* Get an inode for a mounted process.  Release the passed inode and return
 * the inode of the communications stream to the process.
 */
struct inode *
mpget(fstyp, dev, ino, ip)
	dev_t dev;
	struct inode *ip;
{
	struct mount *mp;

	/* dump the inode passed from iget */
	iput(ip);

	/* only the root node can exist */
	if(ino != ROOTINO) {
		if(!u.u_error)
			u.u_error = EMFILE;	/* just in case for nanami */
		printf("mpget ino %d dev %d\n", ino, dev);
		return(0);
	}

	/* get the inode for the stream to the process */
	mp = findmount(fstyp, dev);
	if (mp == NULL) {
		u.u_error = EMFILE;
		return(0);
	}
	mp->m_idev->i_count++;
	return(mp->m_idev);
}

/*	Making this entry 0 in the fstypsw is not equivalent to 
 *	this null routine since that would cause iput to call iupdat.
 */
mpput(ip)
	struct inode *ip;
{
	/* don't do anything */
}

/*	Making this entry 0 in the fstypsw is not equivalent to 
 *	this null routine since there is no fs struct for mounted
 *	processes (see ifree in alloc.c).
 */
mpfree(ip)
	struct inode *ip;
{
	/* don't do anything */
}

mpupdat(ip, ta, tm, waitfor)
	struct inode *ip;
	time_t *ta, *tm;
{
	panic("mpupdat");
}

mpread(ip)
	struct inode *ip;
{
	panic("mpread");
}

mpwrite(ip)
	struct inode *ip;
{
	panic("mpwrite");
}

mptrunc(ip)
	struct inode *ip;
{
	panic("mptrunc");
}

mpstat(ip, ub, adj)
	struct inode *ip;
	struct stat *ub;
	off_t adj;
{
	panic("mpstat");
}

mpnami(p, pflagp, follow)
	struct nx *p;
	struct argnamei **pflagp;
{
	panic("mpnami");
}

mpioctl(ip, cmd, cmaddr)
	struct inode *ip;
	caddr_t cmaddr;
{
	panic("mpioctl");
}

/* new style xmount (called by gmount) */
mpmount()
{	struct a {
		int fstyp;
		int unqname;
		int flag;	/* 0 = mount, 1 = unmount */
		int cfd;
		char *freg;
	} *uap = (struct a *)u.u_ap;

	if (uap->flag) 
		mpunmount(uap->fstyp, uap->unqname);
	else
		mpdomount(uap->fstyp, uap->unqname, uap->cfd, uap->freg);
}

mpdomount(fstyp, dev, cfd, freg)
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
	if (access(dip, IWRITE)) {
		iput(dip);
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

mpunmount(fstyp, dev)
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
	ip = mp->m_inodp;
	if (access(ip, IWRITE))
		return;
	ip->i_flag &= ~IMOUNT;
	plock(ip);
	iput(ip);
	if(mp->m_idev)
		stclose(mp->m_idev, 1);
	iput(mp->m_idev);
	freemount(mp);
}

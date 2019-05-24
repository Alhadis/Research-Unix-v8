#include "../h/param.h"
#include "../h/systm.h"
#include "../h/inode.h"
#include "../h/mount.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/conf.h"

/*
 * Convert a pathname into a pointer to
 * a locked inode.
 *
 * func = function called to get next char of name
 *	&uchar if name is in user space
 *	&schar if name is in system space
 *	length guaranteed <= BUFSIZE if func != uchar
 * flagp
	0 for ordinary searches
	else ->flag structure with more parameters
 * follow = 1 if links are to be followed at the end of the name
 */

struct inode *
namei(func, flagp, follow)
int		(*func)();
struct argnamei	*flagp;
int		follow;
{
	register int	i;
	register char	*cp;
	struct nx	p;

	p.nbp = geteblk();
	if(func == uchar) {
		if((i = fustrlen(u.u_dirp)) < 0) {
			u.u_error = EFAULT;
			brelse(p.nbp);
			return NULL;
		}
		if(i > BUFSIZE) {
			u.u_error = ENOENT;
			brelse(p.nbp);
			return NULL;
		}
		bcopy(u.u_dirp, p.nbp->b_un.b_addr, i);
#ifdef	CHAOS
		u.u_dirp += i;
#endif
	} else {
		cp = p.nbp->b_un.b_addr;
		do; while(*cp++ = (*func)());
	}
	if(flagp != NULL) {
		cp = p.nbp->b_un.b_addr;
		while(i = *cp++) {
			if(i & 0200) {
				u.u_error = ENOENT;
				brelse(p.nbp);
				return NULL;
			}
		}
	}
	cp = p.nbp->b_un.b_addr;
	if(*cp == '/') {
		while(*cp == '/')
			cp++;
		if((p.dp = u.u_rdir) == NULL)
			p.dp = rootdir;
	} else
		p.dp = u.u_cdir;
	p.nlink = 0;
	p.cp = cp;
	plock(p.dp);
	p.dp->i_count++;

	for (;;) {
		if(p.dp->i_fstyp >= nfstyp)
			panic("namei nfstyp");
		switch((*fstypsw[p.dp->i_fstyp].t_nami)(&p, &flagp, follow)) {
		case 1:
			iput(p.dp);
			brelse(p.nbp);
			return NULL;
		case 2:
			if(*p.cp)
				break;
		case 0:
			brelse(p.nbp);
			if(flagp &&
			  (flagp->flag == NI_LINK || flagp->flag == NI_NXCREAT) &&
			   p.dp) {
				u.u_error = EEXIST;
				iput(p.dp);
				return NULL;
			}
			return p.dp;
		default:
			panic("namei ret");
		}
	}
}

ino_t
dsearch(ip, eop)
struct inode	*ip;
off_t		*eop;
{
	register struct direct	*dp;
	register char		*nm;
	register int		i;
	register int		n;
	register int		empty;
	register int		size;
	int			dpb;
	ino_t			ino;
	struct buf		*bp;

	bp = NULL;
	empty = -1;
	ino = 0;
	i = 0;
	n = 0;
	nm = u.u_dbuf;
	dpb = BSIZE(ip->i_dev) / sizeof (struct direct);
	size = ip->i_size / sizeof (struct direct);

	while (--size >= 0) {
		if (--n < 0) {
			if (bp)
				brelse(bp);
			bp = bread(ip->i_dev, bmap(ip, (daddr_t)(i/dpb), B_READ));
			if (bp->b_flags & B_ERROR) {
				brelse(bp);
				return 0;
			}
			dp = (struct direct *)bp->b_un.b_addr;
			n = dpb - 1;
		}
		if (dp->d_ino == 0) {
			if (empty < 0)
				empty = i;
		} else {
#if	DIRSIZ == 14
			if (*(long *)&nm[0] == *(long *)&dp->d_name[0] &&
			    *(long *)&nm[4] == *(long *)&dp->d_name[4] &&
			    *(long *)&nm[8] == *(long *)&dp->d_name[8] &&
			    *(short *)&nm[12] == *(short *)&dp->d_name[12]) {
#else
			if (strncmp(nm, dp->d_name, DIRSIZ) == 0) {
#endif
				bcopy((caddr_t)dp, (caddr_t)&u.u_dent, sizeof (struct direct));
				ino = dp->d_ino;
				break;
			}
		}
		i++;
		dp++;
	}
	if (bp)
		brelse(bp);
	u.u_offset = i * sizeof (struct direct);
	*eop = empty * sizeof (struct direct);
	return ino;
}

     /* USED TO BE rnami */
fsnami(p, pflagp, follow)
struct nx *p;
struct argnamei **pflagp;
{	register struct inode *dp;
	register char *cp;
	register struct buf *bp, *nbp;
	register struct direct *ep;
	register struct inode *dip;
	int i, fstyp;
	dev_t d;
	ino_t ino;
	off_t eo;
	struct argnamei *flagp = *pflagp;
	struct mount *mp;
#ifdef	CHAOS
	extern long cdevpath;
#endif	CHAOS

	cp = p->cp;
	dp = p->dp;

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
#ifdef	OLD_FSNAMI
	eo = -1;
	bp = NULL;

	for (u.u_offset=0; u.u_offset < dp->i_size;
	   u.u_offset += sizeof(struct direct), ep++) {
		/*
		 * If offset is on a block boundary,
		 * read the next directory block.
		 * Release previous if it exists.
		 */
		if((u.u_offset&BMASK(dp->i_dev)) == 0) {
			if(bp != NULL)
				brelse(bp);
			bp = bread(dp->i_dev,
				bmap(dp,(daddr_t)(u.u_offset>>BSHIFT(dp->i_dev))
					, B_READ));
			if (bp->b_flags & B_ERROR) {
				brelse(bp);
				goto out;
			}
			ep = (struct direct *)bp->b_un.b_addr;
		}
		/*
		 * Note first empty directory slot
		 * in eo for possible creat.
		 * String compare the directory entry
		 * and the current component.
		 */
		if(ep->d_ino == 0) {
			if(eo < 0)
				eo = u.u_offset;
			continue;
		}
		if (strncmp(u.u_dbuf, ep->d_name, DIRSIZ) != 0)
			continue;
		/*
		 * Here a component matched in a directory.
		 * If there is more pathname, go back to
		 * dirloop, otherwise return.
		 */
		bcopy((caddr_t)ep, (caddr_t)&u.u_dent, sizeof(struct direct));
		brelse(bp);
#else	OLD_FSNAMI
	if (dsearch(dp, &eo)) {
#endif	OLD_FSNAMI
		if(flagp && flagp->flag == NI_DEL && *cp == 0) {
			/* delete the entry */
			if(access(dp, IWRITE))
				goto out;
			if(dp->i_number == u.u_dent.d_ino) {	/* for '.' */
				dip = dp;
				dp->i_count++;
			} else
				dip = iget(dp->i_dev, u.u_dent.d_ino,
					dp->i_fstyp);
			if(dip == NULL)
				goto out;
			if(dip->i_dev != dp->i_dev) {	/* mounted FS */
				u.u_error = EBUSY;
				iput(dip);
				goto out;
			}
			if((dip->i_mode&IFMT) == IFDIR && !suser()) {
				iput(dip);
				goto out;
			}
			if(dip->i_flag&ITEXT)
				xrele(dip);		/* free busy text */
			u.u_base = (caddr_t)&u.u_dent;
			u.u_count = sizeof(struct direct);
			u.u_dent.d_ino = 0;
			writei(dp);		/* offset, segflg already set*/
			dip->i_nlink--;
			dip->i_flag |= ICHG;
			iput(dip);
			goto out;
		}
		if(flagp && flagp->flag == NI_RMDIR && *cp == 0) {
			/* can't rmdir ., .., or ROOTINO */
			/* can rmdir non-empty directory */
			if(access(dp, IWRITE))
				goto out;
			if(dp->i_number == u.u_dent.d_ino) {
rmdir:
				u.u_error = EINVAL;
				goto out;
			}
			if(u.u_dent.d_name[0] == '.' && u.u_dent.d_name[1] == '.' && u.u_dent.d_name[2] == 0)
				goto rmdir;
			if((dip = iget(dp->i_dev, u.u_dent.d_ino, dp->i_fstyp)) == NULL)
				goto rmdir;
			if(dip->i_number <= ROOTINO) {
				iput(dip);
				goto rmdir;
			}
			if(dip->i_dev != dp->i_dev || dip->i_count > 1) {
				u.u_error = EBUSY;
				iput(dip);
				goto out;
			}
			if((dip->i_mode & IFMT) != IFDIR) {
				u.u_error = ENOTDIR;
				iput(dip);
				goto out;
			}
			u.u_base = (caddr_t)&u.u_dent;
			u.u_count = sizeof(struct direct);
			u.u_dent.d_ino = 0;
			if(dp->i_nlink > 0)
				dp->i_nlink--;
			if(dip->i_nlink <= 2)
				dip->i_nlink = 0;
			else {
				u.u_error = EBUSY;
				iput(dip);
				goto out;
			}
			writei(dp);
			dip->i_flag |= ICHG;
			iput(dip);
			goto out;
		}
		/*
		 * Special handling for ".."
		 */
		if (u.u_dent.d_name[0]=='.' && u.u_dent.d_name[1]=='.' &&
		    u.u_dent.d_name[2]=='\0') {
			if (dp == u.u_rdir)
				u.u_dent.d_ino = dp->i_number;
			else if (dp != rootdir && u.u_dent.d_ino==ROOTINO &&
			   dp->i_number == ROOTINO) {
				mp = findmount(dp->i_fstyp, dp->i_dev);
				if (mp == NULL)
					panic("namei: findmount");
				iput(dp);
				dp = mp->m_inodp;
				plock(dp);
				dp->i_count++;
				cp -= 2;     /* back over .. */
				goto dirloop;
			}
		}
		d = dp->i_dev;
		ino = dp->i_number;
		fstyp = dp->i_fstyp;
		iput(dp);
		dp = iget(d, u.u_dent.d_ino, fstyp);
		if(dp == NULL)
			goto out1;
		if(fstyp != dp->i_fstyp)
			goto more;
		/*
		 * Check for symbolic link
		 */
		if ((dp->i_mode&IFMT)==IFLNK && (follow || *cp=='/')) {
			char *ocp;

			ocp = cp;
			while (*cp++)
				;
			if (dp->i_size + (cp-ocp) >= BSIZE(dp->i_dev)-1
			|| ++p->nlink>8) {
				u.u_error = ELOOP;
				goto out;
			}
			bcopy(ocp, p->nbp->b_un.b_addr+dp->i_size, cp-ocp);
			bp = bread(dp->i_dev, bmap(dp, (daddr_t)0, B_READ));
			if (bp->b_flags & B_ERROR) {
				brelse(bp);
				goto out;
			}
			bcopy(bp->b_un.b_addr, p->nbp->b_un.b_addr, dp->i_size);
			brelse(bp);
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
				dp = iget(d, ino, fstyp);	/* retrieve original directory */
				if (dp == NULL)	/* how can this happen? */
					goto out1;
			}
			goto dirloop;
		}
#ifdef	CHAOS
#define	CHDEV_OFFSET	32
		if ((dp->i_mode & IFMT) == IFCHR && (cdevpath & (1L << (major(dp->i_un.i_rdev) - CHDEV_OFFSET))))
		{
 			char *ocp;
 
 			while (*cp == '/')
 				cp++;
 			ocp = cp;
 			while (*cp++)
 				;
 			u.u_dirp -= (cp - ocp);
 			goto out1;
		}
#endif	CHAOS
		if (*cp == '/') {
			while (*cp == '/')
				cp++;
			goto dirloop;
		}
		goto out1;
	}
	if (u.u_error)
		goto out;
	/*
	 * Search failed.
	 */
	if(flagp) {		/* probably creating a new file */
		register struct inode *dip;

		while(*cp == '/')
			cp++;
		if(*cp != 0) {
			u.u_error = ENOENT;
			goto out;
		}
		bcopy((caddr_t)u.u_dbuf, (caddr_t)u.u_dent.d_name, DIRSIZ);
		u.u_count = sizeof(struct direct);
		u.u_base = (caddr_t)&u.u_dent;
		if(eo >= 0)
			u.u_offset = eo;
		switch(flagp->flag) {
		case NI_LINK:	/* make a link */
			if(access(dp, IWRITE))
				goto out;
			if(dp->i_dev != flagp->idev) {
				u.u_error = EXDEV;
				goto out;
			}
			u.u_dent.d_ino = flagp->ino;
			writei(dp);
			goto out;
		case NI_CREAT:	/* create a new file */
		case NI_NXCREAT:
			if(access(dp, IWRITE))
				goto out;
			dip = ialloc(dp->i_dev);
			if(dip == NULL)
				goto out;
			dip->i_flag |= IACC|IUPD|ICHG;
			dip->i_mode = flagp->mode & ~u.u_cmask;
			if((dip->i_mode & IFMT) == 0)
				dip->i_mode |= IFREG;
			dip->i_nlink = 1;
			dip->i_uid = u.u_uid;
			dip->i_gid = u.u_gid;
			iupdat(dip, &time, &time, 1);
			u.u_dent.d_ino = dip->i_number;
			writei(dp);
			iput(dp);
			dp = dip;
			flagp->ino = dip->i_number;
			*pflagp = 0;
			goto out1;
		case NI_MKDIR:	/* make a new directory */
			if(access(dp, IWRITE))
				goto out;
			dip = ialloc(dp->i_dev);
			if(dip == NULL)
				goto out;
			dip->i_flag = IACC|IUPD|ICHG;
			dip->i_mode = flagp->mode & ~u.u_cmask;
			dip->i_nlink = 2;
			dip->i_uid = u.u_uid;
			dip->i_gid = u.u_gid;
			iupdat(dip, &time, &time, 1);
			dp->i_nlink++;
			u.u_dent.d_ino = dip->i_number;
			writei(dp);
			flagp->ino = dp->i_number;
			iput(dp);
			dp = dip;
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
	for(; *cp == '/'; cp++)
		;
	p->dp = dp;
	p->cp = cp;
	return(2);
}

/*
 * Return the next character from the
 * kernel string pointed at by dirp.
 */
schar()
{

	return(*u.u_dirp++ & 0377);
}

/*
 * Return the next character from the
 * user string pointed at by dirp.
 */
uchar()
{
	register c;

	c = fubyte(u.u_dirp++);
	if(c == -1) {
		u.u_error = EFAULT;
		c = 0;
	}
	return(c);
}

/*  (handled by asm.sed)
strncmp(s1, s2, len)
register char *s1, *s2;
register len;
{
	do {
		if (*s1 != *s2++)
			return(1);
		if (*s1++ == '\0')
			return(0);
	} while (--len);
	return(0);
}
*/

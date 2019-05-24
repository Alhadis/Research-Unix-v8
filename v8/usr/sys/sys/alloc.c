/*	alloc.c	4.8	81/03/08	*/

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/mount.h"
#include "../h/filsys.h"
#include "../h/fblk.h"
#include "../h/conf.h"
#include "../h/buf.h"
#include "../h/inode.h"
#include "../h/ino.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/trace.h"
typedef	struct fblk *FP;

/*
 * alloc will obtain the next available
 * free disk block from the free list of
 * the specified device.
 * The super block has up to NICFREE remembered
 * free blocks; the last of these is read to
 * obtain NICFREE more . . .
 */
struct buf *
alloc(dev, prev)
dev_t dev;
daddr_t prev;
{
	daddr_t bno;
	register struct filsys *fp;
	register struct buf *bp;
	register int i, j;
	register long *p;
	int sn;
	static saveprev;

	fp = getfs(dev);
	while (fp->s_flock)
		sleep((caddr_t)&fp->s_flock, PINOD);
	if(BITFS(dev)) {	/* unfortunately device dependent */
	/* this code is UGLY, fix it */
		if(prev < fp->s_isize)
			goto scan;
		/* try for an acceptable free block in next
		 * three cylinders, then start over at the beginning */
		bno = 0;
		j = prev/(4*10);
		j *= 4 * 10;
		if(j < fp->s_isize)
			j = fp->s_isize;
		j -= fp->s_isize;
		for(i = 0; i < 3 * 4 * 10; i++, j++) {
			if(j >= fp->s_fsize - fp->s_isize)
				break;
			if(!(fp->s_bfree[j>>5] & (1 << (j&31))))
				continue;
			/* same cylinder? */
			if((j+fp->s_isize)/(4*10) != prev/(4*10)) {	/* take best */
				if(bno == 0)
					bno = j;
				fp->s_bfree[bno>>5] &= ~(1 << (bno&31));
				bno += fp->s_isize;
				goto found;
			}
			bno = j;
			p = fp->s_bfree + (j>>5);
			/* same sector or next, continue */
			if((j+fp->s_isize)%4 != prev%4 && (j+fp->s_isize)%4 != (prev+1)%4) {
				*p &= ~(1 << (j&31));
				bno += fp->s_isize;
				goto found;
			}
		}
		/* did we find anything? */
		if(bno != 0) {
			fp->s_bfree[bno>>5] &= ~(1 << (bno&31));
			bno += fp->s_isize;
			goto found;
		}
scan:
		p = fp->s_bfree;
		for(i = 0; i < BITMAP && !*p; i++, p++)
			;
		if(i >= BITMAP)
			goto nospace;
		bno = fp->s_isize + 32 * i;
		for(j = 0; j < 32; j++)	/* BITS PER LONG */
			if(*p & (1 << j))
				break;
		if(j >= 32)
			panic("alloc bitmap");
		bno += j;
		if(bno >= fp->s_fsize)
			goto nospace;
		*p &= ~(1 << j);
		if(fp->s_valid) {	/* was valid, isn't anymore */
			fp->s_valid = 0;
			update();	/* GROSS, but safe */
		}
	}
	else {
		do {
			if (fp->s_nfree <= 0)
				goto nospace;
			if (fp->s_nfree > NICFREE) {
				fserr(fp, "bad free count");
				goto nospace;
			}
			bno = fp->s_free[--fp->s_nfree];
			if (bno == 0)
				goto nospace;
		} while (badblock(fp, bno));
		if (fp->s_nfree <= 0) {
			fp->s_flock++;
			bp = bread(dev, bno);
			if ((bp->b_flags&B_ERROR) == 0) {
				fp->s_nfree = ((FP)(bp->b_un.b_addr))->df_nfree;
				bcopy((caddr_t)((FP)(bp->b_un.b_addr))->df_free,
				    (caddr_t)fp->s_free, sizeof(fp->s_free));
			}
			brelse(bp);
			fp->s_flock = 0;
			wakeup((caddr_t)&fp->s_flock);
			if (fp->s_nfree <= 0)
				goto nospace;
		}
	}
found:
	bp = getblk(dev, bno);
	clrbuf(bp);
	fp->s_fmod = 1;
	fp->s_tfree--;
	return (bp);

nospace:
	fp->s_nfree = 0;
	fp->s_tfree = 0;
	fserr(fp, "file system full");
	/* THIS IS A KLUDGE... */
	/* SHOULD RATHER SEND A SIGNAL AND SUSPEND THE PROCESS IN A */
	/* STATE FROM WHICH THE SYSTEM CALL WILL RESTART */
	uprintf("\n%s: write failed, file system is full\n", fp->s_fsmnt);
	for (i = 0; i < 5; i++)
		sleep((caddr_t)&lbolt, PRIBIO);
	/* END KLUDGE */
	u.u_error = ENOSPC;
	return (NULL);
}

/*
 * place the specified disk block
 * back on the free list of the
 * specified device.
 */
free(dev, bno)
	dev_t dev;
	daddr_t bno;
{
	register struct filsys *fp;
	register struct buf *bp;

	fp = getfs(dev);
	fp->s_fmod = 1;
	while (fp->s_flock)
		sleep((caddr_t)&fp->s_flock, PINOD);
	if (badblock(fp, bno))
		return;
	if(BITFS(dev)) {
		bno -= fp->s_isize;
		fp->s_bfree[bno/32] |= (1 << (bno % 32));
		if(fp->s_valid) {	/* not any more */
			fp->s_valid = 0;
			update();	/* even GROSSER */
		}
	}
	else {
		if (fp->s_nfree <= 0) {
			fp->s_nfree = 1;
			fp->s_free[0] = 0;
		}
		if (fp->s_nfree >= NICFREE) {
			fp->s_flock++;
			bp = getblk(dev, bno);
			((FP)(bp->b_un.b_addr))->df_nfree = fp->s_nfree;
			bcopy((caddr_t)fp->s_free,
			    (caddr_t)((FP)(bp->b_un.b_addr))->df_free,
			    sizeof(fp->s_free));
			fp->s_nfree = 0;
			bwrite(bp);
			fp->s_flock = 0;
			wakeup((caddr_t)&fp->s_flock);
		}
		fp->s_free[fp->s_nfree++] = bno;
	}
	fp->s_tfree++;
	fp->s_fmod = 1;
}

/*
 * Check that a block number is in the
 * range between the I list and the size
 * of the device.
 * This is used mainly to check that a
 * garbage file system has not been mounted.
 */
badblock(fp, bn)
	register struct filsys *fp;
	daddr_t bn;
{

	if (bn < fp->s_isize || bn >= fp->s_fsize) {
		fserr(fp, "bad block");
		return(1);
	}
	return(0);
}

/*
 * Allocate an unused inode on the specified device.
 * Used with file creation.  The algorithm keeps up to
 * NICINOD spare inodes in the super block.  When this runs out,
 * the inodes are searched to pick up more.  We keep searching
 * foreward on the device, remembering the number of inodes
 * which are freed behind our search point for which there is no
 * room in the in-core table.  When this number passes a threshold
 * (or if we search to the end of the ilist without finding any inodes)
 * we restart the search from the beginning.
 */

struct inode *
ialloc(dev)
	dev_t dev;
{
	register struct filsys *fp;
	register struct buf *bp;
	register struct inode *ip;
	register int i;
	struct dinode *dp;
	ino_t ino, inobas;
	int first;
	daddr_t adr;

	fp = getfs(dev);
	while (fp->s_ilock)
		sleep((caddr_t)&fp->s_ilock, PINOD);
loop:
	if (fp->s_ninode > 0) {
		ino = fp->s_inode[--fp->s_ninode];
		ip = iget(dev, ino, 0);
		if (ip == NULL)
			return(NULL);
		if (ip->i_mode == 0 && ip->i_number > ROOTINO) {
			for (i=0; i<NADDR; i++)
				ip->i_un.i_addr[i] = 0;
			fp->s_tinode--;
			fp->s_fmod = 1;
			return(ip);
		}
		/*
		 * Inode was allocated after all.
		 * Look some more.
		 */
		iput(ip);
		goto loop;
	}
	fp->s_ilock++;
	/*
	 * If less than 4*NICINOD inodes are known
	 * to be free behind the current search point,
	 * then search forward; else search from beginning.
	 */
	if (fp->s_nbehind < 4 * NICINOD) {
		first = 1;
		ino = fp->s_lasti;
		if(ino <= ROOTINO)
			goto fromtop;
		if (itod(dev, ino) >= fp->s_isize)
			panic("ialloc");
		adr = itod(dev, ino);
	} else {
fromtop:
		first = 0;
		ino = 1;
		adr = SUPERB+1;
		fp->s_nbehind = 0;
	}
	/*
	 * This is the search for free inodes.
	 */
	for(; adr < fp->s_isize; adr++) {
		inobas = ino;
		bp = bread(dev, adr);
		if ((bp->b_flags&B_CACHE) == 0)
			u.u_vm.vm_inblk--;		/* no charge! */
		if (bp->b_flags & B_ERROR) {
			brelse(bp);
			ino += INOPB(dev);
			continue;
		}
		dp = bp->b_un.b_dino;
		for (i=0; i<INOPB(dev); i++, ino++, dp++) {
			if (dp->di_mode != 0 || ifind(dev, ino, 0))
				continue;
			if (ino > ROOTINO)
				fp->s_inode[fp->s_ninode++] = ino;
			if (fp->s_ninode >= NICINOD)
				break;
		}
		brelse(bp);
		if (fp->s_ninode >= NICINOD)
			break;
	}
	/*
	 * If the search didn't net a full superblock of inodes,
	 * then try it again from the beginning of the ilist.
	 */
	if (fp->s_ninode < NICINOD && first)
		goto fromtop;
	fp->s_lasti = inobas;
	fp->s_ilock = 0;
	wakeup((caddr_t)&fp->s_ilock);
	if (fp->s_ninode > 0)
		goto loop;
	fserr(fp, "out of inodes");
	uprintf("\n%s: create failed, no inodes free\n", fp->s_fsmnt);
	u.u_error = ENOSPC;
	return (NULL);
}

/*
 * Free the specified inode on the specified device.
 * The algorithm stores up to NICINOD inodes in the super
 * block and throws away any more.  It keeps track of the
 * number of inodes thrown away which preceded the current
 * search point in the file system.  This lets us rescan
 * for more inodes from the beginning only when there
 * are a reasonable number of inodes back there to reallocate.
 */

ifree(dev, ino)
	dev_t dev;
	ino_t ino;
{
	register struct filsys *fp;

	if(ino <= ROOTINO)
		return;
	fp = getfs(dev);
	fp->s_tinode++;
	if (fp->s_ilock)
		return;
	if (fp->s_ninode >= NICINOD) {
		if (fp->s_lasti > ino)
			fp->s_nbehind++;
		return;
	}
	fp->s_inode[fp->s_ninode++] = ino;
	fp->s_fmod = 1;
}

/*
 * getfs maps a device number into
 * a pointer to the incore super
 * block.  The algorithm is a linear
 * search through the mount table.
 * A consistency check of the
 * in core free-block and i-node
 * counts is performed.
 *
 * panic: no fs -- the device is not mounted.
 *	this "cannot happen"
 */
struct filsys *
getfs(dev)
	dev_t dev;
{
	register struct mount *mp;
	register struct filsys *fp;

	mp = findmount(0, dev);
	if (mp == NULL)
		panic("getfs");
	fp = mp->m_bufp->b_un.b_filsys;
	if (fp->s_nfree > NICFREE || fp->s_ninode > NICINOD) {
		fserr(fp, "bad count");
		fp->s_nfree = 0;
		fp->s_ninode = 0;
	}
	return(fp);
}

/*
 * Fserr prints the name of a file system
 * with an error diagnostic, in the form
 *	filsys: error message
 */
fserr(fp, cp)
	struct filsys *fp;
	char *cp;
{

	printf("%s: %s\n", fp->s_fsmnt, cp);
}

/*
 * Getfsx returns the index in the file system
 * table of the specified device.  The swap device
 * is also assigned a pseudo-index.  The index may
 * be used as a compressed indication of the location
 * of a block, recording
 *	<getfsx(dev),blkno>
 * rather than
 *	<dev, blkno>
 * provided the information need remain valid only
 * as long as the file system is mounted.
 */
getfsx(dev)
	dev_t dev;
{
	register struct mount *mp;

	if (dev == swapdev)
		return (MSWAPX);
	mp = findmount(0, dev);
	if (mp == NULL)
		return (-1);
	return (mp - &mount[0]);
}

/*
 * Update is the internal name of 'sync'.  It goes through the disk
 * queues to initiate sandbagged IO; goes through the inodes to write
 * modified nodes; and it goes through the mount table to initiate modified
 * super blocks.
 */
update()
{
	register struct inode *ip;
	register struct mount *mp;
	register struct buf *bp;
	struct filsys *fp;

	if (updlock)
		return;
	updlock++;
	/*
	 * Write back modified superblocks.
	 * Consistency check that the superblock
	 * of each file system is still in the buffer cache.
	 */
	for (mp = &mount[0]; mp < &mount[NMOUNT]; mp++)
		if (mp->m_flags&M_MOUNTED && mp->m_fstyp == 0) {
			fp = mp->m_bufp->b_un.b_filsys;
			if (fp->s_fmod==0 || fp->s_ilock!=0 ||
			   fp->s_flock!=0 || fp->s_ronly!=0)
				continue;
			bp = getblk(mp->m_dev, SUPERB);
			fp->s_fmod = 0;
			fp->s_time = time;
			if (bp->b_un.b_filsys != fp)
				panic("update");
			bwrite(bp);
		}
	/*
	 * Write back each (modified) inode.
	 */
	for (ip = inode; ip < inodeNINODE; ip++)
		if((ip->i_flag&ILOCK)==0 && ip->i_count) {
			ip->i_flag |= ILOCK;
			ip->i_count++;
			iupdat(ip, &time, &time, 0);
			iput(ip);
		}
	updlock = 0;
	/*
	 * Force stale buffer cache information to be flushed,
	 * for all devices.
	 */
	trace(TR_BFIN, updlock, 0);
	bflush(NODEV);
	trace(TR_BFOUT, updlock, 0);
}

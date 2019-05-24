#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/inode.h"
#include "../h/file.h"
#include "../h/reg.h"
#include "../h/inline.h"
#include "../h/proc.h"
#include "../h/stream.h"

struct	inode *mkpipend();

/*
 * The sys-pipe entry.
 * Allocate 2 open inodes, stream them, and splice them together
 */
pipe()
{
	struct inode *ip1, *ip2;
	register struct file *rf, *wf;
	register r;
	extern struct streamtab nilinfo;

	rf = falloc();
	if(rf == NULL)
		return;
	r = u.u_r.r_val1;
	wf = falloc();
	if(wf == NULL) {
		rf->f_count = 0;
		u.u_ofile[r] = NULL;
		return;
	}
	u.u_r.r_val2 = u.u_r.r_val1;
	u.u_r.r_val1 = r;
	if (makepipe(&ip1, &ip2)==0) {
		rf->f_count = 0;
		wf->f_count = 0;
		u.u_ofile[u.u_r.r_val1] = NULL;
		u.u_ofile[u.u_r.r_val2] = NULL;
		return;
	}
	wf->f_flag = FREAD|FWRITE;
	wf->f_inode = ip1;
	rf->f_flag = FREAD|FWRITE;
	rf->f_inode = ip2;
}

makepipe(ip1, ip2)
register struct inode **ip1, **ip2;
{

	*ip1 = mkpipend();
	*ip2 = mkpipend();
	if (*ip1==NULL || *ip2==NULL) {
		if (*ip1) {
			stclose(*ip1, 0);
			iput(*ip1);
		}
		if (*ip2) {
			stclose(*ip2, 0);
			iput(*ip2);
		}
		return(0);
	}
	qdetach(RD((*ip1)->i_sptr->wrq->next), 1);
	(*ip1)->i_sptr->wrq->next = RD((*ip2)->i_sptr->wrq);
	qdetach(RD((*ip2)->i_sptr->wrq->next), 1);
	(*ip2)->i_sptr->wrq->next = RD((*ip1)->i_sptr->wrq);
	return(1);
}

struct inode *
mkpipend()
{
	register struct inode *ip;

	ip = ialloc(pipedev);
	if(ip == NULL)
		return;
	ip->i_mode = IFREG | (0666 & ~u.u_cmask);
	stopen(&nilinfo, (dev_t)0, 0, ip);
	if (u.u_error) {
		iput(ip);
		return(NULL);
	}
	ip->i_uid = u.u_uid;
	ip->i_gid = u.u_gid;
	prele(ip);
	return(ip);
}

#ifdef plock
#undef plock
#endif
#ifdef prele
#undef prele
#endif
/*
 * Lock an inode
 * If its already locked,
 * set the WANT bit and sleep.
 */
plock(ip)
register struct inode *ip;
{

	while(ip->i_flag&ILOCK) {
		ip->i_flag |= IWANT;
		sleep((caddr_t)ip, PINOD);
	}
	ip->i_flag |= ILOCK;
}

/*
 * Unlock an inode.
 * If WANT bit is on,
 * wakeup.
 */
prele(ip)
register struct inode *ip;
{
	ip->i_flag &= ~ILOCK;
	if(ip->i_flag&IWANT) {
		ip->i_flag &= ~IWANT;
		wakeup((caddr_t)ip);
	}
}

/*	ioctl.c	4.4	81/03/08	*/

/*
 * Ioctl.
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/inode.h"
#include "../h/file.h"
#include "../h/conf.h"
#include "../h/ioctl.h"

/*
 * ioctl system call
 * Check legality, execute common code, and switch out to individual
 * device routine.
 */
ioctl()
{
	register struct file *fp;
	register struct inode *ip;
	register struct a {
		int	fdes;
		int	cmd;
		caddr_t	cmarg;
	} *uap;
	register dev_t dev;
	register fmt;

	uap = (struct a *)u.u_ap;
	if ((fp = getf(uap->fdes)) == NULL)
		return;
	if ((fp->f_flag & (FREAD|FWRITE)) == 0) {
		u.u_error = EBADF;
		return;
	}
	ip = fp->f_inode;
	switch (uap->cmd) {

	case FIOCLEX:			/* close on exec */
		u.u_pofile[uap->fdes] |= EXCLOSE;
		return;

	case FIONCLEX:			/* no close on exec */
		u.u_pofile[uap->fdes] &= ~EXCLOSE;
		return;

	case FIOALOCK:				/* set an advisory lock */
		if ((ip->i_flag & IALOCK) != 0) {	/* is lock set? */
			u.u_error = EPERM;	/* yes, refuse */
			return;
		}
		ip->i_flag |= IALOCK;
		fp->f_flag |= FALOCK;
		return;

	case FIOAUNLOCK:			/* clear an advisory lock */
		if ((ip->i_flag & IALOCK) == 0)		/* is lock clear? */
			return;				/* yes, ignore */
		if ((fp->f_flag & FALOCK) == 0) { /* do I own it? */
			u.u_error = EPERM;		/* no, refuse */
			return;
		}
		ip->i_flag &= ~IALOCK;
		fp->f_flag &= ~FALOCK;
		return;

	case FIOAISLOCK:			/* test an advisory lock */
		if ((ip->i_flag & IALOCK) != 0) {	/* is lock set? */
			if (fp->f_flag & FALOCK) /* yes, by me? */
				u.u_r.r_val1 = 2;
			else
				u.u_r.r_val1 = 1;
		}
		return;

	}
	if (ip->i_sptr) {
		stioctl(ip, uap->cmd, uap->cmarg);
		return;
	}
	if(ip->i_fstyp && fstypsw[ip->i_fstyp].t_ioctl) {
		(*fstypsw[ip->i_fstyp].t_ioctl)(ip, uap->cmd, uap->cmarg);
		return;
	}
	fmt = ip->i_mode & IFMT;
	if (fmt != IFCHR) {
		if (uap->cmd==FIONREAD && (fmt == IFREG || fmt == IFDIR)) {
			off_t nread = ip->i_size - fp->f_offset;
			if (copyout((caddr_t)&nread, uap->cmarg, sizeof(off_t)))
				u.u_error = EFAULT;
		} else
			u.u_error = ENOTTY;
		return;
	}
	dev = ip->i_un.i_rdev;
	u.u_r.r_val1 = 0;
	if ((u.u_procp->p_flag&SNUSIG) && setjmp(u.u_qsav)) {
		u.u_eosys = RESTARTSYS;
		return;
	}
	(*cdevsw[major(dev)].d_ioctl)(dev, uap->cmd, uap->cmarg, fp->f_flag);
}

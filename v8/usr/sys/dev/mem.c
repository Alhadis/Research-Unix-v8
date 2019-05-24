/*	mem.c	4.3	81/03/08	*/

/*
 * Memory special file
 *	minor device 0 is physical memory
 *	minor device 1 is kernel memory 
 *	minor device 2 is EOF/RATHOLE
 *	minor device 3 is unibus memory (addressed by shorts)
 *	minor device 4 is public part of kernel memory (read only)
 *	minor device 5 is processor registers
 */

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/conf.h"
#include "../h/buf.h"
#include "../h/systm.h"
#include "../h/pte.h"
#include "../h/mtpr.h"
#include "../h/vm.h"
#include "../h/cmap.h"
#include "sparam.h"

#define SYSADR	((caddr_t)0x80000000)	/* virtual address  of system seg. */

#ifndef	NBLKBIG
#define	NBLKBIG	0
#endif
#define	NBLKDATA (1024*NBLKBIG + 64*NBLK64 + 16*NBLK16 + 4*NBLK4)

extern	u_char	blkdata[];

mmread(dev)
{
	register int o; long lbuf;
	register unsigned c, v;

	switch (minor(dev)) {

	case 0:
		while (u.u_count != 0 && u.u_error == 0) {
			if (fubyte(u.u_base) == -1)
				goto fault;
			v = btop(u.u_offset);
			*(int *)mmap = v | (PG_V | PG_KR);
			mtpr(TBIS, vmmap);
			o = (int)u.u_offset & PGOFSET;
			c = min((unsigned)(NBPG - o), u.u_count);
			c = min(c, (unsigned)(NBPG - ((int)u.u_base&PGOFSET)));
			if (copyout((caddr_t)&vmmap[o], u.u_base, c))
				goto fault;
			u.u_count -= c;
			u.u_base += c;
			u.u_offset += c;
		}
		return;

	case 1:
		c = u.u_count;
		if (copyout((caddr_t)u.u_offset, u.u_base, c))
			goto fault;
		u.u_count = 0;
		u.u_base += c;
		u.u_offset += c;
		return;

	case 2:
		return;

	case 3:
		if (!useracc(u.u_base, u.u_count, B_WRITE))
			goto fault;
		if (UNIcpy((caddr_t)u.u_offset, u.u_base, u.u_count, B_READ))
			goto fault;
		c = u.u_count;
		u.u_count = 0;
		u.u_base += c;
		u.u_offset += c;
		return;

	case 4:
		if ((u_long)u.u_offset < (u_long)SYSADR)
			goto fault;
		c = u.u_count;

#define EXCLUDE(laddr, maddr)	\
		if ((u_long)u.u_offset >= (u_long)(laddr)) {	\
			if ((u_long)u.u_offset < (u_long)(maddr))	\
				goto fault;	\
		} else	\
			c = min(c, (u_long)(laddr) - (u_long)u.u_offset)

		/* cf. startup() in machdep.c */
		EXCLUDE(buffers, buffers + BUFSIZE*nbuf);
		EXCLUDE(blkdata, blkdata + NBLKDATA);
		EXCLUDE(ecmap, 0xffffffff);

		if (!kernacc((caddr_t)u.u_offset, c, B_READ))
			goto fault;
		if (copyout((caddr_t)u.u_offset, u.u_base, c))
			goto fault;
		u.u_count -= c;
		u.u_base += c;
		u.u_offset += c;
		return;

	case 5:
		c = min(u.u_count, sizeof(long));
		lbuf = umfpr(u.u_offset/sizeof(long));
		if (copyout((caddr_t)&lbuf, u.u_base, c))
			goto fault;
		u.u_count -= c;
		u.u_base += c;
		u.u_offset += c;
		return;
	}
fault:
	u.u_error = EFAULT;
	return;
}

mmwrite(dev)
{
	register int o; long lbuf;
	register unsigned c, v;

	switch (minor(dev)) {

	case 0:
		while (u.u_count != 0 && u.u_error == 0) {
			if (fubyte(u.u_base) == -1)
				goto fault;
			v = btop(u.u_offset);
			*(int *)mmap = v | (PG_V | PG_KW);
			mtpr(TBIS, vmmap);
			o = (int)u.u_offset & PGOFSET;
			c = min((unsigned)(NBPG - o), u.u_count);
			c = min(c, (unsigned)(NBPG - ((int)u.u_base&PGOFSET)));
			if (copyin(u.u_base, (caddr_t)&vmmap[o], c))
				goto fault;
			u.u_count -= c;
			u.u_base += c;
			u.u_offset += c;
		}
		return;

	case 1:
		if (copyin(u.u_base, (caddr_t)u.u_offset, u.u_count))
			goto fault;
		u.u_base += u.u_count;
		u.u_offset += u.u_count;
		u.u_count = 0;
		return;

	case 2:
		u.u_offset += u.u_count;
		u.u_count = 0;
		return;

	case 3:
		if (!useracc(u.u_base, u.u_count, B_READ))
			goto fault;
		if (UNIcpy((caddr_t)u.u_offset, u.u_base, u.u_count, B_WRITE))
			goto fault;
		u.u_base += u.u_count;
		u.u_offset += u.u_count;
		u.u_count = 0;
		return;

	case 5:
		if (u.u_count < sizeof(long))
			goto fault;
		if (copyin(u.u_base, (caddr_t)&lbuf, sizeof(long)))
			goto fault;
		if (umtpr(u.u_offset / sizeof(long), lbuf) == 0)
			goto fault;
		u.u_count -= sizeof(long);
		u.u_base += sizeof(long);
		u.u_offset += sizeof(long);
		return;
	}
fault:
	u.u_error = EFAULT;
	return;
}

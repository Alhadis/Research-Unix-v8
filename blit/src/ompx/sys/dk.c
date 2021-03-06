/*
 * Datakit driver
 * DR11C version
 */
#include "dk.h"
#ifdef	NDK
#ifdef VMUNIX
#define	bcopy	_bcopy
#define	copyin	_copyin
#define	copyout	_copyout
#else
#define	NDK	1
#define	KERNEL	1
#endif

#if NDK != 0





#include "../h/param.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/inode.h"
#include "../h/file.h"
#include "../h/systm.h"
#include "../h/proc.h"
#include "../h/buf.h"
#include "../h/tty.h"
#include "../h/local.h"
#include "../h/monitor.h"

#ifdef	vax
#include "../h/map.h"
#ifdef VMUNIX
#include "../h/pte.h"
#include "../h/ubavar.h"
#else
# include "../h/page2.h"
extern caddr_t devloc[];
#endif
#endif

/*
 * monitor cell and codes
 */
#ifdef	MONITOR
int *DKP =(int*)(DKBADDR);
#include "../h/monitor.h"
#endif

#ifdef	PDP
#define	DKADDR	(0167760)
#define GETBLK getablk(0)
#define	KDPAR5	((short *)0172372)
#define	DKPDR5	((short *)0172332)
short	savpar, savpdr;
short	*kdpar	= KDPAR5;
short	*kdpdr 	= DKPDR5;
#endif



/*
 * switch commands
 */
#define	RAM_ON	0226
#define	ROM_ON	0322
#define	R_CNTL	0264
#define	W_CNTL	0170



/*
 * dr11c bits
 */
#define	DKTENAB	0100	/* transmit interrupt enable */
#define	DKRENAB	040	/* receiver interrupt enable */
#define	ENABS	0140	/* both enables */
#define	DKCOM	03	/* dr11-c command bits */
#define	DKTDONE	0200	/* transmit done bit */
#define	DKRDONE	0100000	/* receiver done bit */
#define	DKMARK	01000	/* start of packet bit */
#define	DKOVF	040000	/* receiver overflow bit (in drin) */
#define	DKDATA	0400	/* bit 9 ... indicates non-control */
#define	DKPERR	0100000
#define	OFIFO	64	/* size of output fifo */
#define	DKCHUNK	16

/*
 * dr11c commands
 */
#define	D_OSEQ	0
#define	D_READ	1
#define	D_WRITE	2
#define	D_XPACK	3




/*
 * protocol codes generated by driver
 */
#define	T_LOC		2
#define	D_CLOSE		1
#define	D_OPEN		3
#define	D_TIMER		4
struct	lmsg {
	char	type;
	char	srv;
	short	param0;
	short	param1;
	short	param2;
	short	param3;
	short	param4;
	short	param5;
};

#define	NDKMODS		1
#define	CHANMODS	32
#define	NDKCHANS	(NDKMODS*CHANMODS)

/*
 * software states: m_state
 */
#define	M_OPEN	01		/* module is up and being used */
#define	M_BUSY	02		/* transmit interrupts are pending */




/*
 * shorthand
 */
#define	q5	dkimeta[tp-dkchans]
#define	q4	dkometa[tp-dkchans]
#define	q3	tp->t_outq
#define	q2	tp->t_canq
#define	q1	tp->t_rawq
#define	t_dkstate t_ospeed
#define	DKCMD	01		/* command channel */
#define	DKMPX	02		/* user-multiplexed */
#define	DKCALL	04		/* dialout mode */
#define	DKLINGR	010		/* lingering close mode */
#define	DKICHAN	020		/* pass channel numbers to user */
#define	DKCTRL	000		/* cntrl messages only */
#define	DKMERGE	0100
#define	DKBLOCK	0200
#ifndef MIN
#define	MIN(a,b)	((a<b)?a:b)
#endif

/*
 * dr11c device registers
 */
struct device {
	short	dkcsr;
	short	dko;
	short	dki;
};

/*
 * control struct for each module
 */
struct	module {
	short	m_state;
	short	m_lhn;			/* logical host number */
	short	m_chan;
	struct	tty *m_head;		/* head of circular output list */
	struct	tty *m_tail;		/* tail of output list */
	struct	tty *m_rrobin;		/* round robin pointer into list */
	struct	device	*m_addr;	/* hardware device address */
	short	m_xcount;		/* # free bytes in output fifo */
	short	m_mode;
	short	m_achans;		/* # active channels */
	short	m_badpack;		/* # bad packets received */
};

struct	module	modules[NDKMODS]  ={
	0, 0, 0,NULL,NULL,NULL, 0, 0, 0, 0, 0
};
struct	tty	dkchans[NDKCHANS];
struct	clist	dkimeta[NDKCHANS], dkometa[NDKCHANS];

struct	tty	*dklistener;
struct	proc	*dklp;
int dklchan;
struct	tty	*dkcommon;
int	dkrandom;

char metabuf[16];
#ifdef DTRACE
#include <dtrace.h>
#endif
int	dkattach(), dkprobe();
struct	uba_device *dkinfo[NDK];
u_short	dkstd[] = { 0 };
struct	uba_driver dkdriver =
	{ dkprobe, 0, dkattach, 0, dkstd, "dk", dkinfo };

dkprobe(reg)
caddr_t reg;
{
	register int br, cvec;
	register struct device *dkaddr = (struct device *)reg;

	modules[0].m_addr = dkaddr;
	dkaddr->dkcsr = D_OSEQ;
	dkaddr->dko = 0;	/* this clears fifos */
	dkaddr->dkcsr = D_WRITE;
	dkaddr->dko = DKMARK + 511;	/* pack on 511 */
	dkaddr->dkcsr = D_XPACK+DKTENAB;
	dkaddr->dko = 0;
	DELAY(10000);
	dkaddr->dkcsr = 0;
	return(1);
}

dkattach()
{
}

dkioctl(dev, cmd, addr, flag)
caddr_t addr;
{
struct buf *bp;
register struct tty *tp;
struct tty *stp;
struct module *mp;
int mod, chan, x, cc, xcc, s;
short ometabuf[8];
short imetabuf[8];
struct  {
	short	tchan;
} h;
register int (*cin)(), (*cout)();
extern int copyin(), copyout(), bcopy();

	tp = &dkchans[chan=minor(dev)];
	bp = NULL;
	mod = chan/CHANMODS;
	mp = &modules[mod];
	if (flag==0) {
		cmd = (*linesw[tp->t_line].l_ioctl)(tp, cmd, addr);
		if (cmd==0)
			return;
		cin = copyin;
		cout = copyout;
	} else {
		cin = cout = bcopy;
	}


#ifdef	VMUNIX
	if (ttioctl(tp, cmd, addr, flag)) {
#else
	if (ttioccomm(tp, cmd, addr, flag)) {
#endif
	} else 
		switch(cmd) {
		/*
		 * set listener mode on a channel.
		 * side effects are clock ticks and DKCALL mode.
		 */
		case DIOCLSTN:
			if (dklistener==NULL && u.u_uid==0) {
				tp->t_state = CARR_ON|ISOPEN|SPEEDS;
				tp->t_dkstate = DKCALL+DKCTRL;
				dklistener = tp;
				dklp = u.u_procp;
				dklchan = tp - dkchans;
				(*cin)(addr, (caddr_t)&mp->m_lhn, 2);
			} else {
				goto error;
			}
			break;

		case DIOCICHAN:
			tp->t_dkstate |= DKICHAN;
			break;
		case DIOCMERGE:
			chan = (int)addr;
			if (chan<0 || chan>=NDKCHANS) {
				u.u_error = ENXIO;
				return;
			}
			stp = tp;
			tp = &dkchans[chan];
			tp->t_linep = (caddr_t)stp;
			tp->t_dkstate |= DKMERGE;
			break;
		case DIOCUMERGE:
			chan = (int)addr;
			if (chan<1 || chan>=NDKCHANS) {
				u.u_error = ENXIO;
				return;
			}
			tp = &dkchans[chan];
			tp->t_dkstate &= ~DKMERGE;
			tp->t_linep = NULL;
			break;
		case DIOCPGRP:
			dksetgrp(dev, tp);
			break;
		/*
		 * MPX mode: 
		 *  user puts channel number before output data.
		 */
		case DIOCMPX:
			tp->t_dkstate |= DKMPX;
			break;
		case DIOCNMPX:
			tp->t_dkstate &= ~DKMPX;
			break;

		/*
		 * Turn CALL mode on and off.
		 *   In this mode, the driver inserts the
		 *   local host number and channel number
		 *   at the begining of each output packet
		 */
		case DIOCSCALL:
			tp->t_dkstate |= DKCALL+DKCTRL;
			break;
		case DIOCRCALL:
			tp->t_dkstate &= ~(DKCALL+DKCTRL);
			break;

		/*
		 * mark as control channel;
		 * probably should be protected.
		 */
		case DIOCMD:
			tp->t_dkstate |= DKCMD;
			break;
		case DIOCTRL:
			if (addr) {
				tp->t_dkstate |= DKCTRL;
			} else {
				tp->t_dkstate &= ~DKCTRL;
			}
			break;
		/*
		 * mark a channel really closed.
		 * this is for staying in synch
		 * with common control.
		 */
		case DIOCLOSE:
			if (tp!=dklistener)
				break;
			(*cin)(addr, (caddr_t)&x, sizeof x);
			tp = &dkchans[x];
			tp->t_state = 0;
			tp->t_dkstate = 0;
			break;
		/*
		 * enable time ticks to common control
		 */
		case DIOCTIME:
			if (dkcommon==NULL || u.u_ruid==0) {
				dkcommon = tp;
				tp->t_dkstate |= DKICHAN;
			}
			break;
		case DIOCRESET:
			if (tp!=dklistener && u.u_ruid!=0) 
				return;
			(*cin)(addr, (caddr_t)&x, sizeof x);
			dkclear(x);
			break;
		case DIOCSMETA:
			cc = DKCHUNK;
			(*cin)(addr, (caddr_t)ometabuf, cc);
			s = spl6();
			if (q4.c_cc < 6*DKCHUNK) {
				b_to_q(ometabuf, cc, &q4);
				dkqueue(tp);
			} else {
				printf("q4ovf\n");
			}
			splx(s);
			break;
		case DIOCXOUT:
			tp->t_dkstate |= DKBLOCK;
			break;
		case DIOCLOOP:
			if (tp!=dklistener)
				return;
			(*cin)(addr, (caddr_t)&h, sizeof h);
			addr += sizeof h;
			if (h.tchan<1 || h.tchan >=NDKCHANS) 
				goto error;
			tp = &dkchans[h.tchan];
			(*cin)(addr, (caddr_t)imetabuf, DKCHUNK);
			dkfpack(tp, 0, imetabuf, DKCHUNK);
			break;
		default:
		error:
			u.u_error = ENOTTY;
		}
}

/*
 * Make dk channel the control
 * typewriter for calling process.
 */
dksetgrp(dev, tp)
dev_t dev;
register struct tty *tp;
{
register struct proc *pp;

	u.u_ttyp = tp;
	u.u_ttyd = dev;
	pp = u.u_procp;
	pp->p_pgrp = pp->p_pid;
	tp->t_pgrp = pp->p_pgrp;
}
struct	lmsg openpack ={ T_LOC, D_OPEN, 0, 0, 0, 0, 0};
int	dkcalls;	/* total number of connections */
int	dkocalls;	/* total outgoing connnections */

dkopen(dev, flag)
{
register struct	tty *tp;
register chan;
struct	module *mp;
struct	inode *ip;
struct	file *fp;
int	 mod, m;
int	dkqueue();
int	dkioctl();
static	dktimer;

	chan = minor(dev);
	if (chan >= NDKCHANS) {
		u.u_error = ENXIO;
		return;
	}

	tp = &dkchans[chan];
	if (tp->t_state&XCLUDE && u.u_ruid!=0) {
	busy:
		u.u_error = EBUSY;
		return;
	}


	/*
	 * Channel 0 is reserved for maintenance.
	 * An open on channel 0 is interpreted as a request
	 * for an unused odd-numbered channel.
	 */
	spl6();
	if (dktimer==0) {
		dktimer++;
		dktimeout();
	}
	if (chan==0) {
		chan = 3;		/* chan 1 reserved */
		if (dklistener==NULL && u.u_ruid)
			goto busy;
		while (chan < NDKCHANS) {
			tp = &dkchans[chan];
			if (tp->t_state == 0 && tp->t_dkstate == 0)
				goto found;
			chan += 2;
		}
		spl0();
		goto busy;
found:
		/*
		 * throw away inode for dk0.
		 * manufacture new one for chan.
		 */
		tp->t_dkstate = DKCALL+DKLINGR+DKCTRL;
		fp = u.u_ofile[u.u_r.RVAL];
		ip = fp->f_inode;
		m = ip->i_mode;
		if ((ip=ialloc(rootdev))==NULL)
			goto busy;
		iput(fp->f_inode);
		ip->i_mode = m;
		ip->i_flag |= ICHG|IUPD;
		ip->i_un.RDEV = dev = makedev(major(dev), chan);
		fp->f_inode = ip;
		prele(ip);
		dkocalls++;
	}

	/*
	 * On the first open of the hardware interface
	 * initialize and check things out to the
	 * (grumble) limited extent that is possible.
	 */
	mod = chan/CHANMODS;
	mp = &modules[mod];
	if (mp->m_state==0) {
		register struct device *addr;
		register seq;

#ifdef	VAX
#ifdef	VMUNIX
		/*mp->m_addr = DKADDR;*/
#else
		mp->m_addr = (struct device *)(DKADDR+devloc[0]);
#endif
#endif
		addr = mp->m_addr;

		if (dkreset(modules)==0) {
			printf("module not plugged in\n");
			u.u_error = ENXIO;
			addr->dkcsr = 0;
			return;
		}
		mp->m_xcount = OFIFO;
		mp->m_state = M_OPEN;
		addr->dkcsr = ENABS;

#ifdef	PDP
		savpar = *kdpar;
		savpdr = *kdpdr;
		if (dkimeta >= 0120000) {
			panic("can't map pdr5");
		}
#endif
	}


	/*
	 * Finish setting up tp struct.
	 * Note that RAW mode is the default.
	 */
	if (tp->t_state & CARR_ON)
		goto out;
	dkcalls++;
	tp->t_oproc = dkqueue;
	tp->t_iproc = dkioctl;
	FLUSHTTY(tp);
	qflush(&q4); qflush(&q5);
	tp->t_addr = NULL;
	tp->t_state |= CARR_ON|SPEEDS;
	ttyopen(dev, tp);
	ttychars(tp);
	tp->t_flags = RAW;
	mp->m_achans++;
	if (dklp!=u.u_procp) {
		openpack.param0 = chan;
		openpack.param1 = u.u_ruid;
		dkfpack(dklistener, 1, &openpack, sizeof openpack);
	}
out:
	spl0();
}



/*
 * Close a channel:
	notify listener if one is running.
 *	turn off hardware interrupts on last close.
 */
struct	lmsg closepack ={ T_LOC, D_CLOSE, 0, 0, 0, 0, 0};

dkclose(dev, flag)
dev_t dev;
int flag;
{
register struct tty *tp;
register struct module *mp;
register struct device *addr;
int	n, s, state;

	tp = &dkchans[minor(dev)];
	if (tp->t_line) {
		(*linesw[tp->t_line].l_close)(tp);
	}
	else if (tp->t_linep)
		printf("tp->t_line==0; tp->t_linep=%x\n", tp->t_linep);
	s = spl5();
	if (dklistener && tp!=dklistener && tp!=dkcommon) {
		state = tp->t_dkstate & DKLINGR;
		closepack.param0 = minor(dev);
		if (tp->t_chan) {
			n = forceclose(tp->t_dev);
		} else
			n = 0;
		if (n==0)
			dkfpack(dklistener, 1, &closepack, sizeof closepack);
	} else
		state = 0;
	if (dkcommon == tp)
		dkcommon = NULL;
	if (dklistener == tp)  {
		dklistener = NULL;
		dklp = NULL;
		dklchan = 511;

/*
		dkclear(0);
*/
	}
	splx(s);
	mp = &modules[(minor(dev))/CHANMODS];

	tp->t_pgrp = 0;
	tp->t_dkstate = state;
	tp->t_state = 0;
	FLUSHTTY(tp);
	s = spl6();
	mp->m_achans--;
	if (mp->m_achans==0) {
		mp->m_state = 0;
		addr = mp->m_addr;
		addr->dkcsr = 0;
	}
	splx(s);

}



char *msg1	="short pack on %d\n";

/*
 * Input error counts.
 */
int	dkzero;		/* packets received on channel zero */
int	dkdead;		/* packets received for inactive channels */
int	dkheaderr;	/* parity errors in header bytes */
int	dkperrs;	/* packets with parity errors */
int	dkovf;		/* input fifo overflow */
int	dkundf;		/* underflow */
int	dknoise;	/* packets on channel 0777 */
int	dkecho;
int	odkecho;
long	dkinp;		/* total input packets */
long	dkoutp;		/* total output packets */
long	odkinp;
long	odkoutp;
int	resetcount;	/* number of hardware resets */


/*
 * Input buffer to collect data before passing to a line discipline.
 * Used for everything except block transfers
 * to a line discipline-supplied buffer.
 */
char dkibuf[4+(DKCHUNK*2)];


/*
 * Receiver interrupt:
 * unload packets
 * and pass data through linesw input entry.
 */
dkrint()
{
struct	module *mp;
register struct	device	*addr;
register char *cp;
register c;
char	*cb, *ce;
int	mod, chan, cc;
int	badpacks;
int	flag, firstc, mflag;
int	parity;
struct tty *tp;
struct buf *bp;
int s;

	M_ON(Mrint);
	M_OFF(Mrpack);
	mp = &modules[mod=0];
	addr = mp->m_addr;
#ifdef DTRACE
	trtrace(0, 0, Drint, 0, xspl(), 0);
#endif
	if (mp->m_state==0) {
		addr->dkcsr = 0;
		printf("dkrint ignored\n");
		return;
	}
	badpacks = 0;

	/*
	 * while there is data in the rcv fifo,
	 * copy it out.
	 */
	while (addr->dkcsr < 0) {
		M_OFF(Mrpack);
		addr->dkcsr = D_READ + ENABS;
		bp = NULL;

		/* 
		 * scan for first byte of packet.
		 * determine channel number.
		 */
		do {
			c = addr->dki;
begin:
			if (c & DKMARK) {
				if ((c&DKPERR)==0) {
					dkheaderr++;
					continue;
				}
				break;
			}
		} while (addr->dkcsr < 0);



		c &= 0777;
		/*
		 * If too many bad channel numbers are
		 * received, this may mean that power failed
		 * on datakit or that our module fell out
		 * of the backplane, or the cable is open.
		 */
		if (c == 0777) {
			dknoise++;
			if (++badpacks > 20000) {
				dkpanic(mp);
				return;
			}
			continue;
		}
		if (c == CHANMODS-1) {
			dkecho++;
			continue;
		}


		if (addr->dkcsr >= 0) {
			goto out;
		}
		flag = mflag = 0;
		if ((chan=c)==0) {		/* packet on channel zero */
			dkzero++;
			continue;
		}
						/* test for active channel */
		tp = &dkchans[c];
		if (c >= CHANMODS && (tp=dkcommon)==NULL) {
			dkdead++;
			continue;
		}
		if ((tp->t_state&CARR_ON)==0) {
			dkdead++;
			continue;
		}
		firstc = c = addr->dki;		/* look at first char */
		if (c&DKMARK) {			/* bad news - short packet */
			dkovf++;
			goto begin;
		}
						/*
						 * c_cc<0 means block
						 * transfer to a buffer.
						 */
		if (q1.c_cc < 0 && c&DKDATA) {
			flag++;
			if (tp->t_state&BEXT&&(bp=tp->t_ibp)!=NULL&&q1.c_cf) {
#ifdef	PDP
				paddr_t base;
				int off;

				base = paddr(bp) + bp->b_resid;
				off = base & 077;
				s = spl6();
				*kdpar = base>>6;
				*kdpdr = 077406;
				cp = (char *)(0120000 + off);
#endif
#ifdef	VAX
				cp = bp->b_un.b_addr + bp->b_resid;
#endif
			} else
			if (q1.c_cf==NULL)
				cp = dkibuf;  else
				cp = q1.c_cf;
			ce = cp + MIN(-q1.c_cc, DKCHUNK);
		} else {
			cp = dkibuf;
			ce = cp + DKCHUNK;
		}
		cb = cp;
		if (tp->t_dkstate&(DKMERGE+DKICHAN+DKCMD)) {
			if (tp->t_dkstate&DKMERGE) {
				tp = (struct tty *)tp->t_linep;
				mflag++;
			}
			if (tp->t_dkstate&DKICHAN) {
				*cp = chan;
				cp[1] = 0;
				cp += sizeof (short); ce += sizeof (short);
			}
			if (tp->t_dkstate & DKCMD) {
				c &= 0377;
				if (c==W_CNTL) {
					goto begin;
				}
				firstc = c|DKDATA;
				do {
					*cp++ = c;
					c = addr->dki;
				} while (cp<ce);
				goto skip;
			}
		}



		/*
		 * copy packet from dr11c
		 * stop when ce is reached or when
		 * bit 9 is zero or when the start of a new packet
		 * is detected.
		 */
		M_ON(Mrpack);
		parity = -1;
		do {
			*cp++ = c;
			parity &= c;
			if (c&DKMARK) {
				dkovf++;
				goto begin;
			}
		} while (cp<ce && addr->dkcsr<0 && (c=addr->dki)&DKDATA);

		if ((parity&DKPERR)==0) {
			dkperrs++;
			c = 0;
			goto begin;
		}



		/*
		 * flush unused portion of input packet.
		 */
			while (--ce > cp) {
				if (addr->dkcsr>=0) {
					dkundf++;
					break;
				}
				if (c&DKMARK) {
					dkovf++;
					break;
				}
				c = addr->dki;
			}

skip:
		if (mflag && tp->t_dkstate&DKICHAN) {
			dkibuf[1] = cp-cb;
		}

		dkinp++;
		if ((firstc&DKDATA)==0) {
#ifdef	DTRACE
			trtrace(tp, 0, Dcmeta, 0, xspl(), 0);
#endif
			(*linesw[tp->t_line].l_meta)(tp, dkibuf, cp);
			continue;
		}
merge:
		if (flag) {
			c = cp - cb;
			q1.c_cc += c;
			if (q1.c_cf!=NULL) {
				q1.c_cf = cp;
				if (bp) {
					bp->b_resid += c;
					bp = NULL;
#ifdef PDP
					*kdpar = savpar;
					*kdpdr = savpdr;
					splx(s);
#endif
				}
			} else {
				cp = cb = NULL;
			}
			if (q1.c_cc==0)
				(*linesw[tp->t_line].l_rend)(tp, cb, cp);
			continue;
		}
		if (tp->t_dkstate & DKCTRL)
			continue;
		(*linesw[tp->t_line].l_rend)(tp, dkibuf, cp);
	}
out:
	M_OFF(Mrint);
	M_OFF(Mrpack);
	addr->dkcsr = ENABS;

}


/*
 * Output start routine for a channel.
 *   normally called through t_oproc.
 *   calls dkstart if output interrupt process
 *   is not going.
 */
dkqueue(tp)
register struct tty *tp;
{
register struct module *mp;
register mod;
int	s;


	mod = tp-dkchans;
	mod /= CHANMODS;
	mp = &modules[mod];

	s = spl6();

	if (tp->t_state&BUSY || tp->t_addr!=NULL)
		goto out;
	if (mp->m_head == NULL)
		mp->m_rrobin = mp->m_head = mp->m_tail = tp;
	tp->t_addr = (caddr_t)mp->m_head;
	mp->m_head = tp;
	mp->m_tail->t_addr = (caddr_t)tp;
	
	tp->t_state |= BUSY;

	if ((mp->m_state&M_BUSY)==0)
		dkstart(mp);
out:
	splx(s);
}


/*
 * Send dummy packet on channel 511
 * to start output interrupt process.
 * Dkcsr must be saved in case this routine
 * is called from a read interrupt.
 */
dkstart(mp)
register struct module *mp;
{
register struct device *addr;
register short com;

	M_ON(Mstart+Mbusy);
	mp->m_state |= M_BUSY;
	addr = mp->m_addr;
	com = addr->dkcsr;
	addr->dkcsr = D_WRITE + ENABS;
	addr->dko = 511 | DKMARK;
	addr->dko = 0;
	addr->dkcsr = D_XPACK + ENABS;
	addr->dko = 0;
	mp->m_xcount -= 2;
	addr->dkcsr = com;
	M_OFF(Mstart);
}


/*
 * Transmitter interrupt routine:
 *  unload feedback fifo,
 *  copy out data,
 *  keep track of fifo space (m_xcount),
 *  on exit mark driver not-busy if xcount
 *  shows that output fifo is empty.
 */
char dkobuf[4*DKCHUNK+6];
dkxint()
{
register struct device *addr;
register cc;
register char *cp, *xp;
struct buf *bp;
struct	module *mp;
struct	tty *tp, *ltp;
int	dkdata, xfr, xcc, chan, s;

	mp = &modules[0];
	addr = mp->m_addr;
	M_ON(Mxint);
	if (mp->m_state==0) {
		printf("dkxint ignored\n");
		addr->dkcsr = 0;
		return;
	}
	addr->dkcsr = D_OSEQ + ENABS;

	while (addr->dkcsr&DKTDONE && mp->m_xcount < OFIFO*2) {
		mp->m_xcount += ((addr->dki>>10) & 017) + 2;
	}
	if (mp->m_xcount > OFIFO) {
		printf("dk ovf a %d\n", mp->m_xcount);
		mp->m_xcount = OFIFO;
	}

	ltp = mp->m_rrobin;
	bp = NULL;


	while (ltp!=NULL) {

		tp = (struct tty *)ltp->t_addr;
		s = spl6();
#ifndef VMUNIX
		if (tp->t_state&ASLEEP && q3.c_cc >= 0 && q3.c_cc <= TTLOWAT) {
#else
		if (tp->t_state&ASLEEP && q3.c_cc >= 0 &&
		    q3.c_cc <= TTLOWAT(tp)) {
#endif
			tp->t_state &= ~ASLEEP;
			if (tp->t_chan)
				mcstart(tp->t_chan, (caddr_t)&q3); else
				wakeup((caddr_t)&tp->t_outq);
		}
		if (tp->t_state&TTSTOP)
			goto undo;
		if (q3.c_cc==0 && q4.c_cc==0) {
			tp->t_state &= ~BUSY;
			if (tp->t_line)
				(*linesw[tp->t_line].l_start)(tp, &q4);
			if (q3.c_cc || q4.c_cc) {
				tp->t_state |= BUSY;
				goto more;
			}
			wakeup((caddr_t)&q3);

	undo:
			tp->t_state &= ~BUSY;
			/*
			 * no more data, remove tp from list.
			 */
			if (tp == ltp)
				ltp = mp->m_rrobin = mp->m_head = NULL;
			else {
				ltp->t_addr = tp->t_addr;
				if (mp->m_tail == tp)
					mp->m_tail = ltp;
				if (mp->m_head == tp)
					mp->m_head = (struct tty *)tp->t_addr;
				if (mp->m_rrobin == tp)
					mp->m_rrobin = ltp;
			}
			tp->t_addr = NULL;
		more:
			splx(s);
			continue;
		}

		if (addr->dkcsr < 0) {
#ifdef DTRACE
			trtrace(0, 0, Dcrint, 0, xspl(), 0);
#endif
			dkrint();
			ltp = mp->m_rrobin;
			continue;
		}
		splx(s);
		addr->dkcsr = D_WRITE + ENABS;
		chan = (tp-dkchans) % CHANMODS;
		dkdata = DKDATA;

		if (q3.c_cc==0 && q4.c_cc) {
			dkdata = 0;
			xfr = xcc = cc = MIN(q4.c_cc, DKCHUNK);
			cp = dkobuf;
			q_to_b(&q4, cp, xcc);
		} else
		if (q3.c_cc < 0) {
			xfr = xcc = cc = MIN(-q3.c_cc, DKCHUNK);
			if (tp->t_state&BEXT && (bp=tp->t_obp)!=NULL) {
#ifdef	PDP
				paddr_t base;
				int off;
/*
				cp = dkobuf;
				copyio(paddr(bp)+(int)q3.c_cf, cp, xcc, U_RKD);
				M_ON(Mbtop);
				buftopk(paddr(bp)+bp->b_resid, cp, DKCHUNK);
				bp->b_resid += xcc;
				M_OFF(Mbtop);
*/
				base = paddr(bp) + bp->b_resid;
				off = base & 077;
				s = spl6();
				cp = (char *)(0120000 + off);
				*kdpar = base>>6;
				*kdpdr = 077406;
#endif
#ifdef	VAX
				cp = bp->b_un.b_addr + bp->b_resid;
#endif
				bp->b_resid += xcc;
			} else {
				cp = q3.c_cf;
				q3.c_cf += xcc;
			}
			q3.c_cc += xcc;
		} else {
			int	xx, yy;

			xx = (tp->t_dkstate&DKCALL) ? 2 : 0;
			yy = 0;
			if (tp->t_dkstate&DKMPX) {
				chan = getc(&q3);
				yy = getc(&q3);
			}
			if (yy==0)
				yy = q3.c_cc;
			xcc = MIN(yy, DKCHUNK-xx);
			cc = xfr = xcc+xx;
			if (mp->m_xcount <= cc) {
				printf("xe %d\n",mp->m_xcount);
				goto out;
			}

			cp = dkobuf;


			if (tp->t_dkstate & (DKCMD+DKCTRL)) {
				dkdata = 0;
			}
			if (tp->t_dkstate & DKCALL) {
				mp->m_chan = chan;
				xp = (caddr_t)&mp->m_chan;
				while (xx--)
					*cp++ = *xp++;
				cc = xfr = DKCHUNK;
				chan = dklchan;
			}

			if (tp->t_state&NDQB) 
				trq_to_b(&q3, cp, xcc); else
				q_to_b(&q3, cp, xcc);
			cp = dkobuf;
		}

		/*
		 * copy CHUNK to interface
		 */
		M_ON(Mtpack);
		addr->dko = chan | DKMARK;
		addr->dko = (*cp++ & 0377) | dkdata;
		while (--cc)
			addr->dko = (*cp++ & 0377) | DKDATA;
		xfr--;

		if (bp) {
			bp = NULL;
#ifdef PDP
			*kdpar = savpar;
			*kdpdr = savpdr;
			splx(s);
#endif
		}

		/*
		 * send the transmit-packet cmd,
		 * load packet size into feedback fifo.
		 */
		addr->dkcsr = D_XPACK + ENABS;
		dkoutp++;
		addr->dko = xfr<<10;
		mp->m_xcount -= xfr+2;

		/*
		 * read out feedback fifo
		 */
		addr->dkcsr = D_OSEQ+ENABS;
		while(addr->dkcsr&DKTDONE && mp->m_xcount < OFIFO*2) {
			mp->m_xcount += ((addr->dki>>10)&017) + 2;
		}
		if (mp->m_xcount > OFIFO) {
			printf("dk ovf b %d\n", mp->m_xcount);
			mp->m_xcount = OFIFO;
		}

		addr->dkcsr = D_WRITE + ENABS;
		ltp = tp;
		M_OFF(Mtpack);
	}

out:
	s = spl6();
	mp->m_rrobin = ltp;
	addr->dkcsr = ENABS;
	if (mp->m_xcount==OFIFO) {
		mp->m_state &= ~M_BUSY;
		M_OFF(Mbusy);
	}
	M_OFF(Mxint);
	splx(s);
}


dkstop(tp, flag)
register struct tty *tp;
{

	if (q3.c_cc < 0)
		q3.c_cf = NULL;
	return;
}

dkread(dev UIO)
{
register struct tty *tp;

	tp = &dkchans[minor(dev)];
	if ((tp->t_state&CARR_ON)==0)
		return;
	(*linesw[tp->t_line].l_read)(tp UIO);
}

dkwrite(dev UIO)
{
register struct tty *tp;
register struct buf *bp;
register cc;
char *waddr;

	tp = &dkchans[minor(dev)];
	if ((tp->t_state&CARR_ON)==0)
		return(NULL);
	if (tp->t_dkstate&DKBLOCK) {
		cc = U(u_count);
		if (cc <= 0)
			return;
		bp = GETBLK;
		BIOMOVE(paddr(bp), MIN(BSIZE, cc), B_WRITE);
		spl6();
		while (q3.c_cc) {
			sleep((caddr_t)&q3, TTOPRI);
		}
		bp->b_bcount = cc;
		bp->b_resid = 0;
		tp->t_obp = bp;
		tp->t_state |= BEXT;
		q3.c_cc = -cc;
		dkqueue(tp);
		while (q3.c_cc < 0) {
			sleep((caddr_t)&q3, TTOPRI);
		}
		tp->t_obp = NULL;
		spl0();
		brelse(bp);
		return;
	}

horrible:
	waddr = (*linesw[tp->t_line].l_write)(tp UIO);
	if (tp->t_chan && waddr) {
		sleep(waddr, TTOPRI);
		if (u.u_count)
			goto horrible;
	}
}


/*
 * Generate hangups on open channels.
 */
dkclear(x)
register x;
{
register struct tty *tp;
register ch;

	if (x == 0) {
		for (tp= &dkchans[2]; tp < &dkchans[NDKCHANS]; tp++) {
			if (tp == dklistener || tp == dkcommon)
				continue;
			if (tp->t_dkstate & DKCMD)
				continue;
			if (tp->t_state)
				signal(tp->t_pgrp, SIGKIL);
			tp->t_state = 0;
			tp->t_dkstate = 0;
			FLUSHTTY(tp);
		}
		return;
	}
	if ((ch=x)<0)
		ch = -x;
	if (ch >= NDKCHANS)
		return;
	tp = &dkchans[ch];
	if (tp->t_line) {
		(*linesw[tp->t_line].l_modem)(tp);
	}
	if (tp->t_chan==NULL && forceclose(tp->t_dev)==0 && tp->t_line==0) {
		closepack.param0 = ch;
		dkfpack(dklistener, 1, &closepack, sizeof closepack);
		tp->t_state = tp->t_dkstate = 0;
		return;
	}
	if ((x&1)==0 || x<0)
		signal(tp->t_pgrp, (x<0)?SIGKIL:SIGHUP);
	tp->t_state = SPEEDS|CARR_ON;
	wakeup((caddr_t)&tp->t_outq);
	wakeup((caddr_t)&tp->t_rawq);
	if (tp->t_dkstate == DKLINGR) {
		return;
	}
	tp->t_dkstate = DKLINGR;
}


/*
 * Reset hardware.
 */
dkreset(mp)
register struct module *mp;
{
register struct device *addr;
register x, s, save;
	s = spl6();
	addr = mp->m_addr;
	save = addr->dkcsr;
	addr->dkcsr = D_OSEQ|ENABS;
	addr->dko = 0;
	for(x=OFIFO; x; x--) {
		if ((addr->dkcsr&DKTDONE)==0) {
			mp->m_xcount = OFIFO;
			addr->dkcsr = save;
			splx(s);
			return(1);
		}
	}
	addr->dkcsr = save;
	splx(s);
	return(0);
}

struct lmsg timestamp ={ T_LOC, D_TIMER, 0, 0, 0, 0, 0};
dktimeout()
{
register struct tty *tp;
static t;

	if ((tp=dklistener) != NULL)
		dkfpack(tp, 1, &timestamp, sizeof timestamp);
	if ((tp=dkcommon) != NULL)
		dkfpack(tp, 2, &timestamp, sizeof timestamp);

	if (t>1) {
		t = 0;
		if (dkecho==odkecho && (dkinp==odkinp && dkoutp==odkoutp)) {
			dkreset(modules);
			resetcount++;
			dkstart(modules);
		}
	} else 
		t++;
	odkecho = dkecho;
	odkinp = dkinp; odkoutp = dkoutp;

	timeout(dktimeout, (caddr_t)0, 15*hz);
}

dkfpack(tp, skip, m, cc)
register struct tty *tp;
register char *m;
{
register i;
	if (tp==NULL)
		return;
	for(i=0; i<skip; i++)
		putw(-1, &tp->t_rawq);
	ttyrend(tp, m, m+cc);
}


dkpanic(mp)
register struct module *mp;
{
register struct device *addr;

	addr = mp->m_addr;
	addr->dkcsr = 0;
	mp->m_state = 0;
	dkclear(0);
	printf("dk interface bad\n");
}

prstuff(s, cc)
register char *s;
register cc;
{
	while (cc--)
		printf("%o ", *s++&0377);
	printf("\n");
}

qflush(q)
register struct clist *q;
{
register s;

	s = spl6();
	while (getc(q) >= 0)
		;
	splx(s);
}
#ifdef VMUNIX
#undef copyin
_copyin(from, to, len)
	caddr_t from, to;
	int len;
{
	copyin(from, to, len);
}

#undef copyout
_copyout(from, to, len)
	caddr_t from, to;
	int len;
{
	copyout(from, to, len);
}

#undef bcopy
_bcopy(from, to, len)
	caddr_t from, to;
	int len;
{
	bcopy(from, to, len);
}
#endif
#endif

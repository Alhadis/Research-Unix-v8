#include "../chunix/chsys.h"
#include "../chunix/chconf.h"
#include "../chaos/chaos.h"
#include "../chaos/dev.h"
#include "../h/inode.h"
#include "../h/file.h"
#include "../h/tty.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/conf.h"
/*
 * This file contains functions for treating a chaos channel as a UNIX tty
 */
#include "cht.h"
struct tty cht_tty[NCHT * 16];
int cht_cnt = NCHT * 16;

/*
 * Do the additional work necessary to open a channel as a UNIX tty.
 * Basically the existence of an associated connection is much like
 * the existence of a carrier.
 */
/* ARGSUSED */
chtopen(dev, flag)
	dev_t dev;
{
	register struct tty *tp;
	register int unit;
	int chtstart(), chtinput();

	unit = minor(dev);
	if (unit >= cht_cnt) {
		u.u_error = ENXIO;
		return;
	}
	tp = &cht_tty[unit];
	if (tp->t_state & XCLUDE && u.u_uid != 0) {
		u.u_error = EBUSY;
		return;
	}
	tp->t_state |= WOPEN;
	tp->t_oproc = chtstart;
	tp->t_iproc = chtinput;
	if ((tp->t_state&ISOPEN) == 0) {
		ttychars(tp);
		tp->t_ispeed = B9600;
		tp->t_ospeed = B9600;
		tp->t_flags = ODDP|EVENP|ECHO;
		tp->t_state |= HUPCLS;
	}
	tp->t_lstate |= LSCHAOS;
	LOCK;
	/*
	 * Since ttyclose forces CARR_ON off, we turn it on again if
	 * the connection is still around.
	 */
	if (tp->t_addr)
		tp->t_state |= CARR_ON;
	else while ((tp->t_state & CARR_ON) == 0)
		sleep((caddr_t)&tp->t_rawq, TTIPRI);
	UNLOCK;
	tp->t_line = NTTYDISC;
	(*linesw[tp->t_line].l_open)(dev, tp);
}
/*
 * Close the tty associated with the given connection.
 */
chtclose(dev, flag)
	dev_t dev;
	int flag;
{
	register struct connection *conn;
	register struct tty *tp;

	tp = &cht_tty[minor(dev)];
	(*linesw[tp->t_line].l_close)(tp);
	conn = (struct connection *)tp->t_addr;
	if (tp->t_state & HUPCLS || conn->cn_state != CSOPEN) {
		/*
		 * We call the main close routine in RECORD mode, which
		 * closes the connection directly.
		 */
		conn->cn_mode = CHRECORD;
		chclose(conn, flag);
		tp->t_addr = 0;
		tp->t_state &= ~CARR_ON;
		tp->t_lstate &= ~LSCHAOS;
	}
	ttyclose(tp);
}

/*
 * Read from a chaos channel that is a tty.
 */
chtread(dev)
	dev_t dev;
{
	register struct tty *tp = &cht_tty[minor(dev)];
	register struct connection *conn = (struct connection *)tp->t_addr;
	/*
	 * Since ttys are quite possibly interactive, be sure
	 * to flush any output when input is desired.  It would
	 * be soon anyway due to timeouts.
	 */
	spl6();
	if (conn->cn_toutput != NOPKT && !chtfull(conn))
		ch_sflush(conn);
	spl0();
	(*linesw[tp->t_line].l_read)(tp);
}
int chttyraw;
/*
 * Write to a chaos channel that is a tty.
 */
chtwrite(dev)
	dev_t dev;
{
	register struct tty *tp = &cht_tty[minor(dev)];

	if (tp->t_flags & RAW) {
		chttyraw++;
		chwrite((struct connection *)tp->t_addr);
	} else
		(*linesw[tp->t_line].l_write)(tp);
}
/*
 * We only allow tty ioctl's for a chaos tty.
 * We could also allow chaos ioctl's if needed.
 */
chtioctl(dev, cmd, addr, flag)
	dev_t dev;
	caddr_t addr;
{
	register struct tty *tp = &cht_tty[minor(dev)];

	cmd = (*linesw[tp->t_line].l_ioctl)(tp, cmd, addr);
	if (cmd == 0)
		return;
	if (ttioctl(tp, cmd, addr, flag)) {
		if (cmd==TIOCSETP || cmd==TIOCSETN) {
			/*
			 * Send virtual terminal codes here?
			 */
/*			chparam(conn); */
		}
	} else switch(cmd) {
	/*
	 * We need a remote terminal protocol here. - Yick.
	 */
	case TIOCSBRK:
		/* maybe someday a code for break on output ? */
		break;
	case TIOCCBRK:
		break;
	case TIOCSDTR:
		break;
	case TIOCCDTR:
		break;
	default:
		u.u_error = ENOTTY;
	}
}
/*
 * Interrupt routine called when a new packet is available for a tty channel
 * Basically empty the packet into the tty system.
 * Called both from interrupt level when the read packet queue becomes
 * non-empty and also (at high priority) from top level.  THe top level
 * call is needed to retrieve data not queued at interrupt time due to
 * input queue high water mark reached.
 */
chtrint(conn)
struct connection *conn;
{
	register struct packet *pkt;
	register struct tty *tp = conn->cn_ttyp;
	register char *cp;

	while ((pkt = conn->cn_rhead) != NOPKT) {
		if (ISDATOP(pkt) && conn->cn_state == CSOPEN)
			for (cp = &pkt->pk_cdata[conn->cn_roffset];
			     pkt->pk_len != 0; pkt->pk_len--)
				if (tp->t_rawq.c_cc + tp->t_canq.c_cc >=
				    TTYHOG) {
					conn->cn_roffset = cp - pkt->pk_cdata;
					return;
				} else
					(*linesw[tp->t_line].l_rint)
						(*cp++ & 0377, tp);
		ch_read(conn);
	}
	/*
	 * Flush any output since we might have echoed something at
	 * interrupt level.
	 */
	if (conn->cn_toutput != NOPKT)
		ch_sflush(conn);
}
/*
 * Get more data if possible.  This is called from ttread (ntread) to
 * see if more data can be read from the connection.
 * It is only needed to account for the case where packets are not
 * completely emptied at interrupt level due to input clist buffer
 * overflow (>TTYHOG).  In this respect, chaos connections win better
 * than ttys, which just throw the data away.
 */
chtinput(tp)
register struct tty *tp;
{
	register int opl = spl6();

	chtrint((struct connection *)tp->t_addr);
	splx(opl);
}
/*
 * Interrupt routine called when more packets can again be sent after things
 * block due to window full.
 */
chtxint(conn)
register struct connection *conn;
{
	register struct tty *tp = conn->cn_ttyp;
	register int s = spl6();

	if (tp->t_state & ASLEEP) {
		tp->t_state &= ~ASLEEP;
		wakeup((caddr_t)&tp->t_outq);
	}
	if (tp->t_line)
		(*linesw[tp->t_line].l_start)(tp);
	else
		chtstart(tp);
	splx(s);
}
/*
 * Are we output flow controlled?
 */
chtblocked(tp)
	struct tty *tp;
{
	register struct connection *conn = (struct connection *)tp->t_addr;

	return chtfull(conn);
}
/*
 * Are we empty on output?
 */
chtnobuf(tp)
struct tty *tp;
{
	register struct connection *conn = (struct connection *)tp->t_addr;

	return !conn->cn_toutput;
}
/*
 * Flush any buffered output that we can.
 * Called from high priority.
 */
chtflush(tp)
struct tty *tp;
{
	register struct connection *conn = (struct connection *)tp->t_addr;

	
	if (conn->cn_toutput) {
		ch_free((caddr_t)conn->cn_toutput);
		conn->cn_toutput = NOPKT;
	}
}

/*
 * Start sending any buffered output.
 * We just start sending any packet that is partially full.
 */
chtstart(tp)
	struct tty *tp;
{
	ch_sflush((struct connection *)tp->t_addr);
}
/*
 * Put out one character and return non-zero if we couldn't
 */
chtputc(c, tp)
char c;
struct tty *tp;
{
	return chtout(&c, 1, tp);
}
/*
 * Send a contiguous array of bytes.
 * Return the number we can't accept now.
 * Packet allocation strategy:
 * We allocate a packet that is at least large enough to hold all bytes
 * remaining to be sent in this system call. We rely on the fact that
 * packets are really powers of two in size and at least 16 bytes of data,
 * since we don't round up our request at all.
 * If our "clever" request fails, we try for a small packet.
 */
chtout(cp, cc, tp)
	register char *cp;
	struct tty *tp;
{
	register struct connection *conn = (struct connection *)tp->t_addr;
	register struct packet *pkt;
	register int n;
	int sps = spl6();
	extern int ttrstrt();

	if (conn->cn_state == CSOPEN &&
	    (tp->t_state & (TIMEOUT|BUSY|TTSTOP)) == 0) {
		while (cc != 0) {
			if ((pkt = conn->cn_toutput) == NOPKT) {
				n = chtfull(conn) ? CHMAXDATA :
					chroundup(u.u_count + cc);
				if ((pkt = pkalloc(n, 1)) == NOPKT) {
					n = CHMINDATA;
					if ((pkt = pkalloc(n, 1)) == NOPKT)
						break;
				}
				pkt->pk_op = DATOP;
				pkt->pk_type = 0;
				pkt->pk_lenword = 0;
				conn->cn_troom = n;
				conn->cn_toutput = pkt;
			}
			if ((n = cc) > conn->cn_troom)
				n = conn->cn_troom;
			if (n != 0) {
				chmove(cp, &pkt->pk_cdata[pkt->pk_lenword], n);
				pkt->pk_time = Chclock;
				pkt->pk_lenword += n;
				cp += n;
			}
			cc -= n;
			if ((conn->cn_troom -= n) == 0)
				if (chtfull(conn))
					break;
				else
					ch_sflush(conn);
		}
	}
	splx(sps);
	return (cc);
}
/*
 * Process a connection state change for a tty.
 */
chtnstate(conn)
register struct connection *conn;
{
	register struct tty *tp;

	tp = conn->cn_ttyp;
	switch (conn->cn_state) {
	/*
	 * This shouldn't really happen since the connection shouldn't
	 * become a tty until it is open.
	 */
	case CSOPEN:
		if ((tp->t_state & CARR_ON) == 0) {
			wakeup((caddr_t)&tp->t_rawq);
			tp->t_state |= CARR_ON;
		}
		break;
	case CSCLOSED:
	case CSINCT:
	case CSLOST:
		if (tp->t_state & CARR_ON) {
			if ((tp->t_local & LNOHANG) == 0 &&
			    tp->t_state & ISOPEN) {
				gsignal(tp->t_pgrp, SIGHUP);
#ifdef SIGCONT
				gsignal(tp->t_pgrp, SIGCONT);
#endif
				flushtty(tp, FREAD|FWRITE);
			}
			tp->t_state &= ~CARR_ON;
		} else if (!(tp->t_state & ISOPEN)) {
			tp->t_addr = 0;
			ch_close(conn, NOPKT, 1);
			ch_buffree();
		}
		break;
	default:
		panic("chtnstate");
	}
}



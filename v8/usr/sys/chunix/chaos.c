/*
 * UNIX device driver interface to the Chaos N.C.P.
 */
#include "../chunix/chsys.h"
#include "../chunix/chconf.h"
#include "../chaos/chaos.h"
#include "../chaos/user.h"
#include "../chaos/dev.h"
#include "../h/inode.h"
#include "../h/file.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/conf.h"
#include "../h/ioctl.h"
#include "../h/buf.h"
#include "../h/systm.h"
#include "../h/tty.h"

#ifdef VMUNIX
#include "cht.h"
#endif

static int		initted;	/* NCP initialization flag */
int			Rfcwaiting;	/* Someone waiting on unmatched RFC */
static char *chnames[][3] = {
/* reading	writing		read/write */
{ "uread",	"ucreate",	"ureadwrite", },
};

#define CHFCONN(fp)	((fp)->f_conn)
#define CHIOPRIO	(PZERO+1)	/* Just interruptible */
#ifndef VMUNIX
#define setjmp save
#endif
static struct packet *fillpacket();
/*
 * Open a chaos channel. 
 *	Special case devices are indicated if CHHOST(dev) == CHHSPEC.
 *	See <chaos/dev.h> for the definitions.
 *	Non-special device opens imply a request to create a new connection
 *	via an RFC to a specified host, with a specified contact name string.
 *	The host number is specified one of three ways (CHHOST(dev)):
 *		CHHREAD		Read an ascii host number from the remainder
 *				of the pathname (after the component that
 *				matched this device).
 *		CHHUSE		Use the component that matched itself
 *				as an ascii number.
 *		else		Use the CHHOST(dev) as an index into an
 *				internal host table where frequently used
 *				host numbers should reside.
 *	The contact name can also be specified in three ways (CHNAME(dev)):
 *		CHNREAD		Read the contact string from the rest of the
 *				path name (possibly after host number).
 *		CHNUSE		Read the contact string from the path
 *				component that matched (possibly after host
 *				number).
 *		else		Use the CHNAME(dev) as an index into an
 *				internal array of contact names, also
 *				indexed by the type of open (read, write, rw).
 */
chropen(dev, flag)
dev_t dev;
{
	register struct connection *conn;
	register struct packet *pkt;
	int host, name, wstate;

	ch_bufalloc();
	/* initialize the NCP somewhere else? */
	if (!initted) {
		chrreset();	/* Reset drivers */
		chtimeout();	/* Start clock "process" */
		initted++;
	}
	conn = NOCONN;
	name = CHNAME(dev);
	host = CHHOST(dev);
	wstate = 0;
	if (host == CHHSPEC)
		switch (name) {
		case CHUNMATCHED:
			if(Chrfcrcv == 0) {
				Chrfcrcv++;
				return;
			}
			break;
		/*
		 * Listens create a connection waiting for RFC's containing
		 * the given contact name.  The returned file descriptor
		 * corresponds to a connection that may not be open yet.
		 * Use ioctls (CHIOCGSTAT, CHIOCSWAIT), to check or
		 * hang on the state of the connection.
		 */
		case CHLISTEN:
			if ((pkt = fillpacket("", 0)) != NOPKT)
				conn = ch_listen(pkt);
			break;
		}
	else {
		register int len = DIRSIZ;
		char *namp = u.u_dbuf;

		switch(host) {
		case CHHREAD:
			host = uatoi((char **)0);
			break;
		case CHHUSE:
			host = uatoi(&namp);	/* stuffs namp */
			break;
		default:
			host = chhosts[host];
		}
		if (u.u_error)
			host = 0;
		switch(name) {
		case CHNREAD:
			namp = "";
			len = 0;
			break;
		case CHNUSE:
			len = DIRSIZ - (namp - u.u_dbuf);
			break;
		default:
			len = CHMAXRFC;
			if ((namp = chnames[name][flag-1]) == NONAME)
				host = 0;
		}
		if (host != 0 && (pkt = fillpacket(namp, len)) != NOPKT &&
		    (conn = ch_open(host, CHDRWSIZE, pkt)) != NOCONN &&
		    CHHANGDEV(dev))
			wstate = CSRFCSENT;	/* Wait until OPEN/CLOSED */
	}
	if (conn == NOCONN) {
		u.u_error = ENXIO;
		ch_buffree();
	} else {
		register struct file *fp;

		fp = u.u_ofile[u.u_r.r_val1];	/* Yick. (see mx2.c) */
		CHFCONN(fp) = (caddr_t)conn;
		if (wstate) {
			/*
			 * We should hang until the connection changes from
			 * its initial state.
			 * If interrupted, flush the connection.
			 */
#ifdef VMUNIX
			if (setjmp(u.u_qsav) == 0) {
#else
			if (save(u.u_qsav) == 0) {
#endif
				spl6();
				while (conn->cn_state == wstate)	
					sleep((caddr_t)conn, CHIOPRIO);
				spl0();
#ifdef DEBUG
				if (ch_badaddr((char *)conn))
					panic("chopen");
#endif DEBUG
			}
			/*
			 * If the connection is not open, the open failed.
			 * Unless is got an ANS back.
			 */
			if (conn->cn_state != CSOPEN &&
			    (conn->cn_state != CSCLOSED ||
			     (pkt = conn->cn_rhead) == NOPKT ||
			     pkt->pk_op != ANSOP)) {
				rlsconn(conn);
				u.u_error = ENXIO;
				return;
			}
		}
		conn->cn_sflags |= CHRAW;
		conn->cn_mode = CHSTREAM;
	}
}

/* ARGSUSED */
chrclose(dev, flag, cp)
dev_t dev;
struct chan *cp;
{
	register struct connection *conn = (struct connection *)cp;

	if (minor(dev) == CHURFCMIN) {
		Chrfcrcv = 0;
		freelist(Chrfclist);
		Chrfclist = NOPKT;
		return;
	}
	/*
	 * If this connection has been turned into a tty, then the
	 * tty owns it and we don't do anything.
	 */
	if (conn->cn_mode != CHTTY)
		chclose(conn, flag);
}
chclose(conn, flag)
register struct connection *conn;
{
	register struct packet *pkt;

	switch (conn->cn_mode) {
	case CHTTY:
		panic("chclose on tty");
	case CHSTREAM:
		spl6();
		if (setjmp(u.u_qsav)) {
			pkt = pktstr(NOPKT, "User interrupted", 16);
			if (pkt != NOPKT)
				pkt->pk_op = CLSOP;
			ch_close(conn, pkt, 0);
			goto shut;
		}
		if (flag & FWRITE) {
			/*
			 * If any input packets other than the RFC are around
			 * something is wrong and we just abort the connection
			 */
			while ((pkt = conn->cn_rhead) != NOPKT) {
				ch_read(conn);
				if (pkt->pk_op != RFCOP)
					goto recclose;
			}
			/*
			 * We set this flag telling the interrupt time
			 * receiver to abort the connection if any new packets
			 * arrive.
			 */
			conn->cn_sflags |= CHCLOSING;
			/*
			 * Closing a stream transmitter involves flushing
			 * the last packet, sending an EOF and waiting for
			 * it to be acknowledged.  If the connection was
			 * bidirectional, the reader should have already
			 * read until EOF if everything is to be closed
			 * cleanly.
			 */
		checkfull:
			while (chtfull(conn)) {
				conn->cn_sflags |= CHOWAIT;
				sleep((caddr_t)&conn->cn_thead, CHIOPRIO);
			}
			if (conn->cn_state == CSOPEN ||
			    conn->cn_state == CSRFCRCVD) {
				if (conn->cn_toutput) {
					ch_sflush(conn);
					goto checkfull;
				}
				if (conn->cn_state == CSOPEN)
					(void)ch_eof(conn);
			}
			while (!chtempty(conn)) {
				conn->cn_sflags |= CHOWAIT;
				sleep((caddr_t)&conn->cn_thead, CHIOPRIO);
			}
		} else if (conn->cn_state == CSOPEN) {
			/*
			 * If we are only reading then we should read the EOF
			 * before closing and wiat for the other end to close.
			 */
			if (conn->cn_flags & CHEOFSEEN)
				while (conn->cn_state == CSOPEN)
					sleep((caddr_t)conn, CHIOPRIO);
		}
	recclose:
		spl0();
		/* Fall into... */
	case CHRECORD:	/* Record oriented close is just sending a CLOSE */
		if (conn->cn_state == CSOPEN) {
			pkt = pkalloc(0, 0);
			if (pkt != NOPKT) {
				pkt->pk_op = CLSOP;
				pkt->pk_len = 0;
			}
			ch_close(conn, pkt, 0);
		}
	}
shut:
	spl0();
	ch_close(conn, NOPKT, 1);
	ch_buffree();
}
/*
 * Raw read routine.
 */
chrread(dev)
dev_t dev;
{
	register struct connection *conn;
	register struct packet *pkt;
	register int count, n;

	if(minor(dev) == CHURFCMIN) {
		spl6();
		while ((pkt = ch_rnext()) == NOPKT) {
			Rfcwaiting++;
			sleep((caddr_t)&Chrfclist, CHIOPRIO);
		}
		spl0();	
		if (u.u_count < pkt->pk_len)
			u.u_error = EIO;
		else
			iomove((caddr_t)pkt->pk_cdata, pkt->pk_len, B_READ);
		return;
	}
	conn = (struct connection *)CHFCONN(getf(u.u_arg[0]));

	switch (conn->cn_mode) {
	case CHTTY:
		panic("chread on tty");
		break;
	case CHSTREAM:
		for (count = u.u_count; u.u_count != 0; )
			switch (n = ch_sread(conn, (char*)0, u.u_count)) {
			case 0:	/* No data to read */
				if (count != u.u_count)
					return;
				spl6();
				while (chrempty(conn)) {
					conn->cn_sflags |= CHIWAIT;
					sleep((char *)&conn->cn_rhead,
						CHIOPRIO);
				}
				spl0();
				break;
			case CHEOF:
				return;
			default:
				if (n < 0) {
					u.u_error = EIO;
					return;
				}
			}
		break;
	/*
	 * Record oriented mode gives a one byte packet opcode
	 * followed by the data in the packet.  The buffer must
	 * be large enough to fit the data and the opcode, otherwise an
	 * i/o error results.
	 */
	case CHRECORD:
		spl6();
		while (chrempty(conn)) {
			conn->cn_sflags |= CHIWAIT;
			sleep((char *)&conn->cn_rhead,
				CHIOPRIO);
		}
		spl0();
		if ((pkt = conn->cn_rhead) == NOPKT ||
		    pkt->pk_len + 1 > u.u_count)	/* + 1 for opcode */
			u.u_error = EIO;
		else {
			iomove((caddr_t)&pkt->pk_op, 1, B_READ);
			iomove((caddr_t)pkt->pk_cdata, pkt->pk_len, B_READ);
			spl6();
			ch_read(conn);
			spl0();
		}
	}
}
/*
 * Raw write routine
 * Note that user programs can write to a connection
 * that has been CHIOCANSWER"'d, implying transmission of an ANS packet
 * rather than a normal packet.  This is illegal for TTY mode connections,
 * is handled in the system independent stream code for STREAM mode, and
 * is handled here for RECORD mode.
 */
chrwrite(dev)
dev_t dev;
{
	register struct connection *conn;
	
	if(minor(dev) == CHURFCMIN) {
		u.u_error = EIO;
		return;
	}
	conn = (struct connection *)CHFCONN(getf(u.u_arg[0]));
	chwrite(conn);
}
chwrite(conn)
	register struct connection *conn;
{
	register struct packet *pkt;
	register int n;
	
	if (conn->cn_rhead != NOPKT && conn->cn_rhead->pk_op == RFCOP) {
		spl6();
		ch_read(conn);
		spl0();
	}
	switch (conn->cn_mode) {
	case CHTTY:
		/* Fall into (on RAW mode only) */
	case CHSTREAM:
		while (u.u_count != 0)
			switch (n = ch_swrite(conn, (char *)0, u.u_count)) {
			case 0:
				spl6();
				while (chtfull(conn)) {
					conn->cn_sflags |= CHOWAIT;
					sleep((caddr_t)&conn->cn_thead,
						CHIOPRIO);
				}
				spl0();
				break;
			case CHTEMP:
				sleep((caddr_t)&lbolt, CHIOPRIO);
				break;
			default:
				if (n < 0) {
					u.u_error = EIO;
					return;
				}
			}
		break;
	case CHRECORD:	/* one write call -> one packet */
		if (u.u_count < 1 || u.u_count - 1 > CHMAXDATA) {
			u.u_error = EIO;
			return;
		}
	loop:
		spl6();
		while (chtfull(conn)) {
			conn->cn_sflags |= CHOWAIT;
			sleep((caddr_t)&conn->cn_thead, CHIOPRIO);
		}
		spl0();
		if ((pkt = pkalloc((int)(u.u_count - 1), 0)) == NOPKT) {
			sleep((caddr_t)&lbolt, CHIOPRIO);
			goto loop;
		}
		iomove(&pkt->pk_op, 1, B_WRITE);
		pkt->pk_len = u.u_count;
		if (u.u_count)
			iomove(pkt->pk_cdata, u.u_count, B_WRITE);
		if (u.u_error == 0) {
			spl6();
			if (ch_write(conn, pkt))
				u.u_error = EIO;
			spl0();
		}
	}
}

/*
 * This routine allocates a packet and fills it with data from pointer in
 * user space until null or CHMAXDATA characters
 */
static struct packet *
fillpacket(str, len)
register char *str;
int len;	/* assumed <= CHMAXDATA */
{
	register char *ptr;
	register struct packet *pkt;
	int c;
	extern uchar();

	if ((pkt = pkalloc(CHMAXDATA, 0)) != NOPKT) {
		ptr = pkt->pk_cdata;
		for (pkt->pk_len = 0; *str != '\0' && pkt->pk_len < len; pkt->pk_len++)
			*ptr++ = *str++;
		while ((c = uchar()) > 0 && pkt->pk_len < CHMAXDATA) {
			*ptr++ = c;
			pkt->pk_len++;
		}
		if (c > 0) {
			debug(DABNOR,
				printf("Fillpacket too long: %s\n",
					pkt->pk_cdata));
			ch_free((char *)pkt);
		} else
			return(pkt);
	}
	u.u_error = ENXIO;
	return(NOPKT);
}
/*
 * This is atoi from users space terminates on / or null.
 * If cpp is not NULL then get the number from *cpp (u.u_dbuf).
 * *cpp is stuffed where the scan stopped, enabling further usage of the
 * contents of u.u_dbuf.
 */
static
uatoi(cpp)
char **cpp;
{
	register char *p = cpp ? *cpp - 1 : (char *)0;
	register int i = 0, chr;
	extern uchar();

	for (;;) {
		if (p)
			if (++p >= &u.u_dbuf[DIRSIZ])
				break;
			else
				chr = *p;
		else if ((chr = uchar()) == '/')
			break;
		if ((chr -= '0') < 0 || chr > 9)
			break;
		i *= 10;
		i += chr;
	}
	if (cpp)
		*cpp = p;
	return(i);
}

/*
 * Raw ioctl routine - perform non-connection functions, otherwise call down
 */
chrioctl(dev, cmd, addr, flag)
register int cmd;
caddr_t addr;
{
	register char *cp;
	register int c;

	switch (cmd) {
	/*
	 * Skip the first unmatched RFC at the head of the queue
	 * and mark it so that ch_rnext will never pick it up again.
	 */
	case CHIOCRSKIP:
		if (minor(dev) != CHURFCMIN)
			break;
		spl6();
		ch_rskip();
		spl0();
		return;
	/*
	 * Specify the chaosnet address of an interlan ethernet interface.
	 */
	case CHIOCILADDR:
		if (minor(dev) != CHURFCMIN)
			break;
#if NCHIL > 0
		{
			struct chiladdr ca;

			if (addr == 0 || copyin(addr, (caddr_t)&ca, sizeof(ca)))
				break;
			if (chilseta(ca.cil_device, ca.cil_address))
				break;
			return;
		}
#else
		break;
#endif
	case CHIOCNAME:
		if (minor(dev) != CHURFCMIN)
			break;
		for (cp = Chmyname; c = fubyte(addr); *cp++ = c, addr++)
			if (c < 0) {
				cp = Chmyname;
				u.u_error = EFAULT;
				break;
			} else if(cp >= &Chmyname[CHSTATNAME])
				break;
		while (cp < &Chmyname[CHSTATNAME])
			*cp++ = '\0';
		if (c >= 0)
			return;
		break;
	/*
	 * Specify my own network number.
	 */
	case CHIOCADDR:
		if (minor(dev) != CHURFCMIN)
			break;
		Chmyaddr = (int)addr;
		return;
	default:
		if (minor(dev) == CHURFCMIN)
			break;
		chioctl(CHFCONN(getf(u.u_arg[0])), dev, cmd, addr, flag);
		return;
	}
	u.u_error = ENXIO;
}
/* ARGSUSED */
chioctl(conn, dev, cmd, addr, flag)
register struct connection *conn;
dev_t dev;
caddr_t addr;
{
	register struct packet *pkt;
	struct chstatus chst;

	switch(cmd) {
	/*
	 * Read the first packet in the read queue for a connection.
	 * This call is primarily intended for those who want to read
	 * non-data packets (which are normally ignored) like RFC
	 * (for arguments in the contact string), CLS (for error string) etc.
	 * The reader's buffer is assumed to be CHMAXDATA long.  When ioctl's
	 * change on the VAX to have data length arguments, this will be done
	 * right. An error results if there is no packet to read.
	 * No hanging is currently provided for.
	 * The normal mode of operation for reading such packets is to
	 * first do a CHIOCGSTAT call to find out whether there is a packet
	 * to read (and what kind) and then make this call - except for
	 * RFC's when you know it must be there.
	 */	
	case CHIOCPREAD:
		if ((pkt = conn->cn_rhead) == NULL)
			break;
		if (copyout((caddr_t)pkt->pk_cdata, addr, pkt->pk_len))
			break;
		spl6();
		ch_read(conn);
		spl0();
		return;
	/*
	 * Change the mode of the connection.
	 * The default mode is CHSTREAM.
	 */
	case CHIOCSMODE:
		switch ((int)addr) {
		case CHTTY:
#if NCHT > 0
			if (conn->cn_state == CSOPEN &&
			    conn->cn_mode != CHTTY) {
				register struct tty *tp;
				extern struct tty cht_tty[];
				extern int cht_cnt;
				/*
				 * To turn a connection into a tty,
				 * we need to find a tty that is waiting to
				 * be opened and connect ourselves to it.
				 */
				 for (tp = cht_tty; tp < &cht_tty[cht_cnt]; tp++)
				 	if (tp->t_addr == 0 &&
					    tp->t_state & WOPEN) {
						tp->t_addr = (caddr_t)conn;
						tp->t_state |= CARR_ON;
						conn->cn_ttyp = tp;
						conn->cn_mode = CHTTY;
						wakeup((caddr_t)&tp->t_rawq);
						return;
					}

			}
#endif
			break;
		case CHSTREAM:
		case CHRECORD:
			if (conn->cn_mode == CHTTY)
				break;
			conn->cn_mode = (int)addr;
			return;
		}
		break;
	/*
	 * Flush the current output packet if there is one.
	 * This is only valid in stream mode.
	 * If the argument is non-zero an error is returned if the
	 * transmit window is full, otherwise we hang.
	 */
	case CHIOCFLUSH:
		if (conn->cn_mode == CHSTREAM) {
			spl6();
			while ((flag = ch_sflush(conn)) == CHTEMP)
				if (addr)
					break;
				else {
					conn->cn_sflags |= CHOWAIT;
					sleep((caddr_t)&conn->cn_thead,
						CHIOPRIO);
				}
			if (flag)
				u.u_error = EIO;
			spl0();
			return;			
		}
		break;
	/*
	 * Wait for all output to be acknowledged.  If addr is non-zero
	 * an EOF packet is also sent before waiting.
	 * If in stream mode, output is flushed first.
	 */
	case CHIOCOWAIT:
		if (conn->cn_mode == CHSTREAM) {
			spl6();
			while ((flag = ch_sflush(conn)) == CHTEMP) {
				conn->cn_sflags |= CHOWAIT;
				sleep((caddr_t)&conn->cn_thead, CHIOPRIO);
			}
			spl0();
 			if (flag) {
				u.u_error = EIO;
				return;
			}
		}
		if (addr) {
			spl6();
			while (chtfull(conn)) {
				conn->cn_sflags |= CHOWAIT;
				sleep((caddr_t)&conn->cn_thead, CHIOPRIO);
			}
			flag = ch_eof(conn);
			spl0();
			if (flag) {
				u.u_error = EIO;
				return;
			}
		}
		spl6();
		while (!chtempty(conn)) {
			conn->cn_sflags |= CHOWAIT;
			sleep((caddr_t)&conn->cn_thead, CHIOPRIO);
		}
		spl0();
		if (conn->cn_state != CSOPEN)
			u.u_error = EIO;
		return;
	/*
	 * Return the status of the connection in a structure supplied
	 * by the user program.
	 */
	case CHIOCGSTAT:
		chst.st_fhost = conn->cn_faddr;
		chst.st_cnum = conn->cn_ltidx;
		chst.st_rwsize = conn->cn_rwsize;
		chst.st_twsize = conn->cn_twsize;
		chst.st_state = conn->cn_state;
		chst.st_cmode = conn->cn_mode;
		chst.st_oroom = conn->cn_twsize - (conn->cn_tlast - conn->cn_tacked);
		if ((pkt = conn->cn_rhead) != NOPKT) {
			chst.st_ptype = pkt->pk_op;
			chst.st_plength = pkt->pk_len;
		} else {
			chst.st_ptype = 0;
			chst.st_plength = 0;
		}
		if (copyout((caddr_t)&chst, addr, sizeof(chst)))
			break;
		return;
	/*
	 * Wait for the state of the connection to be different from
	 * the given state.
	 */
	case CHIOCSWAIT:
		spl6();
		while (conn->cn_state == (int)addr)
			sleep((caddr_t)conn, CHIOPRIO);
		spl0();
		return;
	/*
	 * Answer an RFC.  Basically this call does nothing except
	 * setting a bit that says this connection should be of the
	 * datagram variety so that the connection automatically gets
	 * closed after the first write, whose data is immediately sent
	 * in an ANS packet.
	 */
	case CHIOCANSWER:
		spl6();
		if (conn->cn_state == CSRFCRCVD && conn->cn_mode != CHTTY)
			conn->cn_flags |= CHANSWER;
		else
			u.u_error = EIO;
		spl0();
		return;
	/*
	 * Reject a RFC, giving a string (null terminated), to put in the
	 * close packet.  This call can also be used to shut down a connection
	 * prematurely giving an ascii close reason.
	 */
	case CHIOCREJECT:
		spl6();
		if (conn->cn_state == CSRFCRCVD || conn->cn_state == CSOPEN) {
			u.u_dirp = addr;	/* a kludge for fillpacket */
			pkt = fillpacket("", 0);
			pkt->pk_op = CLSOP;
			ch_close(conn, pkt, 0);
		} else
			u.u_error = EIO;
		spl0();
		return;
	/*
	 * Accept an RFC causing the OPEN packet to be sent
	 */
	case CHIOCACCEPT:
		spl6();
		if (conn->cn_state == CSRFCRCVD)
			ch_accept(conn);
		else
			u.u_error = EIO;
		spl0();
		return;
	/*
 	 * Count how many bytes can be immediately read.
	 */
	case FIONREAD:
		if (conn->cn_mode != CHTTY) {
			off_t nread = 0;

			for (pkt = conn->cn_rhead; pkt != NOPKT; pkt = pkt->pk_next)
				if (ISDATOP(pkt))
					nread += pkt->pk_len;
			if (conn->cn_rhead != NOPKT)
				nread -= conn->cn_roffset;
			if (copyout((caddr_t)&nread, addr, sizeof(off_t)))
				u.u_error = EFAULT;
			return;
		}
	default:
		break;
	}
	u.u_error = ENXIO;
}
/*
 * Timeout routine that implements the chaosnet clock process.
 */
chtimeout()
{
	register int s = spl6();
	ch_clock();
	timeout(chtimeout, 0, 1);
	splx(s);
}

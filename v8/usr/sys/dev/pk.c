/*
 * Packet protocol processor
 */

#include "../h/param.h"
#include "../h/stream.h"
#include "../h/conf.h"
#include "../h/ioctl.h"
#include "../h/ttyld.h"
#include "pk.h"

struct pack {
	u_char	istate;
	u_char	ostate;
	u_char	xlogseg;	/* code for xmit segment size */
	u_char	S;		/* current output seqno */
	u_char	SN;		/* next output seqno */
	u_char	XW;		/* size of output window */
	u_char	R;		/* current input seqno */
	u_char	timer;
	short	xsegment;	/* output segment size */
	short	iwant;		/* # chars needed before i q enable */
	struct	block *xpacks[8];	/* waiting output blocks */
	struct {
		u_char	k;	/* packet size */
		u_char	c0;	/* low checksum byte */
		u_char	c1;	/* high checksum byte */
		u_char	C;	/* control byte */
		u_char	x;	/* header check */
	} hdr;			/* copy of header of received segment */
	struct	queue	*rdq;	/* associated read queue */
};

#define	PK_TYPE	0xC0
#define	PKCNTL	0x00
#define	PKDATA	0x80
#define	PKSDATA 0xC0

#define	PK_CLOSE 010
#define	PK_RJ	 020
#define	PK_RR	 040
#define	PK_INITC 050
#define	PK_INITB 060
#define	PK_INITA 070

#define	CHECK	0125252

#define	WDATA	01
#define	PKDELIM	02
#define	PKREJ	04

#define	SYN	020		/* sic */

#define	SINITA	1
#define	SINITB	2
#define	SINITC	3
#define	OPEN	4
#define	LCLOSE	5
#define	RCLOSE	6

#define	PKPRI	28
#define	DKDATA	0400

#define	RWINSIZE 3
#define	RSEGSIZE 1		/* 64 bytes */

#define	PKTIME	5

struct	pack	packs[NPK];
short	pksizes[] = { 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };
u_short	pkc_sum;
u_short	pkc_x;
int	pkc_size;

int	pkiput(), pkisrv(), pkoput(), pkosrv(), pkopen(), pkclose();
static	struct qinit pkrinit = { pkiput, pkisrv, pkopen, pkclose, 512, 64 };
static	struct qinit pkwinit = { pkoput, pkosrv, pkopen, pkclose, 128, 65 };
struct	streamtab pkinfo = { &pkrinit, &pkwinit };

pkopen(q)
register struct queue *q;
{
	register struct pack *pkp;
	register s;
	static timer = 0;
	int pktimer();

	if (q->ptr) {
		pkp = (struct pack *)q->ptr;
		if (pkp->ostate != OPEN)
			return(0);	/* don't reopen half-open chans */
		return(1);
	} else {
		for (pkp = packs; pkp->ostate!=0; pkp++)
			if (pkp >= &packs[NPK])
				return(0);
		pkp->rdq = q;
		q->ptr = (caddr_t)pkp;
		WR(q)->ptr = (caddr_t)pkp;
		pkp->S = 1;
		pkp->SN = 1;
		pkp->R = 0;
		putctl(q->next, M_FLUSH);
		pkp->ostate = SINITA;
		pkp->istate = 0;
		pkp->timer = 2;
		pkscntl(pkp, PK_INITA+RWINSIZE);
	}
	if (timer == 0) {
		timer = 1;
		timeout(pktimer, (caddr_t)NULL, 60);
	}
	s = spl5();
	while (pkp->ostate != OPEN) {
		switch (tsleep((caddr_t)pkp, PKPRI, 15)) {
		case TS_OK:
			continue;
		case TS_SIG:
		case TS_TIME:
			pkp->ostate = 0;
			splx(s);
			return(0);
		}
	}
	q->flag |= QDELIM;
	splx(s);
	return(1);
}

/*
 * Shut it down.
 *  The problem is to dispose of unacked stuff in the window.
 *   -- no real solution; the receiver might hang on for hours.
 *   Give it 15 seconds.
 */
pkclose(q)
register struct queue *q;
{
	register struct pack *pkp;
	register s = spl5();
	register i;

	pkp = (struct pack *)q->ptr;
	pkp->ostate = LCLOSE;
	flushq(q, 1);
	for (i=0; pkp->S < pkp->SN && i<15; i++)
		tsleep((caddr_t)pkp, PKPRI, 1);
	splx(s);
	flushq(WR(q), 1);
	pkrack(pkp, pkp->SN-1, 0);
	pkscntl(pkp, PK_CLOSE);
	pkp->ostate = 0;
}


/*
 * Process a bunch of input
 */
pkisrv(q)
register struct queue *q;
{
	register struct pack *pkp = (struct pack *)q->ptr;
	register struct block *bp;
	register segsize, msgsize;
	int R, OK;
	u_char c;

more:
	while ((pkp->istate & WDATA) == 0) {
		if (pkwant(pkp, 6)==0)
			return;
		pkgetb(q, &c, 1);
		if (c != SYN)
			continue;
		pkgetb(q, (u_char *)&pkp->hdr, 5);
		if (pkp->hdr.x != (pkp->hdr.k ^ pkp->hdr.c0 ^
		    pkp->hdr.c1 ^ pkp->hdr.C)) {	/* bad header? */
			continue;
		}
		switch (pkp->hdr.C & PK_TYPE) {
		case PKCNTL:
			pkp->istate &= ~PKREJ;
			pkcntl(pkp);
			continue;
	
		case PKDATA:
		case PKSDATA:
			if (pkp->ostate==SINITC) {
				pkp->ostate = OPEN;
				wakeup((caddr_t)pkp);
			}
			if (pkp->S != pkp->SN)	/* if need an ack */
				pkrack(pkp, pkp->hdr.C&07, 0);
			if (pkp->hdr.k<1 || pkp->hdr.k>8) {
				continue;
			}
			pkp->istate |= WDATA;
			break;

		default:
			continue;
		}
		break;
	}
	if (q->next->flag&QFULL)
		return;
	msgsize = segsize = pksizes[pkp->hdr.k];
	if (pkwant(pkp, segsize) == 0)
		return;
	/*
 	 * Now have a segment's worth of data; if in window, compute checksum
	 * and accept if all is OK
	 */
	R = (pkp->hdr.C>>3) & 07;
	if (R <= pkp->R)
		R += 8;
	/* compute checksum */
	OK = 0;
	pkc_sum = -1;
	pkc_x = 0;
	pkc_size = segsize;
	for (bp = q->first; segsize>0; bp = bp->next) {
		register sz = bp->wptr - bp->rptr;
		pkcksum(bp->rptr, sz);
		segsize -= sz;
	}
	segsize = msgsize;
	pkc_sum ^= pkp->hdr.C;
	pkc_sum += (pkp->hdr.c1<<8) + pkp->hdr.c0;
	if (pkc_sum != CHECK || R != pkp->R+1) {	/* Bad? */
		if (R < pkp->R+RWINSIZE) {
			flushq(q, 0);
			if ((pkp->istate & PKREJ) == 0)
				pkscntl(pkp, PK_RJ+pkp->R);
			pkp->istate &= ~WDATA;
			pkp->istate |= PKREJ;
			goto more;
		}
		msgsize = -1;		/* duplicate; just ignore */
	} else {	/* It's good */
		pkp->R = (pkp->R+1)&07;
		OK = 1;
		if ((pkp->hdr.C&PK_TYPE) == PKSDATA) {
			pkgetb(q, &c, 1);
			msgsize -= c & 0x7F;
			segsize -= 1;
			if (c&0x80) {
				pkgetb(q, &c, 1);
				msgsize -= c << 7;
				segsize -= 1;
			}
		}
	}
	pkp->istate &= ~PKREJ;
	pkscntl(pkp, PK_RR+pkp->R);
	while (segsize > 0) {
		register i;
		bp = getq(q);
		i = bp->wptr - bp->rptr;
		if (i <= msgsize) {
			(*q->next->qinfo->putp)(q->next, bp);
			msgsize -= i;
			segsize -= i;
			continue;
		}
		if (msgsize>0) {
			register struct block *bp1 = allocb(msgsize);
			if (bp1) {
				bcopy(bp->rptr, bp1->wptr, msgsize);
				bp->rptr += msgsize;
				bp1->wptr += msgsize;
				(*q->next->qinfo->putp)(q->next, bp1);
				segsize -= msgsize;
				msgsize = 0;
				putbq(q, bp);
				continue;
			} else
				panic("pk alloc\n");
		}
		if (i <= segsize) {
			freeb(bp);
			segsize -= i;
			continue;
		}
		bp->rptr += segsize;
		putbq(q, bp);
		break;
	}
	if (OK)
		putctl(q->next, M_DELIM);
	pkp->istate &= ~WDATA;
	goto more;
}

/*
 * Packet arrives.
 */
pkiput(q, bp)
struct queue *q;
register struct block *bp;
{
	register struct pack *pkp;
	register c;

	if ((pkp = (struct pack *)q->ptr)==NULL || pkp->ostate==LCLOSE) {
		freeb(bp);
		return;
	}
	switch (bp->type) {

	case M_DATA:
		if (q->flag&QFULL) {
			freeb(bp);
			return;
		}
		if ((pkp->iwant -= bp->wptr - bp->rptr) <= 0) {
			pkp->iwant = 0;
			qenable(q);
		}
		putq(q, bp);
		return;

	case M_HANGUP:
		pkp->ostate = RCLOSE;
		flushq(WR(q), 0);
		pkrack(pkp, pkp->SN-1, 0);
		qenable(q);
		freeb(bp);
		return;

	case M_IOCACK:
	case M_IOCNAK:
		(*q->next->qinfo->putp)(q->next, bp);
		return;

	default:
		freeb(bp);
		return;
	}
}

/*
 * --- Output processor
 */

/*
 * accept data from writer
 *  -- handle all non-data messages
 */
pkoput(q, bp)
register struct queue *q;
register struct block *bp;
{
	register struct pack *pkp = (struct pack *)q->ptr;
	register union stmsg *sp;

	if (pkp->ostate == RCLOSE) {
		freeb(bp);
		return;
	}
	switch (bp->type) {

	case M_DELIM:
		if (pkp->istate & PKDELIM)
			goto isdata;
		pkp->istate |= PKDELIM;
		/* flow through */
	case M_FLUSH:
		freeb(bp);
		return;

	case M_IOCTL:
		sp = (union stmsg *)bp->rptr;
		switch (sp->ioc0.com) {

		case TIOCGETP:
			sp->ioc1.sb.sg_erase = 0;
			sp->ioc1.sb.sg_kill = 0;
			sp->ioc1.sb.sg_flags = RAW;
			bp->wptr = bp->rptr+sizeof(struct ioc1);
			bp->type = M_IOCACK;
			qreply(q, bp);
			break;

		default:
			(*q->next->qinfo->putp)(q->next, bp);
			break;
		}
		return;

	default:
		freeb(bp);
		return;

	case M_DATA:
		pkp->istate &= ~PKDELIM;
	isdata:
		putq(q, bp);
		if (pkp->SN < pkp->S + pkp->XW)
			qenable(q);
		return;
	}
}

/*
 * PK out server:
 *  if space in window, process queue
 *    take care of splitting packets bigger than segment size
 *    if timer has expired, retransmit
 */
pkosrv(q)
register struct queue *q;
{
	register struct pack *pkp = (struct pack *)q->ptr;
	register struct block *bp, *xbp;

	if (pkp->timer==0 && pkp->S != pkp->SN)
		pkxmit(q, pkp->xpacks[pkp->S], pkp->S);
	while (pkp->SN < pkp->S + pkp->XW) {
		if ((bp = getq(q)) == NULL)
			break;
		if (bp->wptr-bp->rptr > pkp->xsegment) {
			if ((xbp = allocb(pkp->xsegment)) == NULL) {
				putbq(q, bp);
				return;
			}
			bcopy(bp->rptr, xbp->wptr, pkp->xsegment);
			bp->rptr += pkp->xsegment;
			xbp->wptr += pkp->xsegment;
			putbq(q, bp);
			bp = xbp;
		}
		bp->type = M_DATA;	/* M_DELIM -> 0-length write */
		pkp->xpacks[pkp->SN & 07] = bp;
		pkxmit(q, bp, pkp->SN);
		pkp->SN++;
	}
}

/*
 *  Send out a packet, with header.
 *    -- only a "pointer" to the data is sent; the original
 *    is kept.
 */
pkxmit(q, bp, pkno)
struct queue *q;
register struct block *bp;
unsigned pkno;
{
	register struct block *hbp;
	register size;
	register seg;
	register struct pack *pkp = (struct pack *)q->ptr;
	register struct block *sbp = NULL;
	register sizeadjust = 0;
	static u_char zero[64];

	if (bp == (struct block *)NULL) {
		printf("0 bp in pkxmit\n");
		return;
	}
	pkp->timer = PKTIME;
	if ((hbp = allocb(8)) == NULL)
		return;		/* hope timeouts will recover */
	pkno &= 07;
	size = bp->wptr - bp->rptr;
	/* compute header */
	hbp->wptr += 6;		/* header size */
	hbp->rptr[0] = SYN;
	hbp->rptr[1] = pkp->xlogseg;
	seg = pkp->xsegment;
	pkc_sum = -1;
	pkc_x = 0;
	pkc_size = seg;
	if (size < seg) {
		hbp->rptr[4] = PKSDATA | (pkno << 3) | pkp->R;
		if ((sbp = allocb(2)) == NULL) {
			freeb(hbp);
			return;
		}
		/* hack in the size bytes */
		if ((seg - size) <= 127) {
			*sbp->wptr++ = seg - size;
			sizeadjust = 1;
		} else {
			*sbp->wptr++ = 0x80 + ((seg - size) & 0x7F);
			*sbp->wptr++ = (seg - size) >> 7;
			sizeadjust = 2;
		}
		pkcksum(sbp->rptr, sizeadjust);
		pkcksum(bp->rptr, size);
		seg -= size;
		while (seg>0) {
			pkcksum(zero, 64);
			seg -= 64;
		}
	} else {
		if (size > seg) {
			printf("pk size botch\n");
			return;
		}
		hbp->rptr[4] = PKDATA | (pkno<<3) | pkp->R;
		pkcksum(bp->rptr, size);
	}
	pkc_sum = CHECK - (pkc_sum ^ hbp->rptr[4]);
	hbp->rptr[2] = pkc_sum;
	hbp->rptr[3] = pkc_sum>>8;
	hbp->rptr[5] = hbp->rptr[1]^hbp->rptr[2]^hbp->rptr[3]^hbp->rptr[4];
	/* send header */
	(*q->next->qinfo->putp)(q->next, hbp);
	/* send size */
	if (sbp)
		(*q->next->qinfo->putp)(q->next, sbp);
	/* send data ptr */
	if (hbp = allocb(0)) {		/* if fail, pray for timeout */
		hbp->rptr = bp->rptr;	/* copy ptrs */
		hbp->wptr = bp->wptr;
		(*q->next->qinfo->putp)(q->next, hbp);	/* send data */
	}
	/* send padding */
	seg = pkp->xsegment - sizeadjust;
	while (size < seg) {
		register deficit = seg - size;
		if (deficit > 64)
			deficit = 64;
		if (hbp = allocb(deficit)) {
			hbp->rptr = zero;
			hbp->wptr = zero + deficit;
			(*q->next->qinfo->putp)(q->next, hbp);
		}
		size += deficit;
	}
}

/*
 * Receive an ack for a transmitted packet.
 *  Advance the window.  Retransmit stacked up stuff if required.
 */
pkrack(pkp, packno, rxmit)
register unsigned packno;
register struct pack *pkp;
{
	register struct block **bpp;
	register s = spl5();

	packno &= 07;
	if (packno < pkp->S)
		packno += 8;
	pkp->timer = PKTIME;
	if (packno < pkp->SN)
		while (pkp->S <= packno) {
			bpp = &pkp->xpacks[pkp->S & 07];
			if (*bpp) {
				freeb(*bpp);
				*bpp = NULL;
			}
			pkp->S++;
		}
	if (WR(pkp->rdq)->count)
		qenable(WR(pkp->rdq));
	if (rxmit) {
		for (packno = pkp->S; packno < pkp->SN; packno++)
			pkxmit(WR(pkp->rdq), pkp->xpacks[packno&07], packno);
	}
	if (pkp->S >= 8) {
		pkp->S -= 8;
		pkp->SN -= 8;
	}
	splx(s);
}

/*
 * If n bytes are available, return 1, else 0.
 */
pkwant(pkp, n)
register n;
register struct pack *pkp;
{
	register int navail = 0;
	register struct block *bp;
	int s = spl5();

	for (bp = pkp->rdq->first; bp; bp = bp->next) {
		navail += bp->wptr - bp->rptr;
		if (navail >= n) {
			splx(s);
			return(1);
		}
	}
	if (pkp->ostate==RCLOSE)
		putctl(pkp->rdq->next, M_HANGUP);
	pkp->iwant = n - navail;
	splx(s);
	return(0);
}

/*
 * Copy  n  bytes from the front of the queue.
 */
pkgetb(q, cp, n)
register struct queue *q;
register u_char *cp;
register n;
{
	register struct block *bp;

	while (n) {
		if ((bp = q->first) == NULL) {
			printf("pkgetb fail\n");
			return;
		}
		if (bp->rptr >= bp->wptr) {
			freeb(getq(q));
			continue;
		}
		*cp++ = *bp->rptr++;
		--n;
	}
	if (bp->rptr >= bp->wptr)
		freeb(getq(q));
}

/*
 * The very strange checksum
 */
pkcksum(cp, nbytes)
register u_char *cp;
register nbytes;
{
	register sum, t, x;

	if (nbytes > pkc_size)
		nbytes = pkc_size;
	sum = pkc_sum;
	x = pkc_x;
	while (--nbytes >= 0) {
		sum <<= 1;
		if (sum & 0x10000)
			sum++;
		t = sum & 0xFFFF;
		sum += *cp++;
		sum &= 0xFFFF;
		x += sum ^ pkc_size;
		pkc_size--;
		if (sum <= t)
			sum ^= x;
	}
	pkc_x = x;
	pkc_sum = sum;
}

/*
 * Send a control packet
 */
pkscntl(pkp, cmd)
struct pack *pkp;
{
	register struct block *bp;
	register u_char *cp;
	register struct queue *q;

	if ((bp = allocb(6)) == NULL)
		return;		/* what else to do? */
	bp->wptr += 6;
	cp = bp->rptr;
	*cp++ = SYN;
	*cp++ = 9;		/* ctl pack marker */
	*cp++ = cmd;
	*cp++ = 0;
	*cp++ = cmd;
	*cp++ = 9;		/* xor of hdr[1..4] */
	q = WR(pkp->rdq);
	(*q->next->qinfo->putp)(q->next, bp);
}

/*
 * Receive a control packet
 */
pkcntl(pkp)
register struct pack *pkp;
{
	register val;
	register reject = 0;

	if (pkp->hdr.k != 9) {
		return;
	}
	val = pkp->hdr.C & 07;
	switch (pkp->hdr.C & 0370) {

	case PK_INITA:
		pkp->timer = 2;
		pkp->XW = val;
		if (pkp->ostate==SINITA || pkp->ostate==SINITB) {
			pkscntl(pkp, PK_INITA+RWINSIZE);
			pkscntl(pkp, PK_INITB+RSEGSIZE);
			pkp->ostate = SINITB;
			return;
		}
		return;

	case PK_INITB:
		pkp->timer = 2;
		pkp->xsegment = pksizes[val+1];
		pkp->xlogseg = val + 1;
		if (pkp->ostate==SINITB) {
			pkp->ostate = SINITC;
			pkscntl(pkp, PK_INITB+RSEGSIZE);
			pkscntl(pkp, PK_INITC+RWINSIZE);
			return;
		}
		return;

	case PK_INITC:
		if (pkp->ostate>=SINITB) {
			if (pkp->ostate==SINITB)
				pkscntl(pkp, PK_INITC+RWINSIZE);
			pkp->ostate = OPEN;
			wakeup((caddr_t)pkp);
			return;
		}
		return;

	case PK_RJ:
		reject = 1;
	case PK_RR:
		pkrack(pkp, val, reject);
		return;

	case PK_CLOSE:
		pkp->ostate = RCLOSE;
		flushq(WR(pkp->rdq), 0);
		pkrack(pkp, pkp->SN-1, 0);
		qenable(pkp->rdq);
		return;

	default:
		return;
	}
}

pktimer()
{
	register struct pack *pkp;

	for (pkp = packs; pkp < &packs[NPK]; pkp++) {
		if (pkp->timer) {
			pkp->timer--;
			continue;
		}
		switch (pkp->ostate) {

		case 0:
			continue;

		case OPEN:
		case LCLOSE:
			qenable(WR(pkp->rdq));
			continue;

		case SINITA:
			pkscntl(pkp, PK_INITA+RWINSIZE);
			pkp->timer = 2;
			continue;

		case SINITB:
			pkscntl(pkp, PK_INITA+RWINSIZE);
			pkscntl(pkp, PK_INITB+RSEGSIZE);
			pkp->timer = 2;
			continue;
		}
	}
	timeout(pktimer, (caddr_t)NULL, 60);
}

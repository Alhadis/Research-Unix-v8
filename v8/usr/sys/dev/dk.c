/*
 *  Datakit driver
 */
#include "dk.h"
#if NDK
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/stream.h"
#include "../h/ioctl.h"
#include "../h/pte.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../h/ubavar.h"
#include "../h/conf.h"
#include "../h/dkstat.h"
#include "../h/dkmod.h"
#include "sparam.h"

struct device {
	unsigned short csr;
	unsigned short dko;
	unsigned short dki;
};

struct	device	*DKADDR;
#ifndef	NDKLINE
#define	NDKLINE	64
#endif	NDKLINE

struct	dk {
	struct	queue *dkrq;
	u_char	chan;
} dk[NDKLINE];

int	dkalive;
struct	dkstat	dkstat;
extern	struct	dkmodule dkmod[];
struct	dkmodule *dkmodp;
char	dkstate[NDKLINE];

/*
 * channel states
 */
#define	CLOSED	0
#define	RCLOSE	1		/* remote hung up, local still around */
#define	LCLOSE	2		/* closed locally, CMC hasn't acked yet */
#define	OPEN	3		/* in use */

/*
 * Hardware control bits
 */
#define	DKTENAB	0100
#define	DKRENAB	040
#define	ENABS	(DKRENAB)
#define	DKTDONE	0200
#define	DKRDONE	0100000
#define	D_OSEQ	0
#define	D_READ	01
#define	D_WRITE	02
#define	D_XPACK	03
#define	DKMARK	01000
#define	DKDATA	0400
#define	DKPARITY 0100000

#define	DKISIZ	16

int	nodev(), dkopen(), dkclose(), dkput();

static	struct qinit dkrinit = { nodev, NULL, dkopen, dkclose, 0, 0 };
	struct qinit dkwinit = { dkput, NULL, dkopen, dkclose, 0, 0 };
struct	streamtab dkinfo = { &dkrinit, &dkwinit };

int	dkattach(), dkprobe();
struct	uba_device *dkstuff[NDK];
u_short	dkstd[] = { 0 };
struct	uba_driver dkdriver =
	{ dkprobe, 0, dkattach, 0, dkstd, "dk", dkstuff };

dkprobe(reg)
caddr_t reg;
{
	register int br, cvec;
	register struct device *dkaddr = (struct device *)reg;

	dkaddr->csr = D_OSEQ;
	dkaddr->dko = 0;	/* this clears fifos */
	DELAY(1000);		/* wait for board to reset */
	dkaddr->csr = D_WRITE;
	dkaddr->dko = DKMARK + 511;	/* pack on 511 */
	dkaddr->csr = D_XPACK+DKTENAB;
	dkaddr->dko = 0;
	DELAY(10000);
	dkaddr->csr = 0;
	DKADDR = dkaddr;
	return(1);
}

dkattach()
{
}

/*
 * open DK channel
 */
dkopen(q, dev)
register struct queue *q;
register dev_t dev;
{
	register struct dk *dkp;
	static opened, badtime;
	register maj = major(dev);

	dev = minor(dev);
	if (dev<=0 || dev>=NDKLINE)
		return(0);
	if (!opened) {
		register sane;

		if (DKADDR == NULL)
			return(0);
		DKADDR->csr = D_OSEQ;
		DKADDR->dko = 0;	/* Clear fifo's */
		sane = 256;
		do {
			if (DKADDR->csr & DKTDONE) {
				dkalive = 0;
				if (time > badtime + 300) {
					badtime = time;
					printf("DK interface bad\n");
				}
				return(0);
			}
		} while (--sane);
		for (dkmodp=dkmod; ; dkmodp++) {
			if (dkmodp->dev==0 || dkmodp->dev==maj) {
				dkmodp->dev = maj;
				dkmodp->dkstate = dkstate;
				dkmodp->nchan = NDKLINE;
				break;
			}
		}
		dkalive = 1;
		for (dkp = &dk[0]; dkp < &dk[NDKLINE]; dkp++)
			dkp->chan = dkp - &dk[0];
		DKADDR->csr = ENABS;
		opened++;
		dkrecover(dev);
	}
	dkp = &dk[dev];
	if (dkmodp->dkstate[dev] != CLOSED) {	/* already open */
		if (dev&01 && dev>1)	/* outgoing channels can't reopen */
			return(0);
		if (dkmodp->dkstate[dev] != OPEN)
			return(0);	/* closing channels can't reopen */
		return(1);
	}
	dkp->dkrq = q;
	q->ptr = (caddr_t)dkp;
	WR(q)->ptr = (caddr_t)dkp;
	dkmodp->dkstate[dev] = OPEN;
	return(1);
}

/*
 * Timer to recover from lost interrupt condition.
 */
dkrecover(dev)
dev_t	dev;
{
	register int	ps = spl5();

	if (DKADDR->csr & DKRDONE)
		dkrint(dev);
	splx(ps);

	timeout(dkrecover, dev, hz/2);
}

/*
 * close DK channel
 */
dkclose(q)
register struct queue *q;
{
	register struct dk *dkp;

	dkp = (struct dk *)q->ptr;
	dkp->dkrq = NULL;
	if (dkmodp->dkstate[dkp->chan] == RCLOSE || dkmodp->listnrq==NULL)
		dkmodp->dkstate[dkp->chan] = CLOSED;
	else if (dkmodp->dkstate[dkp->chan] == OPEN)
		dkmodp->dkstate[dkp->chan] = LCLOSE;
	if (dkmodp->listnrq)
		putctl1(RD(dkmodp->listnrq), M_CLOSE, dkp->chan);
}

/*
 * DK receiver interrupt.
 */
dkrint(dev)
{
	register struct queue *q;
	register struct block *bp, **bpp;
	register c, nbytes, sts, sane, chan;
	struct block *blist[DKISIZ+1];

	sts = DKADDR->csr;
	c = 0;
	while (DKADDR->csr & DKRDONE) {
		DKADDR->csr = D_READ|ENABS;
		if ((c & DKMARK) == 0) {
			/* Search for channel number */
			sane = 256;
			do {
				c = DKADDR->dki;
				if (c & DKMARK || --sane==0)
					break;
			} while (DKADDR->csr & DKRDONE);
		}
		if ((c & DKMARK) == 0) {
			dkstat.markflt++;
			continue;
		}
		if ((c & DKPARITY) == 0) {
			dkstat.markparity++;
			c = 0;
			continue;
		}
		/* check channel */
		chan = c & 0777;
		if (chan == 0 || chan >= NDKLINE || (q = dk[chan].dkrq)==NULL) {
			if (chan>0 && chan<NDKLINE) {
				if (dkmodp->listnrq)
					putctl1(RD(dkmodp->listnrq),
						M_CLOSE, chan);
				dkstat.closepack++;
			} else if (chan == 0)
				dkstat.pack0++;
			else
				dkstat.packstrange++;
			for (nbytes=0; nbytes<DKISIZ; nbytes++) {
				c = DKADDR->dki;
				if (c & DKMARK)
					break;
			}
			continue;
		}
		bp = allocb(DKISIZ);
		if (bp == NULL)
			return;
		bpp = blist;
		for (nbytes=0; nbytes<DKISIZ; nbytes++) {
			if ((DKADDR->csr&DKRDONE) == 0)
				break;
			c = DKADDR->dki;
			if ((c & (DKPARITY|DKDATA|DKMARK))!=(DKPARITY|DKDATA)) {
				if (c & DKMARK) {
					dkstat.shortpack++;
					break;
				}
				if ((c & DKPARITY) == 0) {
					dkstat.parity++;
					break;
				} else { /* it's control */
					if ((c & 0377) == 0)
						continue;
					if (bp->wptr > bp->rptr) {
						*bpp++ = bp;
						bp = allocb(DKISIZ);
						if (bp == NULL)
							break;
					}
				}
				bp->type = M_CTL;
			}
			*bp->wptr++ = c;
		}
		*bpp++ = bp;
		*bpp++ = NULL;
		bpp = blist;
		if (nbytes!=DKISIZ) {
			while (*bpp)
				freeb(*bpp++);
			continue;
		}
		while (*bpp) {
			dkstat.input += bp->wptr - bp->rptr;
			if ((q->next->flag & QFULL) == 0)
				(*q->next->qinfo->putp)(q->next, *bpp++);
			else
				freeb(*bpp++);
		}
	}
	DKADDR->csr = sts;
}

/*
 * DK put procedure
 */
dkput(q, bp)
register struct queue *q;
register struct block *bp;
{
	register n, isdata = DKDATA;
	struct dk *dp;
	int chan, s;
	struct ioctl1 {
		int	com;
		int	lhn;
	} *iop;

	dp = (struct dk *)q->ptr;
	chan = dp->chan;
	switch (bp->type) {

	case M_IOCTL:
		bp->type = M_IOCACK;
		iop = (struct ioctl1 *)bp->rptr;
		switch (iop->com) {

		default:
			bp->type = M_IOCNAK;
			break;
		}
		qreply(q, bp);
		return;

	case M_CTL:
		isdata = 0;
	case M_DATA:
		break;

	case M_CLOSE:
		n = *bp->rptr;
		if (n < NDKLINE) {
			if (dkmodp->dkstate[n] == OPEN) {
				dkmodp->dkstate[n] = RCLOSE;
				putctl(dk[n].dkrq->next, M_HANGUP);
			} else if (dkmodp->dkstate[n] == LCLOSE)
				dkmodp->dkstate[n] = CLOSED;
		}
		freeb(bp);
		return;

	default:
		freeb(bp);
		return;
	}
	dkstat.output += bp->wptr - bp->rptr;
	s = spl6();
	if (dkalive) {
		register struct device *dkaddr = DKADDR;
		int r = dkaddr->csr;
		while ((n = bp->wptr - bp->rptr) > 0) {
			if (n > DKISIZ)
				n = DKISIZ;
			dkaddr->csr = D_WRITE + ENABS;
			dkaddr->dko = DKMARK + chan;
			dkaddr->dko = *bp->rptr++ | isdata;
			while (--n)
				dkaddr->dko = *bp->rptr++ | DKDATA;
			dkaddr->csr = D_XPACK + ENABS;
			dkaddr->dko = 0;
		}
		dkaddr->csr = r;
	}
	splx(s);
	freeb(bp);
}

/*
 * unibus reset
 */
dkreset()
{
	if (DKADDR)
		DKADDR->csr = ENABS;
}

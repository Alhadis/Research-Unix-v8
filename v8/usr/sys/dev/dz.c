/*
 *  DZ-11 Driver
 */

/* Modified 4/84 by J. Wolitzky to allow dialout operation with smart modems */

#include "dz.h"
#if NDZ > 0
#define	TRC(c)

#include "../h/param.h"
#include "../h/stream.h"
#include "../h/ioctl.h"
#include "../h/ttyld.h"
#include "../h/pte.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../h/systm.h"
#include "../h/ubavar.h"
#include "../h/conf.h"
#define	NDZLINE	(8*NDZ)
 
#define BITS7	020
#define BITS8	030
#define TWOSB	040
#define PENABLE	0100
#define OPAR	0200
#define	CLR	020		/* Clear */
#define MSE	040		/* Master Scan Enable */
#define RIE	0100		/* Receiver Interrupt Enable */
#define	SAE	010000		/* Silo Alarm Enable */
#define TIE	040000		/* Transmit Interrupt Enable */
#define	TRDY	0100000		/* Transmitter ready */
#define DZ_IEN	(MSE+RIE+TIE)
#define PERROR	010000
#define FRERROR	020000
#define	OVERRUN	040000
#define SSPEED	9		/* std speed = 1200 baud */

#define	EXISTS	01
#define	ISOPEN	02
#define	WOPEN	04
#define	TIMEOUT	010
#define	CARR_ON	020
#define	DZSTOP	040
#define	HPCL	0100
#define	BRKING	0200
#define	DIALOUT	0400	/* set when used as dialout with smart modems */

#define	DZPRI	30

struct device {
	short	dzcsr;
	short	dzrbuf;
#define	dzlpr	dzrbuf
	char	dztcr;
	char	dzdtr;
	char	dztbuf;
#define	dzrind	dztbuf
	char	dzbrk;
#define	dzmsr	dzbrk
};

int	dzprobe(), dzattach(), dzrint();
int	dzdelay();
struct	uba_device *dzboard[NDZ];
u_short dzstd[] = { 0 };
struct uba_driver dzdriver = { dzprobe, 0, dzattach, 0, dzstd, "dz", dzboard };

/*
 * One structure per line
 */
struct	dz {
	short	state;
	short	flags;
	struct	block	*oblock;
	struct	queue	*rdq;
	char	board;
	char	line;
	char	speed;
	char	brking;
} dz[NDZLINE];
int	dzoverrun;

char	dz_speeds[] = {
	0, 020 , 021 , 022 , 023 , 024 , 0, 025,
	026 , 027 , 030 , 032 , 034 , 036 , 037 , 0
};

int	dzopen(), dzclose(), dzoput(), nodev();

static	struct qinit dzrinit = { nodev, NULL, dzopen, dzclose, 0, 0 };
	struct qinit dzwinit = { dzoput, NULL, dzopen, dzclose, 200, 100 };
struct	streamtab dzinfo = { &dzrinit, &dzwinit };

int	dzmiss;

dzprobe(reg)
caddr_t reg;
{
	register int br, cvec;
	register struct device *dzaddr = (struct device *)reg;

	dzaddr->dzcsr = TIE|MSE;
	dzaddr->dztcr = 1;		/* enable any line */
	DELAY(100000);
	dzaddr->dzcsr = CLR;		/* reset everything */
	if (cvec && cvec != 0x200)
		cvec -= 4;
	return (1);
}

dzattach(ui)
register struct uba_device *ui;
{
	extern dzscan();
	static dz_timer;
	register i;

	for (i=0; i<8; i++) {
		dz[ui->ui_unit*8+i].state = EXISTS;
		dz[ui->ui_unit*8+i].board = ui->ui_unit;
		dz[ui->ui_unit*8+i].line = i;
	}
	if (dz_timer == 0) {
		dz_timer++;
		timeout(dzscan, (caddr_t)0, hz);
	}
	return (1);
}
 
/*ARGSUSED*/
dzopen(q, d, flag)
register struct queue *q;
{
	register dev;
	register struct dz *dzp;
	int dzflag;
 
	dev = minor(d);
	dzp = &dz[dev];
	if (dev >= NDZLINE || (dzp->state&EXISTS)==0)
		return(0);
	q->ptr = (caddr_t)dzp;
	WR(q)->ptr = (caddr_t)dzp;
	dzscan(1);			/* update CARR_ON */
	if ((dzp->state&ISOPEN)==0 || (dzp->state&CARR_ON)==0) {
		register s = spl5();
		for (;;) {
			dzp->flags = ODDP|EVENP;
			dzp->speed = SSPEED;
			dzparam(dzp);
			if (dzp->state & CARR_ON)
				break;
			dzflag = dzboard[dev / 8]->ui_flags;
			/* ignore carrier? */
			if (dzflag & (1 << (dev % 8))) {
				dzp->state |= (DIALOUT | CARR_ON);
				break;
			}
			if (tsleep((caddr_t)dzp, DZPRI, 0) != TS_OK) {
				wakeup((caddr_t)dzp);
				dzp->speed = 0;
				dzparam(dzp);
				splx(s);
				return(0);
			}
		}
		dzp->rdq = q;
		dzp->state |= ISOPEN;
		splx(s);
	}
	TRC('o');
	return(1);
}

dzclose(q, d)
register struct queue *q;
{
	register struct dz *dzp;
	register s;

	dzp = (struct dz *)q->ptr;
	s = spl5();
	if (dzp->oblock) {
		register struct block *bp = dzp->oblock;
		dzp->oblock = NULL;
		freeb(bp);
	}
	flushq(WR(q), 1);
	dzp->rdq = NULL;
	if (dzp->state&HPCL || (dzp->state&CARR_ON)==0) {
		dzp->speed = 0;
		dzparam(dzp);
	}
	dzp->state &= EXISTS;
	splx(s);
	TRC('c');
}

/*
 * DZ write put routine
 */
dzoput(q, bp)
register struct queue *q;
register struct block *bp;
{
	register struct dz *dzp = (struct dz *)q->ptr;
	register union stmsg *sp;
	register s;
	int delaytime;

	TRC('p');
	switch(bp->type) {

	case M_IOCTL:
		TRC('i');
		sp = (union stmsg *)bp->rptr;
		switch (sp->ioc0.com) {

		case TIOCSETP:
		case TIOCSETN:
			delaytime = 0;
			if (dzp->speed != sp->ioc1.sb.sg_ispeed)
				delaytime = 20;
			dzp->speed = sp->ioc1.sb.sg_ispeed;
			dzp->flags = sp->ioc1.sb.sg_flags;
			bp->type = M_IOCACK;
			bp->wptr = bp->rptr;
			qreply(q, bp);
			qpctl1(q, M_DELAY, delaytime);	/* wait a bit */
			qpctl(q, M_CTL);		/* means do dzparam */
			dzstart(dzp);
			return;

		case TIOCGETP:
			sp->ioc1.sb.sg_ispeed =
			  sp->ioc1.sb.sg_ospeed = dzp->speed;
			bp->type = M_IOCACK;
			qreply(q, bp);
			return;

		case TIOCHPCL:
			dzp->state |= HPCL;
			bp->type = M_IOCACK;
			bp->wptr = bp->rptr;
			qreply(q, bp);
			return;

		default:
			bp->wptr = bp->rptr;
			bp->type = M_IOCNAK;
			qreply(q, bp);
			return;
		}

	case M_STOP:
		s = spl5();
		dzp->state |= DZSTOP;
		freeb(bp);
		if (bp = dzp->oblock) {
			dzp->oblock = NULL;
			putbq(q, bp);
		}
		splx(s);
		return;

	case M_START:
		dzp->state &= ~DZSTOP;
		dzstart(dzp);
		break;

	case M_FLUSH:
		flushq(q, 1);
		freeb(bp);
		s = spl5();
		if (bp = dzp->oblock) {
			dzp->oblock = NULL;
			freeb(bp);
		}
		splx(s);
		return;

	case M_BREAK:
		qpctl1(q, M_DELAY, 10);
		putq(q, bp);
		qpctl1(q, M_DELAY, 10);
		dzstart(dzp);
		return;

	case M_HANGUP:
		dzp->state &= ~DZSTOP;
	case M_DELAY:
	case M_DATA:
		putq(q, bp);
		TRC('d');
		dzstart(dzp);
		return;

	default:		/* not handled; just toss */
		break;
	}
	freeb(bp);
}
 
/*
 * Receive interrupt.  Argument is DZ board number
 */
dzrint(dev)
{
	register struct dz *dzp;
	register struct block *bp;
	register struct device *dzaddr;
	register int c;
 
	if (dev >= NDZ) {
		printf("bad dz rd intr\n");
		return;
	}
	dzaddr = (struct device *)dzboard[dev]->ui_addr;
	while ((c = dzaddr->dzrbuf) < 0) {	/* char present */
		dzp = &dz[dev*8 + ((c>>8)&07)];
		if (c&(OVERRUN/*|PERROR*/)) {
			if (c&OVERRUN)
				dzoverrun++;
			continue;
		}
		if (dzp->rdq == NULL)
			continue;
		if (dzp->rdq->next->flag&QFULL) {
			dzmiss++;
			continue;
		}
		if ((bp = allocb(16)) == NULL)
			continue;			/* no space */
		if (c&FRERROR)			/* frame err (break) */
			bp->type = M_BREAK;
		else
			*bp->wptr++ = c;
		(*dzp->rdq->next->qinfo->putp)(dzp->rdq->next, bp);
	}
}
 
/*
 * set device parameters
 */
dzparam(dzp)
register struct dz *dzp;
{
	register short lpr;
	struct device *dzaddr = (struct device *)dzboard[dzp->board]->ui_addr; 

	dzaddr->dzcsr = DZ_IEN;
	if (dzp->speed == 0) {
		dzaddr->dzdtr &= ~(1 << dzp->line);	/* hang up */
		return;
	}
	dzaddr->dzdtr |= 1 << dzp->line;
	lpr = (dz_speeds[dzp->speed]<<8) | dzp->line;
	if (dzp->flags & RAW)
		lpr |= BITS8;
	else
		lpr |= (BITS7|PENABLE);
	if ((dzp->flags & EVENP) == 0)
		lpr |= OPAR;
	if (dzp->speed == 3)
		lpr |= TWOSB; 			/* 110 baud: 2 stop bits */
	dzaddr->dzlpr = lpr;
}
 
/*
 * Transmitter interrupt. dev is board number.
 */
dztint(dev)
{
	register struct device *dzaddr = (struct device *)dzboard[dev]->ui_addr;
	register struct dz *dzp;
	register struct block *bp;
	register sts;
 
	while ((sts = dzaddr->dzcsr) & TRDY) {
		dzp = &dz[dev*8 + ((sts>>8)&07)];
		if (bp = dzp->oblock) {
			if (bp->rptr < bp->wptr) {
				dzaddr->dztbuf = *bp->rptr++;
				continue;
			}
			freeb(bp);
			dzp->oblock = NULL;
		}
		dzaddr->dztcr &= ~(1 << (dzp->line));
		dzstart(dzp);
	}
}

dztimo(dzp)
register struct dz *dzp;
{
	if (dzp->state&BRKING) {
		dzp->brking &= ~(1 << dzp->line);
		((struct device *)dzboard[dzp->board]->ui_addr)->dzbrk =
		    dzp->brking;
	}
	dzp->state &= ~(TIMEOUT|BRKING);
	dzstart(dzp);
}

dzstart(dzp)
register struct dz *dzp;
{
	register s = spl5();
	register struct block *bp;
	register struct device *dzaddr;
 
	TRC('s');
again:
	if (dzp->state & (TIMEOUT|DZSTOP|BRKING) || dzp->oblock) {
		TRC('t');
		goto out;
	}
	if (dzp->rdq == NULL)
		goto out;
	if ((bp = getq(WR(dzp->rdq))) == NULL) {
		TRC('n');
		goto out;
	}
	switch (bp->type) {

	case M_DATA:
		dzp->oblock = bp;
		dzaddr = (struct device *)dzboard[dzp->board]->ui_addr;
		dzaddr->dztcr |= 1 << (dzp->line);
		break;

	case M_BREAK:
		dzaddr = (struct device *)dzboard[dzp->board]->ui_addr;
		dzp->brking |= 1 << dzp->line;
		dzaddr->dzbrk = dzp->brking;
		dzp->state |= BRKING|TIMEOUT;
		timeout(dztimo, (caddr_t)dzp, 15);	/* about 250 ms */
		freeb(bp);
		break;

	case M_DELAY:
		dzp->state |= TIMEOUT;
		timeout(dztimo, (caddr_t)dzp, (int)*bp->rptr + 6);
		freeb(bp);
		break;

	case M_HANGUP:
		dzp->speed = 0;
	case M_CTL:
		freeb(bp);
		dzparam(dzp);
		goto again;

	}
out:
	splx(s);
}
 
dzscan(timo)
caddr_t timo;
{
	register struct device *dzaddr;
	register struct dz *dzp;
	register i, j;
 
	for (i=0; i<NDZ; i++) {
		if (dzboard[i] == NULL)
			continue;
		dzaddr = (struct device *)dzboard[i]->ui_addr;
		for (j=0,dzp = &dz[i*8]; j<8; j++, dzp++) {
			if (dzaddr->dzmsr & (1<<j)) {
				/* carrier present */
				if ((dzp->state & CARR_ON)==0)
					wakeup((caddr_t)dzp);
				dzp->state |= CARR_ON;
			} else if ((dzp->state & CARR_ON)
			  && !(dzp->state & DIALOUT)) {
				/* carrier lost */
				if (dzp->state&ISOPEN) {
					dzaddr->dzdtr &= ~(1<<j);
					if (dzp->rdq)
						putctl(dzp->rdq->next,M_HANGUP);
				}
				dzp->state &= ~CARR_ON;
			}
		}
	}
	if (timo == 0)
		timeout(dzscan, (caddr_t)0, 2*hz);
}
#endif

#include "../chunix/chsys.h"
#include "../chunix/chconf.h"
#include "../chaos/chaos.h"
/*
 * DR11-C Chaos driver.
 * This driver does framing and a block check for each packet.
 * The framing is similar to standard synchronous links, except that
 * the "byte" is 16 bits.  Basically a frame starts after 2 or more
 * DRSYNC words, then the next word being a count and the next word being the
 * complement of the count (both count and its complement must not be DRSYNC).
 * Then the number of words specified by the count are the actual data,
 * followed by an LRC-16.  Data words equal to DRSYNC or DRESC are preceded by
 * DRESC. The count does not include the inserted DRESC words, but the CRC
 * word does. The LRC word does not include the initial DRSYNC's or the count.
 * This framing scheme was thought up quickly and may well be overkill.
 * The LRC is a simple XOR of all data words.
 * Chaos N.C.P.'s that use this file must include <chaos/dr11c.h> in their
 * chconf.h files.
 * Keep in mind that the interrupt priority level of this driver maybe less
 * than either the clock or the LOCK macro supplied by the user.
 * Also remember that the block check is not to great.
 */

#define	xc_draddr	xc_info.xc_dr11c.dr_addr
#define	xc_drtptr	xc_info.xc_dr11c.dr_tptr
#define	xc_drrptr	xc_info.xc_dr11c.dr_rptr
#define	xc_drtcnt	xc_info.xc_dr11c.dr_tcnt
#define	xc_drrcnt	xc_info.xc_dr11c.dr_rcnt
#define	xc_drrstate	xc_info.xc_dr11c.dr_rstate
#define	xc_drtstate	xc_info.xc_dr11c.dr_tstate
#define	xc_drrcheck	xc_info.xc_dr11c.dr_rcheck
#define	xc_drtcheck	xc_info.xc_dr11c.dr_tcheck
#define	xc_drintrup	xc_info.xc_dr11c.dr_intrup

struct chxcvr dr11cxcvr[NCHDR];


/*
 * dr11-c initialization. Also does initialization to the static chxcvr
 * structure since initialized unions don't work anyway.
 */

int chdrstart(), chdrreset();
/*
 * Initialize all dr11c interfaces - called from low priority
 */
chdrinit()
{
	register struct chxcvr *xp;
	register short *hp;
#ifdef VMUNIX
	register int i = 0;
#else
	register char *dp = (char *)DR11CBASE;/* must be char * to use DR11CINC */
#endif

	hp = &dr11chosts[0];
	for (xp = &dr11cxcvr[0]; xp < &dr11cxcvr[NDR11C]; xp++) {
		xp->xc_draddr =
#ifdef VMUNIX
			(struct dr11c *) chdrdinfo[i++]->ui_addr;
#else
			(struct dr11c *) dp;
		dp += DR11CINC;
#endif
		/*DEBUG*/ if (xp->xc_draddr == 0) panic("draddr");
		xp->xc_start = chdrstart;
		xp->xc_reset = chdrreset;
		xp->xc_host = *hp++;
		LOCK;
		chdrreset(xp);
		UNLOCK;
	}
}
/*
 * Check for hung interfaces - if so flush the packet.
 * Assumed called at clock level.
 */
chdrxtime()
{
	register struct chxcvr *xp;

	for (xp = &dr11cxcvr[0]; xp < &dr11cxcvr[NDR11C]; xp++) {
		if (xp->xc_drintrup)
			continue;
		if (xp->xc_rpkt != NOPKT &&
		    Chclock - xp->xc_rtime > DR11CHUNG) {
			ch_free((char *)xp->xc_rpkt);
			xp->xc_rpkt = NOPKT;
			xp->xc_drrstate = DRIDLE;
			debug(DABNOR, printf("Rec pkt timeout\n"));
		}
		if (xp->xc_tpkt != NOPKT &&
		    Chclock - xp->xc_ttime > DR11CHUNG) {
			debug(DABNOR, printf("Trans pkt timeout\n"));
			xp->xc_drtstate = DRTDONE;
			chdrxint(xp - dr11cxcvr);
		}
	}
}

/*
 * Reset things from an unknown state (high priority)
 * Basically enable interrupts and tell him we're ready for input.
 * If he already has a word, take it right away.
 */
chdrreset(xp)
register struct chxcvr *xp;
{
	register struct dr11c *dp = xp->xc_draddr;

	xp->xc_drrstate = xp->xc_drtstate = DRIDLE;
	dp->dr_csr = 0;
	dp->dr_csr = DRIN|DROUT|DRIE|DROE;
}
/*
 * Start output on an idle line - called when nothing happening
 */
chdrstart(xp)
register struct chxcvr *xp;
{

	if (xp->xc_tpkt != NOPKT)
		panic("chdrstart: already busy");
	chdrxint(xp - dr11cxcvr);
}
/*
 * Receiver interrupt
 */
chdrrint(dev)
{
	register unsigned short data; /* register short is dubious - be careful! */
	register struct chxcvr *xp = &dr11cxcvr[dev];
	register struct dr11c *dp = xp->xc_draddr;

	if ((dp->dr_csr & DRIRDY) == 0) {
		debug(DABNOR, printf("extra dr11c rint\n"));
		return;
	}
	dp->dr_csr &= ~DRIN;
	data = dp->dr_ibuf;
	dp->dr_csr |= DRIN;		/* set his DRORDY, and cause his chdrxint */
	
	xp->xc_drintrup = 1;
	switch(xp->xc_drrstate) {
	case DRIDLE:
		if (data == DRSYNC)
			xp->xc_drrstate = DRSYN1;
		break;
	case DRSYN1:
		xp->xc_drrstate = data == DRSYNC ? DRSYN2 : DRIDLE;
		break;
	case DRSYN2:
		if (data != DRSYNC) {
			xp->xc_drrcnt = data;
			xp->xc_drrstate = DRCNT1;
		}
		break;
	case DRCNT1:
		xp->xc_drrstate = DRIDLE;
		data = ~data;
		debug(DTRANS,printf("Rec size=%d\n", data));
		if (data != xp->xc_drrcnt)
			debug(DABNOR, printf("Rec bad count\n"));
		else if ((data <<= 1) < sizeof(struct pkt_header) ||
			 data > CHMAXDATA + sizeof(struct pkt_header))
			debug(DABNOR, printf("Rec bad size: %d", data));
		else {
			LOCK;	/* in case of clock interrupt */
			if ((xp->xc_rpkt = (struct packet *)ch_alloc(
			    (int)data + sizeof(struct ncp_header) , 1))
				!= NOPKT) {
			xp->xc_drrstate = DRDATA;
			xp->xc_drrptr = (short *)&xp->xc_rpkt->pk_phead;
			xp->xc_drrcheck = 0;
			xp->xc_rtime = Chclock;
			} else
				debug(DABNOR, printf("Rec alloc fail: %d\n", data));
		}
		break;
	case DRDATA:
		if (data == DRESC) {
			xp->xc_drrstate = DRESC1;
			xp->xc_drrcheck ^= data;
			break;
		}
		/* Falls into... */
	case DRESC1:
		xp->xc_drrcheck ^= data;
		*xp->xc_drrptr++ = data;
		if (--xp->xc_drrcnt == 0)
			xp->xc_drrstate = DRCHECK;
		else
			xp->xc_drrstate = DRDATA;
		break;
	case DRCHECK:
		xp->xc_drrstate = DRIDLE;
		LOCK;	/* raise to possibly higher priority */
		if (data == xp->xc_drrcheck)
			rcvpkt(xp->xc_rpkt);
		else
			ch_free((char *)xp->xc_rpkt);
		xp->xc_rpkt = NOPKT;
	}
	xp->xc_drintrup = 0;
}
/*
 * Transmit interrupt
 */
chdrxint(dev)
{
	register struct chxcvr *xp = &dr11cxcvr[dev];
	register struct dr11c *dp = xp->xc_draddr;
	register struct packet *pkt;
	unsigned short data;

	if (xp->xc_tpkt != NOPKT && (dp->dr_csr & DRORDY) == 0) {
		debug(DABNOR, printf("Extra dr11c xint\n"));
		/*return;*/
	}
	dp->dr_csr &= ~DROUT;
	xp->xc_drintrup = 1;
	switch (xp->xc_drtstate) {
	case DRIDLE:
		xp->xc_tpkt = NOPKT;
	case DRTDONE:
		LOCK;
		if ((pkt = xcvrdone(xp)) == NOPKT)
			goto out;
		xp->xc_ttime = Chclock;
		data = sizeof(struct pkt_header) + pkt->pk_len;
		data = (data + (sizeof(short) - 1)) >> 1;
		xp->xc_drtcnt = data;
		xp->xc_drtptr = (short *)&pkt->pk_phead;
		debug(DTRANS,printf("Trans size=%d, %x\n",data, pkt));
		data = DRSYNC;
		xp->xc_drtstate = DRSYN1;
		break;
	case DRSYN1:
		data = DRSYNC;
		xp->xc_drtstate = DRSYN2;
		break;
	case DRSYN2:
		data = xp->xc_drtcnt;
		xp->xc_drtstate = DRCNT1;
		break;
	case DRCNT1:
		data = ~xp->xc_drtcnt;
		xp->xc_drtstate = DRDATA;
		xp->xc_drtcheck = 0;
		break;
	case DRDATA:
		if ((data = *xp->xc_drtptr) == DRESC || data == DRSYNC) {
			data = DRESC;
			xp->xc_drtcheck ^= data;
			xp->xc_drtstate = DRESC1;
			break;
		}
		/* Falls into... */
	case DRESC1:
		data = *xp->xc_drtptr++;
		xp->xc_drtcheck ^= data;
		if (--xp->xc_drtcnt == 0)
			xp->xc_drtstate = DRCHECK;
		else
			xp->xc_drtstate = DRDATA;
		break;
	case DRCHECK:
		data = xp->xc_drtcheck;
		xp->xc_drtstate = DRTDONE;
	}
	dp->dr_obuf = data;
	dp->dr_csr |= DROUT;		/* set his DRIRDY, cause his chdrrint */
out:
	xp->xc_drintrup = 0;
}

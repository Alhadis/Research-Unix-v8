/*
 * Terminal-board line discipline for standard Datakit driver
 */

#include "../h/param.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/tty.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/mx.h"
#include "../h/file.h"
#ifdef VMUNIX
#include "../h/dk.h"
#endif
#include "../h/local.h"

extern	char	partab[];

#define	NDT	16	/* number of lines */

struct dt {
	char	dtnack;
	char	dtcack;
	char	dtstate;
	char	dtfcon;
	struct clist outq;
	char	echoq[8];
	char	cecho;
	char	necho;
	struct tty *backp;	/* for debugging only */
} dtl[NDT];

/*
 * DT control messages
 */
#define	D_BREAK	07
#define	D_ECHO	010
#define	D_ACK	020
#define	D_DELAY	030
#define	D_SNDBRK 3

#define	DTOPEN	01
#define	CLOG1	02
#define	CLOG2	04
#define	LSLP	010

#define	OBSIZE	16
#define	TIDEMARK 150
#define	DTTIME	120

/*
 * Open a DT line.
 */
#ifdef VMUNIX
dt_lopen(dev, tp, addr)
#else
dt_lopen(tp, addr)
#endif
register struct tty *tp;
caddr_t addr;
{
	register struct dt *lp;
	static timer_on = 0;
	int dtltimer();

	if (timer_on == 0) {
		timeout(dtltimer, (caddr_t)0, DTTIME);
		timer_on++;
	}
	if (tp->t_line) {
		return;
	}
	for (lp=dtl; lp < &dtl[NDT]; lp++)
		if ((lp->dtstate&DTOPEN) == 0)
			break;
	if (lp >= &dtl[NDT]) {
		u.u_error = ENXIO;
		return;
	}
	tp->t_linep = (caddr_t)lp;
	lp->dtstate = DTOPEN;
	lp->dtnack = 0;
	lp->dtcack = 0;
	lp->cecho = 0;
	lp->necho = 0;
	lp->dtfcon = 0;
	lp->backp = tp;
}

/*
 * Close a DT line.
 */
dt_lclose(tp)
register struct tty *tp;
{
	register struct dt *lp;

	lp = (struct dt *)tp->t_linep;
	lp->dtstate = 0;
	tp->t_linep = 0;
	tp->t_line = 0;
	while (lp->outq.c_cc)
		getc(&lp->outq);
	lp->backp = (struct tty *)0;
}

/*
 * Write from user to output queue
 */
caddr_t
dt_lwrite(tp UIO)
UDCL
register struct tty *tp;
{
	char buf[OBSIZE];
	register struct dt *lp;
	register char *cp;
	register cc;

	if ((tp->t_state&CARR_ON)==0)
		return(NULL);
	lp = (struct dt *)tp->t_linep;
	while (U(u_count)) {
		spl5();
#ifdef VMUNIX
		while (tp->t_state&TTSTOP || lp->outq.c_cc>=TTHIWAT(tp)) {
#else
		while (tp->t_state&TTSTOP || lp->outq.c_cc>=TTHIWAT) {
#endif
			if ((tp->t_state&BUSY)==0) {
				(*tp->t_oproc)(tp);
#ifdef VMUNIX
				if ((tp->t_state&TTSTOP)==0 &&
				    lp->outq.c_cc<TTHIWAT(tp))
#else
				if ((tp->t_state&TTSTOP)==0 &&
				    lp->outq.c_cc<TTHIWAT)
#endif
					break;
			}
			lp->dtstate |= LSLP;
			if (tp->t_chan) {
				spl0();
				return((caddr_t)&lp->outq);
			}
			sleep((caddr_t)&lp->outq, TTOPRI);
		}
		spl0();
		cc = OBSIZE;
		if (U(u_count) < cc)
			cc = U(u_count);
		cp = buf;
		iomove(cp, cc, B_WRITE UIO);
		if (tp->t_flags & RAW) {
			b_to_q(cp, cc, &lp->outq);
			tk_nout += cc;
		} else {
			while (--cc >= 0)
				dtloutput(*cp++, tp);
		}
		if ((tp->t_state&BUSY)==0)
			(*tp->t_oproc)(tp);
	}
	return(NULL);
}

dt_lread(tp UIO)
register struct tty *tp;
UDCL
{
	register s;
	register struct dt *lp;

	if ((tp->t_state&CARR_ON)==0)
		return(0);
	s = spl5();
	if (tp->t_canq.c_cc==0)
		while (canon(tp)<0)
			if (tp->t_chan==NULL) {
				sleep((caddr_t)&tp->t_rawq, TTIPRI); 
			} else
				return(0);
	splx(s);
	while (tp->t_canq.c_cc && passc(getc(&tp->t_canq) UIO)>=0)
		;
	lp = (struct dt *)tp->t_linep;
	if (tp->t_rawq.c_cc<TIDEMARK && lp->cecho!=lp->necho)
		(*tp->t_oproc)(tp);
	return(tp->t_rawq.c_cc+tp->t_canq.c_cc);
}

dt_lstart(tp, ctlq)
register struct tty *tp;
struct clist *ctlq;
{
	register struct dt *lp;
	register c, delay;

	lp = (struct dt *)tp->t_linep;
	if (lp->cecho!=lp->necho &&
	 (tp->t_rawq.c_cc<TIDEMARK
	   || (tp->t_flags&(CBREAK|RAW))==0 && tp->t_delct==0)) {
		putc(lp->echoq[lp->cecho++], ctlq);
		lp->cecho &= 07;
		return;
	}
	if (lp->dtstate&CLOG2) {
		putc(D_ECHO+lp->dtnack, ctlq);
		lp->dtstate &= ~CLOG2;
		return;
	}
	while (lp->outq.c_cc) {
		if (lp->dtcack == ((lp->dtnack+2)&03)) {
			lp->dtstate |= CLOG1;
			return;
		}
		lp->dtstate &= ~(CLOG1|CLOG2);
		if (lp->dtfcon >= 32) {
			lp->dtnack = (lp->dtnack+1) & 03;
			lp->dtfcon -= 32;
			putc(D_ECHO+lp->dtnack, ctlq);
			break;
		}
		c = getc(&lp->outq);
		lp->dtfcon++;
		if (tp->t_flags&RAW || (c&0200)==0) {
			putc(c, &tp->t_outq);
		} else {
			delay = D_DELAY;
			c &= 0177;
			while (c) {
				c >>= 1;
				delay++;
			}
			putc(delay, ctlq);
			break;
		}
	}
#ifdef VMUNIX
	if (lp->dtstate&LSLP && lp->outq.c_cc<=TTLOWAT(tp)) {
#else
	if (lp->dtstate&LSLP && lp->outq.c_cc<=TTLOWAT) {
#endif
		lp->dtstate &= ~LSLP;
		wakeup((caddr_t)&lp->outq);
	}
}

dt_lrend(tp, cp, ce)
register struct tty *tp;
register char *cp;
char *ce;
{

	if (tp->t_flags&RAW) {
		b_to_q(cp, ce-cp, &tp->t_rawq);
		if (tp->t_chan)
			sdata(tp->t_chan);
		else
			wakeup((caddr_t)&tp->t_rawq);
		if (tp->t_rawq.c_cc > TTYHOG)
#ifdef VMUNIX
			flushtty(tp, FREAD);
#else
			flushtty(tp);
#endif
	} else {
		while (cp < ce)
			dtlinput(*cp++, tp);
	}
}

dt_lmeta(tp, cp, ce)
register struct tty *tp;
register char *cp;
char *ce;
{
	register struct dt *lp;

	lp = (struct dt *)tp->t_linep;
	if ((*cp & 0370) == D_ECHO) {
		lp->echoq[lp->necho++] = *cp-D_ECHO+D_ACK;
		lp->necho &= 07;
		if (lp->necho == lp->cecho) {
			lp->cecho++;
			lp->cecho &= 07;
		}
		if ((tp->t_state&BUSY)==0)
			(*tp->t_oproc)(tp);
		return;
	}
	switch (*cp & 0377) {
	case D_BREAK:
		*cp = tp->t_un.t_chr.t_intrc;
		dt_lrend(tp, cp, cp+1);
		return;

	case D_ACK+0:
	case D_ACK+1:
	case D_ACK+2:
	case D_ACK+3:
		lp->dtcack = (*cp) & 03;
		if ((tp->t_state&BUSY)==0)
			(*tp->t_oproc)(tp);
		return;

	default:
		return;
	}
}

dtloutput(c, tp)
register c;
register struct tty *tp;
{
	register char *colp;
	register struct dt *lp;
	register ctype;

	tk_nout++;
	lp = (struct dt *)tp->t_linep;
	/*
	 * In raw mode dump the char unchanged.
	 */
	if (tp->t_flags&RAW) {
		putc(c, &lp->outq);
		return;
	}
	c &= 0177;

	/*
	 * Turn tabs to spaces as required
	 */
	if (c=='\t' && (tp->t_flags&TBDELAY)==XTABS) {
		c = 8;
		do
			dtloutput(' ', tp);
		while (--c >= 0 && tp->t_col&07);
		return;
	}
	/*
	 * for upper-case-only terminals,
	 * generate escapes.
	 */
	if (tp->t_flags&LCASE) {
		colp = "({)}!|^~'`";
		while(*colp++)
			if(c == *colp++) {
				dtloutput('\\', tp);
				c = colp[-2];
				break;
			}
		if ('a'<=c && c<='z')
			c += 'A' - 'a';
	}
	/*
	 * turn <nl> to <cr><lf> if desired.
	 */
	if (c=='\n' && tp->t_flags&CRMOD)
		dtloutput('\r', tp);
	putc(c, &lp->outq);
	/*
	 * Calculate delays.
	 * The numbers here represent clock ticks
	 * and are not necessarily optimal for all terminals.
	 * The delays are indicated by characters above 0200.
	 * In raw mode there are no delays and the
	 * transmission path is 8 bits wide.
	 */
	colp = &tp->t_col;
	ctype = partab[c];
	c = 0;
	switch (ctype&077) {

	/* ordinary */
	case 0:
		(*colp)++;

	/* non-printing */
	case 1:
		break;

	/* backspace */
	case 2:
		if (*colp)
			(*colp)--;
		break;

	/* newline */
	case 3:
		ctype = (tp->t_flags >> 8) & 03;
		if(ctype == 1) { /* tty 37 */
			if (*colp)
				c = max(((unsigned)*colp>>4) + 3, (unsigned)6);
		} else
		if(ctype == 2) { /* vt05 */
			c = 6;
		}
		*colp = 0;
		break;

	/* tab */
	case 4:
		ctype = (tp->t_flags >> 10) & 03;
		if(ctype == 1) { /* tty 37 */
			c = 1 - (*colp | ~07);
			if(c < 5)
				c = 0;
		}
		*colp |= 07;
		(*colp)++;
		break;

	/* vertical motion */
	case 5:
		if(tp->t_flags & VTDELAY) /* tty 37 */
			c = 0177;
		break;

	/* carriage return */
	case 6:
		ctype = (tp->t_flags >> 12) & 03;
		if(ctype == 1) { /* tn 300 */
			c = 5;
		} else if(ctype == 2) { /* ti 700 */
			c = 10;
		}
		*colp = 0;
	}
	if(c)
		putc(c|0200, &lp->outq);
}

dtlinput(c, tp)
register c;
register struct tty *tp;
{
	register struct dt *lp;
	register int t_flags;
	register struct chan *cp;

	tk_nin += 1;
	lp = (struct dt *)tp->t_linep;
	c &= 0377;
	t_flags = tp->t_flags;
	if (t_flags&TANDEM)
		ttyblock(tp);
	if ((t_flags&RAW)==0) {
		c &= 0177;
		if (tp->t_state&TTSTOP) {
			if (c==tun.t_startc) {
				tp->t_state &= ~TTSTOP;
				ttstart(tp);
				return;
			}
			if (c==tun.t_stopc)
				return;
			tp->t_state &= ~TTSTOP;
			ttstart(tp);
		} else {
			if (c==tun.t_stopc) {
				tp->t_state |= TTSTOP;
				(*cdevsw[major(tp->t_dev)].d_stop)(tp);
				return;
			}
			if (c==tun.t_startc)
				return;
		}
		if (c==tun.t_quitc || c==tun.t_intrc) {
#ifdef VMUNIX
			flushtty(tp, FREAD|FWRITE);
#else
			flushtty(tp);
#endif
			while (lp->outq.c_cc)
				getc(&lp->outq);
			c = (c==tun.t_intrc) ? SIGINT:SIGQUIT;
			if (tp->t_chan)
				scontrol(tp->t_chan, M_SIG, c);
			else
#ifdef VMUNIX
				gsignal(tp->t_pgrp, c);
#else
				signal(tp->t_pgrp, c);
#endif
			wakeup(&lp->outq);
			return;
		}
		if (c=='\r' && t_flags&CRMOD)
			c = '\n';
	}
	if (tp->t_rawq.c_cc>TTYHOG) {
#ifdef VMUNIX
		flushtty(tp, FREAD|FWRITE);
#else
		flushtty(tp);
#endif
		return;
	}
	if (t_flags&LCASE && c>='A' && c<='Z')
		c += 'a'-'A';
	putc(c, &tp->t_rawq);
	if (t_flags&(RAW|CBREAK)||(c=='\n'||c==tun.t_eofc||c==tun.t_brkc)) {
		if ((t_flags&(RAW|CBREAK))==0 && putc(0377, &tp->t_rawq)==0)
			tp->t_delct++;
		if ((cp=tp->t_chan)!=NULL)
			sdata(cp); else
			wakeup((caddr_t)&tp->t_rawq);
	}
	if (t_flags&ECHO) {
		if (c != CEOT)		/* Some terminals don't like it */
			dtloutput(c, tp);
		if (c==tp->t_kill && (t_flags&(RAW|CBREAK))==0)
			dtloutput('\n', tp);
		ttstart(tp);
	}
}

dtltimer(arg)
caddr_t arg;
{
	register struct dt *lp;

	for (lp=dtl; lp < &dtl[NDT]; lp++) {
		if ((lp->dtstate & DTOPEN) == 0)
			continue;
		if (lp->dtstate & CLOG2)
			wakeup((caddr_t)&lp->outq);
		if (lp->dtstate & CLOG1)
			lp->dtstate |= CLOG2;
	}
	timeout(dtltimer, (caddr_t)0, DTTIME);
}
dt_modem(tp)
register struct tty *tp;
{
	register struct dt *lp;

	lp = (struct dt *)tp->t_linep;

	wakeup((caddr_t)&lp->outq);
}

dt_lioctl(tp, cmd, addr)
register struct tty *tp;
caddr_t addr;
{
	register struct dt *lp;

	lp = (struct dt *)tp->t_linep;
	if (cmd == TIOCSETP) {
		while (lp->outq.c_cc) {
			lp->dtstate |= LSLP;
			sleep((caddr_t)&lp->outq, TTOPRI);
		}
	}
	return(cmd);
}

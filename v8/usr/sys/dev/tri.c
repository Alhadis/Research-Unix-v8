/*
 * trilog driver
 */

#include "tri.h"
#if NTRI>0
#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/pte.h"
#include "../h/ubavar.h"
#include "../h/ubareg.h"
#include "../h/systm.h"


#define LPPRI   PZERO+10
#define LPLOWAT 50

struct device {
	short   tricsr, tribuf;
};

#define ERRBIT  0100000
#define BUFSIZ	512
#define LPSIZ	(2*BUFSIZ)

struct tri {
	char	tricbuf[LPSIZ];	/* one buffer, no clist */
	char	*trir,*triw;	/* and two chasing pointers */
	int	tricc;		/* char count to disambiguate */
	char    flag;		/* the case of trir == triw */
} tritab[NTRI];

int tritimeout;

/*
 * flag bits
 */
#define OPEN    010
#define LBUSY   020
#define NOCR    040
#define ASLP    0100
#define ESLP    0200

#define IENABLE 0100
#define DONE    0200

int	triattach(), triprobe();
struct	uba_device *triinfo[NTRI];
u_short	tristd[] = { 0 };
struct	uba_driver tridriver =
	{ triprobe, 0, triattach, 0, tristd, "tri", triinfo };

triprobe(reg)
caddr_t reg;
{
	register int br, cvec;
	register struct device *triaddr = (struct device *)reg;

	triaddr->tricsr = IENABLE;
	triaddr->tribuf = '\0';
	DELAY(10000);
	return(1);
}

triattach()
{
}

triopen(dev, flag)
dev_t dev;
{
	register int s;
	register unit, d;
	int tritimer();
	register struct tri *tri;
	register struct uba_device *ui;

	d = minor(dev);
	unit = d&07;
	tri = &tritab[unit];
	ui = triinfo[unit];
	if (unit>=NTRI || tri->flag || ui->ui_alive==0) {
		u.u_error = ENXIO;
		return;
	}
	tri->flag = (d&077)|OPEN;
	tri->triw = tri->trir = tri->tricbuf;
	tri->tricc = 0;
	while ((((struct device *)ui->ui_addr)->tricsr&DONE) == 0)
		sleep((caddr_t)&lbolt, LPPRI);
	if (tritimeout==0) {
		tritimeout++;
		timeout(tritimer, (caddr_t)0, 10*hz);
	}
}

tritimer()
{
register struct tri *tri;
register unit, flag;

	for (unit=flag=0; unit<NTRI; unit++) {
		tri = &tritab[unit];
		if (tri->flag==0) {
			continue;
		}
		flag++;
		if (tri->flag&ESLP) {
			tri->flag &= ~ESLP;
			triintr(unit);
		}
	}
	if (flag) {
		timeout(tritimer, (caddr_t)0, 10*hz);
	} else {
		tritimeout = 0;
	}
}



triclose(dev)
register dev_t dev;
{
	register unit;
	register struct tri *tri;

	unit = minor(dev)&07;
	tri = &tritab[unit];

	triwait(tri, 0);
	tri->flag = 0;
	((struct device *)triinfo[unit]->ui_addr)->tricsr = 0;
}


triwait(tri, count)
register struct tri *tri;
register count;
{
	register int s, times;
	register r;

	times = 60;
	while (tri->tricc > count) {
		tri->flag |= ASLP;
		r = tsleep((caddr_t)tri, LPPRI, 10);
		if (r==TS_TIME) {
			s = spl4();
			if ((tri->flag&LBUSY)==0)
				triintr(tri-tritab);
			splx(s);
			times--;
			if (times==0) {
				printf ("cpr timeout, buffer flushed\n");
				goto flush;
			}
		}
		if (r==TS_SIG) {
		flush:
			tri->trir = tri->triw = tri->tricbuf;
			tri->tricc = 0;
		}
	}
}

triwrite(dev)
{
	register struct tri *tri;
	register char *p;
	register c, cc;
	register s;
	register char *trir;

	dev = minor(dev);
	tri = &tritab[dev];
	while(u.u_count) {
		trir = tri->trir;
		if (trir > tri->triw)
			cc = trir - tri->triw;
		else if (trir == tri->triw && tri->tricc != 0)
			cc = 0;	/* buffer full */
		else		/* from triw to the end of the buffer */
			cc = tri->tricbuf + LPSIZ - tri->triw;
		if (u.u_count < cc)
			cc = u.u_count;
		if (cc == 0) {	/* buffer must be full */
			s = spl4();
			if ((tri->flag&LBUSY) == 0) {
				triintr(dev);	/* poke it */
			}
			splx(s);
			triwait(tri,LPLOWAT);
		} else {	/* queue up some characters */
			iomove(tri->triw, cc, B_WRITE);
			if ((tri->triw += cc) >= tri->tricbuf + LPSIZ)
				tri->triw = tri->tricbuf;
			tri->tricc += cc;
			s = spl4();
			if ((tri->flag&LBUSY) == 0)
				triintr(dev);
			splx(s);
		}
	}
}


triintr(dev)
register dev;
{
	register struct tri *tri;
	register struct device *addr;
	register c, cc;

	tri = &tritab[dev];
	addr = (struct device *)triinfo[dev]->ui_addr;
	if ((addr->tricsr&DONE)==0) {
		return;
	}
	tri->flag &= ~LBUSY;
	if (addr->tricsr&ERRBIT) {
		addr->tricsr = 0;
		tri->flag &= ~LBUSY;
		tri->flag |= ESLP;
		return;
	}
	cc = 0;
	while (addr->tricsr&DONE && (tri->tricc > 0)) {
		addr->tribuf = *(tri->trir)++;
		if (tri->trir == tri->tricbuf + LPSIZ)
			tri->trir = tri->tricbuf;
		tri->tricc--;
		cc++;
	}
	if (cc) {
		addr->tricsr |= IENABLE;
		tri->flag |= LBUSY;
	}
	if (tri->tricc <= LPLOWAT && tri->flag&ASLP) {
		tri->flag &= ~ASLP;
		wakeup((caddr_t)tri);
	}
}
#endif

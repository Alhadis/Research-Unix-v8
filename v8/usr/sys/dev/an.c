#include "an.h"
#if NAN > 0

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/pte.h"
#include "../h/map.h"
#include "../h/vm.h"
#include "../h/anet.h"
#include "../h/cmap.h"
#include "../h/ubareg.h"
#include "../h/ubavar.h"
#include "../h/cpu.h"
#include "../h/proc.h"
#include "../h/status.h"


#ifdef ANDEBUG
int andebug = 1;
#else
int andebug = 0;
#endif

/* flags for status */
#define INPUT	1
#define OUTPUT	2

#define min(x,y) ((x) < (y) ? (x) : (y))
#define	swab(x)	(((x<<8) | ((x&0xFF00)>>8)) & 0xFFFF)

#define ANPRI	28

struct device {
	short drcs;
	short drout;
	short drin;
};

/* Command and Status word */
#define CSR0 0x1
#define CSR1 0x2
#define RIE 0x20
#define TIE 0x40
#define TEMPTY 0x80
#define RFULL 0x8000

/*
 *	CSR1 CSR0
 *	 0    0 	Write Data
 *	 0    1 	Write EOP
 *	 1    0 	Write Command
 *	 1    1 	Read Status
 */
#define DATA_MODE	(RIE)
#define EOP_MODE	(CSR0 | RIE)
#define CMD_MODE	(CSR1 | RIE)
#define STATUS_MODE	(CSR0 | CSR1 | RIE)

#define MAXMSGLEN 2042
#define	IBUF	1
#define	OBUF	2

struct an_dev {
	char open;
	char flag;
	short uid;
	short icnt, ocnt;
	short ibuf[MAXMSGLEN/2];
	short obuf[MAXMSGLEN/2];
} an_dev[NAN];

#ifdef TIMEOUT
#define CKFUZZ 	4	/* number of timeouts we wait for a response */
#define CKTICKS	(hz/4)	/* number of ticks per timeout */

int an_state;
#define	OPEN 1		/* set if at least one sn? is open */
#define	TIMER 2		/* set if at timer is running */

int cktimchk();
int an_opncnt;
#endif TIMEOUT

/*struct anmach anmach[8] = {
	{ 1, 255 },
	{ 2, 254 },
	{ 3, 253 },
	{ 4, 252 },
	{ 5, 251 },
	{ 6, 250 },
	{ 7, 249 },
	{ 8, 248 } };*/

#define RETURN(x)	{u.u_error = x; return(0);}

int	anprobe(), anattach();
struct	uba_device *andinfo[NAN];
u_short	anstd[] = {0};
struct uba_driver andriver =
	{ anprobe, 0, anattach, 0, anstd, "an", andinfo};

anprobe(reg)
caddr_t reg;
{
	register int br, cvec;	/* value result */

	br = 0x15;
	cvec = 0510;
	return(1);
}

anattach()
{
	register struct device *draddr;
	register struct uba_device *ui;

	ui = andinfo[0];
	draddr = (struct device *)ui->ui_addr;
	draddr->drcs = CMD_MODE;
	draddr->drout = 0;
	draddr->drout = MASTER_CLEAR;
	draddr->drout = 0;
	draddr->drout = BOARD_RESET;
	draddr->drout = 0;
	draddr->drcs = STATUS_MODE;
	printf("S/NET BIB #%d\n", (draddr->drin>>10)&0xF);

}

/*
	open the argus network
*/

anopen(dev)
	register dev_t dev;
{
	register struct device *draddr;
	register struct an_dev *an;
	register struct uba_device *ui;
	int net = NET(dev);

	if(andebug)
		printf("anopen(%x)\n", dev);
	if(net >= NAN || (ui = andinfo[net]) == 0 || ui->ui_alive == 0)
		RETURN(ENXIO);
	draddr = (struct device *)ui->ui_addr;

	an = &an_dev[net];
	spl6();
	if (an->open++ != 0) {
		spl0();
		RETURN(EBUSY);
	}
	an->uid = u.u_uid;
#ifdef TIMEOUT
	sn_opncnt++;
	sn_state |= OPEN;
	if ((sn_state & TIMER) == 0) {
		sn_state |= TIMER;
		timeout(cktimchk, (caddr_t)0, CKTICKS);
	}
#endif TIMEOUT
	draddr->drcs = CMD_MODE;
	draddr->drout = 0;
	draddr->drout = BOARD_RESET;
	draddr->drout = 0;
	draddr->drcs = DATA_MODE;

	spl0();
	return(1);
}

anclose(dev)
{
	struct uba_device *ui;
	register struct an_dev *an;
	int net = NET(dev);
	
	if(andebug)
		printf("anclose(%x)\n", dev);
	if(net >= NAN)
		RETURN(ENXIO)
	an = &an_dev[net];
	spl6();
	an->open = 0;
	spl0();
}

anread(dev)
	dev_t dev;
{
	int net;
	register struct device *draddr;
	register struct an_dev *an;
	register struct uba_device *ui;
	short stat;

	net = NET(dev);
	if(net >= NAN)
		RETURN(ENXIO)
	ui = andinfo[net];
	draddr = (struct device *)ui->ui_addr;
	an = &an_dev[net];
	if(an->open == 0) RETURN(ENXIO)
	spl6();
	if((an->flag&IBUF) == 0)
		RETURN(EIO);
	if(copyout(an->ibuf, u.u_base, an->icnt))
	{
		spl0();
		RETURN(EFAULT);
	}
	u.u_count -= an->icnt;
	an->flag &= ~IBUF;
	spl0();
}

anwrite(dev)
	dev_t dev;
{
	int net;
	register struct device *draddr;
	register struct an_dev *an;
	register struct uba_device *ui;
	short stat;

	net = NET(dev);
	if(net >= NAN)
		RETURN(ENXIO)
	ui = andinfo[net];
	draddr = (struct device *)ui->ui_addr;
	an = &an_dev[net];
	if(an->open == 0) RETURN(ENXIO)
	if((an->ocnt = u.u_count) > MAXMSGLEN)
		an->ocnt = MAXMSGLEN;
	if(an->ocnt < 2)
		RETURN(EIO);
	if(copyin(u.u_base, an->obuf, an->ocnt))
		RETURN(EFAULT);
	if(andebug)
		printf("write on dev %d of %d bytes, sh[0]=%x, sh[1]=%x\n",
			net, u.u_count, an->obuf[0], an->obuf[1]);
	an->flag |= OBUF;
	if(anxstart(net))
		u.u_count -= an->ocnt;
}

anrint(net)
	int net;
{
	register struct an_dev *an;
	register short *dp, *sp;
	register struct device *draddr;
	register struct uba_device *ui;
	short stat;
	int i, len, clen;

	an = &an_dev[net];
	if(an->flag&IBUF)
	{
		andrain(net);
		return;
	}
	ui = andinfo[net];
	draddr = (struct device *)ui->ui_addr;
	dp = &draddr->drin;

	len = *dp;
	len = swab(len);
	if(len > MAXMSGLEN)
		len = MAXMSGLEN;
	sp = an->ibuf;
	an->icnt = len;
	for(i = (len+1)/2; i; i--)
		*sp++ = *dp;
	draddr->drcs = STATUS_MODE;
	*sp = *dp;
	draddr->drcs = DATA_MODE;
	i = spl6();
	an->flag |= IBUF;
	splx(i);
	if ((*sp&INBUF_E) == 0)
		andrain(net);
}

andrain(net)
	int net;
{
	register struct an_dev *an;
	register short *dp;
	short sp;
	register struct device *draddr;
	register struct uba_device *ui;
	short stat;
	int i, len;

	ui = andinfo[net];
	draddr = (struct device *)ui->ui_addr;
	dp = &draddr->drin;

	do {
		len = *dp;
		len = swab(len);
		if(len > MAXMSGLEN)
			len = MAXMSGLEN;
		for(i = (len+1)/2; i; i--)
			sp = *dp;
		sp = *dp;
		sp = *dp;
		draddr->drcs = STATUS_MODE;
		sp = *dp;
		draddr->drcs = DATA_MODE;
	} while((sp&INBUF_E) == 0);
}

anxstart(net)
	int net;
{
	register struct an_dev *an;
	register short *dp, *sp;
	register struct device *draddr;
	register struct uba_device *ui;
	int retrys, nreps;
	int i, len;

	ui = andinfo[net];
	draddr = (struct device *)ui->ui_addr;
	an = &an_dev[net];
	dp = &draddr->drout;
	draddr->drcs = STATUS_MODE;

	i = draddr->drin;
	nreps = retrys = 0;
	if ( (i&OUTBUF_E) == 0 ) {
		do {
			if(++retrys > 100000)
			{
				nreps++;
				printf("anxstart: %d retrys on OUTBUF_E loop\n", nreps*retrys);
				retrys = 0;
				draddr->drcs = CMD_MODE;
				if(nreps >= 2)
				{
					printf("S/NET: MASTER CLEAR\n");
					draddr->drout = 0;
					draddr->drout = MASTER_CLEAR;
				}
				draddr->drout = 0;
				draddr->drout = BOARD_RESET;
				draddr->drout = 0;
				draddr->drcs = DATA_MODE;
				if(nreps >= 3) return(0);
			}
			i = draddr->drin;
		} while( (i&OUTBUF_E) == 0 );
	}
	retrys = 0;
	len = an->ocnt;

loop:
	sp = an->obuf;
	draddr->drcs = CMD_MODE;
	*dp = *sp++;
	draddr->drcs = DATA_MODE;
	for(i = (len+1)/2-1; i > 1; i--)
		*dp = *sp++;
	draddr->drcs = EOP_MODE;
	*dp = *sp;

	draddr->drcs = STATUS_MODE;
	i = draddr->drin;
	draddr->drcs = DATA_MODE;
	if(i&SNACK) {
		if(++retrys < 100)
			goto loop;
		else
			return(0);
	}
	return(1);
}

anioctl(dev, cmd, addr, flag)
	caddr_t addr;
	dev_t dev;
{
	int i;
	int error;
	int net = NET(dev);

	if(net >= NAN)
		RETURN(ENXIO)
	switch(cmd)
	{
	case ANRESET:
		/*if(copyin(addr, (caddr_t)&args, sizeof(args)))
			RETURN(EFAULT)*/
		break;
	}
}

#ifdef TIMEOUT
cktimchk()
{
	register struct chdat *cp;
	register struct sninfo *sninfop;
	int unit;
	int s;

	for(cp=sn_chans; cp < &sn_chans[NUMLCH]; cp++) {
		if( cp->flags&(SENTDATA|SENTRDY) && --cp->ckticks == 0 ) {
			unit = cp->snunit;
			sninfop = &sn_sninfo[unit];
			sninfop->snsched++;
			sninfop->snenqlost[cp->machno]++;
			cp->flags |= RETRY;
			if (cp->flags & SENTDATA) {
				wakeup((caddr_t)cp->output.buf);
			} else {
				wakeup((caddr_t)cp->input.buf);
			}
		}
	}
	s = spl6();
	if (sn_state & OPEN) {
		timeout(cktimchk, (caddr_t)0, CKTICKS);
	} else {
		sn_state &= ~TIMER;
	}
	splx(s);
}
#endif TIMEOUT

#endif

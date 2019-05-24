/*	scn.c		84/09/05	*/

#include "scn.h"
#if NSCN > 0
/*
 * UNIBUS DR11-B driver for teletype scanner
 *
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/cpu.h"
#include "../h/nexus.h"
#include "../h/dk.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/map.h"
#include "../h/pte.h"
#include "../h/mtpr.h"
#include "../h/vm.h"
#include "../h/ubavar.h"
#include "../h/ubareg.h"
#include "../h/cmap.h"

#include "../h/scncmd.h"

struct scndevice{
	u_short scn_recvst;	/* receive status register */
	u_short scn_recv;	/* receive data register */
	u_short scn_sendst;	/* send status register */
	u_short scn_send;	/* send data register */
	short scnwc;		/* word count reg - unused but load -1 */
	u_short scnba;		/* base address reg - must be 16k bdry */
	u_short scncs;		/* command/status reg */
	u_short scndata;	/* data reg */
};
struct scnsoftc {
	u_short sbmode;		/*select bit mode*/
	u_short bpmode;		/* bit pack mode*/
	u_short sc_intno;	/* dr11b interrupt number */
	int scnreg;		/* index into uba_map */
	u_short scn_offset;	/* offset from 16k bdry */
	u_int sc_maxi;	/* max interrupt */
	caddr_t scnb_addr;
	unsigned scn_bdp;
	u_short scn_sync;
} scnsoftc[NSCN];

#define	SCN_GO	01
#define	SCN_IE	0100
#define	SCN_NEX	040000
#define	SCN_ERROR	0100000

#define SCNSIZE 16384
#define SCNPAD	16384
#define SCNMASK (SCNPAD-1)
#define SCNHSIZE 8192
#define SCNHREG 16
#define SCNREGMASK	0760
#define SCNREST 037777740000

#define SCN_LADDR	037777
#define SCN_HADDR	074

int	scnprobe(), scnattach(), scndgo(), scnintr();
int	scnstrategy(), scnstart();
u_int scnminp();
struct	uba_ctlr *scnminfo[NSCN];
struct	uba_device *scndinfo[NSCN];
struct	uba_ctlr scnctlr[NSCN];	/* scminfo points to this */
u_short scnstd[]	= { 0174020, 0};	
struct	uba_driver scndriver =
	{ scnprobe,0,scnattach,scndgo,scnstd,"scn",scndinfo,"scn",scnminfo };

#define ui_open	ui_type
struct	buf	scnbuf[NSCN];
struct	buf	scnutab[NSCN];




/*ARGSUSED*/
scnprobe(reg)
caddr_t reg;
{
	register int br, cvec;	/*int vecs at 310 & 314 */
	register struct scndevice *scnaddr = (struct scndevice *) reg;
	register short bucket;
#ifdef LINT
	br = 0; cvec = br; br = cvec;
#endif

	scnaddr->scn_recvst = SCN_REC_IE;
	scnaddr->scn_send = SCNRS_S;
	DELAY(10000);
	bucket = scnaddr->scn_recv;	
	scnaddr->scn_recvst = 0;
	return(1);
}

scnattach(ui)
register struct uba_device *ui;
{
	register struct uba_ctlr *um;
	register int unit;

	unit = ui->ui_unit;

	um = &scnctlr[unit];
	scnminfo[unit] = um;
	ui->ui_ctlr = unit;
	ui->ui_mi = um;
	um->um_driver = ui->ui_driver;
	um->um_ctlr = unit;
	um->um_ubanum = ui->ui_ubanum;
	um->um_alive = 1;
	um->um_intr = ui->ui_intr;
	um->um_addr = ui->ui_addr;
	um->um_hd = ui->ui_hd;
}

scnopen(dev)
dev_t dev;
{
	register int unit = minor(dev);
	register struct scnsoftc *scnp = &scnsoftc[minor(dev)];
	register struct uba_device *ui = scndinfo[unit];
	register struct scndevice *draddr = (struct scndevice *)ui->ui_addr;

	if((unit >= NSCN) || (scndinfo[unit]->ui_open)) {
		u.u_error = ENXIO;
		return;
	}
	scndinfo[unit]->ui_open++;
	draddr->scn_recvst = 0;
	scnp->sbmode = SCN_MOD_BW;
	scnp->bpmode = 0;
}

scnclose(dev)
dev_t dev;
{
	register int unit;

	scndinfo[minor(dev)]->ui_open = 0;
}

scnread(dev)
dev_t dev;
{
	register int unit = minor(dev);
	physio(scnstrategy,&scnbuf[unit],dev,B_READ,scnminp);

}

scnwrite(dev)
dev_t dev;
{
	register int unit = minor(dev);

	physio(scnstrategy,&scnbuf[unit],dev,B_WRITE,scnminp);
}
u_int
scnminp(bp)
struct buf *bp;
{
	register struct scnsoftc *scnp;
	scnp = &scnsoftc[minor(bp->b_dev)];
	scnp->sc_maxi = ((bp->b_bcount - SCNPAD)/SCNHSIZE);
}

/*
 * Due to the fact the scstrategy routine is called only by scnread
 * and scnwrite via physio, there will only be one transaction in each
 * DR11-B's queue at any time.  Therefore, one can just tack the given
 * buffer header pointer on at the end of the queue, and call scstart.
 */
scnstrategy(bp)
register struct buf *bp;
{
	register struct uba_device *ui;
	register struct uba_ctlr *um;
	register struct buf *dp;
	register int s;
	struct scndevice *draddr;
	dev_t unit;


	unit = minor(bp->b_dev);	/* chose a DR11-B */
	ui = scndinfo[unit];
	um = ui->ui_mi;		/* get ctlr ptr */
	dp = &scnutab[unit];
	s = spl5();
	dp->b_actf = bp;
	dp->b_actl = bp;
	bp->av_forw = NULL;
	um->um_tab.b_actf = dp;
	um->um_tab.b_actl = dp;
	scnstart(um);
	splx(s);	/*dennis says this should go away*/
	switch(tsleep((caddr_t)bp, PRIBIO+1, 20)) {
	case TS_OK:
		break;
	case TS_SIG:	/* not supposed to happen*/
	case TS_TIME:
		draddr = (struct scndevice *)um->um_addr;
		bp->b_flags |=  B_ERROR;
		s = spl6();	/* if other goes this can to*/
		printf("timeout calling scintr\n");
		scnintr(unit);
		splx(s);	/*likewise this*/
	}
/*	splx(s);	this is where dennis thinks it should be*/

}

scnstart(um)
register struct uba_ctlr *um;
{
	register struct buf *bp,*dp;
	register struct scndevice *draddr;
	register struct scnsoftc *scnp;
	int cmd;

	dp = um->um_tab.b_actf;
	bp = dp->b_actf;
	scnp = &scnsoftc[minor(bp->b_dev)];

	scnp->sc_intno = 0;
	um->um_tab.b_active++;
	draddr = (struct scndevice *)um->um_addr;
	draddr->scnwc = -1;
	if(bp->b_flags & B_READ)
		cmd = SCN_IE;	
	else{
		u.u_error = EFAULT;
	}
	um->um_cmd = cmd|SCN_GO;
	draddr->scndata = scnp->sbmode|scnp->bpmode|SCN_ARM;

	draddr->scncs = cmd;
	bp->b_bcount = SCNSIZE + SCNPAD;

	DELAY(10);
	if( ubago(scndinfo[minor(bp->b_dev)])== 0)
		printf("ubago returned 0\n");
}

scndgo(um)
struct uba_ctlr *um;
{
	register struct scndevice *draddr = (struct scndevice *)um->um_addr;
	register unsigned addr, temp;
	register struct scnsoftc *scnp;
	struct buf *bp, *dp;

	dp = um->um_tab.b_actf;
	bp = dp->b_actf;
	scnp = &scnsoftc[minor(bp->b_dev)];

/*	temp = bp->b_un.b_addr;*/
	addr = um->um_ubinfo;
	addr = (addr + (SCNSIZE-1)) & ~(SCNSIZE-1);
/*	temp = (temp + (SCNSIZE-1)) & ~(SCNSIZE-1);*/
	scnp->scnreg = UBAI_MR(addr);
	scnp->scn_offset = (addr - um->um_ubinfo) & (SCNSIZE-1);
					/* add half so intr can */
	scnp->scnb_addr = bp->b_un.b_addr  + SCNHSIZE + scnp->scn_offset;
							/*was temp*/
	temp = UBAI_BDP(addr);
	scnp->scn_bdp = (temp << UBAMR_DPSHIFT) | UBAMR_MRV;
	printf("scndgo uminfo %o addr %o reg %o b_addr %o\n",um->um_ubinfo,
		addr, scnp->scnreg,bp->b_un.b_addr);
	printf("ba %o cs %o scnbaddr %o offset %o\n",(addr&040000),
		(um->um_cmd|((addr>>12)&SCN_HADDR)), scnp->scnb_addr,
		scnp->scn_offset);

	draddr->scnba = 0;
	scnp->scn_sync = 0;
	draddr->scncs = um->um_cmd|((addr>>12)&SCN_HADDR);
	draddr->scn_recvst |= SCN_REC_IE;
	draddr->scn_send = SCN_SCAN;
}

scnioctl(dev, cmd, arg)
dev_t dev;
int cmd;
register caddr_t arg;
{
	register struct scnsoftc *scnp = &scnsoftc[minor(dev)];
	register struct uba_device *ui = scndinfo[minor(dev)];
	register struct scndevice *draddr = (struct scndevice *)ui->ui_addr;
	register struct uba_ctlr *um;
	u_short realcmd;
	u_short stat;
	if( (cmd != SCNRECST) && (cmd != SCNREC) && (cmd != SCNSENDST)
		& (cmd != SCNHACK)){
		if(copyin(arg, (caddr_t)&realcmd, sizeof(realcmd))){
			u.u_error = EFAULT;
			return;
		}
	}
	switch(cmd){
	case SCNSENDST:
		realcmd = draddr->scn_sendst;
		break;
	case SCNSEND:
		stat = draddr->scn_sendst;
		if(stat & SCN_SREADY)
			draddr->scn_send = realcmd;
		else u.u_error = EFAULT;
		return;
	case SCNRECST:
		realcmd = draddr->scn_recvst;
		break;
	case SCNREC:
		realcmd = draddr->scn_recv;
		break;
	case SCNHACK:
		realcmd = scnp->scn_offset;
		break;
	case SCNSBMOD:
		scnp->sbmode = realcmd & SCN_SBMASK;
		return;
	case SCNBPMOD:
		scnp->bpmode = realcmd & SCN_BPMASK;
		return;
	default:
		u.u_error = ENXIO;
		return;
	}
	if(copyout((caddr_t)&realcmd, arg, sizeof(realcmd)))
		u.u_error = EFAULT;
}
scnreset() {}

scnxintr(dr11)
register dr11;
{
	register struct scndevice *draddr;
	register struct uba_ctlr *um;
	struct buf *dp, *bp;
	register int stat;
	struct scnsoftc *scnp;

	um = scnminfo[dr11];
	draddr = (struct scndevice *)um->um_addr;

	stat = draddr->scn_recv;
	if(stat == SCN_STATE2){
		draddr->scn_send = SCN_WAIT;
		return;
	}
	if(stat == SCN_STATE3){
		scnp = &scnsoftc[minor(dr11)];
		printf("got ack state 3:intno %o max %o\n", scnp->sc_intno,scnp->sc_maxi);
		dp = um->um_tab.b_actf;
		bp= dp->b_actf;
		draddr->scndata = 0;
		draddr->scn_recvst = 0;
		draddr->scncs = 0;
		um->um_tab.b_active = 0;
		um->um_tab.b_errcnt = 0;
		um->um_tab.b_actf = dp->b_forw;
		dp->b_errcnt = 0;
		dp->b_active = 0;
		bp->b_resid = 0;
		ubadone(um);
		iodone(bp);
		return;
	}
	return;
}

scnintr(dr11)
register dr11;
{
	register struct buf *bp,*dp;
	register struct scndevice *draddr;
	register struct uba_ctlr *um;
	register unsigned stat, reg,temp;
	u_short bit;
	struct pte *io, *pte;
	struct scnsoftc *scnp;

	um = scnminfo[dr11];
	scnp = &scnsoftc[dr11];

	if(um->um_tab.b_active == 0)
		return;

	if(scndinfo[dr11]->ui_open == 0)
		return;
	dp = um->um_tab.b_actf;
	bp = dp->b_actf;
	draddr = (struct scndevice *)um->um_addr;
	stat = draddr->scncs;
	if(stat & SCN_ERROR){
		reg = draddr->scnba;
		printf("scanner error %o addr %o intno %d\n",stat,
			reg,scnp->sc_intno);
		goto scbad;
	}
	if(scnp->sc_intno >= scnp->sc_maxi){
		printf("quiting on maxi %o\n",scnp->sc_intno);
		goto scbad;
	}
	bit = draddr->scnba & 020000;
	if(scnp->scn_sync == bit)
		printf("scn out of sync %d addr=%o\n",
			scnp->sc_intno,draddr->scnba);
	scnp->scn_sync = bit;
	reg = scnp->scnreg;
	if(scnp->sc_intno++ & 01)
		reg += SCNHREG;
	scnp->scnb_addr += SCNHSIZE;

	stat = btop(scnp->scnb_addr);
	pte = vtopte(bp->b_proc, stat);
 	
	io = &um->um_hd->uh_uba->uba_map[reg];
	stat = SCNHREG;
	temp = scnp->scn_bdp;
	while(stat-- > 0){
		if(pte->pg_pfnum == 0){
			printf("about to write to 0 pte int %o stat %d\n",
				scnp->sc_intno,stat);
			goto scbad;
		}
		*(int *)io++ = pte++->pg_pfnum | temp;
	}
	return;

scbad:
	draddr->scndata = 0;	/*turn off arm bit*/
	draddr->scn_recvst = 0;	/*turn off recv intr*/
	draddr->scncs = 0;	/*turn off scn intr*/
	um->um_tab.b_active = 0;
	um->um_tab.b_errcnt = 0;
	um->um_tab.b_actf = dp->b_forw;
	dp->b_errcnt = 0;
	dp->b_active = 0;
	bp->b_resid = 0;
	ubadone(um);
	iodone(bp);

}
#endif

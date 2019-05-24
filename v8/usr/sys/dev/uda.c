#include "ra.h"
#if NUDA > 0
/*
 * UDA50/RAxx disk device driver
 * this should really be two drivers!
 * can only handle unit numbers less than 8 for now
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/pte.h"
#include "../h/map.h"
#include "../h/vm.h"
#include "../h/ubareg.h"
#include "../h/ubavar.h"
#include "../h/dk.h"
#include "../h/cpu.h"
#include "../h/cmap.h"
#include "../h/trace.h"
#include "../h/file.h"

int udadebug = 0;
#define	printd	if(udadebug&1)printf

/*
 * Parameters for the communications area
 */

#define	NRSPL2	5
#define	NCMDL2	5
#define	NRSP	(1<<NRSPL2)
#define	NCMD	(1<<NCMDL2)

#include "../h/udareg.h"
#include "../h/udaioc.h"
#include "../h/mscp.h"

#define	SECTOR	512		/* size of an MSCP logical block */
#define	MAXUNIT	8		/* largest unit number we can handle */

struct uda_softc {
	short	sc_state;	/* state of controller */
	short	sc_flags;
	struct udadevice *sc_addr;
	int	sc_ubainfo;	/* Unibus mapping info */
	struct uda *sc_uda;	/* Unibus address of uda struct */
	int	sc_ivec;	/* interrupt vector address */
	short	sc_credits;	/* transfer credits */
	short	sc_lastcmd;	/* pointer into command ring */
	short	sc_lastrsp;	/* pointer into response ring */
	short	sc_ctime;	/* controller timeout interval */
} uda_softc[NUDA];

/*
 * sc_flags
 */

#define	SCCRED	01		/* someone waiting for credits */
#define	SCMAP	02		/* control structure is mapped */
#define	SCFIRST	04		/* allow packets without credit */

/*
 * Controller states
 */
#define	S_IDLE	0		/* hasn't been initialized */
#define	S_STEP1	1		/* doing step 1 init */
#define	S_STEP2	2		/* doing step 2 init */
#define	S_STEP3	3		/* doing step 3 init */
#define	S_SCHAR	4		/* doing "set controller characteristics" */
#define	S_RUN	5		/* running */

/*
 * communication area, per controller
 * mapped into UNIBUS space
 */

struct uda {
	struct udaca	uda_ca;		/* communications area */
	struct mscp	uda_rsp[NRSP];	/* response packets */
	struct mscp	uda_cmd[NCMD];	/* command packets */
} uda[NUDA];

/*
 * a word to the wise:
 * part of the minor device number selects a drive number
 * this is not necessarily the same as the physical unit number
 * in an attempt to clear this up, the former is referred to
 * as the `dev unit,' the latter as the `real unit'
 * most places use the real unit
 */
#define	NPART	8		/* 8 disk partitions */
#define	PART(x)	((x) & 07)	/* minor device -> partition */
#define	DUNIT(x) (((x) >> 3) & 07)	/* minor device -> dev unit */

#define	HUGE	0x7fffffff

struct size {
	daddr_t	nblocks;
	daddr_t	blkoff;
} ra_sizes[NPART] ={
	10240,	0,		/* like emulex */
	20480,	10240,		/* ra81 has 891072 blocks */
	249848,	30720,
	249848,	280568,
	249848,	530416,
	110808,	780264,
	749544,	30720,		/* slices 2,3,4 */
	HUGE,	0,		/* whole drive */
};

/* device flags (ui_flags) */
#define	DUP	1	/* up */
#define	DGTUNIT	2	/* needs GTUNIT */

#define	ui_offl	ui_type
#define	MAXOFFL	10

/* per unit stuff */
struct ud_unit ud_units[MAXUNIT];	/* per real unit */
int ud_open[NRA];			/* per dev unit */

char	ud_rflags;	/* replacement flags.  should be per controller */
#define	RPLOCK	01	/* someone is doing a replacement */
#define	RPWANT	02	/* somebody else wants to */
#define	RPDONE	04	/* replacement finished */

int	ud_rret;	/* status returned from replacement */

int	udprobe(), udslave(), udattach(), udintr();
struct	mscp *udgetcp();
struct	uba_ctlr *udminfo[NUDA];
struct	uba_device *uddinfo[NRA];	/* per dev unit */
struct	uba_device *udip[NUDA][MAXUNIT]; /* (ctlr, realunit) -> device */

u_short	udstd[] = { 0777550, 0 };
struct	uba_driver udadriver =
 { udprobe, udslave, udattach, 0, udstd, "ra", uddinfo, "uda", udminfo, 0 };
struct	buf rudbuf[NRA];		/* raw io buffer header */
struct	buf rctbuf[NRA];		/* buffer for RCT io */
struct	buf udutab[NRA];		/* drive queues */
struct	buf udwtab[NUDA];		/* I/O wait queue, per controller */

#define	b_qsize		b_resid		/* queue size per drive, in udutab */
#define	b_ubinfo	b_resid		/* Unibus mapping info, per buffer */

udprobe(reg, ctlr)
caddr_t reg;
int ctlr;
{	register int br, cvec;
	register struct uda_softc *sc = &uda_softc[ctlr];

#ifdef lint
	br = 0; cvec = br; br = cvec; reg = reg;
	udread(0); udwrite(0); udreset(0); udintr(0);
#endif
	/* SHOULD CHECK THAT IT REALLY IS A UDA */
	br = 0x15;
	cvec = sc->sc_ivec = (uba_hd[numuba].uh_lastiv -= 4);
	sc->sc_addr = (struct udadevice *)reg;
	return(1);
}

udslave(ui, reg)
	struct uba_device *ui;
	caddr_t reg;
{
	/*
	 * TOO HARD TO FIND OUT IF DISK IS THERE UNTIL
	 * INITIALIZED.  WE'LL FIND OUT WHEN WE FIRST
	 * TRY TO ACCESS IT.
	 */
#ifdef lint
	ui = ui; reg = reg;
#endif
	return(1);
}

udattach(ui)
	register struct uba_device *ui;
{

	if (ui->ui_dk > 0)
		dk_mspw[ui->ui_dk] = 1.0 / (60 * 51 * 256);	/* approx */
	ui->ui_flags = 0;
	udip[ui->ui_ctlr][ui->ui_slave] = ui;
	printf("\n");		/* we're expected to babble */
	ud_units[ui->ui_unit].radsize = (daddr_t)HUGE;
}

udopen(dev)
dev_t dev;
{
	register int unit;

	unit = DUNIT(minor(dev));
	if (unit >= NRA) {
		u.u_error = ENXIO;
		return;
	}
	ud_open[unit] |= 1 << PART(minor(dev));
}

udclose(dev)
dev_t dev;
{
	register int unit;
	register struct mscp *mp;
	struct uba_device *ui;
	struct uba_ctlr *um;
	struct uda_softc *sc;

	unit = DUNIT(minor(dev));
	ud_open[unit] &=~ (1 << PART(minor(dev)));
	if (ud_open[unit])
		return;
	/*
	 * last close on this drive
	 * put it offline
	 */
	ui = uddinfo[unit];
	if (ui->ui_flags == 0)
		return;
	um = ui->ui_mi;
	sc = &uda_softc[um->um_ctlr];
	if (udcredit(sc, 1) == 0)
		return;
	if ((mp = udgetcp(um)) == NULL)
		return;
	mp->mscp_opcode = M_OP_AVAIL;
	mp->mscp_unit = ui->ui_slave;
	mp->mscp_modifier = 0;
	mp->mscp_cmdref = (long)mp;	/* any old number */
	udsend(mp, sc);
	ui->ui_flags = 0;	/* drive is offline now */
}
	
/*
 * Initialize a UDA.  Set up UBA mapping registers,
 * initialize data structures, and perform hardware
 * initialization sequence.
 */

udinit(d)
{
	register struct uda_softc *sc;
	register struct uba_ctlr *um;
	register struct buf *bp;
	register struct mscp *mp;
	register int i;
	int s;

	sc = &uda_softc[d];
	um = udminfo[d];
	um->um_tab.b_active++;
	if ((sc->sc_flags & SCMAP) == 0) {
		sc->sc_ubainfo = uballoc(um->um_ubanum, (caddr_t)&uda[d],
		    sizeof (struct uda), 0);
		sc->sc_uda = (struct uda *)(sc->sc_ubainfo & 0x3ffff);
		sc->sc_flags |= SCMAP;
	}
	for (i = 0; i < NRA; i++)
		if (uddinfo[i] && uddinfo[i]->ui_mi == um)
			uddinfo[i]->ui_flags = 0;
	if (udhwinit(d) == 0)
		return;
	sc->sc_state = S_SCHAR;
	bp = &udwtab[d];
	bp->av_forw = bp->av_back = bp;
	sc->sc_lastcmd = 0;
	sc->sc_lastrsp = 0;
	s = spl5();
	if ((mp = udgetcp(um)) == NULL)
		panic("uda: no packets");
	mp->mscp_opcode = M_OP_STCON;
	mp->mscp_cntflgs = M_CF_ATTN|M_CF_MISC|M_CF_THIS;
	sc->sc_flags |= SCFIRST;
	udsend(mp, sc);
	splx(s);
	return;
}

/*
 * hardware initialization handshake
 * for simplicity, don't bother with init interrupts
 * returns nonzero if ok
 */

#define	UDA_STEPS	(UDA_ERR|UDA_STEP4|UDA_STEP3|UDA_STEP2|UDA_STEP1)

udhwinit(d)
int d;
{
	register struct uda_softc *sc;
	register struct udadevice *udaddr;
	register int i;
	register struct uda *ud;
	register struct uda *uud;
	time_t out;

	sc = &uda_softc[d];
	udaddr = sc->sc_addr;
	out = time + 1;
	udaddr->udaip = 0;		/* start initialization */
	while((udaddr->udasa & UDA_STEP1) == 0 && time <= out)
		;
	if((udaddr->udasa & UDA_STEPS) != UDA_STEP1) {
		porterror(udaddr->udasa, d);
		return (0);
	}
	sc->sc_state = S_STEP1;
	sc->sc_credits = 0;
	udaddr->udasa = UDA_ERR|(NCMDL2<<11)|(NRSPL2<<8)|(sc->sc_ivec/4);
	/* no diagnostic wrap, no interrupts during initialization */
	out = time + 11;
	while((udaddr->udasa & UDA_STEP2) == 0 && time <= out)
		;
	if((udaddr->udasa & UDA_STEPS) != UDA_STEP2) {
		porterror(udaddr->udasa, d);
		return (0);
	}
	sc->sc_state = S_STEP2;
	/* assert that &ringbase & 03 == 0 */
	udaddr->udasa = (short)&sc->sc_uda->uda_ca.ca_ringbase | UDA_PI;
	out = time + 11;
	while((udaddr->udasa & UDA_STEP3) == 0 && time <= out)
		;
	if((udaddr->udasa & UDA_STEPS) != UDA_STEP3) {
		porterror(udaddr->udasa, d);
		return (0);
	}
	sc->sc_state = S_STEP3;
	udaddr->udasa = ((long)&sc->sc_uda->uda_ca.ca_ringbase) >> 16;
	out = time + 11;
	while((udaddr->udasa & UDA_STEP4) == 0 && time <= out)
		;
	if((udaddr->udasa & UDA_STEPS) != UDA_STEP4) {
		porterror(udaddr->udasa, d);
		return (0);
	}
	udaddr->udasa = (3<<2)|UDA_LF;
	/* (3+1) double words per burst, last fail packets */
	uud = sc->sc_uda;
	ud = &uda[d];
	for (i = 0; i < NRSP; i++) {
		ud->uda_ca.ca_rspdsc[i] = UDA_OWN|UDA_INT|
			(long)&uud->uda_rsp[i].mscp_cmdref;
		ud->uda_rsp[i].mscp_dscptr = &ud->uda_ca.ca_rspdsc[i];
		ud->uda_rsp[i].mscp_header.uda_msglen =
			sizeof(struct mscp);
	}
	for (i = 0; i < NCMD; i++) {
		ud->uda_ca.ca_cmddsc[i] = UDA_INT|
			(long)&uud->uda_cmd[i].mscp_cmdref;
		ud->uda_cmd[i].mscp_dscptr = &ud->uda_ca.ca_cmddsc[i];
		ud->uda_cmd[i].mscp_header.uda_msglen =
			sizeof(struct mscp);
	}
	udaddr->udasa = UDA_GO;		/* finish init */
	return (1);
}

static char *pmesg[] = {
	"(error 0?)",
	"data read",
	"data write",
	"parity",
	"ram parity",
	"rom parity",
	"ring read",
	"ring write",
	"interrupt master",
	"host timeout",
	"credit limit",
	"unibus master",
	"diag controller",
	"instruction loop timeout",
	"invalid connection id",
	"interrupt write",
	"maint region",
	"maint load",
	"ram",
	"init sequence",
	"protocol incompat",
	"purge/poll",
};

porterror(sa, d)
short sa;
int d;
{
	register struct uda_softc *sc;
	int n;

	sc = &uda_softc[d];
	printf("uda%d port error, sa x%x ", d, sa);
	if(!(sa & UDA_ERR)) {
		printf("(time out?), state %d\n", sc->sc_state);
		sc->sc_state = S_IDLE;
		return;
	}
	n = (sa & 0x3ff);
	if(n > sizeof(pmesg)/sizeof(pmesg[0]))
		printf("(?)\n");
	else
		printf("%s\n", pmesg[n]);
	sc->sc_state = S_IDLE;
}

udstrategy(bp)
	register struct buf *bp;
{
	register struct uba_device *ui;
	register struct uba_ctlr *um;
	register struct buf *dp;
	register int unit;
	int part = PART(minor(bp->b_dev));
	daddr_t sz, maxsz;
	int s;

	sz = (bp->b_bcount+(SECTOR-1)) / SECTOR;
	unit = dkunit(bp);
	if (unit >= NRA) {
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}
	ui = uddinfo[unit];
	um = ui->ui_mi;
	if (ui == 0 || ui->ui_alive == 0) {
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}
	maxsz = ud_units[unit].radsize - ra_sizes[part].blkoff;
	if (maxsz > ra_sizes[part].nblocks)
		maxsz = ra_sizes[part].nblocks;
	if (bp != &rctbuf[unit]) {
		if (bp->b_blkno == maxsz) {
			bp->b_resid = bp->b_bcount;
			iodone(bp);
			return;
		}
		if (bp->b_blkno < 0 || bp->b_blkno+sz > maxsz) {
			bp->b_flags |= B_ERROR;
			iodone(bp);
			return;
		}
	}
	s = spl5();
	/*
	 * Link the buffer onto the drive queue
	 */
	dp = &udutab[ui->ui_unit];
	if (dp->b_actf == 0)
		dp->b_actf = bp;
	else
		dp->b_actl->av_forw = bp;
	dp->b_actl = bp;
	bp->av_forw = NULL;
	/*
	 * Link the drive onto the controller queue (why bother?)
	 */
	if (dp->b_active == 0) {
		dp->b_forw = NULL;
		if (um->um_tab.b_actf == NULL)
			um->um_tab.b_actf = dp;
		else
			um->um_tab.b_actl->b_forw = dp;
		um->um_tab.b_actl = dp;
		dp->b_active = 1;
	}
	if (um->um_tab.b_active == 0)
		(void) udstart(um);
	splx(s);
}

udstart(um)
	register struct uba_ctlr *um;
{
	register struct buf *bp, *dp;
	register struct mscp *mp;
	register struct udadevice *udaddr;
	register struct uda_softc *sc;
	register struct uba_device *ui;
	int i;

	sc = &uda_softc[um->um_ctlr];
	
loop:
	if ((dp = um->um_tab.b_actf) == NULL) {
		/*
		 * Release uneeded UBA resources and return
		 */
		um->um_tab.b_active = 0;
		return (0);
	}
	if ((bp = dp->b_actf) == NULL) {
		/*
		 * No more requests for this drive, remove
		 * from controller queue and look at next drive.
		 * We know we're at the head of the controller queue.
		 */
		dp->b_active = 0;
		um->um_tab.b_actf = dp->b_forw;
		goto loop;
	}
	um->um_tab.b_active++;
	udaddr = sc->sc_addr;
	if ((udaddr->udasa&UDA_ERR) || sc->sc_state != S_RUN) {
		if(sc->sc_state != S_IDLE) {
			harderr(bp, "ra");
			printf("uda%d udasa x%x, state %d\n",
				um->um_ctlr, udaddr->udasa&0xffff, sc->sc_state);
		}
		udinit(um->um_ctlr);
		/* SHOULD REQUEUE OUTSTANDING REQUESTS, LIKE UDRESET */
		return (0);
	}
	ui = uddinfo[dkunit(bp)];
	/*
	 * get a packet, fill it in, and send it
	 */
	if (udcredit(sc, 0) == 0)
		return (0);
	if ((mp = udgetcp(um)) == NULL)
		return (0);
	if (ui->ui_flags == 0) {	/* not online */
		if (++ui->ui_offl > MAXOFFL) {
			ui->ui_offl = 0;
			printf("uda%d ra%d offline\n", um->um_ctlr, ui->ui_slave);
			bp->b_flags |= B_ERROR;
			iodone(bp);
			dp->b_actf = bp->av_forw;
			if (dp->b_forw) {
				um->um_tab.b_actf = dp->b_forw;
				um->um_tab.b_actl->b_forw = dp;
				um->um_tab.b_actl = dp;
				dp->b_forw = NULL;
			}
			goto loop;
		}
		mp->mscp_opcode = M_OP_ONLIN;
		mp->mscp_unit = ui->ui_slave;
		dp->b_active = 2;	/* MAGIC */
		um->um_tab.b_actf = dp->b_forw;	/* remove from controller q */
		printd("uda: bring unit %d online\n", ui->ui_slave);
		udsend(mp, sc);
		goto loop;
	}
	ui->ui_offl = 0;
	if(ui->ui_flags & DGTUNIT) {
		mp->mscp_opcode = M_OP_GTUNT;
		mp->mscp_unit = ui->ui_slave;
		udsend(mp, sc);
		ui->ui_flags &= ~DGTUNIT;
		goto loop;
	}
	if ((i = ubasetup(um->um_ubanum, bp, UBA_WANTBDP|UBA_CANTWAIT)) == 0) {
		/* this is issued just to get an interrupt */
		/* to try ubasetup again, a doubtful procedure */
		mp->mscp_opcode = M_OP_GTUNT;
		mp->mscp_unit = ui->ui_slave;
		udsend(mp, sc);
		return(1);		/* wait for interrupt */
	}
	mp->mscp_cmdref = (long)bp;	/* pointer to get back */
	mp->mscp_opcode = bp->b_flags&B_READ ? M_OP_READ : M_OP_WRITE;
	mp->mscp_unit = ui->ui_slave;
	mp->mscp_lbn = bp->b_blkno + ra_sizes[PART(minor(bp->b_dev))].blkoff;
	mp->mscp_bytecnt = bp->b_bcount;
	/* address; datapath number in high byte */
	mp->mscp_buffer = (i & 0x3ffff) | (((i>>28)&0xf)<<24);
	bp->b_ubinfo = i;		/* save mapping info */
	udsend(mp, sc);
	if (ui->ui_dk >= 0) {
		dk_busy |= 1<<ui->ui_dk;
		dp->b_qsize++;
		dk_xfer[ui->ui_dk]++;
		dk_wds[ui->ui_dk] += bp->b_bcount>>6;
		/* the way initialization is done now, this is useless */
	}

	/*
	 * Move drive to the end of the controller queue
	 */
	if (dp->b_forw != NULL) {
		um->um_tab.b_actf = dp->b_forw;
		um->um_tab.b_actl->b_forw = dp;
		um->um_tab.b_actl = dp;
		dp->b_forw = NULL;
	}
	/*
	 * Move buffer to I/O wait queue
	 */
	dp->b_actf = bp->av_forw;
	dp = &udwtab[um->um_ctlr];
	bp->av_forw = dp;
	bp->av_back = dp->av_back;
	dp->av_back->av_forw = bp;
	dp->av_back = bp;
	goto loop;
}

/*
 * UDA interrupt routine.
 */
udintr(d)
	int d;
{
	register struct uba_ctlr *um;
	register struct udadevice *udaddr;
	register int i;
	register struct uda_softc *sc;
	register struct uda *ud;

	if (d < 0 || d >= NUDA)
		panic("udintr");
	um = udminfo[d];
	sc = &uda_softc[d];
	ud = &uda[d];
	udaddr = sc->sc_addr;
	switch (sc->sc_state) {
	case S_IDLE:
		printf("uda%d: random intr, status x%x\n", udaddr->udasa);
		return;

	case S_SCHAR:
	case S_RUN:
		break;

	default:
		printf("uda%d: intr in unknown state %d ignored\n",
			d, sc->sc_state);
		porterror(udaddr->udasa, d);
		return;
	}

	trace(TR_UDRESP, 0, udaddr->udasa);
	if (udaddr->udasa&UDA_ERR) {
		printf("uda%d: fatal error, udasa x%x\n",
			d, udaddr->udasa&0xffff);
		udaddr->udaip = 0;
		sc->sc_state = S_IDLE;
	}
	if (ud->uda_ca.ca_bdp == 0
	&&  ud->uda_ca.ca_rspint == 0
	&&  ud->uda_ca.ca_cmdint == 0)
		printf("uda%d: spurious intr\n", um->um_ctlr);
	/*
	 * buffer purge request
	 */
	if (ud->uda_ca.ca_bdp) {
		/*
		 * THIS IS A KLUDGE.
		 * Maybe we should change the entire
		 * UBA interface structure.
		 */
		int s = spl7();

		i = um->um_ubinfo;
		printd("uda%d purge bdp %d\n", um->um_ctlr, ud->uda_ca.ca_bdp);
		um->um_ubinfo = ud->uda_ca.ca_bdp<<28;
		ubapurge(um);
		um->um_ubinfo = i;
		(void) splx(s);
		ud->uda_ca.ca_bdp = 0;
		udaddr->udasa = 0;	/* signal purge complete */
	}
	/*
	 * response ring transition
	 */
	if (ud->uda_ca.ca_rspint) {
		ud->uda_ca.ca_rspint = 0;
		for (i = sc->sc_lastrsp;; i++) {
			i %= NRSP;
			if (ud->uda_ca.ca_rspdsc[i]&UDA_OWN)
				break;
			udrsp(um, ud, sc, i);
			ud->uda_ca.ca_rspdsc[i] |= UDA_OWN;
		}
		sc->sc_lastrsp = i;
	}
	/*
	 * command ring transition
	 */
	if (ud->uda_ca.ca_cmdint) {
		printf("uda%d command ring transition\n", um->um_ctlr);
		ud->uda_ca.ca_cmdint = 0;
	}
	(void) udstart(um);
}

/*
 * send a packet to the uda
 */

udsend(mp, sc)
struct mscp *mp;
struct uda_softc *sc;
{
	register short junk;

	if ((sc->sc_flags & SCFIRST) == 0) {
		if (sc->sc_credits < 2)
			panic("udsend no credits");
		sc->sc_credits--;
	}
	trace(TR_UDCMND, mp->mscp_opcode, mp->mscp_cmdref);
	*((long *)mp->mscp_dscptr) |= UDA_OWN | UDA_INT;
	junk = sc->sc_addr->udaip;		/* trigger polling */
}

/*
 * check whether a packet may be sent
 * wait until possible if desired
 * call at spl6, and stay there until you've sent the packet
 * else your credit may be stolen
 */

udcredit(sc, wait)
register struct uda_softc *sc;
int wait;
{

	if (wait == 0 && sc->sc_credits < 2)
		return (0);
	while (sc->sc_credits < 2) {
		sc->sc_flags |= SCCRED;
		sleep((caddr_t)&sc->sc_credits, PRIBIO);
	}
	return (1);
}
	
/*
 * Process a response packet
 */
udrsp(um, ud, sc, i)
	register struct uba_ctlr *um;
	register struct uda *ud;
	register struct uda_softc *sc;
	int i;
{
	register struct mscp *mp;
	register struct uba_device *ui;
	register struct buf *dp, *bp;
	register int st;
	struct ud_unit *up;

	mp = &ud->uda_rsp[i];
	st = mp->mscp_status;
	trace(TR_UDRESP, mp->mscp_cmdref, mp->mscp_status);
	printd("udrsp: 0x%x,st=x%x\n", mp->mscp_opcode, st);
	mp->mscp_header.uda_msglen = sizeof (struct mscp);
	sc->sc_credits += mp->mscp_header.uda_credits & 0xf;
	sc->sc_flags &=~ SCFIRST;
	if (sc->sc_flags & SCCRED) {
		sc->sc_flags &=~ SCCRED;
		wakeup((caddr_t)&sc->sc_credits);
	}
	if((mp->mscp_header.uda_credits & 0xf0) > 0x10)	/* credits only */
		return;
	/*
	 * If it's an error log message (datagram),
	 * pass it on for more extensive processing.
	 */
	if ((mp->mscp_header.uda_credits & 0xf0) == 0x10) {
		uderror(um, (struct mslg *)mp);
		return;
	}
	if (mp->mscp_unit >= MAXUNIT)
		return;
	ui = udip[um->um_ctlr][mp->mscp_unit];
	switch (mp->mscp_opcode) {
	case M_OP_STCON|M_OP_END:
		if ((st & M_ST_MASK) == M_ST_SUCC) {
			sc->sc_state = S_RUN;
			printf("uda%d run\n", um->um_ctlr);
		}
		else {
			sc->sc_state = S_IDLE;
			printf("uda%d: can't set ctlr char, status x%x\n",
				um->um_ctlr, st);
		}
		sc->sc_ctime = mp->mscp_un.mscp_setcntchar.Mscp_hsttmo;
		um->um_tab.b_active = 0;
		break;

	case M_OP_ONLIN|M_OP_END:
		/*
		 * Link the drive onto the controller queue
		 */
		if(ui == 0) {
bad:
			printf("uda%d ra%d op x%x st x%x unexpected\n",
				um->um_ctlr, mp->mscp_unit, mp->mscp_opcode, st);
			return;
		}
		dp = &udutab[ui->ui_unit];
		dp->b_forw = NULL;
		if (um->um_tab.b_actf == NULL)
			um->um_tab.b_actf = dp;
		else
			um->um_tab.b_actl->b_forw = dp;
		um->um_tab.b_actl = dp;
		if ((st & M_ST_MASK) == M_ST_SUCC) {
			ui->ui_flags = DUP;	/* mark it online */
			ud_units[ui->ui_unit].radsize = (daddr_t)mp->mscp_untsize;
		} else {
			printf("uda%d ra%d online failed; status x%x\n",
				um->um_ctlr, mp->mscp_unit, st);
			while (bp = dp->b_actf) {
				dp->b_actf = bp->av_forw;
				bp->b_flags |= B_ERROR;
				iodone(bp);
			}
		}
		ui->ui_flags |= DGTUNIT;
		dp->b_active = 1;
		break;

	case M_OP_AVATN:	/* is this the only kind of attn? */
		if(ui == 0)
			goto bad;
		ui->ui_flags = 0;	/* it went offline, get status */
		break;

	case M_OP_READ|M_OP_END:
	case M_OP_WRITE|M_OP_END:
		if(ui == 0)
			goto bad;
		for (bp = udwtab[um->um_ctlr].b_actf; bp; bp = bp->av_forw)
			if (bp == (struct buf *)mp->mscp_cmdref)
				break;
		if (bp == NULL) {
			printf("uda%d ra%d: unknown cmdref x%x\n",
				um->um_ctlr, mp->mscp_unit, mp->mscp_cmdref);
			return;
		}
		ubarelse(um->um_ubanum, (int *)&bp->b_resid);
		/*
		 * Unlink buffer from I/O wait queue.
		 */
		bp->av_back->av_forw = bp->av_forw;
		bp->av_forw->av_back = bp->av_back;
		dp = &udutab[ui->ui_unit];
		/* do we have drive queues just to gather statistics? */
		if (ui->ui_dk >= 0)
			if (--dp->b_qsize == 0)
				dk_busy &= ~(1<<ui->ui_dk);
		if (st == M_ST_OFFLN || st == M_ST_AVLBL) {
			ui->ui_flags = DGTUNIT;	/* mark unit offline, GTUNT */
			/*
			 * Link the buffer onto the front of the drive queue
			 */
			if ((bp->av_forw = dp->b_actf) == 0)
				dp->b_actl = bp;
			dp->b_actf = bp;
			/*
			 * Link the drive onto the controller queue
			 */
			if (dp->b_active == 0) {
				dp->b_forw = NULL;
				if (um->um_tab.b_actf == NULL)
					um->um_tab.b_actf = dp;
				else
					um->um_tab.b_actl->b_forw = dp;
				um->um_tab.b_actl = dp;
				dp->b_active = 1;
			}
			return;
		}
		if ((st & M_ST_MASK) != M_ST_SUCC) {
			harderr(bp, "ra");
			printf("uda%d; status x%x\n", um->um_ctlr, st);
			bp->b_flags |= B_ERROR;
		}
		bp->b_resid = bp->b_bcount - mp->mscp_bytecnt;
		iodone(bp);
		break;

	case M_OP_GTUNT|M_OP_END:
		if(ui == 0)
			goto bad;
		up = ud_units + ui->ui_unit;
		ui->ui_flags &= ~DGTUNIT;
		up->rctsize = mp->mscp_rctsize;
		up->medium = mp->mscp_mediaid;
		up->tracksz = mp->mscp_track;
		up->groupsz = mp->mscp_group;
		up->cylsz = mp->mscp_cylinder;
		up->rbns = mp->mscp_rbns;
		up->copies = mp->mscp_rctcpys;
		break;

	case M_OP_REPLC|M_OP_END:
		if (ui == 0)
			goto bad;
		ud_rret = 0;
		if ((st & M_ST_MASK) != M_ST_SUCC) {
			printf("uda%d ra%d bad replace, status x%x\n",
				um->um_ctlr, mp->mscp_unit, st);
			ud_rret = EIO;
		}
		ud_rflags |= RPDONE;
		wakeup((caddr_t)&ud_rret);
		break;

	case M_OP_AVAIL|M_OP_END:
		break;

	default:
		printf("uda%d ra%d unknown packet type x%x\n",
			um->um_ctlr, mp->mscp_unit, mp->mscp_opcode);
	}
}


/*
 * Process an error log message
 *
 * For now, just log the error on the console.
 * Only minimal decoding is done, only "useful"
 * information is printed.  Eventually should
 * send message to an error logger.
 */

static char *events[] = {
	"ok",
	"inv cmd",
	"op aborted",
	"offline",
	"available",
	"med fmt",
	"write prot",
	"comp err",
	"data err",
	"host buf access err",
	"cntl err",
	"drive err",
};
#define	MAXEVT	0xb

uderror(um, mp)
	register struct uba_ctlr *um;
	register struct mslg *mp;
{
	register u_short *sp;		/* for sdi crap */

	printf("uda%d ra%d seq %d: %s err; fmt x%x ev x%x fl x%x\n",
		um->um_ctlr, mp->mslg_unit, mp->mslg_seqnum,
		mp->mslg_flags&(M_LF_SUCC|M_LF_CONT) ? "soft" : "hard",
		mp->mslg_format, mp->mslg_event, mp->mslg_flags);
	if ((mp->mslg_event & M_ST_MASK) <= MAXEVT)
		printf("%s; ", events[mp->mslg_event & M_ST_MASK]);
	switch (mp->mslg_format) {
	case M_FM_CNTERR:
		/* now the thing should be marked disastrously bad */
		printf("oops\n");
		break;

	case M_FM_BUSADDR:
		printf("host mem access; addr x%x\n",
			*((long *)&mp->mslg_busaddr[0]));
		break;

	case M_FM_DISKTRN:
		printf("%sbn %d; lev x%x, retry x%x\n",
			(mp->mslg_hdr & 0xf0000000) == 0 ? "l" : "r",
			mp->mslg_hdr & 0x0fffffff,
			mp->mslg_level, mp->mslg_retry);
		break;

	case M_FM_SDI:
		printf("%sbn %d;",
			(mp->mslg_hdr & 0xf0000000) == 0 ? "l" : "r",
			mp->mslg_hdr & 0x0fffffff);
		/*
		 * print the bytes in the same order used
		 * by the dec diagnostics
		 */
		sp = (u_short *)&mp->mslg_sdi[3];
		while (sp > (u_short *)mp->mslg_sdi)
			printf(" %x", *--sp);
		printf(" xx\n"); 
		break;

	case M_FM_SMLDSK:
		printf("cyl %d\n", mp->mslg_sdecyl);
		break;
	}
}

/*
 * Find an unused command packet
 * more exactly, return the next command packet if possible,
 * return NULL if we can't
 * the device insists that entries in the ring be used in sequence
 */
struct mscp *
udgetcp(um)
	struct uba_ctlr *um;
{
	register struct mscp *mp;
	register struct udaca *cp;
	register struct uda_softc *sc;
	register int i;

	cp = &uda[um->um_ctlr].uda_ca;
	sc = &uda_softc[um->um_ctlr];
	i = sc->sc_lastcmd;
	if ((cp->ca_cmddsc[i] & (UDA_OWN|UDA_INT)) == UDA_INT) {
		cp->ca_cmddsc[i] &= ~UDA_INT;
		mp = &uda[um->um_ctlr].uda_cmd[i];
		mp->mscp_unit = mp->mscp_modifier = 0;
		mp->mscp_opcode = mp->mscp_flags = 0;
		mp->mscp_bytecnt = mp->mscp_buffer = 0;
		mp->mscp_errlgfl = mp->mscp_copyspd = 0;
		sc->sc_lastcmd = (i + 1) % NCMD;
		return(mp);
	}
	return(NULL);
}

udaphys(bp)
struct buf *bp;
{
	if(bp->b_bcount > 65536)
		bp->b_bcount = 65536;
}

udread(dev)
	dev_t dev;
{
	int unit = DUNIT(minor(dev));

	if (unit >= NRA)
		u.u_error = ENXIO;
	else
		physio(udstrategy, &rudbuf[unit], dev, B_READ, udaphys);
}

udwrite(dev)
	dev_t dev;
{
	int unit = DUNIT(minor(dev));

	if (unit >= NRA)
		u.u_error = ENXIO;
	else
		physio(udstrategy, &rudbuf[unit], dev, B_WRITE, udaphys);
}

udioctl(dev, cmd, addr, flag)
dev_t dev;
caddr_t addr;
{
	register int unit;
	register union arg {
		struct ud_rctbuf r;
		struct ud_repl b;
	} *uap;
	register struct ud_unit *uu;
	register int i;

	unit = DUNIT(minor(dev));
	if (unit >= NRA) {
		u.u_error = ENXIO;
		return;
	}
	if ((uddinfo[unit]->ui_flags & (DGTUNIT | DUP)) != DUP) {
		/* should bring it online here */
		u.u_error = EIO;
		return;
	}
	uu = &ud_units[unit];
	uap = (union arg *)addr;
	switch(cmd) {
	default:
		u.u_error = ENOTTY;
		return;

	case UIOCHAR:
		if (copyout((caddr_t)uu, addr, sizeof(struct ud_unit)))
			u.u_error = EFAULT;
		return;

	case UIORRCT:
		if(uap->r.lbn < 0 || uap->r.lbn > uu->rctsize) {
			u.u_error = EIO;
			return;
		}
		/*
		 * try different copies until one works
		 */
		for (i = 0; i < uu->copies; i++) {
			u.u_count = SECTOR;	/* block size on disk */
			u.u_offset = (uap->r.lbn + uu->radsize) * (daddr_t)SECTOR;
			u.u_offset += i * uu->rctsize * (daddr_t)SECTOR;
			u.u_base = uap->r.buf;
			u.u_segflg = 0;
			u.u_error = 0;
			physio(udstrategy, &rctbuf[unit], dev, B_READ, udaphys);
			if (u.u_error == 0)
				break;
		}
		return;

	case UIOWRCT:
		if ((flag & FWRITE) == 0) {
			u.u_error = EBADF;
			return;
		}
		if(uap->r.lbn < 0 || uap->r.lbn > uu->rctsize) {
			u.u_error = EIO;
			return;
		}
		/*
		 * write every copy we can
		 * should do read-after-write crap
		 */
		for (i = 0; i < uu->copies; i++) {
			u.u_count = SECTOR;	/* block size on disk */
			u.u_offset = (uap->r.lbn + uu->radsize) * (daddr_t)SECTOR;
			u.u_offset += i * uu->rctsize * (daddr_t)SECTOR;
			u.u_base = uap->r.buf;
			u.u_segflg = 0;
			physio(udstrategy, &rctbuf[unit], dev, B_WRITE, udaphys);
			u.u_error = 0;
		}
		return;

	case UIOREPL:
		if ((flag & FWRITE) == 0) {
			u.u_error = EBADF;
			return;
		}
		udreplace(dev, uap->b.lbn, uap->b.replbn, uap->b.prim);
		return;
	}
}

udreplace(dev, badlbn, replbn, prim)
int dev;
daddr_t badlbn;
daddr_t replbn;
int prim;
{
	register struct mscp *mp;
	register struct uba_ctlr *um;
	register int unit;
	struct uda_softc *sc;

	unit = DUNIT(minor(dev));
	um = uddinfo[unit]->ui_mi;
	sc = &uda_softc[um->um_ctlr];
	spl6();
	while (ud_rflags & RPLOCK) {
		ud_rflags |= RPWANT;
		sleep((caddr_t)&ud_rflags, PZERO + 1);
	}
	ud_rflags = RPLOCK;
	printf("uda%d ra%d replace %D with %D\n",
		um->um_ctlr, unit, badlbn, replbn);
	udcredit(sc, 1);
	if ((mp = udgetcp(um)) == NULL) {
		u.u_error = EIO;
		goto out;
	}
	mp->mscp_cmdref = 42;	/* anything you like */
	mp->mscp_unit = unit;
	mp->mscp_opcode = M_OP_REPLC;
	mp->mscp_rbn = replbn;
	mp->mscp_lbn = badlbn;
	mp->mscp_modifier = prim ? M_MD_PRIMR : 0;
	udsend(mp, sc);
	while ((ud_rflags & RPDONE) == 0)
		sleep((caddr_t)&ud_rret, PZERO);
	u.u_error = ud_rret;
out:
	if (ud_rflags & RPWANT)
		wakeup((caddr_t)&ud_rflags);
	ud_rflags = 0;
	spl0();
}

udreset(uban)
	int uban;
{
	register struct uba_ctlr *um;
	register struct uba_device *ui;
	register struct buf *bp, *dp;
	register int unit;
	struct buf *nbp;
	int d;

	for (d = 0; d < NUDA; d++) {
		if ((um = udminfo[d]) == 0 || um->um_ubanum != uban ||
		    um->um_alive == 0)
			continue;
		printf(" uda%d", d);
		um->um_tab.b_active = 0;
		um->um_tab.b_actf = um->um_tab.b_actl = 0;
		uda_softc[d].sc_state = S_IDLE;
		for (unit = 0; unit < NRA; unit++) {
			if ((ui = uddinfo[unit]) == 0)
				continue;
			if (ui->ui_alive == 0 || ui->ui_mi != um)
				continue;
			udutab[unit].b_active = 0;
			udutab[unit].b_qsize = 0;
		}
		for (bp = udwtab[d].av_forw; bp != &udwtab[d]; bp = nbp) {
			nbp = bp->av_forw;
			ubarelse(uban, (int *)&bp->b_ubinfo);
			/*
			 * Link the buffer onto the drive queue
			 */
			dp = &udutab[dkunit(bp)];
			if (dp->b_actf == 0)
				dp->b_actf = bp;
			else
				dp->b_actl->av_forw = bp;
			dp->b_actl = bp;
			bp->av_forw = 0;
			/*
			 * Link the drive onto the controller queue
			 */
			if (dp->b_active == 0) {
				dp->b_forw = NULL;
				if (um->um_tab.b_actf == NULL)
					um->um_tab.b_actf = dp;
				else
					um->um_tab.b_actl->b_forw = dp;
				um->um_tab.b_actl = dp;
				dp->b_active = 1;
			}
		}
		udinit(d);
	}
}

/*
 * crash dumps
 */

#define DBSIZE 64

#define ca_Rspdsc	ca_rspdsc[0]
#define ca_Cmddsc	ca_rspdsc[1]
#define uda_Rsp		uda_rsp[0]
#define uda_Cmd		uda_cmd[0]

uddump(dev)
dev_t dev;
{
	struct udadevice *udaddr;
	struct uda *ud_ubaddr;
	char *start;
	int num, blk, unit;
	int maxsz;
	int blkoff;
	register struct uba_regs *uba;
	register struct uba_device *ui;
	register struct uda *udp;
	register struct pte *io;
	register int i;

	unit = DUNIT(minor(dev));
	if (unit >= NRA)
		return (ENXIO);
#define	phys(cast, addr) ((cast)((int)addr & 0x7fffffff))
	ui = phys(struct uba_device *, uddinfo[unit]);
	if (ui->ui_alive == 0)
		return (ENXIO);
	uba = phys(struct uba_hd *, ui->ui_hd)->uh_physuba;
	ubainit(uba);
	udaddr = (struct udadevice *)ui->ui_physaddr;
	DELAY(2000000);
	udp = phys(struct uda *, &uda[ui->ui_ctlr]);

	num = btoc(sizeof(struct uda)) + 1;
	io = &uba->uba_map[NUBMREG-num];
	for(i = 0; i<num; i++)
		*(int *)io++ = UBAMR_MRV|(btop(udp)+i);
	ud_ubaddr = (struct uda *)(((int)udp & PGOFSET)|((NUBMREG-num)<<9));

	udaddr->udaip = 0;
	while ((udaddr->udasa & UDA_STEP1) == 0)
		if(udaddr->udasa & UDA_ERR) return(EFAULT);
	udaddr->udasa = UDA_ERR;
	while ((udaddr->udasa & UDA_STEP2) == 0)
		if(udaddr->udasa & UDA_ERR) return(EFAULT);
	udaddr->udasa = (short)&ud_ubaddr->uda_ca.ca_ringbase;
	while ((udaddr->udasa & UDA_STEP3) == 0)
		if(udaddr->udasa & UDA_ERR) return(EFAULT);
	udaddr->udasa = (short)(((int)&ud_ubaddr->uda_ca.ca_ringbase) >> 16);
	while ((udaddr->udasa & UDA_STEP4) == 0)
		if(udaddr->udasa & UDA_ERR) return(EFAULT);
	udaddr->udasa = UDA_GO;
	udp->uda_ca.ca_Rspdsc = (long)&ud_ubaddr->uda_Rsp.mscp_cmdref;
	udp->uda_ca.ca_Cmddsc = (long)&ud_ubaddr->uda_Cmd.mscp_cmdref;
	udp->uda_Cmd.mscp_cntflgs = 0;
	udp->uda_Cmd.mscp_version = 0;
	if (dudcmd(M_OP_STCON, udp, udaddr) == 0)
		return(EFAULT);
	udp->uda_Cmd.mscp_unit = ui->ui_slave;
	if (dudcmd(M_OP_ONLIN, udp, udaddr) == 0)
		return(EFAULT);
	num = physmem;
	start = 0;
	maxsz = ra_sizes[PART(minor(dev))].nblocks;
	blkoff = ra_sizes[PART(minor(dev))].blkoff;
	if(maxsz < 0)
		maxsz = ud_units[unit].radsize-blkoff;
	if (dumplo < 0 || dumplo + num >= maxsz)
		return (EINVAL);
	blkoff += dumplo;
	while (num > 0) {
		blk = num > DBSIZE ? DBSIZE : num;
		io = uba->uba_map;
		for (i = 0; i < blk; i++)
			*(int *)io++ = (btop(start)+i) | UBAMR_MRV;
		*(int *)io = 0;
		udp->uda_Cmd.mscp_lbn = btop(start) + blkoff;
		udp->uda_Cmd.mscp_unit = ui->ui_slave;
		udp->uda_Cmd.mscp_bytecnt = blk*NBPG;
		udp->uda_Cmd.mscp_buffer = 0;
		if (dudcmd(M_OP_WRITE, udp, udaddr) == 0) {
			return(EIO);
		}
		start += blk*NBPG;
		num -= blk;
	}
	return (0);
}

static
dudcmd(op, udp, udaddr)
int op;
register struct uda *udp;
struct udadevice *udaddr;
{
	int i;

	udp->uda_Cmd.mscp_opcode = op;
	udp->uda_Rsp.mscp_header.uda_msglen = sizeof (struct mscp);
	udp->uda_Cmd.mscp_header.uda_msglen = sizeof (struct mscp);
	udp->uda_ca.ca_Rspdsc |= UDA_OWN|UDA_INT;
	udp->uda_ca.ca_Cmddsc |= UDA_OWN|UDA_INT;
	i = udaddr->udaip;
	for (;;) {
		if (udp->uda_ca.ca_cmdint)
			udp->uda_ca.ca_cmdint = 0;
		if (udp->uda_ca.ca_rspint)
			break;
	}
	udp->uda_ca.ca_rspint = 0;
	if (udp->uda_Rsp.mscp_opcode != (op|M_OP_END) ||
	    (udp->uda_Rsp.mscp_status&M_ST_MASK) != M_ST_SUCC) {
		if(udaddr->udasa&UDA_ERR)
			printf("error: udasa 0x%x\ndump ", udaddr->udasa);
		printf("error: com %d opc 0x%x stat 0x%x\ndump ",
			op,
			udp->uda_Rsp.mscp_opcode,
			udp->uda_Rsp.mscp_status);
		return(0);
	}
	return(1);
}

/* Jupiter driver -- both 8 bit programmed i/o and 16 bit dma i/o */

#include "ju.h"
#if NJU>0
#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/pte.h"
#include "../h/ubavar.h"
#include "../h/ubareg.h"
#include "../h/systm.h"

#define JUPRI PZERO+10
#define JULOWAT 50

struct judevice {
	unsigned short	jucsr,	/* interface card csr */
		jutc,	/* transfer count */
		juba,	/* bus address */
		juxba,	/* extended bus address */
		judcsr,	/* device csr */
		judcw,	/* device command word */
		judsw,	/* device status word */
		juvec;	/* interrupt vector */
};

#define JBUFSIZ	512
#define JUSIZ (2*JBUFSIZ)

#define JNTRIES 2000
#define MAXSAME 60
struct ju {
	unsigned char jucbuf[JUSIZ];	/* one buffer */
	unsigned char *jur, *juw;	/* two chasing pointers */
	int jucc;		/* char count to dismbiguate */
	unsigned short flag;		/* the case of jur == juw */
	unsigned int writes, lastwrite;
	unsigned short samewrite;
	int waitcount;
} jutab[NJU];

int jutimeout;
int waitpoke, timerpoke;
/* FLAG BITS */
#define OPEN	010
#define DIWAIT	020
#define DEBUG	040
#define ASLP	0100
#define ESLP	0200
#define DMA	0400
#define TIWAIT	01000

/*	I/O LOCATIONS	*/
#define	DA	0777320
#define	CSR	DA+0
#define TC	DA+02
#define BA	DA+04
#define XBA	DA+06
#define DCSR	DA+010
#define DCW	DA+012
#define DSW	DA+014
#define VEC	DA+016
/*
	DA	device base address
	CSR	csr for interface card
	TC	transfer count
	BA	bus address
	XBA	extended bus address
	DCSR	csr for device
	DCW	command word to device
	DSW	status word from device
	VEC	interrupt vector
*/

/*	BIT DEFINITIONS:	*/

/*	interface csr register	*/
#define C_GO	1
#define C_DFD	2
#define C_BY	020
#define C_DIR	040
#define C_TIE	0100
#define C_DON	0200
#define C_FIFO	01400
#define C_WDF	02000
#define C_PGE	010000
#define C_VE	020000
#define C_MTO	040000
#define C_ERR	0100000
/*
	C_GO	dma go bit
	C_DFD	data flag done
	C_BY	byte mode
	C_DIR	xfer direction (1 for read from device)
	C_TIE	enable transfer complete interrupt
	C_DON	transfer done
	C_FIFO	fifo level
	C_WDF	waiting for data flag
	C_PGE	program error
	C_VE	vector error
	C_MTO	memory timeout
	C_ERR	any error
*/

/*	device csr register	*/
#define D_RST	1
#define	D_SNS	2
#define D_DIE	0100
#define D_DIR	0200
/*
	D_RST	reset terminal
	D_SNS	sense line
	D_DIE	enable devive interrupt
	D_DIR	device interrupt request
*/

/*	device command word	*/
#define DC_SI	01000
#define DC_RST	0400
/*
	DC_SI	suppress interrupt from terminal
	DC_RST	reset terminal
*/
/*	device status word	*/
#define DS_BSY	0100000
#define DS_PBR	040000
#define DS_KSR	020000
/*
	DS_BSY	device busy
	DS_PBR	parallel byte ready
	DS_KSR	keystroke ready
*/

int juattach(), juprobe(), judgo(), judintr(), jutintr();
int justrategy(), justart();

struct uba_ctlr *jucinfo[NJU];
struct uba_ctlr juctlr[NJU];	/* juattach() sets jucinfo to point to this */
struct uba_device *judinfo[NJU];
u_short justd[] = { 0 };
struct uba_driver judriver =
	{ juprobe, 0, juattach, judgo, justd, "ju", judinfo ,"ju", jucinfo };

#define ui_open ui_type
struct buf jubuf[NJU];
struct buf judtab[NJU];
juprobe(reg)
caddr_t reg;
{
	register int br, cvec;
	register struct judevice *juaddr = (struct judevice *)reg;

	juaddr->juvec = 0500;	/* set interrupt vector xxx ? */
/*
	juaddr->judcsr &= ~D_DIR;	|* clear interrupt request bit *|
	juaddr->judcsr |= D_DIE;	|* enable interrupt, the one that *|
					|* happens when terminal is ready *|
	juaddr->judcsr |= D_DIR;	|*cause an interrupt *|
	DELAY(10000);
*/
	br = 0x14;			/* BR Level 14 = UNIBUS 4 */
	cvec = 0500;
	return(1);
}

juattach(ui)	/* initialization on system boot */
register struct uba_device *ui;
{
	register struct uba_ctlr *um;
	register int unit;

	unit = ui->ui_unit;

	um = &juctlr[unit];
	jucinfo[unit] = um;
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

static int jerror,
	entries,direntries,breturns,oreturns,
	impossible, jutotali, jubytes;
static int jwrites, jwaits, jwritepokes, jfullpokes, jreads;
juopen(dev, flag)
dev_t dev;
{
	register int s;
	register unit, d;
	int jutimer();
	register struct ju *ju;
	register struct uba_device *ui;

	d = minor(dev);
	unit = d&07;
	ju = &jutab[unit];
	ui = judinfo[unit];
	if(unit >= NJU || ui->ui_alive == 0 || (ju->flag & OPEN)) {
		u.u_error = ENXIO;
		return;	
	}
	jerror=waitpoke=timerpoke=0;
	entries=direntries=breturns=oreturns=0;
	jwrites=jwaits=jwritepokes=jfullpokes=jreads=0;
	jubytes=jutotali=impossible=0;
	ju->flag = (d&077)|OPEN;
	ju->juw = ju->jur = ju->jucbuf;
	ju->jucc = 0;

	ui->ui_open++;
	while(((struct judevice *)ui->ui_addr)->judsw&DS_BSY)
		sleep((caddr_t)&lbolt,JUPRI);
	if(jutimeout == 0) {	/* i.e. there is not a timer already going */
		jutimeout++;
		timeout(jutimer, (caddr_t)0, 60);
	}

}

jutimer()
{
register struct ju *ju;
register unit, flag, ps;
register struct judevice *juaddr;

	for (unit=flag=0; unit < NJU; unit++) {
		ju = &jutab[unit];
		if (ju->flag&OPEN==0) {
			continue;
		}
		juaddr = (struct judevice *)(judinfo[unit]->ui_addr);
		flag++;	/* unit is active */
		ps = spl4();
		if (ju->flag & TIWAIT) {
			if (ju->writes == ju->lastwrite) {
				if (++(ju->samewrite) > MAXSAME) {
				if(ju->flag & DEBUG)printf("timer TIWAIT\n");
					juaddr->jucsr &= ~C_GO;
					jutintr(unit);
				}
			} else
				ju->lastwrite = ju->writes;
		}
		if (ju->flag&DIWAIT) {
			/* judintr thinks it has work to do, */
			/* is waiting for a device interrupt */
			if ((juaddr->judsw & DS_BSY) == 0) {
				/* device is not busy! */
				timerpoke++;
				judintr(unit);
			}
		}
		splx(ps);
	}
	if(flag) {	/* keep timer going */
		timeout(jutimer, (caddr_t)0, 60);
	} else {	/* no active devices, shut down timer */
		jutimeout = 0;
	}
}
#define DEBUGON	(('J'<<8)|1)
#define DEBUGOFF (('J'<<8)|2)
#define DMAON	(('J'<<8)|3)
#define DMAOFF	(('J'<<8)|4)
#define READREGS (('J'<<8)|6)

struct juregs {
	unsigned short	jucsr,
			judcsr,
			judsw;
};

juioctl(dev, cmd, addr)
register dev_t	dev;
register struct juregs	*addr;
register cmd;
{
	int unit;
	struct ju *ju;
	struct judevice *juaddr;

	unit = minor(dev)&07;
	ju = &jutab[unit];
	juaddr = (struct judevice *)judinfo[unit]->ui_addr;

	switch(cmd){
		case DMAON:
			if(ju->jucc > 0)juwait(ju,0); /* flush pg buffer */
			ju->flag |= DMA;
			if(ju->flag & DEBUG)printf("ju: DMAON\n");
			break;
		case DMAOFF:
			ju->flag &= ~DMA;
			break;
		case READREGS:
			addr->jucsr = juaddr->jucsr;
			addr->judcsr = juaddr->judcsr;
			addr->judsw = juaddr->judsw;
			break;
		case DEBUGON:
			ju->flag |= DEBUG;
			printf("ju: DEBUGON\n");
			break;
		case DEBUGOFF:
			ju->flag &= ~DEBUG;
			break;
		default:
			u.u_error = EFAULT;
	}
}
juclose(dev)
register dev_t dev;
{
	register unit;
	register struct ju *ju;

	unit = minor(dev)&07;
	ju = &jutab[unit];

	ju->flag &= ~OPEN;	/* to shut off timer */
	juwait(ju,0);
	((struct judevice *)judinfo[unit]->ui_addr)->jucsr = 0;
	((struct judevice *)judinfo[unit]->ui_addr)->judcsr = 0;

	judinfo[unit]->ui_open = 0;
	if(ju->flag&DEBUG) {
	printf("juclose: errors %d waitpokes %d timerpokes %d\n",
		jerror,waitpoke,timerpoke);
	printf("	judintr: %d entries, of which %d had D_DIR set\n",
			entries, direntries);
	printf("		%d returns on busy, %d out bottom\n",
			breturns, oreturns);
	printf("%d writes, %d reads, %d writepokes, %d fullpokes, %d waits\n",
		jwrites,jreads,jwritepokes,jfullpokes,jwaits);
	if(jubytes)
	printf("totali %d on %d bytes sent, %d average\n",jutotali,jubytes,
		jutotali/jubytes);
	}
	ju->flag = 0;
}

juwait(ju, count)
register struct ju *ju;
register count;
{
	register int s, times;
	register r;

	jwaits++;
	times = 60;
	ju->waitcount = count;
	while(ju->jucc > count){
		ju->flag |= ASLP;
		r = tsleep((caddr_t)ju, JUPRI, 1);
		if(r == TS_TIME) {	/* timed out */
			if ((ju->flag&DIWAIT) == 0) {
				/* oddly enough judintr doesn't think it has */
				/* any work to do */
				s = spl4();
				waitpoke++;
				judintr(ju-jutab);
				splx(s);
			}
			times--;
			if(times==0) {
				printf("jupiter timeout, buffer flushed\n");
				goto flush;
			}
		}
		if (r==TS_SIG) {
		flush:
			ju->jur = ju->juw = ju->jucbuf;
			ju->jucc = 0;
		}
	}
}

unsigned short jcheck(addr,bitmask,value)
register unsigned short *addr;
register unsigned int bitmask, value;
{
	register int times;
	unsigned short data;

	times = 0;
	while (((data = *addr) & bitmask) != value)
		if (times++ >= JNTRIES) {
			times = 0;
			if (tsleep((caddr_t)addr, JUPRI, 1) == TS_SIG)
				return 0;
		}

	return data;
}

juread(dev)
{
	register struct judevice *addr;
	register struct ju *ju;
	register int unit, count;
	register unsigned char *bptr;

	unit = dev & 07;
	ju = &jutab[unit];
	addr = (struct judevice *)judinfo[unit]->ui_addr;
	jreads++;

	if (ju->jucc)
		juwait(ju, 0);
	if(ju->flag&DMA) {
		if(ju->flag&DEBUG)
			printf("attempting dma read to ju\n");
		ju->flag |= TIWAIT;
		physio(justrategy,&jubuf[unit],dev,B_READ,minphys);
		return;
	}
	count = u.u_count;
	bptr = ju->jucbuf;

	addr->judcsr &= ~D_DIR;
	if(ju->flag&DEBUG)printf("DSW address %o\n",&(addr->judsw));
	while (count--) {
		*bptr++ = (unsigned char)(jcheck(&(addr->judsw),DS_PBR, DS_PBR)
						&0xFF) ;
		if(ju->flag&DEBUG)printf("DSW is %x\n",(unsigned int)addr->judsw);
		jcheck(&(addr->judsw), DS_PBR, 0);
			/* send PBA command */
		jcheck(&(addr->judsw), DS_BSY, 0);
		addr->judcw = 033;
		if(ju->flag & DEBUG)printf("end of read loop, data %x\n",
					(int)(*(bptr-1)));
	}

	iomove(ju->jucbuf, u.u_count, B_READ);
}

juwrite(dev)
{
	register struct ju *ju;
	register unsigned char *p;
	register c, cc;
	register s;
	register unsigned char *jur;
	register int unit;

	unit = minor(dev) & 07;
	ju = &jutab[unit];
	ju->writes++;
	jwrites++;

	if(ju->flag&DMA) {
		if(ju->flag&DEBUG)
			printf("attempting dma write to ju\n");
		ju->flag |= TIWAIT;
		physio(justrategy,&jubuf[unit],dev,B_WRITE,minphys);
	}
	else {
	while(u.u_count) {
		jur = ju->jur;
		if(jur > ju->juw)
			cc = jur - ju->juw;
		else if (jur == ju->juw && ju->jucc != 0)
			cc = 0;	/* buffer full */
		else		/* from juw to end of buffer */
			cc = ju->jucbuf + JUSIZ - ju->juw;
		if(u.u_count < cc)
			cc = u.u_count;
		if(cc == 0) {	/* the buffer must be full */
			s = spl4(); /* correct priority?xxx */
			if ((ju->flag&DIWAIT) == 0) {
				jfullpokes++;
				judintr(unit);
			}
			splx(s);
			juwait(ju,JULOWAT);
		}
		else {	/* queue up some characters */
			iomove(ju->juw, cc, B_WRITE);
			if((ju->juw += cc) >= ju->jucbuf + JUSIZ)
				ju->juw = ju->jucbuf;
			s = spl4();
			ju->jucc += cc;
			if (( ju->flag&DIWAIT) == 0) {
				jwritepokes++;
				judintr(unit);
			}
			splx(s);
		}
	}
	}
}

/* Since justrategy is currently called only by juwrite
	via physio, there will be only one transaction in each
	DMA-U's queue at a given time. So we tack the given buffer
	header pointer on at the end of the queue, and call justart
*/
justrategy(bp)
register struct buf *bp;
{
	register struct uba_device *ui;
	register struct uba_ctlr *um;
	register struct buf *dp;
	register judvma;
	dev_t unit;
	int ps;

	ps = spl4();
	unit = minor(bp->b_dev);
	ui = judinfo[unit];
	um = ui->ui_mi;		/* ctlr pointer */
	dp = &judtab[unit];
	dp->b_actf = bp;
	dp->b_actl = bp;
	bp->av_forw = NULL;
	um->um_tab.b_actf = dp;
	um->um_tab.b_actl = dp;
	um->um_cmd = C_TIE | C_GO;
	if (bp->b_flags & B_READ)
		um->um_cmd |= C_DIR;

	justart(um);
	splx(ps);
}

justart(um)
register struct uba_ctlr *um;
{
	register struct buf *bp, *dp;
	register struct judevice *juaddr;

	dp = um->um_tab.b_actf;
	bp = dp->b_actf;
	um->um_tab.b_active++;
	juaddr = (struct judevice *)um->um_addr;
	jcheck(&(juaddr->judsw), DS_BSY, 0);
	juaddr->jutc = -bp->b_bcount / sizeof(short);
	juaddr->judcsr = juaddr->jucsr = 0;
	(void) ubago(judinfo[minor(bp->b_dev)]);
}

judgo(um)
register struct uba_ctlr *um;
{
	register struct judevice *juaddr = (struct judevice *)um->um_addr;

	juaddr->juba = um->um_ubinfo;
	juaddr->juxba = (um->um_ubinfo>>16) & 3;
	juaddr->jucsr = um->um_cmd;
}
jutintr(dev)
register dev_t	dev;
{
	register struct buf *bp, *dp;
	register struct judevice *juaddr;
	register struct uba_ctlr *um;
	register unit;
	register struct ju *ju;

	unit = minor(dev) & 07;
	um = jucinfo[unit];

	ju = &jutab[unit];
	ju->flag &= ~TIWAIT;
	ju->samewrite = 0;
	juaddr = (struct judevice *)um->um_addr;
	juaddr->jucsr = 0;
	if(um->um_tab.b_active == 0)
			return;

	dp = um->um_tab.b_actf;
	bp = dp->b_actf;

	/* should check for errors? */

	um->um_tab.b_active = 0;
	um->um_tab.b_errcnt = 0;
	um->um_tab.b_actf = dp->b_forw;
	dp->b_errcnt = 0;
	dp->b_active = 0;
	bp->b_resid = (-juaddr->jutc * sizeof(short));
	ubadone(um);
	iodone(bp);
}

static int jcount;
judintr(dev)
register dev_t	dev;
{
	register struct ju *ju;
	register struct judevice *addr;
	register c, cc, unit;

	unit = minor(dev) & 07;
	ju = &jutab[unit];
	ju->flag &= ~DIWAIT;
	addr = (struct judevice *)judinfo[unit]->ui_addr;

	entries++; if(addr->judcsr&D_DIR)direntries++;

	if (addr->jucsr&C_ERR) {
		printf("error: ju %d %x\n",dev,(int)addr->jucsr);
		addr->jucsr &= ~C_ERR;
		ju->flag &= ~DIWAIT;
		ju->flag |= ESLP;
		jerror++;
		return;
	}

	cc = 0;
	while (ju->jucc > 0) {
		addr->judcsr &= ~D_DIR;	/* clear interrupt request bit */
		for(jcount=0;jcount<JNTRIES;jcount++)
			if( (addr->judsw&DS_BSY) == 0 )goto putout;
			/* busy too long -- sleep */
			addr->judcsr |= D_DIE;
			ju->flag |= DIWAIT;
			if(ju->flag&ASLP && ju->jucc <= ju->waitcount){
				ju->flag &= ~ASLP;
				wakeup((caddr_t)ju);
			}
			breturns++;
			return;
	putout:		/* not busy: output a byte */
		addr->judcw = *(ju->jur)++;
		if (ju->jur == ju->jucbuf + JUSIZ)
			ju->jur = ju->jucbuf;
		ju->jucc--;
		cc++;
		jutotali += jcount;
		jubytes++;
	}


	out:	if(cc && ju->flag&ASLP && ju->jucc <= ju->waitcount) {
			ju->flag &= ~ASLP;
			wakeup ((caddr_t)ju);
		}
		oreturns++;
}
#endif

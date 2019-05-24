
#include <nlist.h>
#include "/usr/sys/h/param.h"
#include <ctype.h>
#include "/usr/sys/h/stream.h"
#include "/usr/sys/h/inode.h"
#include "/usr/sys/h/conf.h"

struct nlist nl[] ={
#define	QNL	0
	{"_queue"},
#define	BLNL	1
	{"_cblock"},
#define	STNL	2
	{"_streams"},
#define	QFNL	3
	{"_qfreelist"},
#define	DTNL	4
	{"_dtlinfo"},
#define DKPNL	5
	{ "_dkpinfo" },
#define	NBLOCK	6
	{ "_Nblock"},
#define	NQUEUE	7
	{ "_Nqueue"},
#define	NSTREAM	8
	{ "_Nstream"},
#define	C	9
	{"_cblockC"},
#define	M	10
	{"_cblockM"},
#define	N4	11
	{"_Nblk4"},
#define	N16	12
	{"_Nblk16"},
#define	N64	13
	{"_Nblk64"},
#define	NBIG	14
	{"_Nblkbig"},
#define	DZNL	15
	{"_dzinfo"},
#define CDKPNL	16
	{ "_cdkpinfo" },
	{ "" },
};

struct	qinit **qall;
char	*ball;
char	*core = "/dev/mem";
char	*sys  = "/unix";
int	mem;
int	Nblock;
int	Nqueue;
int	Nstream;

struct	dt {
	char	nack;
	char	cack;
	char	nchar;
	char	state;
	short	lastecho;
	struct	queue *queue;
};


struct dkp {
	struct	queue	*rdq;	/* associated read queue */
	struct	block	*inp;	/* msg being collected */
	struct	block	*inpe;	/*  end of msg */
	short	state;		/* flags */
	short	trx;		/* # bytes in trailer being collected */
	short	indata;		/* # bytes in message being collected */
	u_char	iseq;		/* last good input sequence number */
	u_char	lastecho;	/* last echo/rej sent */
	u_char	WS;		/* first non-accepted message */
	u_char	WACK;		/* first non-received message */
	u_char	WNX;		/* next message to be sent */
	u_char	XW;		/* size of xmit window */
	u_char	timer;		/* timeout for xmit */
	u_char	outcnt;		/* count output chars cor char mode */
	u_char	trbuf[3];	/* trailer being collected */
	struct	block *xb[8];	/* the xmit window buffer */
};
#define	DK_OPEN	01
#define	DK_LCLOSE	02
#define	DK_RCLOSE	04
#define	DK_XCHARMODE	010
#define	DK_RCHARMODE	0200
#define	DK_OPENING	020

struct dz {
	short	state;
	short	flags;
	struct	block	*oblock;
	struct	queue	*rdq;
	char	board;
	char	line;
	char	speed;
};

#define	getme(s, kind, off, type)  _get(kind, off, sizeof(type), &s)

struct flags {
	int	flag;
	char	*name;
};

struct flags stflags[] = {
	IOCWAIT,	"IOC",
	RSLEEP,		"RSLEEP",
	WSLEEP,		"WSLEEP",
	HUNGUP,		"HUNGUP",
	RSEL,		"RSEL",
	WSEL,		"WSEL",
	EXCL,		"EXCL",
	STWOPEN,	"WOPEN",
	0
};

struct flags qflags[] = {
	QUSE,		"use",
	QREADR,		"read",
	QNOENB,		"noenb",
	QENAB,		"enab",
	QWANTR,		"wantr",
	QWANTW,		"wantw",
	QFULL,		"full",
	QDELIM,		"delim",
	QBIGB,		"bigb",
	0
};

#define	CLOG1	01
#define	CLOG2	02
#define	STOPDT	04
#define	USEDT	010
struct flags dtflags[] = {
	CLOG1,	"clog1",
	CLOG2,	"clog2",
	STOPDT,	"stop",
	USEDT,	"use",
	0
};

struct	flags dkpflags[] = {
	DK_OPEN,	"open",
	DK_LCLOSE,	"lclose",
	DK_RCLOSE,	"rclose",
	DK_XCHARMODE,	"xcharmode",
	DK_RCHARMODE,	"rcharmode",
	DK_OPENING,	"opening",
	0
};

#define	DZEXIST	01
#define	DZISOPEN 02
#define	DZWOPEN	04
#define	DZTIME	010
#define	DZCARR	020
#define	DZSTOP	040
#define	DZHUPCL	0100

struct flags dzflags[] = {
	DZEXIST,	"exists",
	DZISOPEN,	"open",
	DZWOPEN,	"wopen",
	DZTIME,		"time",
	DZCARR,		"carrier",
	DZSTOP,		"stop",
	DZHUPCL,	"hupcl",
	0
};

int	vflag;
int	sflag;
int	mflag;

main(argc, argv)
char **argv;
{
	struct queue q;
	struct block b;
	struct stdata s;
	int i;
	int nq = 0;
	int blkcur[4],blkmin[4];
	int b4, b16, b64, bBIG;

	for (;;argc--, argv++) {
		if (argc>1 && strcmp(argv[1], "-v")==0) {
			vflag++;
			continue;
		}
		if (argc>1 && strcmp(argv[1], "-m")==0) {
			mflag++;
			continue;
		}
		if (argc>1 && strcmp(argv[1], "-V")==0) {
			vflag = 2;
			continue;
		}
		if (argc>1 && strcmp(argv[1], "-s")==0) {
			sflag++;
			continue;
		}
		break;
	}
	if (argc>1)
		sys = argv[1];
	if (argc>2)
		core = argv[2];
	nlist(sys, nl);
	mem = open(core, 0);
	if (mem<0) {
		printf("can't open %s\n", core);
		exit(1);
	}
	getme(blkcur[0], C, 0, blkcur);
	getme(blkmin[0], M, 0, blkmin);
	getme(b4, N4, 0, b4);
	getme(b16, N16, 0, b16);
	getme(b64, N64, 0, b64);
	getme(bBIG, NBIG, 0, int);
	getme(Nqueue, NQUEUE, 0, int);
	getme(Nstream, NSTREAM, 0, int);
	getme(Nblock, NBLOCK, 0, int);
	printf("blocks max: %.0f%% (%d,%d,%d,%d)\n",
		100.*(Nblock-blkmin[0]-blkmin[1]-blkmin[2])/Nblock,
		b4-blkmin[0], b16-blkmin[1], b64-blkmin[2], bBIG-blkmin[3]);
	ball = (char *)malloc(Nblock*sizeof(char));
	qall = (struct qinit **)malloc(Nqueue*sizeof(struct queue *));
	for (i=0; i<Nblock; i++)
		ball[i] = 0;
	for (i=0; i<Nqueue; i++) {
		getme(q, QNL, i, struct queue);
		if (q.flag==0)
			qall[i] = 0;
		else {
			qall[i] = q.qinfo;
			nq++;
		}
	}
	for (i=0; i<Nstream; i++) {
		pstream(i);
	}
	for (i=0; i<Nqueue; i++) {
		if (qall[i])
			printf("Loose queue %d, info %x\n", i, qall[i]);
	}
	printf("%d queues in use\n", nq);
	bfreecount();
	nq = 0;
	for (i=0; i<Nblock; i++)
		if (ball[i]==0) {
			struct block b;
			if (mflag) {
				getme(b, BLNL, i, struct block);
				pblock(&b);
			}
			nq++;
		}
	if (!sflag)
		printf("%d blocks missing\n", nq);
	nq = 0;
	for (i=0; i<Nblock; i++) {
		if (ball[i]==0 && nq < 5) {
			printf("%x\n", (struct block *)nl[BLNL].n_value+i);
			nq++;
		}
	}
}

pstream(i)
{
	struct stdata s;
	struct queue rq, wq;
	struct inode ino;
	struct queue *rqp, *wqp;
	struct stdata *sp;
	struct streamtab st, st1, st2, st3;
	int d;
	int forward = 1;

	sp = (struct stdata *)getme(s, STNL, i, struct stdata);
	if (s.wrq==NULL)
		return;
	if (!sflag)
		printf("\nstream %d: ", i);
	getme(ino, s.inode, 0, struct inode);
	d = ino.i_un.i_rdev;
	if (!sflag) {
		printf("dev %d,%d  ", major(d), minor(d));
		printf("count %d ", s.count);
		flags(stflags, s.flag);
		if (vflag)
			printf(" pgrp %d", s.pgrp);
		printf("\n");
	}
	wqp = (struct queue *)getme(wq, s.wrq, 0, struct queue);
	rqp = (struct queue *)getme(rq, RD(s.wrq), 0, struct queue);
	if ((struct stdata *)rq.ptr != sp)
		printf("rq ptr? = %x\n", rq.ptr);
	getme(st, DTNL, 0, struct streamtab);
	getme(st1, DKPNL, 0, struct streamtab);
	getme(st3, CDKPNL, 0, struct streamtab);
	getme(st2, DZNL, 0, struct streamtab);
	for (;;) {
		d = rqp - (struct queue *)nl[QNL].n_value;
		if (d<0 || d>=Nqueue)
			printf("?");
		else
			qall[d] = 0;
		d = wqp - (struct queue *)nl[QNL].n_value;
		if (d<0 || d>=Nqueue)
			printf("?");
		else
			qall[d] = 0;
		if (!sflag) {
			printf("   RQ %x: ", rqp);
			flags(qflags, rq.flag);
			if (vflag)
				printf(" ptr %x, inf %x", rq.ptr, rq.qinfo);
			printf("\n");
			pblocks(&rq);
			printf("   WQ %x: ", wqp);
			flags(qflags, wq.flag);
			if (vflag)
				printf(" ptr %x, inf %x", wq.ptr, wq.qinfo);
			printf("\n");
			pblocks(&wq);
			if (wq.qinfo == st.wrinit)
				pdtline(&wq);
			if (wq.qinfo == st1.wrinit || wq.qinfo == st3.wrinit)
				dkpline(&wq);
			if (wq.qinfo == st2.wrinit)
				dzfacts(&wq);
		}
		if (forward) {
			if (wq.next == NULL)
				break;
			wqp = (struct queue *)
			     getme(wq, wq.next, 0, struct queue);
			if (wq.flag&QREADR) {
				printf("   <>\n");
				forward = 0;
				rqp = wqp;
				rq = wq;
				wqp = (struct queue *)
				     getme(wq, WR(rqp), 0, struct queue);
			} else
				rqp = (struct queue *)
				     getme(rq, RD(wqp), 0, struct queue);
		} else {
			if (rq.next == NULL)
				break;
			rqp = (struct queue *)
			     getme(rq, rq.next, 0, struct queue);
			if ((rq.flag&QREADR) == 0) {
				printf("   <>\n");
				forward = 1;
				wqp = rqp;
				wq = rq;
				rqp = (struct queue *)
				     getme(rq, RD(wqp), 0, struct queue);
			} else
				wqp = (struct queue *)
				     getme(wq, WR(rqp), 0, struct queue);
		}
	}
}

pdtline(q)
register struct queue *q;
{
	struct dt dt;

	getme(dt, q->ptr, 0, struct dt);
	printf("	nack %o cack %o nchar %d lastecho %o q %lx  ", dt.nack,dt.cack,
	  dt.nchar, dt.lastecho, dt.queue);
	flags(dtflags, dt.state);
	printf("\n");
}

dzfacts(q)
register struct queue *q;
{
	struct dz dz;

	getme(dz, q->ptr, 0, struct dz);
	printf("     DZ: ");
	flags(dzflags, dz.state);
	printf("\n");
}

dkpline(q)
register struct queue *q;
{
	struct dkp dkp;
	register i;

	getme(dkp, q->ptr, 0, struct dkp);
	printf("	WS %o WACK %o WNX %o iseq %o; ", dkp.WS, dkp.WACK,
	    dkp.WNX, dkp.iseq);
	flags(dkpflags, dkp.state);
	printf("\n	");
	for (i=0; i<8; i++)
		printf(" %x", dkp.xb[i]);
	printf("\n");
}

pblocks(q)
register struct queue *q;
{
	struct block b, *bp;
	int nb = 0;
	u_char buf[512];
	register i, bno;

	bp = q->first;
	for (i = 0; i < 1000; i++) {
		if (bp==NULL)
			break;
		bno = bp - (struct block *)nl[BLNL].n_value;
		nb++;
		baccount(bno);
		getme(b, bp, 0, struct block);
		if (vflag && (i<10 || vflag>1))
			pblock(&b);
		bp = b.next;
	}
	if (i>=10)
		printf("(gave up) ");
	if (nb > 0)
		printf("       %d blocks\n", nb);
}

pblock(bp)
register struct block *bp;
{
	register u_char *cp;
	register i = 0;
	u_char buf[1024];

	printf("	%.3o:", bp->type);
	cp = buf;
	getme(buf[0], bp->rptr, 0, buf);
	while (bp->rptr < bp->wptr) {
		if (i >= 16) {
			printf("\n	    ");
			i = 0;
		}
		if (*cp<040 || *cp>=0177)
			printf(" %.3o ", *cp);
		else
			printf("%c", *cp);
		cp++;
		bp->rptr++;
		i++;
	}
	printf("\n");
}

flags(fp, w)
register struct flags *fp;
{
	int any = 0;
	while (fp->name) {
		if (fp->flag&w) {
			if (any)
				printf(", ");
			any++;
			printf("%s", fp->name);
		}
		fp++;
	}
}

_get(addr, off, size, loc)
unsigned long addr;
char *loc;
{

	if (addr < 20)
		addr = nl[addr].n_value;
	addr += off*size;
	lseek(mem, addr&0x7fffffff, 0);
	read(mem, loc, size);
	return(addr);
}

bfreecount()
{
	struct block b;
	struct block *bp;
	register i;

	for (i=0; i<3; i++) {
		getme(bp, QFNL, i, struct block *);
		while (bp) {
			baccount(bp - (struct block *)nl[BLNL].n_value);
			getme(b, bp, 0, struct block);
			bp = b.next;
		}
	}
}

baccount(bno)
{
	if (bno < 0 || bno >= Nblock)
		printf("funny block %d\n", bno);
	else if (ball[bno])
		printf("dup block %d\n", bno);
	else
		ball[bno]++;
}

/*	conf.c	4.32+	82/12/05	*/

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/text.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/file.h"
#include "../h/inode.h"
#include "../h/acct.h"
#include "../h/pte.h"
#include "sparam.h"

int	nulldev();
int	nodev();

#include "hp.h"
#if NHP > 0
int	hpstrategy(),hpread(),hpwrite(),hpioctl(),hpintr(),hpdump();
#else
#define	hpstrategy	nodev
#define	hpread		nodev
#define	hpwrite		nodev
#define	hpioctl		nodev
#define	hpintr		nodev
#define	hpdump		nodev
#endif
 
#include "tu.h"
#if NHT > 0
int	htopen(),htclose(),htstrategy(),htread(),htwrite(),htdump(),htioctl();
#else
#define	htopen		nodev
#define	htclose		nodev
#define	htstrategy	nodev
#define	htread		nodev
#define	htwrite		nodev
#define	htdump		nodev
#define	htioctl		nodev
#endif

#include "mu.h"
#if NMT > 0
int	mtopen(),mtclose(),mtstrategy(),mtread(),mtwrite();
int	mtioctl(),mtdump();
#else
#define	mtopen		nodev
#define	mtclose		nodev
#define	mtstrategy	nodev
#define	mtread		nodev
#define	mtwrite		nodev
#define	mtioctl		nodev
#define	mtdump		nodev
#endif

#include "rk.h"
#if NHK > 0
int	rkstrategy(),rkread(),rkwrite(),rkintr(),rkdump(),rkreset();
#else
#define	rkstrategy	nodev
#define	rkread		nodev
#define	rkwrite		nodev
#define	rkintr		nodev
#define	rkdump		nodev
#define	rkreset		nodev
#endif

#include "te.h"
#if NTE > 0
int	tmopen(),tmclose(),tmstrategy(),tmread(),tmwrite();
int	tmioctl(),tmdump(),tmreset();
#else
#define	tmopen		nodev
#define	tmclose		nodev
#define	tmstrategy	nodev
#define	tmread		nodev
#define	tmwrite		nodev
#define	tmioctl		nodev
#define	tmdump		nodev
#define	tmreset		nodev
#endif

#include "ts.h"
#if NTS > 0
int	tsopen(),tsclose(),tsstrategy(),tsread(),tswrite();
int	tsioctl(),tsdump(),tsreset();
#else
#define	tsopen		nodev
#define	tsclose		nodev
#define	tsstrategy	nodev
#define	tsread		nodev
#define	tswrite		nodev
#define	tsioctl		nodev
#define	tsdump		nodev
#define	tsreset		nodev
#endif

#include "up.h"
#if NSC > 0
int	upstrategy(),upread(),upwrite(),upreset(),updump();
#else
#define	upstrategy	nodev
#define	upread		nodev
#define	upwrite		nodev
#define	upreset		nulldev
#define	updump		nodev
#endif

#include "ra.h"
#if NUDA > 0
int	udstrategy(), udread(), udwrite(), udioctl(), udreset(), uddump();
#else
#define udstrategy	nodev
#define udread		nodev
#define udwrite		nodev
#define udioctl		nodev
#define udreset		nulldev
#define uddump		nodev
#endif

int	swstrategy(),swread(),swwrite();

struct bdevsw	bdevsw[] =
{
	nulldev,	nulldev,	hpstrategy,	hpdump,	0,	/*0*/
	htopen,		htclose,	htstrategy,	htdump,	B_TAPE,	/*1*/
	nulldev,	nulldev,	upstrategy,	updump,	0,	/*2*/
	nulldev,	nulldev,	rkstrategy,	rkdump,	0,	/*3*/
	nodev,		nodev,		swstrategy,	nodev,	0,	/*4*/
	tmopen,		tmclose,	tmstrategy,	tmdump,	B_TAPE,	/*5*/
	tsopen,		tsclose,	tsstrategy,	tsdump,	B_TAPE,	/*6*/
	nulldev,	nulldev,	udstrategy,	uddump,	0,	/*7*/
	mtopen,		mtclose,	mtstrategy,	mtdump,	B_TAPE,	/*8*/
	0,
};

extern	struct streamtab cninfo;

#if VAX780
int	flopen(),flclose(),flread(),flwrite();
#endif

#include "ctu.h"
#if NCTU > 0
extern	ctuopen(), ctuclose(), cturead(), ctuwrite();
#else
#define	ctuopen	nodev
#define	ctuclose	nodev
#define	cturead	nodev
#define	ctuwrite	nodev
cturint() {printf("cons mass storage stray\n");}
ctuxint() {printf("cons mass storage stray\n");}
#endif

#include "dk.h"
#if NDK > 0
extern	struct streamtab dkinfo;
int	dkreset();
#else
#define dkinfo	NULL
#define	dkreset	nulldev
#endif

#include "dz.h"
#if NDZ > 0
extern	struct streamtab dzinfo;
#else
#define	dzinfo	NULL
#endif

#include "inet.h"
#if NINET > 0
extern		struct streamtab ipdinfo;
extern		struct streamtab tcpdinfo;
#else
#define ipdinfo *(struct streamtab *)NULL
#define tcpdinfo *(struct streamtab *)NULL
#endif

#include "il.h"
#if NIL > 0
extern struct streamtab ilsinfo;
#else
#define ilsinfo		*(struct streamtab *)NULL
#endif

#include "lp.h"
#if NLP > 0
int	lpopen(),lpclose(),lpwrite(),lpreset();
#else
#define	lpopen		nodev
#define	lpclose		nodev
#define	lpwrite		nodev
#define	lpreset		nulldev
#endif

int 	mmread(),mmwrite();

#include "va.h"
#if NVA > 0
int	vaopen(),vaclose(),vawrite(),vaioctl(),vareset();
#else
#define	vaopen	nodev
#define	vaclose	nodev
#define	vawrite	nodev
#define	vaopen	nodev
#define	vaioctl	nodev
#define	vareset	nulldev
#endif

#include "vp.h"
#if NVP > 0
int	vpopen(),vpclose(),vpwrite(),vpioctl(),vpreset();
#else
#define	vpopen	nodev
#define	vpclose	nodev
#define	vpwrite	nodev
#define	vpioctl	nodev
#define	vpreset	nulldev
#endif

#include "sp.h"
#if NSP>0
struct	streamtab spinfo;
#else
#define	spinfo NULL
#endif

#include "dn.h"
#if NDN > 0
int	dnopen(), dnclose(), dnwrite();
#else
#define	dnopen	nodev
#define	dnclose	nodev
#define	dnwrite	nodev
#endif

#include "tri.h"
#if NTRI > 0
int	triopen(), triclose(), triwrite();
#else
#define	triopen	nodev
#define	triclose nodev
#define triwrite nodev
#endif

#include "rtk.h"
#if NRTK > 0
int	rtkopen(), rtkclose(), rtkread(), rtkwrite(), rtkioctl();
#else
#define	rtkopen		nodev
#define	rtkclose	nodev
#define	rtkread		nodev
#define	rtkwrite	nodev
#define	rtkioctl	nodev
#endif

#include "ekx.h"
#if NEKX > 0
int	ekxopen(), ekxclose(), ekxread(), ekxwrite(), ekxioctl();
#else
#define ekxopen		nodev
#define ekxclose	nodev
#define ekxread		nodev
#define ekxwrite	nodev
#define ekxioctl	nodev
#endif

#include "scn.h"
#if NSCN > 0
int	scnopen(), scnclose(), scnread(), scnwrite(), scnioctl();
#else
#define scnopen	nodev
#define scnclose	nodev
#define scnread	nodev
#define scnwrite	nodev
#define scnioctl	nodev
#endif

#include "cv.h"
#if NCV > 0
int	cvopen(), cvclose(), cvread(), cvwrite(), cvioctl(), cvreset();
#else
#define cvopen		nodev
#define cvclose		nodev
#define cvread		nodev
#define cvwrite		nodev
#define cvioctl		nodev
#define	cvreset		nodev
#endif

#include "iti.h"
#if NITI > 0
int	itiopen(), iticlose() , itiioctl();
#else
#define itiopen		nodev
#define iticlose	nodev
#define itiioctl	nodev
#endif

#include "sn.h"
#if NSN > 0
int	snopen(), snclose(), snioctl(), snreset();
#else
#define snopen		nodev
#define snclose		nodev
#define snread		nodev
#define snwrite		nodev
#define snioctl		nodev
#define	snreset		nodev
#endif

#include "an.h"
#if NAP > 0
int	anopen(), anclose(), anread(), anwrite(), anreset(), anioctl();
#else
#define anopen		nodev
#define anclose		nodev
#define anread		nodev
#define anwrite		nodev
#define	anreset		nodev
#define	anioctl		nodev
#endif

#include "kmc.h"
#if NKMC > 0
int	kmcopen(), kmcclose(), kmcread(), kmcwrite(), kmcioctl();
#else
#define	kmcopen		nodev
#define	kmcclose	nodev
#define	kmcread		nodev
#define	kmcwrite	nodev
#define	kmcioctl	nodev
#endif

#include "ec.h"
#if NEC > 0
extern struct streamtab ecsinfo;
#else
#define ecsinfo		*(struct streamtab *)NULL
#endif

#include "kdi.h"
#if NKDI == 0
#define	kdi	NULL
#define	kdishutdown nodev
#else
extern	struct streamtab kdiinfo;
int	kdishutdown();
#define	kdi	&kdiinfo
#endif

#include "lex.h"
#if NLEX == 0
#define	lexopen		nodev
#define	lexclose	nodev
#define	lexread		nodev
#define	lexwrite	nodev
#define	lexioctl	nodev
#else
int	lexopen(), lexclose(), lexread(), lexwrite(), lexioctl();
#endif

#include "bpd.h"
#if NBPD == 0
#define	bpdopen		nodev
#define	bpdclose	nodev
#define	bpdread		nodev
#define	bpdioctl	nodev
#else
int	bpdopen(), bpdclose(), bpdread(), bpdioctl();
#endif

/* merganthaler 202 */
#include "mg.h"
#if	NMG>0
extern	struct streamtab mginfo;
#define	mg	&mginfo
#else
#define	mg	NULL
#endif

#include "ju.h"
#if NJU == 0
#define juopen nodev
#define juclose nodev
#define juread nodev
#define juwrite nodev
#define juioctl nodev
#else
int	juopen(), juclose(), juread(), juwrite(), juioctl();
#endif

#include "om.h"
#if NOM > 0
int	omopen(),omclose(),omread(),omwrite(),omioctl();
#else
#define	omopen	nodev
#define	omclose	nodev
#define	omread	nodev
#define	omwrite	nodev
#define	omioctl	nodev
#endif

#include "chaos.h"
#if NCHAOS > 0

#define	CHDEV_OFFSET	32
long cdevpath = 1L << (34 - CHDEV_OFFSET);
int	chropen(),chrclose(),chrread(),chrwrite(),chrioctl(),chreset();

#else NCHAOS == 0

#define chropen nodev
#define chrclose nodev
#define chrread nodev
#define chrwrite nodev
#define chrioctl nodev
#define chreset nodev

#endif NCHAOS

#include "cht.h"
#if NCHT > 0

#if NCHAOS == 0
***this won't work**** you must have CHAOS to have CHT
#endif NCHAOS

int	chtopen(),chtclose(),chtread(),chtwrite(),chtioctl();
struct tty cht_tty[];

#else	NCHT == 0

#define chtopen nodev
#define chtclose nodev
#define chtread nodev
#define chtwrite nodev
#define chtioctl nodev
#define cht_tty 0

#endif NCHT

#include "chil.h"
#define chilopen	nodev
#define chilclose	nodev
#define chilread	nodev
#define	chilwrite	nodev
#define chilioctl	nodev
#define chilreset	nulldev

#include "ds.h"
#if NDS > 0
int	dsopen(),dsclose(),dsread(),dswrite(),dsioctl();
#else
#define	dsopen	nodev
#define	dsclose	nodev
#define	dsread	nodev
#define	dswrite	nodev
#define	dsioctl	nodev
#endif

#include "ch.h"
#if NCH > 0
extern struct streamtab	chdinfo;
#else
#define chdinfo *(struct streamtab *)NULL
#endif

struct cdevsw	cdevsw[] =
{
/*cn*/	nodev,		nodev,		nodev,		nodev,		/*0*/
	nodev,		nulldev,	&cninfo,
/*dz*/	nodev,		nodev,		nodev,		nodev,		/*1*/
	nodev,		nulldev,	&dzinfo,
/*ctu*/	ctuopen,	ctuclose,	cturead,	ctuwrite,	/*2*/
	nodev,		nodev,		NULL,
/*mem*/	nulldev,	nulldev,	mmread,		mmwrite,	/*3*/
	nodev,		nulldev,	NULL,
/*hp*/	nulldev,	nulldev,	hpread,		hpwrite,	/*4*/
	hpioctl,	nulldev,	NULL,
/*ht*/	htopen,		htclose,	htread,		htwrite,	/*5*/
	htioctl,	nulldev,	NULL,
/*vp*/	vpopen,		vpclose,	nodev,		vpwrite,	/*6*/
	vpioctl,	vpreset,	NULL,
/*sw*/	nulldev,	nulldev,	swread,		swwrite,	/*7*/
	nodev,		nulldev,	NULL,
#if VAX780
/*fl*/	flopen,		flclose,	flread,		flwrite,	/*8*/
	nodev,		nulldev,	NULL,
#else
/*fl*/	nodev,		nodev,		nodev,		nodev,		/*8*/
	nodev,		nodev,		NULL,
#endif
/*xx*/	nodev,		nodev,		nodev,		nodev,		/*9mx*/
	nodev,		nulldev,	NULL,
/*va*/	vaopen,		vaclose,	nodev,		vawrite,	/*10*/
	vaioctl,	vareset,	NULL,
/*rk*/	nulldev,	nulldev,	rkread,		rkwrite,	/*11*/
	nodev,		rkreset,	NULL,
/*xx*/	nodev,		nodev,		nodev,		nodev,		/*12dh*/
	nodev,		nodev,		NULL,
/*up*/	nulldev,	nulldev,	upread,		upwrite,	/*13*/
	nodev,		upreset,	NULL,
/*tm*/	tmopen,		tmclose,	tmread,		tmwrite,	/*14*/
	tmioctl,	tmreset,	NULL,
/*lp*/	lpopen,		lpclose,	nodev,		lpwrite,	/*15*/
	nodev,		lpreset,	NULL,
/*ts*/	tsopen,		tsclose,	tsread,		tswrite,	/*16*/
	tsioctl,	tsreset,	NULL,
/*dk*/	nodev,		nodev,		nodev,		nodev,		/*17*/
#if NDK > 0
	nodev,		nulldev,	&dkinfo,
#else
	nodev,		dkreset,	NULL,
#endif
/*sp*/	nodev,		nodev,		nodev,		nodev,		/*18*/
	nodev,		nulldev,	&spinfo,
/*dn*/	dnopen,		dnclose,	nodev,		dnwrite,	/*19*/
	nodev,		nulldev,	NULL,
/*ptm*/	nodev,		nodev,		nodev,		nodev,		/*20pt*/
	nodev,		nulldev,	NULL,
/*pts*/	nodev,		nodev,		nodev,		nodev,		/*21pt*/
	nodev,		nulldev,	NULL,
/*mt*/	mtopen,		mtclose,	mtread,		mtwrite,	/*22*/
	mtioctl,	nulldev,	NULL,
/*tri*/	triopen,	triclose,	nodev,		triwrite,	/*23*/
	nodev,		nulldev,	NULL,
/*xx*/	nodev,		nodev,		nodev,		nodev,		/*24*/
	nodev,		nulldev,	NULL,
/* 25-29 reserved to local sites */
/*rtk*/	rtkopen,	rtkclose,	rtkread,	rtkwrite,	/*25*/
	rtkioctl,	nulldev,	NULL,
/*kmc*/	kmcopen,	kmcclose,	kmcread,	kmcwrite,	/*26*/
	kmcioctl,	nulldev,	NULL,
/*ec*/	nodev,		nodev,		nodev,		nodev,		/*27*/
	nodev,		nulldev,	&ecsinfo,
/*uda*/	nulldev,	nulldev,	udread,		udwrite,	/*28*/
	udioctl,	nulldev,	NULL,
/*ekx*/	ekxopen,	ekxclose,	ekxread,	ekxwrite,	/*29*/
	ekxioctl,	nulldev,	NULL,
/*cv*/	cvopen,		cvclose,	cvread,		cvwrite,	/*30*/
	cvioctl,	cvreset,	NULL,
/*kdi*/	nodev,		nodev,		nodev,		nodev,		/*31*/
	nodev,		kdishutdown,	kdi,
/*mg*/	nodev,		nodev,		nodev,		nodev,		/*32*/
	nodev,		nulldev,	mg,
/*ju*/	juopen,		juclose,	juread,		juwrite,	/*33*/
	juioctl,	nulldev,	NULL,
/*chr*/	chropen,	chrclose,	chrread,	chrwrite,	/*34*/
	chrioctl,	chreset,	NULL,
/*cht*/	chtopen,	chtclose,	chtread,	chtwrite,	/*35*/
	chtioctl,	nulldev,	cht_tty,
/*chil*/chilopen,	chilclose,	chilread,	chilwrite,	/*36*/
	chilioctl,	chilreset,	NULL,
/*lex*/	lexopen,	lexclose,	lexread, 	lexwrite,	/*37*/
	lexioctl,	nulldev,	NULL,
/*bpd*/	bpdopen,	bpdclose,	bpdread, 	nulldev,	/*38*/
	bpdioctl,	nulldev,	NULL,
/*om*/	omopen,		omclose,	omread,		omwrite,	/*39*/
	omioctl,	nulldev,	NULL,
/*std*/	nodev,		nodev,		nodev,		nodev,		/*40*/
	nodev,		nulldev,	NULL,
/*ds*/	dsopen,		dsclose,	dsread,		dswrite,	/*41*/
	dsioctl,	nulldev,	NULL,
/*ip*/	nodev,		nodev,		nodev,		nodev,		/*42*/
	nodev,		nulldev,	&ipdinfo,
/*tcp*/ nodev,		nodev,		nodev,		nodev,		/*43*/
	nodev,		nulldev,	&tcpdinfo,
/*il*/	nodev,		nodev,		nodev,		nodev,		/*44*/
	nodev,		nulldev,	&ilsinfo,
/*scn*/	scnopen,	scnclose,	scnread,	scnwrite,	/*45*/
	scnioctl,	nulldev,	NULL,
/*sn*/	snopen,		snclose,	nodev,		nodev,		/*46*/
	snioctl,	nulldev,	NULL,
/*an*/	anopen, 	anclose, 	anread, 	anwrite,	/*47*/
	anioctl,	nulldev,	NULL,
/*ch*/	nodev,		nodev,		nodev,		nodev,		/*48*/
	nodev,		nodev,		&chdinfo,
/*iti*/	itiopen, 	iticlose, 	nodev,		nodev,		/*49*/
	itiioctl,	nulldev,	NULL,
/*udp*/ nodev,		nodev,		nodev,		nodev,		/*50*/
	nodev,		nulldev,	&udpdinfo,
	0	
};

int	nchrdev = sizeof(cdevsw)/sizeof(struct cdevsw) - 1;

extern int rnami(), smount();
extern int naput(), nafree(), naupdat(), naread(), nawrite(), natrunc();
extern int nastat(), nanami(), namount();
extern struct inode *naget();
extern int prput(), prfree(), prupdat(), prread(), prwrite(), prtrunc();
extern int prstat(), prnami(), prmount(), prioctl();
extern struct inode *prget();
extern int mpput(), mpfree(), mpupdat(), mpread(), mpwrite();
extern int mptrunc(), mpstat(), mpnami(), mpioctl();
extern struct inode *mpget();
struct fstypsw fstypsw[] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, rnami, smount, 0},
	{ naput, naget, nafree, naupdat, naread, nawrite, natrunc,
		nastat, nanami, namount, 0},
	{ prput, prget, prfree, prupdat, prread, prwrite, prtrunc,
		prstat, prnami, prmount, prioctl},
	{ mpput, mpget, mpfree, mpupdat, mpread, mpwrite, mptrunc,
		mpstat, mpnami, mpmount, mpioctl}
};
int	nfstyp = 4;
int	tty_no = 2;	/* major device number of /dev/tty special file */
int	mem_no = 3; 	/* major device number of memory special file */
int	stdio_no = 40;	/* major device number of stdio special file */

extern struct	streamtab ttyinfo;
extern struct	streamtab rdkinfo;
extern struct	streamtab pkinfo;
extern struct	streamtab msginfo;
extern struct	streamtab dkpinfo;
extern struct	streamtab cdkpinfo;
extern struct	streamtab nttyinfo;
extern struct	streamtab trcinfo;
extern struct	streamtab bufinfo;
extern struct	streamtab rmsginfo;
extern struct	streamtab ipinfo;
extern struct	streamtab tcpinfo;
extern struct	streamtab chrouteinfo;
extern struct	streamtab arpinfo;
extern struct	streamtab udpinfo;
extern struct	streamtab chinfo;
extern struct	streamtab filtinfo;
extern struct	streamtab dumpinfo;

#include "pk.h"
#include "dkp.h"
#include "cm.h"
#include "ntty.h"
#include "trc.h"
#include "bf.h"
#include "inet.h"
#include "chroute.h"
#include "arp.h"
#include "udp.h"
#include "ch.h"
#include "filter.h"
#include "dumpld.h"

struct	streamtab *streamtab[] = {
	&ttyinfo,		/* 0 */
#if NDKP > 0
	&cdkpinfo,		/* 1 */
#else
	NULL,
#endif
#if NCM > 0
	&rdkinfo,		/* 2 */
#else
	NULL,
#endif
#if NPK > 0
	&pkinfo,		/* 3 */
#else
	NULL,
#endif
	&msginfo,		/* 4 */
#if NDKP > 0
	&dkpinfo,		/* 5 */
#else
	NULL,
#endif
#if NNTTY > 0
	&nttyinfo,		/* 6 */
#else
	NULL,
#endif
#if NBF > 0
	&bufinfo,		/* 7 */
#else
	NULL,
#endif
#if NTRC > 0
	&trcinfo,		/* 8 */
#else
	NULL,
#endif
	&rmsginfo,		/* 9 */
#if NINET > 0
	&ipinfo,		/* 10 */
	&tcpinfo,		/* 11 */
#else
	NULL,
	NULL,
#endif
#if NCHROUTE > 0
	&chrouteinfo,		/* 12 */
#else
	NULL,
#endif
#if NARP > 0
	&arpinfo,		/* 13 */
#else
	NULL,
#endif
#if NUDP > 0
	&udpinfo,		/* 14 */
#else
	NULL,
#endif
#if NCH > 0
	&chinfo,		/* 15 */
#else
	NULL,
#endif
#if NFILTER > 0
	&filtinfo,		/* 16 */
#else
	NULL,
#endif
#if NDUMPLD > 0
	&dumpinfo,		/* 17 */
#else
	NULL,
#endif
};

int	nstream = sizeof(streamtab)/sizeof(struct streamtab *);

struct	buf	bfreelist[BQUEUES];	/* buffer chain headers */
struct	buf	bswlist;	/* free list of swap headers */
struct	buf	*bclnlist;	/* header for list of cleaned pages */
struct	acct	acctbuf;
struct	inode	*acctp;

/*
 * Swapdev is a fake device implemented
 * in sw.c used only internally to get to swstrategy.
 * It cannot be provided to the users, because the
 * swstrategy routine munches the b_dev and b_blkno entries
 * before calling the appropriate driver.  This would horribly
 * confuse, e.g. the hashing routines. Instead, /dev/drum is
 * provided as a character (raw) device.
 */
dev_t	swapdev = makedev(4, 0);

extern struct user u;

/*
 * Fetchable stream parameters
 */
int	Nqueue	= NQUEUE;
int	Nstream	= NSTREAM;
int	Nblk64	= NBLK64;
int	Nblk16	= NBLK16;
int	Nblk4	= NBLK4;
#ifndef	NBLKBIG
#define	NBLKBIG	0
#endif
int	Nblkbig	= NBLKBIG;
int	Nblock	= (NBLKBIG+NBLK64+NBLK16+NBLK4);

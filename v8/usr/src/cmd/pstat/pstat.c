static char *sccsid = "@(#)pstat.c	4.9 (Berkeley) 5/7/81";
/*
 * Print system stuff
 */

#define mask(x) (x&0377)
#define	clear(x) ((int)x&0x7fffffff)

#include <sys/param.h>
#include <sys/dir.h>
#include <sys/file.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/text.h>
#include <sys/inode.h>
#include <sys/map.h>
#include <sys/conf.h>
#include <sys/vm.h>
#include <nlist.h>
#include <sys/pte.h>

char	*fcore	= "/dev/kmem";
char	*fnlist	= "/unix";
int	fc;

struct nlist nl[] = {
#define	SINODE	0
	{ "_inode" },
#define	STEXT	1
	{ "_text" },
#define	SPROC	2
	{ "_proc" },
#define	SDZ	3
	{ "_dz_tty" },
#define	SNDZ	4
	{ "_dz_cnt" },
#define	SKL	5
	{ "_cons" },
#define	SFIL	6
	{ "_file" },
#define	USRPTMA	7
	{ "_Usrptmap" },
#define	USRPT	8
	{ "_usrpt" },
#define	SNSWAP	9
	{ "_nswap" },
#define	SWAPMAP	10
	{ "_swapmap" },
#define	SDH	11
	{ "_dh11" },
#define	SNDH	12
	{ "_ndh11" },
#define	SGROUP	13
	{ "_groups" },
#define	SCHANS	14
	{ "_chans" },
#define	SSCHANS	15
	{ "_schans" },
#define	SNPROC	16
	{ "_nproc" },
#define	SNTEXT	17
	{ "_ntext" },
#define	SNFILE	18
	{ "_nfile" },
#define	SNINODE	19
	{ "_ninode" },
#define	SNSWAPMAP 20
	{ "_nswapmap" },
#define	STRIMPROF 21
	{ "_trimprof" },
	0,
};

int	inof;		/* inode table */
int	txtf;		/* text table */
int	prcf;		/* proc table */
int	ttyf;		/* streams, some day */
int	usrf;		/* user struct */
long	ubase;
int	filf;		/* file table */
int	swpf;		/* swap usage */
int	totflg;		/* total table usage */
int	clkf;		/* clock trims */
char	partab[1];
struct	cdevsw	cdevsw[1];
struct	bdevsw	bdevsw[1];
int	allflg;
int	kflg;
struct	pte *Usrptma;
struct	pte *usrpt;
long	seekmask = 0xffffffff;

main(argc, argv)
char **argv;
{
	register char *argp;

	argc--, argv++;
	while (argc > 0 && **argv == '-') {
		argp = *argv++;
		argp++;
		argc--;
		while (*argp++)
		switch (argp[-1]) {

		case 'T':
			totflg++;
			break;

		case 'a':
			allflg++;
			break;

		case 'i':
			inof++;
			break;

		case 'k':
			kflg++;
			fcore = "/vmcore";
			seekmask = 0x7fffffff;
			break;

		case 'x':
			txtf++;
			break;

		case 'p':
			prcf++;
			break;

		case 't':
			ttyf++;
			break;

		case 'u':
			if (argc == 0)
				break;
			argc--;
			usrf++;
			sscanf( *argv++, "%x", &ubase);
			break;

		case 'f':
			filf++;
			break;
		case 's':
			swpf++;
			break;
		case 'c':
			clkf++;
			break;
		}
	}
	if (argc>0) {
		fcore = argv[0];
		seekmask = 0x7fffffff;
	}
	if ((fc = open(fcore, 0)) < 0) {
		printf("Can't find %s\n", fcore);
		exit(1);
	}
	if (argc>1)
		fnlist = argv[1];
	nlist(fnlist, nl);
	if (nl[0].n_type == 0) {
		printf("no namelist\n");
		exit(1);
	}
	usrpt = (struct pte *)nl[USRPT].n_value;
	Usrptma = (struct pte *)nl[USRPTMA].n_value;
	if (filf||totflg)
		dofile();
	if (inof||totflg)
		doinode();
	if (prcf||totflg)
		doproc();
	if (txtf||totflg)
		dotext();
	if (ttyf)
		dotty();
	if (usrf)
		dousr();
	if (swpf||totflg)
		doswap();
	if (clkf||totflg)
		doclk();
}

doinode()
{
	register struct inode *ip;
	struct inode *xinode, *ainode;
	register int nin;
	int ninode;

	nin = 0;
	ninode = getw(nl[SNINODE].n_value);
	xinode = (struct inode *)calloc(ninode, sizeof (struct inode));
	xxseek(fc, (int)(ainode = (struct inode *)getw(nl[SINODE].n_value)), 0);
	read(fc, xinode, ninode * sizeof(struct inode));
	for (ip = xinode; ip < &xinode[ninode]; ip++)
		if (ip->i_count)
			nin++;
	printf("%5d/%5d inodes\n", nin, ninode);
	if (totflg)
		return;
	printf("   LOC    FLAGS  CNT DEVICE  INO   MODE  NLN UID   SPTR   SIZE/DEV\n");
	for (ip = xinode; ip < &xinode[ninode]; ip++) {
		if (ip->i_count == 0)
			continue;
		printf("%8.1x ", ainode + (ip - xinode));
		putf(ip->i_flag&ILOCK, 'L');
		putf(ip->i_flag&IUPD, 'U');
		putf(ip->i_flag&IACC, 'A');
		putf(ip->i_flag&IMOUNT, 'M');
		putf(ip->i_flag&IWANT, 'W');
		putf(ip->i_flag&ITEXT, 'T');
		printf("%4d", ip->i_count&0377);
		printf("%3d,%3d", major(ip->i_dev), minor(ip->i_dev));
		printf("%6d", ip->i_number);
		printf("%7o", ip->i_mode & 0xffff);
		printf("%4d", ip->i_nlink);
		printf("%4d", ip->i_uid);
		printf("%8x", (int)ip->i_sptr&0xffffff);
		if ((ip->i_mode&IFMT)==IFBLK || (ip->i_mode&IFMT)==IFCHR)
			printf("%6d,%3d", major(ip->i_un.i_rdev), minor(ip->i_un.i_rdev));
		else
			printf("%10ld", ip->i_size);
		printf("\n");
	}
	free(xinode);
}

getw(loc)
	off_t loc;
{
	int word;

	if (kflg)
		loc &= 0x7fffffff;
	xxseek(fc, loc, 0);
	read(fc, &word, sizeof (word));
	if (kflg)
		word &= 0x7fffffff;
	return (word);
}

putf(v, n)
{
	if (v)
		printf("%c", n);
	else
		printf(" ");
}

dotext()
{
	register struct text *xp;
	int ntext;
	struct text *xtext, *atext;
	int ntx;

	ntx = 0;
	ntext = getw(nl[SNTEXT].n_value);
	xtext = (struct text *)calloc(ntext, sizeof (struct text));
	xxseek(fc, (int)(atext = (struct text *)getw(nl[STEXT].n_value)), 0);
	read(fc, xtext, ntext * sizeof (struct text));
	for (xp = xtext; xp < &xtext[ntext]; xp++)
		if (xp->x_iptr!=NULL)
			ntx++;
	printf("%5d/%5d active texts\n", ntx, ntext);
	if (totflg)
		return;
	printf("   LOC   FLAGS DADDR      CADDR  RSS SIZE      IPTR  CNT CCNT\n");
	for (xp = xtext; xp < &xtext[ntext]; xp++) {
		if (xp->x_iptr == NULL)
			continue;
		printf("%8.1x", atext + (xp - xtext));
		printf(" ");
		putf(xp->x_flag&XPAGI, 'P');
		putf(xp->x_flag&XTRC, 'T');
		putf(xp->x_flag&XWRIT, 'W');
		putf(xp->x_flag&XLOAD, 'L');
		putf(xp->x_flag&XLOCK, 'K');
		putf(xp->x_flag&XWANT, 'w');
		printf("%5x", xp->x_daddr[0]);
		printf("%11x", xp->x_caddr);
		printf("%5d", xp->x_rssize);
		printf("%5d", xp->x_size);
		printf("%10.1x", xp->x_iptr);
		printf("%5d", xp->x_count&0377);
		printf("%5d", xp->x_ccount);
		printf("\n");
	}
	free(xtext);
}

doproc()
{
	struct proc *xproc, *aproc;
	int nproc;
	register struct proc *pp;
	register loc, np;
	struct pte apte;

	nproc = getw(nl[SNPROC].n_value);
	xproc = (struct proc *)calloc(nproc, sizeof (struct proc));
	xxseek(fc, (int)(aproc = (struct proc *)getw(nl[SPROC].n_value)), 0);
	read(fc, xproc, nproc * sizeof (struct proc));
	np = 0;
	for (pp=xproc; pp < &xproc[nproc]; pp++)
		if (pp->p_stat)
			np++;
	printf("%5d/%5d processes\n", np, nproc);
	if (totflg)
		return;
	printf("   LOC    S    F POIP PRI      SIG  UID SLP TIM  CPU  NI   PGRP    PID   PPID    ADDR   RSS SRSS SIZE    WCHAN    LINK   TEXTP CLKT\n");
	for (pp=xproc; pp<&xproc[nproc]; pp++) {
		if (pp->p_stat==0 && allflg==0)
			continue;
		printf("%8x", aproc + (pp - xproc));
		printf(" %2d", pp->p_stat);
		printf(" %4x", pp->p_flag & 0xffff);
		printf(" %4d", pp->p_poip);
		printf(" %3d", pp->p_pri);
		printf(" %8x", pp->p_sig);
		printf(" %4d", pp->p_uid);
		printf(" %3d", pp->p_slptime);
		printf(" %3d", pp->p_time);
		printf(" %4d", pp->p_cpu&0377);
		printf(" %3d", pp->p_nice);
		printf(" %6d", pp->p_pgrp);
		printf(" %6d", pp->p_pid);
		printf(" %6d", pp->p_ppid);
		if (kflg)
			pp->p_addr = (struct pte *)clear((int)pp->p_addr);
		xxseek(fc, (long)(Usrptma+btokmx(pp->p_addr)), 0);
		read(fc, &apte, sizeof(apte));
		printf(" %8x", ctob(apte.pg_pfnum+1) - sizeof(struct pte) * UPAGES);
		printf(" %4x", pp->p_rssize);
		printf(" %4x", pp->p_swrss);
		printf(" %5x", pp->p_dsize+pp->p_ssize);
		printf(" %7x", clear(pp->p_wchan));
		printf(" %7x", clear(pp->p_link));
		printf(" %7x", clear(pp->p_textp));
		printf("    %u", pp->p_clktim);
		printf("\n");
	}
}

dotty()
{
printf("redo tty stuff for new system\n");
/*	struct tty dz_tty[64];
/*	int ndz;
/*	register struct tty *tp;
/*	register char *mesg;
/*
/*	printf("1 cons\n");
/*	if (kflg)
/*		nl[SKL].n_value = clear(nl[SKL].n_value);
/*	xxseek(fc, (long)nl[SKL].n_value, 0);
/*	read(fc, dz_tty, sizeof(dz_tty[0]));
/*	mesg = " # RAW CAN OUT   MODE    ADDR   DEL COL  STATE   PGRP DISC\n";
/*	printf(mesg);
/*	ttyprt(&dz_tty[0], 0);
/*	if (nl[SNDZ].n_type == 0)
/*		goto dh;
/*	if (kflg) {
/*		nl[SNDZ].n_value = clear(nl[SNDZ].n_value);
/*		nl[SDZ].n_value = clear(nl[SDZ].n_value);
/*	}
/*	xxseek(fc, (long)nl[SNDZ].n_value, 0);
/*	read(fc, &ndz, sizeof(ndz));
/*	printf("%d dz lines\n", ndz);
/*	xxseek(fc, (long)nl[SDZ].n_value, 0);
/*	read(fc, dz_tty, sizeof(dz_tty));
/*	for (tp = dz_tty; tp < &dz_tty[ndz]; tp++)
/*		ttyprt(tp, tp - dz_tty);
/*dh:
/*	if (nl[SNDH].n_type == 0)
/*		return;
/*	if (kflg) {
/*		nl[SNDH].n_value = clear(nl[SNDH].n_value);
/*		nl[SDH].n_value = clear(nl[SDH].n_value);
/*	}
/*	xxseek(fc, (long)nl[SNDH].n_value, 0);
/*	read(fc, &ndz, sizeof(ndz));
/*	printf("%d dh lines\n", ndz);
/*	xxseek(fc, (long)nl[SDH].n_value, 0);
/*	read(fc, dz_tty, sizeof(dz_tty));
/*	for (tp = dz_tty; tp < &dz_tty[ndz]; tp++)
/*		ttyprt(tp, tp - dz_tty);
/*}
/*
/*ttyprt(atp, line)
/*struct tty *atp;
/*{
/*	register struct tty *tp;
/*
/*	printf("%2d", line);
/*	tp = atp;
/*	switch (tp->t_line) {
/*
/*	case NETLDISC:
/*		if (tp->t_rec)
/*			printf("%4d%4d", 0, tp->t_inbuf);
/*		else
/*			printf("%4d%4d", tp->t_inbuf, 0);
/*		break;
/*
/*	default:
/*		printf("%4d", tp->t_rawq.c_cc);
/*		printf("%4d", tp->t_canq.c_cc);
/*	}
/*	printf("%4d", tp->t_outq.c_cc);
/*	printf("%8.1o", tp->t_flags);
/*	printf(" %8.1x", tp->t_addr);
/*	printf("%3d", tp->t_delct);
/*	printf("%4d ", tp->t_col);
/*	putf(tp->t_state&TIMEOUT, 'T');
/*	putf(tp->t_state&WOPEN, 'W');
/*	putf(tp->t_state&ISOPEN, 'O');
/*	putf(tp->t_state&CARR_ON, 'C');
/*	putf(tp->t_state&BUSY, 'B');
/*	putf(tp->t_state&ASLEEP, 'A');
/*	putf(tp->t_state&XCLUDE, 'X');
/*
/*	putf(tp->t_state&HUPCLS, 'H');
/*
/*	printf("%6d", tp->t_pgrp);
/*	switch (tp->t_line) {
/*
/*	case NTTYDISC:
/*		printf(" ntty");
/*		break;
/*
/*	case NETLDISC:
/*		printf(" net");
/*		break;
/*	}
/*	printf("\n");*/
}

dousr()
{
	struct user U;
	register i, j, *ip;

	/* This wins only if PAGSIZ > sizeof (struct user) */
	xxseek(fc, ubase * NBPG, 0);
	read(fc, &U, sizeof(U));
	printf("pcb");
	ip = (int *)&U.u_pcb;
	while (ip < &U.u_arg[0]) {
		if ((ip - (int *)&U.u_pcb) % 4 == 0)
			printf("\t");
		printf("%x ", *ip++);
		if ((ip - (int *)&U.u_pcb) % 4 == 0)
			printf("\n");
	}
	if ((ip - (int *)&U.u_pcb) % 4 != 0)
		printf("\n");
	printf("arg\t");
	for (i=0; i<5; i++)
		printf(" %.1x", U.u_arg[i]);
	printf("\n");
	for (i=0; i<sizeof(label_t)/sizeof(int); i++) {
		if (i%5==0)
			printf("\t");
		printf("%9.1x", U.u_ssav[i]);
		if (i%5==4)
			printf("\n");
	}
	if (i%5)
		printf("\n");
	printf("segflg\t%d\nerror %d\n", U.u_segflg, U.u_error);
	printf("uids\t%d,%d,%d,%d\n", U.u_uid,U.u_gid,U.u_ruid,U.u_rgid);
	printf("procp\t%.1x\n", U.u_procp);
	printf("ap\t%.1x\n", U.u_ap);
	printf("r_val?\t%.1x %.1x\n", U.u_r.r_val1, U.u_r.r_val2);
	printf("base, count, offset %.1x %.1x %ld\n", U.u_base,
		U.u_count, U.u_offset);
	printf("cdir rdir %.1x %.1x\n", U.u_cdir, U.u_rdir);
	printf("dbuf %.14s\n", U.u_dbuf);
	printf("dirp %.1x\n", U.u_dirp);
	printf("dent %d %.14s\n", U.u_dent.d_ino, U.u_dent.d_name);
	printf("pdir %.1o\n", U.u_pdir);
	printf("file\t");
	for (i=0; i<10; i++)
		printf("%9.1x", U.u_ofile[i]);
	printf("\n\t");
	for (i=10; i<NOFILE; i++)
		printf("%9.1x", U.u_ofile[i]);
	printf("\n");
	printf("pofile\t");
	for (i=0; i<10; i++)
		printf("%9.1x", U.u_pofile[i]);
	printf("\n\t");
	for (i=10; i<NOFILE; i++)
		printf("%9.1x", U.u_pofile[i]);
	printf("\n");
	printf("ssav");
	for (i=0; i<sizeof(label_t)/sizeof(int); i++) {
		if (i%5==0)
			printf("\t");
		printf("%9.1x", U.u_ssav[i]);
		if (i%5==4)
			printf("\n");
	}
	if (i%5)
		printf("\n");
	printf("sigs\t");
	for (i=0; i<NSIG; i++)
		printf("%.1x ", U.u_signal[i]);
	printf("\n");
	printf("ar0\t%.1x\n", U.u_ar0);
	printf("prof\t%X %X %X %X\n", U.u_prof.pr_base, U.u_prof.pr_size,
	    U.u_prof.pr_off, U.u_prof.pr_scale);
	printf("\neosys\t%d\n", U.u_eosys);
	printf("sep\t%d\n", U.u_sep);
	/*
	printf("ttyp\t%.1x\n", U.u_ttyp);
	printf("ttyd\t%d,%d\n", major(U.u_ttyd), minor(U.u_ttyd));
	*/
	printf("exdata\t");
	ip = (int *)&U.u_exdata;
	for (i = 0; i < 8; i++)
		printf("%.1D ", *ip++);
	printf("\n");
	printf("comm %.14s\n", U.u_comm);
	printf("start\t%D\n", U.u_start);
	printf("acflag\t%D\n", U.u_acflag);
	printf("fpflag\t%D\n", U.u_fpflag);
	printf("cmask\t%D\n", U.u_cmask);
	printf("sizes\t%.1x %.1x %.1x\n", U.u_tsize, U.u_dsize, U.u_ssize);
	printf("vm\t");
	ip = (int *)&U.u_vm;
	for (i = 0; i < sizeof(U.u_vm)/sizeof(int); i++)
		printf("%D ", ip[i]);
	printf("\n");
	ip = (int *)&U.u_cvm;
	printf("cvm\t");
	for (i = 0; i < sizeof(U.u_vm)/sizeof(int); i++)
		printf("%D ", ip[i]);
	printf("\n");
/*
	i =  U.u_stack - &U;
	while (U[++i] == 0);
	i &= ~07;
	while (i < 512) {
		printf("%x ", 0140000+2*i);
		for (j=0; j<8; j++)
			printf("%9x", U[i++]);
		printf("\n");
	}
*/
}

oatoi(s)
char *s;
{
	register v;

	v = 0;
	while (*s)
		v = (v<<3) + *s++ - '0';
	return(v);
}

dofile()
{
	int nfile;
	struct file *xfile, *afile;
	register struct file *fp;
	register nf;
	int loc;

	nf = 0;
	nfile = getw(nl[SNFILE].n_value);
	xfile = (struct file *)calloc(nfile, sizeof (struct file));
	xxseek(fc, (int)(afile = (struct file *)getw(nl[SFIL].n_value)), 0);
	read(fc, xfile, nfile * sizeof (struct file));
	for (fp=xfile; fp < &xfile[nfile]; fp++)
		if (fp->f_count)
			nf++;
	printf("%5d/%5d open files\n", nf, nfile);
	if (totflg)
		return;
	printf("   LOC   FLG  CNT   INO    OFFS\n");
	for (fp=xfile,loc=nl[SFIL].n_value; fp < &xfile[nfile]; fp++,loc+=sizeof(xfile[0])) {
		if (fp->f_count==0)
			continue;
		printf("%8x ", loc);
		putf(fp->f_flag&FREAD, 'R');
		putf(fp->f_flag&FWRITE, 'W');
		putf(fp->f_flag&FPIPE, 'P');
		printf("%4d", mask(fp->f_count));
		printf("%9.1x", fp->f_inode);
		printf("  %ld\n", fp->f_offset);
	}
}

doswap()
{
	struct proc *proc;
	int nproc;
	struct text *xtext;
	int ntext;
	struct map *swapmap;
	int nswapmap;
	register struct proc *pp;
	int nswap, used, tused, free;
	register struct mapent *me;
	register struct text *xp;

	nproc = getw(nl[SNPROC].n_value);
	proc = (struct proc *)calloc(nproc, sizeof (struct proc));
	xxseek(fc, getw(nl[SPROC].n_value), 0);
	read(fc, proc, nproc * sizeof (struct proc));
	nswapmap = getw(nl[SNSWAPMAP].n_value);
	swapmap = (struct map *)calloc(nswapmap, sizeof (struct map));
	xxseek(fc, getw(nl[SWAPMAP].n_value), 0);
	read(fc, swapmap, nswapmap * sizeof (struct map));
	nswap = getw(nl[SNSWAP].n_value);
	free = 0;
	for (me = (struct mapent *)(swapmap+1);
	    me < (struct mapent *)&swapmap[nswapmap]; me++)
		free += me->m_size;
	ntext = getw(nl[SNTEXT].n_value);
	xtext = (struct text *)calloc(ntext, sizeof (struct text));
	xxseek(fc, getw(nl[STEXT].n_value), 0);
	read(fc, xtext, ntext * sizeof (struct text));
	tused = 0;
	for (xp = xtext; xp < &xtext[ntext]; xp++)
		if (xp->x_iptr!=NULL)
			tused += xdsize(xp);
	used = tused;
	for (pp = proc; pp < &proc[nproc]; pp++) {
		if (pp->p_stat == 0 || pp->p_stat == SZOMB)
			continue;
		if (pp->p_flag & SSYS)
			continue;
		used += up(pp->p_dsize) + up(pp->p_ssize);
		if ((pp->p_flag&SLOAD) == 0)
			used += vusize(pp);
	}
	/* a DMMAX/2 block goes to argmap */
	if (totflg)
		printf("%5d/%5d 00k swap\n", used/2/100, (used+free)/2/100);
	printf("%d used (%d text), %d free, %d missing\n",
	    used/2, tused/2, free/2, (nswap - DMMAX/2 - (used + free))/2);
}

doclk()
{
	register int	i;
	unsigned int	trimprof[3];

	if (!nl[STRIMPROF].n_value) {
		printf("Clock trim info unavailable.\n");
		return;
	}
	for (i = 0; i < 3; ++i)
		trimprof[i] = getw(nl[STRIMPROF].n_value + (i * sizeof trimprof[0]));
	printf("%12.12s %12.12s %12.12s\n", "SLOW", "ON TIME", "FAST");
	printf("%12d %12d %12d\n", trimprof[0], trimprof[1], trimprof[2]);
}

up(size)
	register int size;
{
	register int i, block;

	i = 0;
	block = DMMIN;
	while (i < size) {
		i += block;
		if (block < DMMAX)
			block *= 2;
	}
	return (i);
}

vusize(p)
struct proc *p;
{
	register int tsz = p->p_tsize / NPTEPG;

	return (clrnd(UPAGES + clrnd(ctopt(p->p_tsize+p->p_dsize+p->p_ssize+UPAGES)) - tsz));
}

xdsize(xp)
struct text *xp;
{

	if (xp->x_flag & XPAGI)
		return (clrnd(xp->x_size + ctopt(xp->x_size)));
	return (xp->x_size);
}

xxseek(f, l)
long l;
{
	return(lseek(f, l&seekmask, 0));
}

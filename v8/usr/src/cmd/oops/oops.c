static	char *sccsid = "@(#)ps.c	4.13 (Berkeley) 81/07/08";
/*
 * ps; VAX 4BSD version
 */

#include <stdio.h>
#include <ctype.h>
#include <nlist.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/pte.h>
#include <sys/vm.h>
#include <sys/text.h>
#include <sys/stat.h>
#include <math.h>
#include <sys/vlimit.h>
#include <ftw.h>

struct nlist nl[] = {
	{ "_proc" },
#define	X_PROC		0
	{ "_Usrptmap" },
#define	X_USRPTMA	1
	{ "_usrpt" },
#define	X_USRPT		2
	{ "_text" },
#define	X_TEXT		3
	{ "_nswap" },
#define	X_NSWAP		4
	{ "_maxslp" },
#define	X_MAXSLP	5
	{ "_ccpu" },
#define	X_CCPU		6
	{ "_ecmx" },
#define	X_ECMX		7
	{ "_nproc" },
#define	X_NPROC		8
	{ "_ntext" },
#define	X_NTEXT		9
	{ "_hz" },
#define	X_HZ		10
	{ 0 },
};

struct	savcom {
	union {
		struct	lsav *lp;
		float	u_pctcpu;
		struct	vsav *vp;
		int	s_ssiz;
	} sun;
	struct	asav *ap;
} *savcom;

struct	asav {
	char	*a_cmdp;
	int	a_flag;
	short	a_stat, a_uid, a_pid, a_nice, a_pri, a_slptime, a_time;
	size_t	a_size, a_rss, a_tsiz, a_txtrss;
	short	a_xccount;
	char	a_tty[DIRSIZ+1];
	dev_t	a_ttyd;
	time_t	a_cpu;
	size_t	a_maxrss;
};

char	*lhdr;
struct	lsav {
	short	l_ppid;
	char	l_cpu;
	int	l_addr;
	caddr_t	l_wchan;
};

char	*uhdr;
char	*shdr;

char	*vhdr;
struct	vsav {
	u_int	v_majflt;
	size_t	v_swrss, v_txtswrss;
	float	v_pctcpu;
};

struct	proc proc[8];		/* 8 = a few, for less syscalls */
struct	proc *mproc;
struct	text *text;

int	paduser1;		/* avoid hardware mem clobbering botch */
union {
	struct	user user;
	char	upages[UPAGES][NBPG];
} user;
#define u	user.user
int	paduser2;		/* avoid hardware mem clobbering botch */

#define clear(x) 	((int)x & 0xffffff)

int	chkpid;
int	aflg, cflg, eflg, gflg, kflg, lflg, sflg, uflg, vflg, xflg;
char	*tptr;
char	*gettty(), *getcmd(), *getname(), *savestr(), *alloc(), *state();
double	pcpu(), pmem();
int	pscomp();
int	nswap, maxslp;
struct	text *atext;
double	ccpu;
int	ecmx;
struct	pte *Usrptma, *usrpt;
int	nproc, ntext, hz;

struct	ttys {
	char	name[DIRSIZ+1];
	dev_t	ttyd;
	struct	ttys *next;
} *allttys;


int	npr;

int	cmdstart;
int	twidth;
char	*kmemf, *memf, *swapf, *nlistf;
int	kmem, mem, swap;
int	rawcpu, sumcpu;

int	pcbpf;
int	argaddr;
extern	char _sobuf[];

main(argc, argv)
	char **argv;
{
	register int i, j;
	register char *ap;
	int uid, pgrp;
	off_t procp;

	argc--, argv++;
	twidth = 80;
	setbuf(stdout, _sobuf);
	if (argc > 0) {
		ap = argv[0];
		while (*ap) switch (*ap++) {

		case 'C':
			rawcpu++;
			break;
		case 'S':
			sumcpu++;
			break;
		case 'a':
			aflg++;
			break;
		case 'c':
			cflg = !cflg;
			break;
		case 'e':
			eflg++;
			break;
		case 'g':
			gflg++;
			break;
		case 'k':
			kflg++;
			break;
		case 'l':
			lflg++;
			break;
		case 's':
			sflg++;
			break;
		case 't':
			if (*ap)
				tptr = ap;
			aflg++;
			gflg++;
			if (*tptr == '?')
				xflg++;
			while (*ap)
				ap++;
			break;
		case 'u':
			uflg++;
			break;
		case 'v':
			cflg = 1;
			vflg++;
			break;
		case 'w':
			if (twidth == 80)
				twidth = 132;
			else
				twidth = BUFSIZ;
			break;
		case 'x':
			xflg++;
			break;
		default:
			if (!isdigit(ap[-1]))
				break;
			chkpid = atoi(--ap);
			*ap = 0;
			aflg++;
			xflg++;
			break;
		}
	}
	openfiles(argc, argv);
	getkvars(argc, argv);
	getdev("/dev");
	getdev("/dev/dk");
	getdev("/dev/pt");
	uid = getuid();
	pgrp = getpgrp();
	printhdr();
	procp = getw(nl[X_PROC].n_value);
	nproc = getw(nl[X_NPROC].n_value);
	hz = getw(nl[X_HZ].n_value);
	savcom = (struct savcom *)calloc(nproc, sizeof (*savcom));
	for (i=0; i<nproc; i += 8) {
		KMlseek(kmem, (char *)procp, 0);
		j = nproc - i;
		if (j > 8)
			j = 8;
		j *= sizeof (struct proc);
		if (read(kmem, (char *)proc, j) != j)
			cantread("proc table", kmemf);
		procp += j;
		for (j = j / sizeof (struct proc) - 1; j >= 0; j--) {
			mproc = &proc[j];
			if (mproc->p_stat == 0 ||
			    mproc->p_pgrp == 0 && xflg == 0)
				continue;
/*
			if (tptr == 0 && gflg == 0 && xflg == 0 &&
			    mproc->p_ppid == 1 && (mproc->p_flag&SDETACH) == 0)
				continue;
*/

			if(uid != mproc->p_uid && !aflg && mproc->p_pgrp != pgrp ||
			    chkpid != 0 && chkpid != mproc->p_pid)
				continue;
			if (vflg && gflg == 0 && xflg == 0) {
				if (mproc->p_stat == SZOMB ||
				    mproc->p_flag&SWEXIT)
					continue;
				if (mproc->p_slptime > MAXSLP &&
				    (mproc->p_stat == SSLEEP ||
				     mproc->p_stat == SSTOP))
				continue;
			}
			save();
		}
	}
	qsort(savcom, npr, sizeof(savcom[0]), pscomp);
	for (i=0; i<npr; i++) {
		register struct savcom *sp = &savcom[i];
		if (lflg)
			lpr(sp);
		else if (vflg)
			vpr(sp);
		else if (uflg)
			upr(sp);
		else
			spr(sp);
		if (sp->ap->a_flag & SWEXIT)
			printf(" <exiting>");
		else if (sp->ap->a_stat == SZOMB)
			printf(" <defunct>");
		else if (sp->ap->a_pid == 0)
			printf(" swapper");
		else if (sp->ap->a_pid == 2)
			printf(" pagedaemon");
		else if (sp->ap->a_pid == 3 && sp->ap->a_flag & SSYS)
			printf(" ip input");
		else
			printf(" %.*s", twidth - cmdstart - 2, sp->ap->a_cmdp);
		printf("\n");
	}
	exit(npr == 0);
}

getw(loc)
	off_t loc;
{
	long word;

	KMlseek(kmem, loc, 0);
	if (read(kmem, &word, sizeof (word)) != sizeof (word))
		printf("error reading kmem at %x\n", loc);
	return (word);
}

openfiles(argc, argv)
	char **argv;
{

	kmemf = "/dev/kmem";
	if (kflg)
		kmemf = argc > 1 ? argv[1] : "/vmcore";
	kmem = open(kmemf, 0);
	if (kmem < 0) {
		perror(kmemf);
		exit(1);
	}
	if (kflg)  {
		mem = kmem;
		memf = kmemf;
	} else {
		memf = "/dev/mem";
		mem = open(memf, 0);
		if (mem < 0) {
			perror(memf);
			exit(1);
		}
	}
	swapf = argc>3 ? argv[3]: "/dev/drum";
	swap = open(swapf, 0);
	if (swap < 0) {
		perror(swapf);
		exit(1);
	}
}

getkvars(argc, argv)
	char **argv;
{
	register struct nlist *nlp;

	nlistf = argc > 2 ? argv[2] : "/unix";
	nlist(nlistf, nl);
	if (nl[0].n_type == 0) {
		fprintf(stderr, "%s: No namelist\n", nlistf);
		exit(1);
	}
	if (kflg)
		for (nlp = nl; nlp < &nl[sizeof (nl)/sizeof (nl[0])]; nlp++)
			nlp->n_value = clear(nlp->n_value);
	Usrptma = (struct pte *)nl[X_USRPTMA].n_value;
	usrpt = (struct pte *)nl[X_USRPT].n_value;
	KMlseek(kmem, (long)nl[X_NSWAP].n_value, 0);
	if (read(kmem, &nswap, sizeof (nswap)) != sizeof (nswap)) {
		cantread("nswap", kmemf);
		exit(1);
	}
	KMlseek(kmem, (long)nl[X_MAXSLP].n_value, 0);
	if (read(kmem, &maxslp, sizeof (maxslp)) != sizeof (maxslp)) {
		cantread("maxslp", kmemf);
		exit(1);
	}
	KMlseek(kmem, (long)nl[X_CCPU].n_value, 0);
	if (read(kmem, &ccpu, sizeof (ccpu)) != sizeof (ccpu)) {
		cantread("ccpu", kmemf);
		exit(1);
	}
	KMlseek(kmem, (long)nl[X_ECMX].n_value, 0);
	if (read(kmem, &ecmx, sizeof (ecmx)) != sizeof (ecmx)) {
		cantread("ecmx", kmemf);
		exit(1);
	}
	if (uflg || vflg) {
		ntext = getw(nl[X_NTEXT].n_value);
		text = (struct text *)alloc(ntext * sizeof (struct text));
		if (text == 0) {
			fprintf(stderr, "no room for text table\n");
			exit(1);
		}
		atext = (struct text *)getw(nl[X_TEXT].n_value);
		KMlseek(kmem, (int)atext, 0);
		if (read(kmem, (char *)text, ntext * sizeof (struct text))
		    != ntext * sizeof (struct text)) {
			cantread("text table", kmemf);
			exit(1);
		}
	}
}

printhdr()
{
	char *hdr;

	if (sflg+lflg+vflg+uflg > 1) {
		fprintf(stderr, "ps: specify only one of s,l,v and u\n");
		exit(1);
	}
	hdr = lflg ? lhdr : (vflg ? vhdr : (uflg ? uhdr : shdr));
	if (lflg+vflg+uflg+sflg == 0)
		hdr += strlen("SSIZ ");
	cmdstart = strlen(hdr);
	printf("%s COMMAND\n", hdr);
	fflush(stdout);
}

cantread(what, fromwhat)
	char *what, *fromwhat;
{
	fprintf(stderr, "ps: error reading %s from %s\n", what, fromwhat);
}

getdev(dir)
	register char *dir;
{
	register FILE *df;
	struct direct dirent;
	register struct ttys *dp;

	df = fopen (dir, "r");
	if (df != NULL) {
		while (fread (&dirent, sizeof (dirent), 1, df) == 1) {
			dp = (struct ttys *) alloc (sizeof (struct ttys));
			strncpy(dp->name, dirent.d_name, DIRSIZ);
			dp->ttyd = dirent.d_ino;
			dp->next = allttys;
			allttys = dp;
		}
		fclose (df);
	}
}

char *
gettty()
{
	register char *p;
	register struct ttys *dp;

	if (u.u_ttyino != 0) {
		for (dp = allttys; dp; dp = dp -> next) {
			if (dp->ttyd == u.u_ttyino) {
				p = dp->name;
				if (p[0]=='t' && p[1]=='t' && p[2]=='y')
					p += 3;
				return (p);
			}
		}
	}
	return ("?");
}

save()
{
	register struct savcom *sp;
	register struct asav *ap;
	register char *cp;
	register struct text *xp;
	char *ttyp, *cmdp;

	if (mproc->p_stat != SZOMB && getu() == 0)
		return;
	ttyp = gettty();
	if (xflg == 0 && ttyp[0] == '?' || tptr && strcmp(tptr, ttyp))
		return;
	sp = &savcom[npr];
	cmdp = getcmd();
	if (cmdp == 0)
		return;
	sp->ap = ap = (struct asav *)alloc(sizeof (struct asav));
	sp->ap->a_cmdp = cmdp;
#define e(a,b) ap->a = mproc->b
	e(a_flag, p_flag); e(a_stat, p_stat); e(a_nice, p_nice);
	e(a_uid, p_uid); e(a_pid, p_pid); e(a_pri, p_pri);
	e(a_slptime, p_slptime); e(a_time, p_time);
	strncpy(ap->a_tty, ttyp, 5);
	if (ap->a_stat == SZOMB) {
		register struct xproc *xp = (struct xproc *)mproc;

		ap->a_cpu = xp->xp_vm.vm_utime + xp->xp_vm.vm_stime;
	} else {
		ap->a_size = mproc->p_dsize + mproc->p_ssize;
		e(a_rss, p_rssize); 
		ap->a_ttyd = u.u_ttyino;
		ap->a_cpu = u.u_vm.vm_utime + u.u_vm.vm_stime;
		if (sumcpu)
			ap->a_cpu += u.u_cvm.vm_utime + u.u_cvm.vm_stime;
		if (mproc->p_textp && text) {
			xp = &text[mproc->p_textp - atext];
			ap->a_tsiz = xp->x_size;
			ap->a_txtrss = xp->x_rssize;
			ap->a_xccount = xp->x_ccount;
		}
	}
#undef e
	ap->a_cpu /= hz;
	ap->a_maxrss = mproc->p_maxrss;
	if (lflg) {
		register struct lsav *lp;

		sp->sun.lp = lp = (struct lsav *)alloc(sizeof (struct lsav));
#define e(a,b) lp->a = mproc->b
		e(l_ppid, p_ppid); e(l_cpu, p_cpu);
		if (ap->a_stat != SZOMB)
			e(l_wchan, p_wchan);
#undef e
		lp->l_addr = pcbpf;
	} else if (vflg) {
		register struct vsav *vp;

		sp->sun.vp = vp = (struct vsav *)alloc(sizeof (struct vsav));
#define e(a,b) vp->a = mproc->b
		if (ap->a_stat != SZOMB) {
			e(v_swrss, p_swrss);
			vp->v_majflt = u.u_vm.vm_majflt;
			if (mproc->p_textp)
				vp->v_txtswrss = xp->x_swrss;
		}
		vp->v_pctcpu = pcpu();
#undef e
	} else if (uflg)
		sp->sun.u_pctcpu = pcpu();
	else if (sflg) {
		if (ap->a_stat != SZOMB) {
			for (cp = (char *)u.u_stack;
			    cp < &user.upages[UPAGES][0]; )
				if (*cp++)
					break;
			sp->sun.s_ssiz = (&user.upages[UPAGES][0] - cp);
		}
	}
	npr++;
}

double
pmem(ap)
	register struct asav *ap;
{
	double fracmem;
	int szptudot;

	if ((ap->a_flag&SLOAD) == 0)
		fracmem = 0.0;
	else {
		szptudot = UPAGES + clrnd(ctopt(ap->a_size+ap->a_tsiz));
		fracmem = ((float)ap->a_rss+szptudot)/CLSIZE/ecmx;
		if (ap->a_xccount)
			fracmem += ((float)ap->a_txtrss)/CLSIZE/
			    ap->a_xccount/ecmx;
	}
	return (100.0 * fracmem);
}

double
pcpu()
{
	time_t time;

	time = mproc->p_time;
	if (time == 0 || (mproc->p_flag&SLOAD) == 0)
		return (0.0);
	if (rawcpu)
		return (100.0 * mproc->p_pctcpu);
	return (100.0 * mproc->p_pctcpu / (1.0 - exp(time * log(ccpu))));
}

getu()
{
	struct pte *pteaddr, apte;
	int pad1;	/* avoid hardware botch */
	struct pte arguutl[UPAGES+CLSIZE];
	int pad2;	/* avoid hardware botch */
	register int i;
	int ncl, size;

	size = sflg ? ctob(UPAGES) : sizeof (struct user);
	if ((mproc->p_flag & SLOAD) == 0) {
		lseek(swap, ctob(mproc->p_swaddr), 0);
		if (read(swap, (char *)&user.user, size) != size) {
			fprintf(stderr, "ps: cant read u for pid %d from %s\n",
			    mproc->p_pid, swapf);
			return (0);
		}
		pcbpf = 0;
		argaddr = 0;
		return (1);
	}
	pteaddr = &Usrptma[btokmx(mproc->p_p0br) + mproc->p_szpt - 1];
	KMlseek(kmem, kflg ? clear(pteaddr) : (int)pteaddr, 0);
	if (read(kmem, (char *)&apte, sizeof(apte)) != sizeof(apte)) {
		printf("ps: cant read indir pte at %x to get u for pid %d from %s\n",
		    pteaddr, mproc->p_pid, kmemf);
		return (0);
	}
	lseek(mem,
	    ctob(apte.pg_pfnum+1) - (UPAGES+CLSIZE) * sizeof (struct pte), 0);
	if (read(mem, (char *)arguutl, sizeof(arguutl)) != sizeof(arguutl)) {
		printf("ps: cant read page table for u of pid %d from %s\n",
		    mproc->p_pid, memf);
		return (0);
	}
	if (arguutl[0].pg_fod == 0 && arguutl[0].pg_pfnum)
		argaddr = ctob(arguutl[0].pg_pfnum);
	else
		argaddr = 0;
	pcbpf = arguutl[CLSIZE].pg_pfnum;
	ncl = (size + NBPG*CLSIZE - 1) / (NBPG*CLSIZE);
	while (--ncl >= 0) {
		i = ncl * CLSIZE;
		lseek(mem, ctob(arguutl[CLSIZE+i].pg_pfnum), 0);
		if (read(mem, user.upages[i], CLSIZE*NBPG) != CLSIZE*NBPG) {
			printf("ps: cant read page %d of u of pid %d from %s\n",
			    arguutl[CLSIZE+i].pg_pfnum, mproc->p_pid, memf);
			return(0);
		}
	}
	return (1);
}

char *
getcmd()
{
	char cmdbuf[BUFSIZ];
	int pad1;		/* avoid hardware botch */
	union {
		char	argc[CLSIZE*NBPG];
		int	argi[CLSIZE*NBPG/sizeof (int)];
	} argspac;
	int pad2;		/* avoid hardware botch */
	register char *cp;
	register int *ip;
	char c;
	int nbad;
	struct dblock db;

	if (mproc->p_stat == SZOMB || mproc->p_flag&(SSYS|SWEXIT))
		return ("");
	if (cflg) {
		strncpy(cmdbuf, u.u_comm, sizeof (u.u_comm));
		return (savestr(cmdbuf));
	}
	if ((mproc->p_flag & SLOAD) == 0 || argaddr == 0) {
		vstodb(0, CLSIZE, &u.u_smap, &db, 1);
		lseek(swap, ctob(db.db_base), 0);
		if (read(swap, (char *)&argspac, sizeof(argspac))
		    != sizeof(argspac))
			goto bad;
	} else {
		lseek(mem, argaddr, 0);
		if (read(mem, (char *)&argspac, sizeof (argspac))
		    != sizeof (argspac))
			goto bad;
	}
	ip = &argspac.argi[CLSIZE*NBPG/sizeof (int)];
	ip -= 2;		/* last arg word and .long 0 */
	while (*--ip)
		if (ip == argspac.argi)
			goto retucomm;
	*(char *)ip = ' ';
	ip++;
	nbad = 0;
	for (cp = (char *)ip; cp < &argspac.argc[CLSIZE*NBPG]; cp++) {
		c = *cp & 0177;
		if (c == 0)
			*cp = ' ';
		else if (c < ' ' || c > 0176) {
			if (++nbad >= 5*(eflg+1)) {
				*cp++ = ' ';
				break;
			}
			*cp = '?';
		} else if (eflg == 0 && c == '=') {
			while (*--cp != ' ')
				if (cp <= (char *)ip)
					break;
			break;
		}
	}
	*cp = 0;
	while (*--cp == ' ')
		*cp = 0;
	cp = (char *)ip;
	strncpy(cmdbuf, cp, &argspac.argc[CLSIZE*NBPG] - cp);
	if (cp[0] == '-' || cp[0] == '?' || cp[0] <= ' ') {
		strcat(cmdbuf, " (");
		strncat(cmdbuf, u.u_comm, sizeof(u.u_comm));
		strcat(cmdbuf, ")");
	}
	if (xflg == 0 && gflg == 0 && tptr == 0 && cp[0] == '-')
		return (0);
	return (savestr(cmdbuf));

bad:
	fprintf(stderr, "ps: error locating command name for pid %d\n",
	    mproc->p_pid);
retucomm:
	strcpy(cmdbuf, " (");
	strncat(cmdbuf, u.u_comm, sizeof (u.u_comm));
	strcat(cmdbuf, ")");
	return (savestr(cmdbuf));
}

char	*lhdr =
"     F UID   PID  PPID CP PRI NI ADDR  SZ  RSS WCHAN STAT TTY    TIME";
lpr(sp)
	struct savcom *sp;
{
	register struct asav *ap = sp->ap;
	register struct lsav *lp = sp->sun.lp;

	printf("%6x%4d%6u%6u%3d%4d%3d%5x%4d%5d",
	    ap->a_flag, ap->a_uid,
	    ap->a_pid, lp->l_ppid, lp->l_cpu&0377, ap->a_pri-PZERO,
	    ap->a_nice-NZERO, lp->l_addr, ap->a_size/2, ap->a_rss/2);
	printf(lp->l_wchan ? " %5x" : "      ", (int)lp->l_wchan&0xfffff);
	printf(" %4.4s ", state(ap));
	ptty(ap->a_tty);
	ptime(ap);
}

ptty(tp)
	char *tp;
{

	printf("%-5.5s", tp);
}

ptime(ap)
	struct asav *ap;
{

	printf("%3ld:%02ld", ap->a_cpu / hz, ap->a_cpu % hz);
}

char	*uhdr =
"USER       PID %CPU %MEM   SZ  RSS TTY   STAT  TIME";
upr(sp)
	struct savcom *sp;
{
	register struct asav *ap = sp->ap;
	int vmsize, rmsize;

	vmsize = (ap->a_size + ap->a_tsiz)/2;
	rmsize = ap->a_rss/2;
	if (ap->a_xccount)
		rmsize += ap->a_txtrss/ap->a_xccount/2;
	printf("%-8.8s %5d%5.1f%5.1f%5d%5d",
	    getname(ap->a_uid), ap->a_pid, sp->sun.u_pctcpu, pmem(ap),
	    vmsize, rmsize);
	putchar(' ');
	ptty(ap->a_tty);
	printf(" %4.4s", state(ap));
	ptime(ap);
}

char *vhdr =
"  PID TTY   STAT  TIME SL RE PAGEIN SIZE  RSS  LIM TSIZ TRS %CPU %MEM";
vpr(sp)
	struct savcom *sp;
{
	register struct vsav *vp = sp->sun.vp;
	register struct asav *ap = sp->ap;

	printf("%5u ", ap->a_pid);
	ptty(ap->a_tty);
	printf(" %4.4s", state(ap));
	ptime(ap);
	printf("%3d%3d%7d%5d%5d",
	   ap->a_slptime > 99 ? 99 : ap-> a_slptime,
	   ap->a_time > 99 ? 99 : ap->a_time, vp->v_majflt,
	   ap->a_size/2, ap->a_rss/2);
	if (ap->a_maxrss == (INFINITY/NBPG))
		printf("   xx");
	else
		printf("%5d", ap->a_maxrss/2);
	printf("%5d%4d%5.1f%5.1f",
	   ap->a_tsiz/2, ap->a_txtrss/2, vp->v_pctcpu, pmem(ap));
}

char	*shdr =
"SSIZ   PID TTY   STAT  TIME";
spr(sp)
	struct savcom *sp;
{
	register struct asav *ap = sp->ap;

	if (sflg)
		printf("%4d ", sp->sun.s_ssiz);
	printf("%5u", ap->a_pid);
	putchar(' ');
	ptty(ap->a_tty);
	printf(" %4.4s", state(ap));
	ptime(ap);
}

char *
state(ap)
	register struct asav *ap;
{
	char stat, load, nice, anom;
	static char res[5];

	switch (ap->a_stat) {

	case SSTOP:
		stat = 'T';
		break;

	case SSLEEP:
		if (ap->a_pri >= PZERO)
			if (ap->a_slptime >= MAXSLP)
				stat = 'I';
			else
				stat = 'S';
		else if (ap->a_flag & SPAGE)
			stat = 'P';
		else
			stat = 'D';
		break;

	case SWAIT:
	case SRUN:
	case SIDL:
		stat = 'R';
		break;

	case SZOMB:
		stat = 'Z';
		break;

	default:
		stat = '?';
	}
	load = ap->a_flag & SLOAD ? (ap->a_rss>ap->a_maxrss ? '>' : ' ') : 'W';
	if (ap->a_nice < NZERO)
		nice = '<';
	else if (ap->a_nice > NZERO)
		nice = 'N';
	else
		nice = ' ';
	anom = (ap->a_flag&SUANOM) ? 'A' : ((ap->a_flag&SSEQL) ? 'S' : ' ');
	res[0] = stat; res[1] = load; res[2] = nice; res[3] = anom;
	return (res);
}

/*
 * Given a base/size pair in virtual swap area,
 * return a physical base/size pair which is the
 * (largest) initial, physically contiguous block.
 */
vstodb(vsbase, vssize, dmp, dbp, rev)
	register int vsbase;
	int vssize;
	struct dmap *dmp;
	register struct dblock *dbp;
{
	register int blk = DMMIN;
	register swblk_t *ip = dmp->dm_map;

	if (vsbase < 0 || vsbase + vssize > dmp->dm_size)
		panic("vstodb");
	while (vsbase >= blk) {
		vsbase -= blk;
		if (blk < DMMAX)
			blk *= 2;
		ip++;
	}
	if (*ip <= 0 || *ip + blk > nswap)
		panic("vstodb *ip");
	dbp->db_size = min(vssize, blk - vsbase);
	dbp->db_base = *ip + (rev ? blk - (vsbase + dbp->db_size) : vsbase);
}

/*ARGSUSED*/
panic(cp)
	char *cp;
{

#ifdef DEBUG
	printf("%s\n", cp);
#endif
}

min(a, b)
{

	return (a < b ? a : b);
}

pscomp(s1, s2)
	struct savcom *s1, *s2;
{
	register int i;

	if (uflg)
		return (s2->sun.u_pctcpu > s1->sun.u_pctcpu ? 1 : -1);
	if (vflg)
		return (vsize(s2) - vsize(s1));
	i = s1->ap->a_ttyd - s2->ap->a_ttyd;
	if (i == 0)
		i = s1->ap->a_pid - s2->ap->a_pid;
	return (i);
}

vsize(sp)
	struct savcom *sp;
{
	register struct asav *ap = sp->ap;
	register struct vsav *vp = sp->sun.vp;
	
	if (ap->a_flag & SLOAD)
		return (ap->a_rss +
		    ap->a_txtrss / (ap->a_xccount ? ap->a_xccount : 1));
	return (vp->v_swrss + (ap->a_xccount ? 0 : vp->v_txtswrss));
}

#define	NMAX	8
#define	NUID	2048

char	names[NUID][NMAX+1];

/*
 * Stolen from ls...
 */
char *
getname(uid)
{
	register struct passwd *pw;
	static init;
	struct passwd *getpwent();

	if (uid >= 0 && uid < NUID && names[uid][0])
		return (&names[uid][0]);
	if (init == 2)
		return (0);
	if (init == 0)
		setpwent(), init = 1;
	while (pw = getpwent()) {
		if (pw->pw_uid >= NUID)
			continue;
		if (names[pw->pw_uid][0])
			continue;
		strncpy(names[pw->pw_uid], pw->pw_name, NMAX);
		if (pw->pw_uid == uid)
			return (&names[uid][0]);
	}
	init = 2;
	endpwent();
	return (0);
}

char	*freebase;
int	nleft;

char *
alloc(size)
	int size;
{
	register char *cp;
	register int i;

	if (size > nleft) {
		freebase = (char *)sbrk(i = size > 2048 ? size : 2048);
		if (freebase == 0) {
			fprintf(stderr, "ps: ran out of memory\n");
			exit(1);
		}
		nleft = i - size;
	} else
		nleft -= size;
	cp = freebase;
	for (i = size; --i >= 0; )
		*cp++ = 0;
	freebase = cp;
	return (cp - size);
}

char *
savestr(cp)
	char *cp;
{
	register int len;
	register char *dp;

	len = strlen(cp);
	dp = (char *)alloc(len+1);
	strcpy(dp, cp);
	return (dp);
}

KMlseek(f, a, c)
long a;
{
	if (kflg)
		a = clear(a);
	lseek(f, a, c);
}

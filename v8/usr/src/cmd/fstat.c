/*
 * ps; VAX 4BSD version
 */

#include <stdio.h>
#include <ctype.h>
#include <nlist.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/tty.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/pte.h>
#include <sys/vm.h>
#include <sys/text.h>
#include <sys/stat.h>
#include <math.h>
#include <sys/vlimit.h>
#include <sys/inode.h>
#include <sys/file.h>

#define vprintf	if (vflg) printf
#define WD	-1
#define TEXT	-2

int	pcbpf, nswap, kmem, mem, swap, uid, pid;
int	uflg, fflg, inum, Mdev, mdev, special, vflg, nproc, pflg;
char	*uname;

struct nlist nl[] = {
	{ "_proc" },
#define	X_PROC		0
	{ "_Usrptmap" },
#define	X_USRPTMA	1
	{ "_usrpt" },
#define	X_USRPT		2
	{ "_nswap" },
#define	X_NSWAP		3
	{ "_nproc" },
#define	X_NPROC		4
	{ 0 },
};

struct	proc proc[8], *mproc;		/* 8 = a few, for less syscalls */
struct	pte *Usrptma, *usrpt;

int	paduser1;		/* avoid hardware mem clobbering botch */
union {
	struct	user user;
	char	upages[UPAGES][NBPG];
} user;
int	paduser2;		/* avoid hardware mem clobbering botch */
#define u	user.user

char	*kmemf, *memf, *swapf, *nlistf;

extern int	errno;

main(argc, argv)
char **argv;
{
	register int i, j;
	off_t procp;
	dev_t	dev;

	argv++;
	while (--argc > 0) {
		if (strcmp(*argv, "-v") == 0) {
			vflg++;
			argv++;
			continue;
		} 
		if (strcmp(*argv, "-u") == 0) {
			if (uflg++)
				usage();
			if ((uid = getuname(*(++argv))) < 0) {
				fprintf(stderr, "%s: unknown user\n", *argv);
				exit(1);
			}
			--argc;
			argv++;
			continue;
		} 
		if (strcmp(*argv, "-f") == 0) {
			if (fflg++)
				usage();
			if ((dev = getfname(*(++argv))) < 0) {
				perror(*argv);
				exit(1);
			}
			--argc;
			argv++;
			continue;
		}

		if (strcmp(*argv, "-p") == 0) {
			if (pflg++ || ((pid = Atoi(*(++argv))) <= 0)) {
				usage();
				perror(*argv);
				exit(1);
			}
			--argc;
			argv++;
			continue;
		}

		/* admit missing -u, -f, -p */
		/* it's an expert system! */
		if ((pid = Atoi(*argv)) > 0) {
			if (pflg++)
				usage();
			continue;
		}
		if (fflg && uflg)
			usage();
		if (uflg) {
			/* it must be a file */
			fflg++;
			if ((dev = getfname(*argv)) < 0) {
				perror(*argv);
				exit(1);
			}
			argv++;
			continue;
		}
		if (fflg) {
			/* it must be a user */
			uflg++;
			if ((uid = getuname(*argv)) < 0) {
				fprintf(stderr,
					"%s: unknown user\n", *argv);
				exit(1);
			}
			argv++;
			continue;
		}
		/* !uflg && !fflg -- which is it? */
		if ((dev = getfname(*argv)) >= 0)
			fflg++;		/* could be a file */
		if ((uid = getuname(*argv)) >= 0)
			uflg++;		/* could be a user */
		if ((!uflg ^ !fflg) == 0)
			usage();	/* could be either/neither */
		argv++;
		continue;
	}

	if (fflg) {
		Mdev = major(dev);
		mdev = minor(dev);
	}

	if (chdir("/dev") < 0) {
		perror("/dev");
		exit(1);
	}

	printf("user\t\t  pid\t  fd   major   minor\tinode\n");
	openfiles();
	getkvars();
	procp = getw((off_t) nl[X_PROC].n_value);
	nproc = getw((off_t) nl[X_NPROC].n_value);
	for (i=0; i<nproc; i += 8) {
		lseek(kmem, (long) procp, 0);
		j = nproc - i;
		if (j > 8)
			j = 8;
		j *= sizeof (struct proc);
		if (read(kmem, (char *) proc, j) != j)
			cantread("proc table", kmemf);
		procp += j;
		for (j = j / sizeof (struct proc) - 1; j >= 0; j--) {
			mproc = &proc[j];
			if (mproc->p_stat == 0)
				continue;
			doproc();
		}
	}
	exit(0);
}

long
getw(loc)
	off_t loc;
{
	long word;

	lseek(kmem, (long) loc, 0);
	if (read(kmem, (char *) &word, sizeof (word)) != sizeof (word))
		vprintf("error reading kmem at %x\n", loc);
	return (word);
}

openfiles()
{

	kmemf = "kmem";
	kmem = open(kmemf, 0);
	if (kmem < 0) {
		perror(kmemf);
		exit(1);
	}
	memf = "mem";
	mem = open(memf, 0);
	if (mem < 0) {
		perror(memf);
		exit(1);
	}
	swapf = "drum";
	swap = open(swapf, 0);
	if (swap < 0) {
		perror(swapf);
		exit(1);
	}
}

getkvars()
{
	nlistf = "/unix";
	nlist(nlistf, nl);
	if (nl[0].n_type == 0) {
		fprintf(stderr, "%s: No namelist\n", nlistf);
		exit(1);
	}
	Usrptma = (struct pte *) nl[X_USRPTMA].n_value;
	usrpt = (struct pte *) nl[X_USRPT].n_value;
	lseek(kmem, (long) nl[X_NSWAP].n_value, 0);
	if (read(kmem, (char *) &nswap, sizeof (nswap)) != sizeof (nswap)) {
		cantread("nswap", kmemf);
		exit(1);
	}
}

cantread(what, fromwhat)
	char *what, *fromwhat;
{

	vprintf("fstat: error reading %s from %s", what, fromwhat);
}

doproc()
{
	struct passwd	*getpwuid();

	if (uflg && mproc->p_uid != uid)
		return;
	if (pflg && mproc->p_pid != pid)
		return;
	if (mproc->p_stat != SZOMB && getu() == 0)
		return;
	uname = getpwuid(u.u_uid)->pw_name;
	dotext();
	getf();
}

getu()
{
	struct pte *pteaddr, apte;
	int pad1;	/* avoid hardware botch */
	struct pte arguutl[UPAGES+CLSIZE];
	int pad2;	/* avoid hardware botch */
	register int i;
	int ncl, size, argaddr;

	size = sizeof (struct user);
	if ((mproc->p_flag & SLOAD) == 0) {
		lseek(swap, (long) ctob(mproc->p_swaddr), 0);
		if (read(swap, (char *) &user.user, size) != size) {
			vprintf("fstat: cant read u for pid %d from %s\n",
			    mproc->p_pid, swapf);
			return (0);
		}
		pcbpf = 0;
		argaddr = 0;
		return (1);
	}
	pteaddr = &Usrptma[btokmx(mproc->p_p0br) + mproc->p_szpt - 1];
	lseek(kmem, (long) pteaddr, 0);
	if (read(kmem, (char *) &apte, sizeof(apte)) != sizeof(apte)) {
		vprintf("fstat: can't read indir pte to get u for pid %d from %s\n",
		    mproc->p_pid, swapf);
		return (0);
	}
	lseek(mem,
	    (long) ctob(apte.pg_pfnum+1) - (UPAGES+CLSIZE) * sizeof (struct pte), 0);
	if (read(mem, (char *) arguutl, sizeof(arguutl)) != sizeof(arguutl)) {
		vprintf("fstat: can't read page table for u of pid %d from %s\n",
		    mproc->p_pid, swapf);
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
		lseek(mem, (long) ctob(arguutl[CLSIZE+i].pg_pfnum), 0);
		if (read(mem, user.upages[i], CLSIZE*NBPG) != CLSIZE*NBPG) {
			vprintf("fstat: can't read page %d of u of pid %d from %s\n",
			    arguutl[CLSIZE+i].pg_pfnum, mproc->p_pid, memf);
			return(0);
		}
	}
	return (1);
}

#define	NMAX	8
#define	NUID	2048

dotext()
{
	struct text	text;

	lseek(kmem, (long) mproc->p_textp, 0);
	if (read(kmem, (char *) &text, sizeof(text)) != sizeof(text)) {
		cantread("text table", kmemf);
		return;
	}
	if (text.x_flag == 0)
		return;
	itrans(text.x_iptr, TEXT);
}

itrans(i, fno)
struct inode	*i;
{
	struct inode	inode;
	dev_t	idev;

	lseek(kmem, (long) i, 0);
	if (read(kmem, (char *) &inode, sizeof(inode))
					!= sizeof(inode)) {
		vprintf("error %d reading inode at %x from kmem\n", errno, i);
		return;
	}
	if (special)
		idev = inode.i_dev;
	else
		idev = inode.i_dev;
	if (fflg && major(idev) != Mdev)
		return;	
	if (fflg && minor(idev) != mdev)
		return;	
	if (inum && inode.i_number != inum)
		return;
	printf("%-16s%5d\t", uname, mproc->p_pid);
	if (fno == WD)
		printf("  wd");
	else if (fno == TEXT)
		printf("text");
	else
		printf("%4d", fno);
	printf("\t%2d\t%2d\t%5d\n", major(inode.i_dev), minor(inode.i_dev),
		inode.i_number);
}

getf()
{
	int	i;
	struct file	file;

	itrans(u.u_cdir, WD);
	for (i = 0; i < NOFILE; i++) {
		if (u.u_ofile[i] == 0)
			continue;
		lseek(kmem, (long) u.u_ofile[i], 0);
		if (read(kmem, (char *) &file, sizeof(file)) != sizeof(file)) {
			cantread("file", kmemf);
			continue;
		}
		itrans(file.f_inode, i);
	}
}

usage()
{
	fputs("usage: fstat [-u user] [-f filename] [-p pid]\n", stderr);
	exit(1);
}

getuname(uname)
char	*uname;
{
	struct passwd	*passwd, *getpwnam();
	
	if ((passwd = getpwnam(uname)) == NULL)
		return(-1);
	return(passwd->pw_uid);
}

getfname(filename)
char	*filename;
{
	struct	stat statbuf;

	if (stat(filename, &statbuf) != 0)
		return(-1);

	/*
	 *	if file is block special, look for open files on it
	 */
	if ((statbuf.st_mode & S_IFMT) != S_IFBLK) {
		inum = statbuf.st_ino;
		return(statbuf.st_dev);
	} else {
		special++;
		inum = 0;
		return(statbuf.st_rdev);
	}
}

Atoi(p)
register char *p;
{
	register int n = 0;

	while(*p >= '0' && *p <= '9')
		n = n*10 + *p++ - '0';
	return(*p ? -n : n);
}
/* @(#)getpwent.c	4.1 (Berkeley) 12/21/80 */

static char PASSWD[]	= "/etc/passwd";
static char EMPTY[] = "";
static FILE *pwf = NULL;
static char line[BUFSIZ+1];
static struct passwd passwd;

setpwent()
{
	if( pwf == NULL )
		pwf = fopen( PASSWD, "r" );
	else
		rewind( pwf );
}

endpwent()
{
	if( pwf != NULL ){
		fclose( pwf );
		pwf = NULL;
	}
}

static char *
pwskip(p)
register char *p;
{
	while( *p && *p != ':' )
		++p;
	if( *p ) *p++ = 0;
	return(p);
}

struct passwd *
getpwent()
{
	register char *p;

	if (pwf == NULL) {
		if( (pwf = fopen( PASSWD, "r" )) == NULL )
			return(0);
	}
	p = fgets(line, BUFSIZ, pwf);
	if (p==NULL)
		return(0);
	passwd.pw_name = p;
	p = pwskip(p);
	passwd.pw_passwd = p;
	p = pwskip(p);
	passwd.pw_uid = atoi(p);
	p = pwskip(p);
	passwd.pw_gid = atoi(p);
	passwd.pw_quota = 0;
	passwd.pw_comment = EMPTY;
	p = pwskip(p);
	passwd.pw_gecos = p;
	p = pwskip(p);
	passwd.pw_dir = p;
	p = pwskip(p);
	passwd.pw_shell = p;
	while(*p && *p != '\n') p++;
	*p = '\0';
	return(&passwd);
}
/* @(#)getpwuid.c	4.1 (Berkeley) 12/21/80 */

struct passwd *
getpwuid(uid)
register uid;
{
	register struct passwd *p;
	struct passwd *getpwent();

	setpwent();
	while( (p = getpwent()) && p->pw_uid != uid );
	endpwent();
	return(p);
}

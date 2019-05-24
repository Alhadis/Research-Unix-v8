#include <sys/param.h>
#include <sys/dir.h>
#include <sys/file.h>
#include <sys/user.h>
#include <sys/inode.h>
#include <sys/proc.h>
#include <sys/pte.h>
#include <sys/text.h>
#include <sys/vm.h>
#include <stdio.h>
#include <ctype.h>
#include <nlist.h>
#include <pwd.h>
#include <sys/stat.h>
#include <ftw.h>

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

#define PROCCHUNK 8		/* a few, for fewer syscalls */
struct	proc proc[PROCCHUNK];
struct	proc *mproc;


int	paduser1;		/* avoid hardware mem clobbering botch */
union {
	struct	user user;
	char	upages[UPAGES][NBPG];
} user;
#define u	user.user
int	paduser2;		/* avoid hardware mem clobbering botch */

#define clear(x) 	((int)x & 0xfffff)

int	nswap;
struct	pte *Usrptma, *usrpt;
int	nproc;

char	*kmemf, *memf, *swapf, *nlistf;
int	kmem, mem, swap;
long	lseek();

int	pcbpf;
int	argaddr;
extern	char _sobuf[];

main(argc, argv)
	char **argv;
{
	register int i, j;
	off_t procp;

	initialize (argc, argv);
	setbuf(stdout, _sobuf);
	openfiles();
	getkvars();
	procp = getw((off_t) nl[X_PROC].n_value);
	nproc = getw((off_t) nl[X_NPROC].n_value);
	for (i=0; i<nproc; i += PROCCHUNK) {
		KMlseek(kmem, (long) (char *)procp, 0);
		j = nproc - i;
		if (j > PROCCHUNK)
			j = PROCCHUNK;
		j *= sizeof (struct proc);
		if (read(kmem, (char *)proc, j) != j)
			cantread("proc table", kmemf);
		procp += j;
		for (j = j / sizeof (struct proc) - 1; j >= 0; j--) {
			mproc = &proc[j];
			if (mproc->p_stat == 0)
				continue;
			if (mproc->p_stat == SZOMB || getu() == 0)
				continue;
			doproc (mproc, &u);
		}
	}
	return 0;
}

getw(loc)
	off_t loc;
{
	long word;

	KMlseek(kmem, (long) loc, 0);
	if (read(kmem, (char *) &word, sizeof (word)) != sizeof (word))
		printf("error reading kmem at %x\n", loc);
	return (word);
}

openfiles()
{

	kmemf = "/dev/kmem";
	kmem = open(kmemf, 0);
	if (kmem < 0) {
		perror(kmemf);
		exit(1);
	}
	memf = "/dev/mem";
	mem = open(memf, 0);
	if (mem < 0) {
		perror(memf);
		exit(1);
	}
	swapf = "/dev/drum";
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
	Usrptma = (struct pte *)nl[X_USRPTMA].n_value;
	usrpt = (struct pte *)nl[X_USRPT].n_value;
	KMlseek(kmem, (long)nl[X_NSWAP].n_value, 0);
	if (read(kmem, (char *) &nswap, sizeof (nswap)) != sizeof (nswap)) {
		cantread("nswap", kmemf);
		exit(1);
	}
}

cantread(what, fromwhat)
	char *what, *fromwhat;
{
	fprintf(stderr, "ps: error reading %s from %s\n", what, fromwhat);
}



getu()
{
	struct pte *pteaddr, apte;
	int pad1;	/* avoid hardware botch */
	struct pte arguutl[UPAGES+CLSIZE];
	int pad2;	/* avoid hardware botch */
	register int i;
	int ncl, size;

	size = sizeof (struct user);
	if ((mproc->p_flag & SLOAD) == 0) {
		lseek(swap, (long) ctob(mproc->p_swaddr), 0);
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
	KMlseek(kmem, (long)pteaddr, 0);
	if (read(kmem, (char *)&apte, sizeof(apte)) != sizeof(apte)) {
		printf("ps: cant read indir pte to get u for pid %d from %s\n",
		    mproc->p_pid, swapf);
		return (0);
	}
	lseek(mem,
	    (long) (ctob(apte.pg_pfnum+1) - (UPAGES+CLSIZE) * sizeof (struct pte)),
	    0);
	if (read(mem, (char *)arguutl, sizeof(arguutl)) != sizeof(arguutl)) {
		printf("ps: cant read page table for u of pid %d from %s\n",
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
			printf("ps: cant read page %d of u of pid %d from %s\n",
			    arguutl[CLSIZE+i].pg_pfnum, mproc->p_pid, memf);
			return(0);
		}
	}
	return (1);
}

KMlseek(f, a, c)
	long a;
{
	lseek(f, a, c);
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

#define new(type) ((type *) alloc (sizeof (type)))

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

/*
 *	read /dev, and record device names, block/character, and
 *	major/minor devices.
 */

#define DEVDIR "/dev"

struct devdesc {
	struct devdesc *next;
	char *name;
	int mode;
	dev_t dev;
} *devhead;

int
tryfile (name, p, code)
	char *name;
	register struct stat *p;
	int code;
{
	if (code == FTW_F) {
		register int mode = p->st_mode & S_IFMT;
		if (mode == S_IFBLK || mode == S_IFCHR) {
			register struct devdesc *dp;
			dp = new (struct devdesc);
			dp->name = savestr (name + sizeof (DEVDIR));
			dp->mode = mode;
			dp->dev = p->st_rdev;
			dp->next = devhead;
			devhead = dp;
		}
	}
	return 0;
}

struct procdesc {
	int pid;
	struct procdesc *next;
} *prochead;

struct pdevdesc {
	dev_t dev;
	int mode;
	ino_t ino;
	struct pdevdesc *next;
} *pdevhead;

initialize(c, v)
	int c;
	char **v;
{
	register int n;

	for (n = 1; n < c; n++) {
		if (isnumber (v[n])) {
			register struct procdesc *p;
			p = new (struct procdesc);
			p->pid = atoi (v[n]);
			p->next = prochead;
			prochead = p;
		} else {
			struct stat sb;
			if (stat (v[n], &sb) < 0)
				perror (v[n]);
			else {
				register int mode;
				register struct pdevdesc *pd;
				pd = new (struct pdevdesc);
				mode = sb.st_mode & S_IFMT;
				pd->mode = mode;
				if (mode == S_IFBLK || mode == S_IFCHR) {
					pd->dev = sb.st_rdev;
					pd->ino = 0;
				} else {
					pd->dev = sb.st_dev;
					pd->ino = sb.st_ino;
				}
				pd->next = pdevhead;
				pdevhead = pd;
			}
		}
	}

	if (pdevhead == NULL)
		ftw (DEVDIR, tryfile, 3);
}

int
isnumber (p)
	register char *p;
{
	while (isdigit (*p))
		p++;
	return *p == '\0';
}

/*
 *	Here, we handle a process.
 *
 *	Everything that has come before is essentially independent
 *	of what we are doing with the particular process.
 *
 *	If pdevhead is set, we are just going to print process numbers.
 *	The global variable "found" will greatly assist us in this:
 *	it will be set nonzero as soon as we decide to print this process.
 */

int found;

#undef u

void
printdev (str, dev, mode, rmode, ino)
	char *str;
	dev_t dev;
	int mode, rmode;
	ino_t ino;
{
	register struct devdesc *dp;
	register struct pdevdesc *pd;

	mode &= IFMT;
	rmode &= IFMT;

	if (pdevhead) {
		if (!found) {
			for (pd = pdevhead; pd; pd = pd->next) {
				if (dev == pd->dev && (pd->ino ?
				    (ino == pd->ino && rmode == pd->mode) :
				    (mode == pd->mode))) {
					found = 1;
					return;
				}
			}
		}
		return;
	}
		
	for (dp = devhead; dp; dp = dp->next) {
		if (mode == dp->mode && dev == dp->dev) {
			if (*str)
				printf ("%s ", str);
			if (ino)
				printf ("%d ", ino);
			if (*str)
				printf ("on ");
			printf ("%s", dp->name);
			return;
		}
	}

	printf ("(%d/%d)", major (dev), minor (dev));
}

struct inode *
getinode (ip, pid)
	struct inode *ip;
{
	static struct inode ibuf;
	KMlseek (kmem, (long) ip, 0);
	if (read (kmem, &ibuf, sizeof (ibuf)) != sizeof (ibuf)) {
		fprintf (stderr, "proc %d: can't read inode at %lx\n", pid, ip);
		return NULL;
	}
	return &ibuf;
}

void
doinode (str, ip, pid)
	register char *str;
	register struct inode *ip;
{
	int mode;

	if (ip && (ip = getinode (ip, pid))) {

		if (pdevhead) {
			if (found)
				return;
		} else
			printf ("\t%s: ", str);

		mode = ip->i_mode & IFMT;
		switch (mode) {

		case IFDIR:
			printdev ("dir", ip->i_dev, IFBLK, mode, ip->i_number);
			break;
		
		case IFREG:
			printdev ("file", ip->i_dev, IFBLK, mode, ip->i_number);
			break;
		
		case IFLNK:
			printdev ("slink", ip->i_dev, IFBLK, mode, ip->i_number);
			break;
		
		case IFBLK:
		case IFCHR:
			printdev ("", ip->i_un.i_rdev, ip->i_mode, mode, (ino_t) 0);
			break;
		
		default:
			fprintf (stderr, "??? mode %o", ip->i_mode);
			break;
		}
		if (pdevhead == NULL)
			printf ("\n");
	}

}

void
dotext (tp, pid)
	register struct text *tp;
{
	if (tp) {
		struct text t;
		KMlseek (kmem, (long) tp, 0);
		if (read (kmem, &t, sizeof (t)) != sizeof (t)) {
			fprintf (stderr, "can't read file at %lx\n", tp);
			return;
		}
		doinode ("prog", t.x_iptr, pid);
	}
}

void
dofile (n, fp)
	int n;
	register struct file *fp;
{
	if (fp) {
		struct file f;
		char buf[100];

		KMlseek (kmem, (long) fp, 0);
		if (read (kmem, &f, sizeof (f)) != sizeof (f)) {
			fprintf (stderr, "can't read file at %lx\n", fp);
			return;
		}
		sprintf (buf, "fd %d at %ld", n, f.f_offset);
		doinode (buf, f.f_inode);
	}
}

doproc (p, u)
	register struct proc *p;
	register struct user *u;
{
	struct inode *ip;
	register int i;

	found = 0;

	/* handle only selected processes if asked */
	if (prochead) {
		register struct procdesc *pd;

		pd = prochead;
		while (pd && pd->pid != p->p_pid)
			pd = pd->next;
		if (pd == NULL)
			return;
	}

	if (pdevhead == NULL)
		printf ("process %d, user %s\n", p->p_pid, getname(p->p_uid));

	dotext (p->p_textp, p->p_pid);
	doinode ("cdir", u->u_cdir, p->p_pid);

	if (!found)
		doinode ("rdir", u->u_rdir, p->p_pid);

	for (i = 0; i < NOFILE && !found; i++)
		dofile (i, u->u_ofile[i]);

	if (pdevhead) {
		if (found)
			printf ("%d\n", p->p_pid);
	} else {
		printf ("\n");
		fflush (stdout);
	}
}

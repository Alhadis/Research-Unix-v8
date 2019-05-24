static char *sccsid = "@(#)icheck.c	4.1 (Berkeley) 10/1/80";
#define	NI	4
#define	NB	500
#define	BITS	8
#define	MAXFN	500

#ifndef STANDALONE
#include <stdio.h>
#endif
#include <sys/param.h>
#include <sys/inode.h>
#include <sys/ino.h>
#include <sys/fblk.h>
#include <sys/filsys.h>
#include <sys/stat.h>

#define	BITFSBIT	64	/* should be in param.h */
#define	BIGINOPB	INOPB(BITFSBIT)
#define	BIGBSIZE	BSIZE(BITFSBIT)
#define	BIGNINDIR	NINDIR(BITFSBIT)

struct	filsys	sblock;
struct	stat	status;
#define	dev	status.st_rdev
struct	dinode	itab[BIGINOPB*NI];
daddr_t	iaddr[NADDR];
daddr_t	blist[NB];
char	*bmap;

int	sflg;
int	mflg;
int	dflg;
int	fi;
ino_t	ino;
int	bigflag;

ino_t	nrfile;
ino_t	ndfile;
ino_t	nbfile;
ino_t	ncfile;
ino_t	nlfile;

daddr_t	ndirect;
daddr_t	nindir;
daddr_t	niindir;
daddr_t	niiindir;
daddr_t	nfree;
daddr_t	ndup;

int	nerror;

long	atol();
daddr_t	alloc();
#ifndef STANDALONE
char	*malloc();
#endif

main(argc, argv)
char *argv[];
{
	register i;
	long n;

	blist[0] = -1;
#ifndef STANDALONE
	while (--argc) {
		argv++;
		if (**argv=='-')
		switch ((*argv)[1]) {
		case 'd':
			dflg++;
			continue;


		case 'm':
			mflg++;
			continue;

		case 's':
			sflg++;
			continue;

		case 'b':
			for(i=0; i<NB; i++) {
				n = atol(argv[1]);
				if(n == 0)
					break;
				blist[i] = n;
				argv++;
				argc--;
			}
			blist[i] = -1;
			continue;

		case 'B':
			bigflag = BITFSBIT;
			continue;

		default:
			printf("Bad flag\n");
		}
		check(*argv);
	}
#else
	{
		static char fname[128];

		printf("File: ");
		gets(fname);
		check(fname);
	}
#endif
	return(nerror);
}

check(file)
char *file;
{
	register i, j;
	ino_t mino;
	daddr_t d;
	long n;

	fi = open(file, sflg?2:0);
	if (fi < 0) {
		printf("cannot open %s\n", file);
		nerror |= 04;
		return;
	}
#ifndef STANDALONE
	if (fstat(fi, &status) < 0) {
		printf("cannot fstat %s\n", file);
		nerror |= 04;
		close(fi);
		return;
	}
	if ((status.st_mode & S_IFMT) == S_IFREG)
		dev = makedev(0, bigflag);
#else
	dev = makedev(0, BITFSBIT);	/* hack */
#endif
	printf("%s:\n", file);
	nrfile = 0;
	ndfile = 0;
	ncfile = 0;
	nbfile = 0;
	nlfile = 0;

	ndirect = 0;
	nindir = 0;
	niindir = 0;
	niiindir = 0;

	ndup = 0;
#ifndef STANDALONE
	sync();
#endif
	bread((daddr_t)1, (char *)&sblock, sizeof(sblock));
	mino = ((int)sblock.s_isize-2) * INOPB(dev);
	ino = 0;
	n = (sblock.s_fsize - (int)sblock.s_isize + BITS-1) / BITS;
	if (n != (unsigned)n) {
		printf("Check fsize and isize: %ld, %u\n",
		   sblock.s_fsize, (int)sblock.s_isize);
	}
#ifdef STANDALONE
	bmap = NULL;
#else
	bmap = malloc((unsigned)n);
#endif
	if (bmap==NULL) {
		printf("Not enough core; duplicates unchecked\n");
		dflg++;
		sflg = 0;
	}
	if(!dflg)
	for(i=0; i<(unsigned)n; i++)
		bmap[i] = 0;
	for(i=2;; i+=NI) {
		if(ino >= mino)
			break;
		bread((daddr_t)i, (char *)itab, BSIZE(dev)*NI);
		for(j=0; j<INOPB(dev)*NI; j++) {
			if(ino >= mino)
				break;
			ino++;
			pass1(&itab[j]);
		}
	}
	ino = 0;
#ifndef STANDALONE
	sync();
#endif
	bread((daddr_t)1, (char *)&sblock, sizeof(sblock));
	if (sflg) {
		makefree();
		close(fi);
#ifndef STANDALONE
		if (bmap)
			free(bmap);
#endif
		return;
	}
	nfree = 0;
	while(n = alloc()) {
		if (chk(n, "free"))
			break;
		nfree++;
	}
	close(fi);
#ifndef STANDALONE
	if (bmap)
		free(bmap);
#endif

	i = nrfile + ndfile + ncfile + nbfile + nlfile;
#ifndef STANDALONE
	printf("files %6u (r=%u,d=%u,b=%u,c=%u,l=%u)\n",
		i, nrfile, ndfile, nbfile, ncfile, nlfile);
#else
	printf("files %u (r=%u,d=%u,b=%u,c=%u,l=%u)\n",
		i, nrfile, ndfile, nbfile, ncfile, nlfile);
#endif
	n = ndirect + nindir + niindir + niiindir;
#ifdef STANDALONE
	printf("used %ld (i=%ld,ii=%ld,iii=%ld,d=%ld)\n",
		n, nindir, niindir, niiindir, ndirect);
	printf("free %ld\n", nfree);
#else
	printf("used %7ld (i=%ld,ii=%ld,iii=%ld,d=%ld)\n",
		n, nindir, niindir, niiindir, ndirect);
	printf("free %7ld\n", nfree);
#endif
	if(!dflg) {
		n = 0;
		for(d=(int)sblock.s_isize; d<sblock.s_fsize; d++)
			if(!duped(d)) {
				if(mflg)
					printf("%ld missing\n", d);
				n++;
			}
		printf("missing%5ld\n", n);
	}
}

pass1(ip)
register struct dinode *ip;
{
	daddr_t ind1[BIGNINDIR];
	daddr_t ind2[BIGNINDIR];
	daddr_t ind3[BIGNINDIR];
	register i, j;
	int k, l;

	i = ip->di_mode & IFMT;
	if(i == 0) {
		sblock.s_tinode++;
		return;
	}
	if(i == IFCHR) {
		ncfile++;
		return;
	}
	if(i == IFBLK) {
		nbfile++;
		return;
	}
	if(i == IFDIR)
		ndfile++;
	else if(i == IFREG)
		nrfile++;
	else if(i == IFLNK)
		nlfile++;
	else {
		printf("bad mode %u\n", ino);
		return;
	}
	l3tol(iaddr, ip->di_addr, NADDR);
	for(i=0; i<NADDR; i++) {
		if(iaddr[i] == 0)
			continue;
		if(i < NADDR-3) {
			ndirect++;
			chk(iaddr[i], "data (small)");
			continue;
		}
		nindir++;
		if (chk(iaddr[i], "1st indirect"))
				continue;
		bread(iaddr[i], (char *)ind1, BSIZE(dev));
		for(j=0; j<NINDIR(dev); j++) {
			if(ind1[j] == 0)
				continue;
			if(i == NADDR-3) {
				ndirect++;
				chk(ind1[j], "data (large)");
				continue;
			}
			niindir++;
			if(chk(ind1[j], "2nd indirect"))
				continue;
			bread(ind1[j], (char *)ind2, BSIZE(dev));
			for(k=0; k<NINDIR(dev); k++) {
				if(ind2[k] == 0)
					continue;
				if(i == NADDR-2) {
					ndirect++;
					chk(ind2[k], "data (huge)");
					continue;
				}
				niiindir++;
				if(chk(ind2[k], "3rd indirect"))
					continue;
				bread(ind2[k], (char *)ind3, BSIZE(dev));
				for(l=0; l<NINDIR(dev); l++)
					if(ind3[l]) {
						ndirect++;
						chk(ind3[l], "data (garg)");
					}
			}
		}
	}
}

chk(bno, s)
daddr_t bno;
char *s;
{
	register n;

	if (bno<(int)sblock.s_isize || bno>=sblock.s_fsize) {
		printf("%ld bad; inode=%u, class=%s\n", bno, ino, s);
		return(1);
	}
	if(duped(bno)) {
		printf("%ld dup; inode=%u, class=%s\n", bno, ino, s);
		ndup++;
	}
	for (n=0; blist[n] != -1; n++)
		if (bno == blist[n])
			printf("%ld arg; inode=%u, class=%s\n", bno, ino, s);
	return(0);
}

duped(bno)
daddr_t bno;
{
	daddr_t d;
	register m, n;

	if(dflg)
		return(0);
	d = bno - (int)sblock.s_isize;
	m = 1 << (d%BITS);
	n = (d/BITS);
	if(bmap[n] & m)
		return(1);
	bmap[n] |= m;
	return(0);
}

daddr_t
alloc()
{
	daddr_t bno;
	union {
		char	data[BIGBSIZE];
		struct	fblk fb;
	} buf;
	register int i, j;
	register long *p;

	sblock.s_tfree--;
	if (BITFS(dev)) {
		p = sblock.s_bfree;
		for(i = 0; i < BITMAP && !*p; i++, p++)
			;
		if(i >= BITMAP)
			return (0);
		bno = sblock.s_isize + 32 * i;
		for(j = 0; j < 32; j++)	/* BITS PER LONG */
			if(*p & (1 << j))
				break;
		if(j >= 32)
			return (0);
		bno += j;
		if(bno >= sblock.s_fsize)
			return (0);
		*p &= ~(1 << j);
		return (bno);
	}
	if (sblock.s_nfree<=0)
		return(0);
	if (sblock.s_nfree>NICFREE) {
		printf("Bad free list, s.b. count = %d\n", sblock.s_nfree);
		return(0);
	}
	bno = sblock.s_free[--sblock.s_nfree];
	sblock.s_free[sblock.s_nfree] = (daddr_t)0;
	if(bno == 0)
		return(bno);
	if(sblock.s_nfree <= 0) {
		bread(bno, buf.data, BSIZE(dev));
		sblock.s_nfree = buf.fb.df_nfree;
		if (sblock.s_nfree<0 || sblock.s_nfree>NICFREE) {
			printf("Bad free list, entry count of block %ld = %d\n",
				bno, sblock.s_nfree);
			sblock.s_nfree = 0;
			return(0);
		}
		for(i=0; i<NICFREE; i++)
			sblock.s_free[i] = buf.fb.df_free[i];
	}
	return(bno);
}

bfree(bno)
daddr_t bno;
{
	union {
		char	data[BIGBSIZE];
		struct	fblk fb;
	} buf;
	int i;

	sblock.s_tfree++;
	if (BITFS(dev)) {
		bno -= sblock.s_isize;
		sblock.s_bfree[bno/32] |= (1 << (bno % 32));
		return;
	}
	if(sblock.s_nfree >= NICFREE) {
		for(i=0; i<BSIZE(dev); i++)
			buf.data[i] = 0;
		buf.fb.df_nfree = sblock.s_nfree;
		for(i=0; i<NICFREE; i++)
			buf.fb.df_free[i] = sblock.s_free[i];
		bwrite(bno, buf.data);
		sblock.s_nfree = 0;
	}
	sblock.s_free[sblock.s_nfree] = bno;
	sblock.s_nfree++;
}

bread(bno, buf, cnt)
daddr_t bno;
char *buf;
{
	register i;

	lseek(fi, bno*BSIZE(dev), 0);
	if (read(fi, buf, cnt) != cnt) {
		printf("read error %ld\n", bno);
		if (sflg) {
			printf("No update\n");
			sflg = 0;
		}
		for(i=0; i<BSIZE(dev); i++)
			buf[i] = 0;
	}
}

bwrite(bno, buf)
daddr_t bno;
char	*buf;
{

	lseek(fi, bno*BSIZE(dev), 0);
	if (write(fi, buf, BSIZE(dev)) != BSIZE(dev))
		printf("write error %ld\n", bno);
}

makefree()
{
	char flg[MAXFN];
	int adr[MAXFN];
	register i, j;
	daddr_t f, d;
	int m, n;

	n = sblock.s_n;
	if(n <= 0 || n > MAXFN)
		n = MAXFN;
	sblock.s_n = n;
	m = sblock.s_m;
	if(m <= 0 || m > sblock.s_n)
		m = 3;
	sblock.s_m = m;

	for(i=0; i<n; i++)
		flg[i] = 0;
	i = 0;
	for(j=0; j<n; j++) {
		while(flg[i])
			i = (i+1)%n;
		adr[j] = i+1;
		flg[i]++;
		i = (i+m)%n;
	}

	if (!BITFS(dev))
		sblock.s_nfree = 0;
	sblock.s_ninode = 0;
	sblock.s_flock = 0;
	sblock.s_ilock = 0;
	sblock.s_fmod = 0;
	sblock.s_ronly = 0;
#ifndef STANDALONE
	time(&sblock.s_time);
#endif

	if (!BITFS(dev))
		bfree((daddr_t)0);
	sblock.s_tfree = 0;
	sblock.s_tinode = 0;
	d = sblock.s_fsize-1;
	while(d%sblock.s_n)
		d++;
	for(; d > 0; d -= sblock.s_n)
	for(i=0; i<sblock.s_n; i++) {
		f = d - adr[i];
		if(f < sblock.s_fsize && f >= (int)sblock.s_isize)
			if(!duped(f))
				bfree(f);
	}
	bwrite((daddr_t)1, (char *)&sblock);
#ifndef STANDALONE
	sync();
#endif
	return;
}

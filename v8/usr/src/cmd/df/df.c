static	char *sccsid = "@(#)df.c	4.6 (Berkeley) 7/8/81";
#include <stdio.h>
#include <fstab.h>
#include <sys/param.h>
#include <sys/filsys.h>
#include <sys/fblk.h>
#include <sys/stat.h>
/*
 * df
 */

#define NFS	32	/* Max number of filesystems */

struct mtab {
	char path[FSNMLG];
	char spec[FSNMLG];
} mtab[NFS];
struct stat stb;
int dev;
#define	L10BS	6
#define	L10IS	5
#define PCTFW	3
int	DEVNMLG;		/* length of longest device name */
int	DIRNMLG;		/* length of longest mount point name */

char *mpath();

daddr_t	blkno	= 1;

int	lflag;
int	iflag;

struct	filsys sblock;

int	fi;
daddr_t	alloc();

main(argc, argv)
register int argc;
register char **argv;
{
	register int i;
	register int r = 0;

	while (argc >= 1 && argv[1][0]=='-') {
		switch(argv[1][1]) {

		case 'l':
			lflag++;
			break;

		case 'i':
			iflag++;
			break;

		default:
			fprintf(stderr, "usage: df [-i] [-l] [filsys...]\n");
			exit(0);
		}
		argc--, argv++;
	}

	if ((i=open("/etc/mtab", 0)) >= 0) {
		r = read(i, mtab, sizeof mtab);	/* Probably returns short */
		(void) close(i);
		r /= sizeof mtab[0];
	}
	devlen(r);	/* reads in all of /etc/fstab, too */
	printf("%-*.*s %-*.*s %*.*s %*.*s %*.*s",
		DIRNMLG, DIRNMLG, "dir",
		DEVNMLG, DEVNMLG, "dev",
		L10BS, L10BS, "kbytes",
		L10BS, L10BS, "used",
		L10BS, L10BS, "free");
	if (lflag)
		printf(" %*.*s", L10BS, L10BS, "hardway");
	printf(" %*.*s", PCTFW + 1, PCTFW + 1, "%use");
	if (iflag)
		printf(" %*.*s %*.*s %*.*s",
			L10IS, L10IS, "iused",
			L10IS, L10IS, "ifree",
			PCTFW + 1, PCTFW + 1, "%ino");
	putchar('\n');
	if(argc <= 1) {
		for (i = 0; i < NFS && mtab[i].spec[0]; ++i)
			dfree(mtab[i].path);
		return (0);
	}

	for(i=1; i<argc; i++)
		dfree(argv[i]);
	return (0);
}

dfree(file)
char *file;
{
	register daddr_t i;
	register char	*mp;
	long	blocks;
	long	free;
	long	used;
	long	hardway;
	struct	stat stbuf;
	static char specbuf[FSNMLG + sizeof "/dev/"] = "/dev/";

	if(stat(file, &stbuf) == 0 && (stbuf.st_mode&S_IFMT) == S_IFDIR)
	{
		struct stat mstbuf;

		for (i = 0; i < NFS && mtab[i].spec[0]; ++i)
		{
			strcpy(&specbuf[5], mtab[i].spec);
			if(!stat(specbuf, &mstbuf) && mstbuf.st_rdev == stbuf.st_dev)
			{
				file = specbuf;
				break;
			}
		}
		if (i == NFS || mtab[i].spec[0] == '\0')
		{
			fprintf(stderr, "%s mounted on unknown device\n", file);
			return;
		}
	}
	else
	if (strncmp("/dev/", file, sizeof "/dev/" - 1) != 0)
		strcpy(&specbuf[5], file), file = specbuf;
	fi = open(file, 0);
	if(fi < 0)
	{
		fprintf(stderr,"cannot open %s\n", file);
		return;
	}
	fstat(fi, &stb);
	dev = stb.st_rdev;
	if (lflag)
		sync();
	bread(1L, (char *)&sblock, sizeof(sblock));
	blocks = (long) sblock.s_fsize - (long)sblock.s_isize;
	free = sblock.s_tfree;
	used = blocks - free;
	if(BITFS(dev)) {
		blocks *= BSIZE(dev) / BSIZE(0);
		free *= BSIZE(dev) / BSIZE(0);
		used *= BSIZE(dev) / BSIZE(0);
	}
	printf("%-*.*s %-*.*s %*ld %*ld %*ld",
		DIRNMLG, DIRNMLG, mp = mpath(file),
		DEVNMLG, DEVNMLG, file + sizeof "/dev",
		L10BS, blocks, L10BS, used, L10BS, free);

	if (lflag) {
		hardway = 0;
		if(BITFS(dev))
			hardway = alloc();
		else
			while(alloc())
				hardway++;
		printf(" %*ld", L10BS, free = hardway);
	}
	printf(" %*.0f%%", 
		PCTFW, blocks == 0 ?
		0.0 : (double) used / (double) blocks * 100.0);
	if (iflag) {
		int inodes = (sblock.s_isize - 2) * INOPB(dev);
		used = inodes - sblock.s_tinode;
		printf(" %*ld %*ld %*.0f%%",
			L10IS, used,
			L10IS, sblock.s_tinode, 
			PCTFW, inodes == 0 ?
			0.0 : (double) used / (double) inodes * 100.0);
	}
	printf("\n");
	close(fi);
}

daddr_t
alloc()
{
	int i, j, n;
	daddr_t b;
	struct fblk buf;

	if(!BITFS(dev)) {
		i = --sblock.s_nfree;
		if(i<0 || i>=NICFREE) {
			printf("bad free count, b=%D\n", blkno);
			return(0);
		}
		b = sblock.s_free[i];
		if(b == 0)
			return(0);
		if(b<sblock.s_isize || b>=sblock.s_fsize) {
			printf("bad free block (%D)\n", b);
			return(0);
		}
		if(sblock.s_nfree <= 0) {
			bread(b, (char *)&buf, sizeof(buf));
			blkno = b;
			sblock.s_nfree = buf.df_nfree;
			for(i=0; i<NICFREE; i++)
				sblock.s_free[i] = buf.df_free[i];
		}
		return(b);
	}
	n = 0;
	for(i = 0; i < BITMAP; i++)
		for(j = 0; j < 32; j++)		/* 32: bits per int */
			if(sblock.s_bfree[i] & (1 << j))
				n++;
	return(n * BSIZE(dev) / BSIZE(0));
}

bread(bno, buf, cnt)
daddr_t bno;
char *buf;
{
	register int n;
	extern errno;

	lseek(fi, bno<<BSHIFT(dev), 0);
	if((n=read(fi, buf, cnt)) != cnt) {
		printf("\nread error bno = %ld\n", bno);
		printf("count = %d; errno = %d\n", n, errno);
		exit(0);
	}
}

/*
 * Given a name like /dev/rrp0h, returns the mounted path, like /usr.
 */
char *mpath(file)
char *file;
{
	register int i;

	for (i=0; i<NFS; i++)
		if (eq(file, mtab[i].spec))
			return mtab[i].path;
	return "";
}

eq(f1, f2)
char *f1, *f2;
{
	if (strncmp(f1, "/dev/", 5) == 0)
		f1 += 5;
	if (strncmp(f2, "/dev/", 5) == 0)
		f2 += 5;
	if (strcmp(f1, f2) == 0)
		return 1;
	if (*f1 == 'r' && strcmp(f1+1, f2) == 0)
		return 1;
	if (*f2 == 'r' && strcmp(f1, f2+1) == 0)
		return 1;
	if (*f1 == 'r' && *f2 == 'r' && strcmp(f1+1, f2+1) == 0)
		return 1;
	return 0;
}

mtabcmp(mp0, mp1)
struct mtab *mp0;
struct mtab *mp1;
{
	/*
	 * don't let empty mtab slots sort to the front
	 * as dfree will break
	 * the wrong way to fix it: the whole algorithm is wrong
	 */
	if (mp0->path[0] == 0)
		return (1);
	if (mp1->path[0] == 0)
		return (-1);
	return (strncmp(mp0->path, mp1->path, sizeof (mp0->path)));
}

devlen(r)
register int r;
{
	register struct	fstab	*fsp;
	register int		i;

	DEVNMLG = 0;
	DIRNMLG = 0;
	if (setfsent() == 0)
		perror(FSTAB), exit(1);
	while( (fsp = getfsent()) != 0){
		if (  (strcmp(fsp->fs_type, FSTAB_RW) != 0)
			&&(strcmp(fsp->fs_type, FSTAB_RO) != 0) )
			continue;
		for (i = 0; mtab[i].spec[0]; ++i)
		{
			if (strncmp(mtab[i].spec, fsp->fs_spec + 5,
				sizeof fsp->fs_spec - 5) == 0)
				break;
		}
		if (i == r && i < NFS)
		{
			strncpy(mtab[r].spec, fsp->fs_spec + 5,
				sizeof fsp->fs_spec - 5);
			strncpy(mtab[r].path, fsp->fs_file, sizeof fsp->fs_file);
			++r;
		}
		if (DEVNMLG < (i = strlen(fsp->fs_spec)))
			DEVNMLG = i;
		if (DIRNMLG < (i = strlen(fsp->fs_file)))
			DIRNMLG = i;
	}
	endfsent();
	DEVNMLG -= sizeof "/dev";
	if (DEVNMLG < sizeof "dev" - 1)
		DEVNMLG = sizeof "dev" - 1;
	if (DIRNMLG < sizeof "dir" - 1)
		DIRNMLG = sizeof "dir" - 1;
	qsort(&mtab[0], r, sizeof mtab[0], mtabcmp);
}

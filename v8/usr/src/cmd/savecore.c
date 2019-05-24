/*
 * savecore
 */

#include <stdio.h>
#include <nlist.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/filsys.h>
#include <time.h>

#define	DAY	(60L*60L*24L)
#define	LEEWAY	(3*DAY)

#define eq(a,b) (!strcmp(a,b))
#define ok(number) ((number)&0x7fffffff)

#define SHUTDOWNLOG "/usr/adm/shutdownlog"

struct nlist nl[] = {
#define X_DUMPDEV	0
	{ "_dumpdev" },
#define X_DUMPLO	1
	{ "_dumplo" },
#define X_TIME		2
	{ "_time" },
#define X_PHYSMEM	3
	{ "_physmem" },
#define X_VERSION	4
	{ "_version" },
#define X_PANICSTR	5
	{ "_panicstr" },
#define X_DOADUMP	6
	{ "_doadump" },
	{ 0 },
};

/*
 *	this magic number is found in the kernel at label "doadump"
 *
 *	It is derived as follows:
 *
 *		doadump:	nop			01
 *				nop			01
 *				bicl2 $...		ca
 *							8f
 *
 *	Thus, it is likely to be moderately stable, even across
 *	operating system releases.
 */
#define DUMPMAG 0x8fca0101

char	*system;
char	*dirname;			/* directory to save dumps in */
char	*ddname;			/* name of dump device */
char	*find_dev();
dev_t	dumpdev;			/* dump device */
time_t	dumptime;			/* time the dump was taken */
int	dumplo;				/* where dump starts on dumpdev */
int	physmem;			/* amount of memory in machine */
time_t	now;				/* current date */
char	*path();
unsigned malloc();
char	*ctime();
char	vers[80];
char	core_vers[80];
char	panic_mesg[80];
int	panicstr;
off_t	lseek();
off_t	Lseek();

main(argc, argv)
	char **argv;
	int argc;
{

	if (argc != 2 && argc != 3) {
		fprintf(stderr, "usage: savecore dirname [ system ]\n");
		exit(1);
	}
	dirname = argv[1];
	if (argc == 3)
		system = argv[2];
	if (access(dirname, 2) < 0) {
		perror(dirname);
		exit(1);
	}

	read_kmem();
	if (dump_exists()) {
		(void) time(&now);
		if (system==NULL)
			check_kmem();
		log_entry();
		if ((get_crashtime() || system) && check_space()) {
			save_core();
			clear_dump();
		} else
			exit(1);
	}
	return 0;
}

int
dump_exists()
{
	register int dumpfd;
	int word;

	dumpfd = Open(ddname, 0);
	Lseek(dumpfd, (off_t)(dumplo + ok(nl[X_DOADUMP].n_value)), 0);
	Read(dumpfd, (char *)&word, sizeof word);
	close(dumpfd);
	
	return (word == DUMPMAG);
}

clear_dump()
{
	register int dumpfd;
	int zero = 0;

	dumpfd = Open(ddname, 1);
	Lseek(dumpfd, (off_t)(dumplo + ok(nl[X_DOADUMP].n_value)), 0);
	Write(dumpfd, (char *)&zero, sizeof zero);
	close(dumpfd);
}

char *
find_dev(dev, type)
	register dev_t dev;
	register int type;
{
	register int dfd = Open("/dev", 0);
	struct direct dir;
	struct stat statb;
	static char devname[DIRSIZ + 1];
	char *dp;

	strcpy(devname, "/dev/");
	while(Read(dfd, (char *)&dir, sizeof dir) > 0) {
		if (dir.d_ino == 0)
			continue;
		strncpy(devname + 5, dir.d_name, DIRSIZ);
		devname[DIRSIZ] = '\0';
		if (stat(devname, &statb)) {
			perror(devname);
			continue;
		}
		if ((statb.st_mode&S_IFMT) != type)
			continue;
		if (dev == statb.st_rdev) {
			close(dfd);
			dp = (char *)malloc(strlen(devname)+1);
			strcpy(dp, devname);
			return dp;
		}
	}
	close(dfd);
	fprintf(stderr, "Can't find device %d,%d\n", major(dev), minor(dev));
	exit(1);
	/*NOTREACHED*/
}

read_kmem()
{
	int kmem;
	FILE *fp;
	register char *cp;

	nlist("/unix", nl);
	if (nl[X_DUMPDEV].n_value == 0) {
		fprintf(stderr, "/unix: dumpdev not in namelist\n");
		exit(1);
	}
	if (nl[X_DUMPLO].n_value == 0) {
		fprintf(stderr, "/unix: dumplo not in namelist\n");
		exit(1);
	}
	if (nl[X_TIME].n_value == 0) {
		fprintf(stderr, "/unix: time not in namelist\n");
		exit(1);
	}
	if (nl[X_PHYSMEM].n_value == 0) {
		fprintf(stderr, "/unix: physmem not in namelist\n");
		exit(1);
	}
	if (nl[X_VERSION].n_value == 0) {
		fprintf(stderr, "/unix: version not in namelist\n");
		exit(1);
	}
	if (nl[X_PANICSTR].n_value == 0) {
		fprintf(stderr, "/unix: panicstr not in namelist\n");
		exit(1);
	}
	if (nl[X_DOADUMP].n_value == 0) {
		fprintf(stderr, "/unix: doadump not in namelist\n");
		exit(1);
	}
	kmem = Open("/dev/kmem", 0);
	Lseek(kmem, (long)nl[X_DUMPDEV].n_value, 0);
	Read(kmem, (char *)&dumpdev, sizeof dumpdev);
	Lseek(kmem, (long)nl[X_DUMPLO].n_value, 0);
	Read(kmem, (char *)&dumplo, sizeof dumplo);
	Lseek(kmem, (long)nl[X_PHYSMEM].n_value, 0);
	Read(kmem, (char *)&physmem, sizeof physmem);
	dumplo *= 512L;
	ddname = find_dev(dumpdev, S_IFBLK);
	if ((fp = fdopen(kmem, "r")) == NULL) {
		fprintf(stderr, "Couldn't fdopen kmem\n");
		exit(1);
	}
	if (system)
		return;
	fseek(fp, (long)nl[X_VERSION].n_value, 0);
	fgets(vers, sizeof vers, fp);
	fclose(fp);
}

check_kmem() {
	FILE *fp;
	register char *cp;

	if ((fp = fopen(ddname, "r")) == NULL) {
		perror(ddname);
		exit(1);
	}
	fseek(fp, (off_t)(dumplo+ok(nl[X_VERSION].n_value)), 0);
	fgets(core_vers, sizeof core_vers, fp);
	fclose(fp);
	if (!eq(vers, core_vers))
	{
		register int	i;

		/* this to prevent printing garbage on console */
		for (i = 0; i < sizeof core_vers; ++i)
		{
			if ((core_vers[i] < ' ' && core_vers[i] != '\n') || core_vers[i] > '~')
				break;
		}
		if (core_vers[i])	/* not '\0', must have been garbage */
			fprintf(stderr, "Warning: can't find version in core image.\n");
		else
			fprintf(stderr, "Warning: unix version mismatch:\n\t%sand\n\t%s",
				vers,core_vers);
	}
	fp = fopen(ddname, "r");
	fseek(fp, (off_t)(dumplo + ok(nl[X_PANICSTR].n_value)), 0);
	fread((char *)&panicstr, sizeof panicstr, 1, fp);
	if (panicstr) {
		fseek(fp, dumplo + ok(panicstr), 0);
		cp = panic_mesg;
		do
			*cp = getc(fp);
		while (*cp++);
	}
	fclose(fp);
}

get_crashtime()
{
	int dumpfd;
	time_t clobber = (time_t)0;

	if (system)
		return (1);
	dumpfd = Open(ddname, 0);
	Lseek(dumpfd, (off_t)(dumplo + ok(nl[X_TIME].n_value)), 0);
	Read(dumpfd, (char *)&dumptime, sizeof dumptime);
	close(dumpfd);
	if (dumptime == 0) {
#ifdef DEBUG
		printf("dump time is 0\n");
#endif
		return 0;
	}
	printf("System went down at %s", ctime(&dumptime));
	if (dumptime < now - LEEWAY || dumptime > now + LEEWAY) {
		printf("Dump time is unreasonable\n");
		return 0;
	}
	return 1;
}

char *
path(file)
	char *file;
{
	register char *cp = (char *)malloc(strlen(file) + strlen(dirname) + 2);

	(void) strcpy(cp, dirname);
	(void) strcat(cp, "/");
	(void) strcat(cp, file);
	return cp;
}

check_space()
{
	struct stat dsb;
	register char *ddev;
	register int dfd, n;
	struct filsys sblk;

	if (stat(dirname, &dsb) < 0) {
		perror(dirname);
		exit(1);
	}
	ddev = find_dev(dsb.st_dev, S_IFBLK);
	dfd = Open(ddev, 0);
	Lseek(dfd, 1L<<BSHIFT(dsb.st_dev), 0);
	Read(dfd, (char *)&sblk, sizeof sblk);
	close(dfd);
	if ((n = read_number("minfree")) > (sblk.s_tfree * (BSIZE(dsb.st_dev) / 1024))) {
		fprintf(stderr, "savecore: no dump, need %d have %d on device %d,%d\n",
			n * (BSIZE(dsb.st_dev) / 1024),
			sblk.s_tfree * (BSIZE(dsb.st_dev) / 1024),
			major(dsb.st_dev), minor(dsb.st_dev));
		return (0);
	}
	return (1);
}

read_number(fn)
	char *fn;
{
	char lin[80];
	register FILE *fp;

	if ((fp = fopen(path(fn), "r")) == NULL)
		return 0;
	if (fgets(lin, 80, fp) == NULL) {
		fclose(fp);
		return 0;
	}
	fclose(fp);
	return atoi(lin);
}

save_core()
{
	register int n;
	char buffer[32*NBPG];
	register char *cp = buffer;
	register int ifd, ofd, bounds;
	register FILE *fp;

	bounds = read_number("bounds");
	ifd = Open(system?system:"/unix", 0);
	sprintf(cp, "unix.%d", bounds);
	ofd = Create(path(cp), 0644);
	while((n = Read(ifd, cp, BUFSIZ)) > 0)
		Write(ofd, cp, n);
	close(ifd);
	close(ofd);
	ifd = Open(ddname, 0);
	sprintf(cp, "vmcore.%d", bounds);
	ofd = Create(path(cp), 0644);
	Lseek(ifd, (off_t)dumplo, 0);
	printf("Saving %d bytes of image in vmcore.%d\n", NBPG*physmem, bounds);
	while(physmem > 0) {
		n = Read(ifd, cp, (physmem > 32 ? 32 : physmem) * NBPG);
		Write(ofd, cp, n);
		physmem -= n/NBPG;
	}
	close(ifd);
	close(ofd);
	fp = fopen(path("bounds"), "w");
	fprintf(fp, "%d\n", bounds+1);
	fclose(fp);
}

char *days[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

char *months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep",
	"Oct", "Nov", "Dec"
};

log_entry()
{
	FILE *fp;
	struct tm *tm, *localtime();

	tm = localtime(&now);
	fp = fopen("/usr/adm/shutdownlog", "a");
	if (fp == 0)
		return;
	fseek(fp, 0L, 2);
	fprintf(fp, "%02d:%02d  %s %s %2d, %4d.  Reboot", tm->tm_hour,
		tm->tm_min, days[tm->tm_wday], months[tm->tm_mon],
		tm->tm_mday, tm->tm_year + 1900);
	if (panicstr)
		fprintf(fp, " after panic: %s\n", panic_mesg);
	else
		putc('\n', fp);
	fclose(fp);
}

/*
 * Versions of std routines that exit on error.
 */

Open(name, rw)
	char *name;
	int rw;
{
	int fd;

	if ((fd = open(name, rw)) < 0) {
		perror(name);
		exit(1);
	}
	return fd;
}

Read(fd, buff, size)
	int fd, size;
	char *buff;
{
	int ret;

	if ((ret = read(fd, buff, size)) < 0) {
		perror("read");
		exit(1);
	}
	return ret;
}

off_t
Lseek(fd, off, flag)
	int fd, flag;
	long off;
{
	long ret;

	if ((ret = lseek(fd, off, flag)) == -1L) {
		perror("lseek");
		exit(1);
	}
	return ret;
}

Create(file, mode)
	char *file;
	int mode;
{
	register int fd;

	if ((fd = creat(file, mode)) < 0) {
		perror(file);
		exit(1);
	}
	return fd;
}

Write(fd, buf, size)
	int fd, size;
	char *buf;

{

	if (write(fd, buf, size) < size) {
		perror("write");
		exit(1);
	}
}

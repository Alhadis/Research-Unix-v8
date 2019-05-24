#include "stdio.h"
#include "sys/param.h"
#include "sys/stat.h"
#include "sys/filsys.h"
#include "sys/dir.h"
#include "sys/ino.h"
#include "sys/inode.h"

#define ICOUNT BUFSIZE/sizeof(struct dinode)
struct filsys sb;
struct dinode ib[ICOUNT];
struct direct db[BUFSIZE/sizeof(struct direct)];
struct stat statbuf;
char buf[BUFSIZE];
long where[NADDR];

main(argc, argv)
char **argv;
{
	register int i, j;
	daddr_t size, isize;
	register daddr_t bno, bfree;
	int fd;
	long atol();
	off_t lseek();

	if(argc != 3) {
		fprintf(stderr, "%s: bit-dev no-of-4k-blocks\n", argv[0]);
		exit(1);
	}
	fd = open(argv[1], 2);
	if(fd < 0) {
		perror(argv[1]);
		exit(1);
	}
	if(fstat(fd, &statbuf) < 0) {
		perror(argv[1]);
		exit(1);
	}
	if(!BITFS(statbuf.st_rdev)) {
		fprintf(stderr, "%s device %d, 0%o can't have a 4k system\n",
			argv[1], major(statbuf.st_rdev), minor(statbuf.st_rdev));
		exit(1);
	}
	size = atoi(argv[2]);
	if(size <= 3) {
		fprintf(stderr, "size %ld too small\n", size);
		exit(1);
	}
	if((i = lseek(fd, (size -1) * BUFSIZE, 0)) < 0 ||
		(j = read(fd, buf, BUFSIZE)) != BUFSIZE) {
		fprintf(stderr, "size %ld too large (lseek %d, read %d)\n", size,
			i, j);
		exit(1);
	}
	isize = (size - 2)/(1 + ICOUNT);	/* DOUBTFUL */
	if(isize * ICOUNT > 65536)
		isize = 65536/ICOUNT;	/* 65535 is largest short */
	fprintf(stderr, "%ld 4k blocks, %ld blocks of inodes, %d inodes\n",
		size, isize - 2, ICOUNT * (isize - 2));
	for(bno = 2; bno < isize; bno++) {	/* zero out all inodes */
		lseek(fd, (off_t)(bno * BUFSIZE), 0);
		if (write(fd, (char *)ib, BUFSIZE) != BUFSIZE) {
			perror("inode write");
			exit(1);
		}
	}
	where[0] = isize;
	/* next block has the root directory */
	db[0].d_ino = ROOTINO;
	db[0].d_name[0] = '.';
	db[1].d_ino = ROOTINO;
	db[1].d_name[1] = db[1].d_name[0] = '.';
	if (write(fd, (char *)db, BUFSIZE) != BUFSIZE) {
		perror("root dir write");
		exit(1);
	}
	/* now for its inode */
	ib[ROOTINO-1].di_mode = IFDIR | IREAD | IWRITE | IEXEC;
	ib[ROOTINO-1].di_mode |= (IREAD | IEXEC | IWRITE) >> 3;
	ib[ROOTINO-1].di_mode |= (IREAD | IEXEC | IWRITE) >> 6;
	ib[ROOTINO-1].di_nlink = 2;
	ib[ROOTINO-1].di_uid = ib[ROOTINO-1].di_gid = 0;
	ib[ROOTINO-1].di_size = 2*sizeof(struct direct);
	ltol3(ib[ROOTINO-1].di_addr, where, 1);
	ib[ROOTINO-1].di_atime = ib[ROOTINO-1].di_mtime =
		ib[ROOTINO-1].di_ctime = time(0);
	lseek(fd, (off_t)(2*BUFSIZE), 0);
	if (write(fd, (char *)ib, BUFSIZE) != BUFSIZE) {
		perror("root inode write");
		exit(1);
	}
	/* and now the super block */
	sb.s_isize = isize;
	sb.s_fsize = size;
	sb.s_time = ib[ROOTINO-1].di_atime;
	sb.s_tfree = size - isize - 1;
	sb.s_tinode = (isize - 2) * ICOUNT - 1;
	sb.s_valid = 1;
	for(i = 0; i < BITMAP; i++)
		sb.s_bfree[i] = 0;
	for(bno = isize + 1; bno < size; bno++) {
		bfree = bno - isize;
		sb.s_bfree[bfree>>5] |= (1 << (bfree & 31));
	}
	if((++bfree >> 5) >= BITMAP) {
		fprintf(stderr, "free map won't fit, blk = %ld, wd = %ld\n",
			bno, bfree);
		exit(1);
	}
	lseek(fd, (off_t)BUFSIZE, 0);
	if (write(fd, (char *)&sb, BUFSIZE) != BUFSIZE) {
		perror("superblock write");
		exit(1);
	}
	exit(0);
}

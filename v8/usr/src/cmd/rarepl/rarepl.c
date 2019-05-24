/*% cc -O -o rarepl %
 *
 * replace a bad sector on an RA disk
 */

#include <stdio.h>
#include <sys/param.h>
#include <sys/udaioc.h>
#include "rct.h"

struct ud_unit ch;
int primblk;		/* ugh */

#define	BADRBN	(-1L)

main(argc, argv)
int argc;
char **argv;
{
	int fd;
	int errs = 0;

	if (argc < 3) {
		fprintf(stderr, "usage: %s dev lbn ...\n", argv[0]);
		exit(1);
	}
	if ((fd = open(argv[1], 2)) < 0) {
		perror(argv[1]);
		exit(1);
	}
	if (ioctl(fd, UIOCHAR, &ch) < 0) {
		perror("unit info");
		exit(1);
	}
	argc--;
	argv++;
	while (--argc > 0)
		errs += repl(fd, atol(*++argv));
	exit(errs);
}

/*
 * replace a block
 */

repl(fd, bad)
int fd;
daddr_t bad;
{
	struct ud_repl r;
	char block[RBNSEC];
	struct rbd rct[RBNPB];
	daddr_t rctb;
	int rctoff;
	daddr_t rbn;
	daddr_t pickrbn();

	if (bad >= ch.radsize) {
		fprintf(stderr, "%ld: out of range\n", bad);
		return (1);
	}
	clrbuf(block);
	lseek(fd, (off_t)(bad * RBNSEC), 0);
	read(fd, block, RBNSEC);	/* ignore error on purpose */
	if ((rbn = pickrbn(fd, bad)) == BADRBN) {
		fprintf(stderr, "can't find replacement for %ld\n", bad);
		return (1);
	}
	rctb = (rbn / RBNPB) + RCTTAB;
	rctoff = rbn % RBNPB;
	if (rdrct(fd, rctb, (char *)rct) == 0)
		return (1);
	if (rct[rctoff].rb_code) {
		fprintf(stderr, "rbn %ld in use or bad\n", rbn);
		return (1);
	}
	rct[rctoff].rb_code = primblk ? RALLOC : RALLOC | RALT;
	rct[rctoff].rb_lbn = bad;
	if (wrrct(fd, rctb, (char *)rct) == 0)
		return (1);
	r.lbn = bad;
	r.replbn = rbn;
	r.prim = primblk;
	if (ioctl(fd, UIOREPL, &r) < 0) {
		perror("ioctl");
		return (1);
	}
	lseek(fd, (off_t)(bad * RBNSEC), 0);
	if (write(fd, block, RBNSEC) != RBNSEC) {
		perror("write back");
		return (1);
	}
	return (0);
}

/*
 * find a replacement block
 * remember in primblk whether it's the primary replacement
 */

daddr_t
pickrbn(fd, lbn)
int fd;
daddr_t lbn;
{
	struct rbd rct[RBNPB];
	daddr_t bno, rbn, prbn;
	daddr_t low, high;
	register int i;
	daddr_t size;
	daddr_t rctmax();

	prbn = (lbn / ch.tracksz) * ch.rbns;
	low = high = BADRBN;
	size = rctmax();
	for (bno = RCTTAB, rbn = 0L; bno < size; bno++) {
		if (rdrct(fd, bno, (char *)rct) == 0) {
			rbn += RBNPB;
			continue;
		}
		for (i = 0; i < RBNPB; i++, rbn++) {
			if (rct[i].rb_code == 0) {
				if (rbn < prbn)
					low = rbn;
				else if (high == BADRBN)
					high = rbn;
			}
			else if (rct[i].rb_lbn == lbn
			     &&  rct[i].rb_code != RBAD) {
				rct[i].rb_code = RBAD;
				wrrct(fd, bno, (char *)rct);
			}
		}
	}
	if (low == BADRBN && high == BADRBN)
		return (BADRBN);
	else if (low == BADRBN)
		rbn = high;
	else if (high == BADRBN)
		rbn = low;
	else if (prbn - low < high - prbn)
		rbn = low;
	else
		rbn = high;
	primblk = (rbn == prbn);
	return (rbn);
}

daddr_t
rctmax()
{
	daddr_t nrbns;

	nrbns = (ch.radsize / ch.tracksz) * ch.rbns;
	return ((nrbns / RBNPB) + RCTTAB);
}


rdrct(fd, bno, buf)
int fd;
daddr_t bno;
char *buf;
{
	struct ud_rctbuf b;

	b.buf = buf;
	b.lbn = bno;
	if (ioctl(fd, UIORRCT, &b) < 0) {
		perror("rrct");
		fprintf(stderr, "can't read block %ld of rct\n", bno);
		return (0);
	}
	return (1);
}

wrrct(fd, bno, buf)
int fd;
daddr_t bno;
char *buf;
{
	struct ud_rctbuf b;

	b.buf = buf;
	b.lbn = bno;
	if (ioctl(fd, UIOWRCT, &b) < 0) {
		perror("wrct");
		fprintf(stderr, "can't write block %ld of rct\n", bno);
		return (0);
	}
	return (1);
}

clrbuf(b)
register char *b;
{
	register int i;

	for (i = 0; i < RBNSEC; i++)
		*b++ = 0;
}

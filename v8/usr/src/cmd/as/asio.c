/* Coypright (c) 1980 Regents of the University of California */
static	char sccsid[] = "@(#)asio.c 4.3 8/16/80";
#include <stdio.h>
#include "as.h"
/*
 *	Block I/O routines for logical I/O concurrently in
 *	more than one place in the same file.
 */
int	biofd;			/* file descriptor for block I/O file */
off_t	boffset;		/* physical position in logical file */
BFILE	*biobufs;		/* the block I/O buffers */

#define	error(severity, message) \
	{yyerror(message); if (severity) delexit();}

Flushfield(n)
	register int n;
{
	while (n>0) {
		outb(bitfield);
		bitfield >>= 8;
		n -= 8;
	}
	bitoff=0;
	bitfield=0;
}

/*
 *	Block I/O Routines
 */
bopen(bp, off)
	struct biobuf *bp;
	off_t	off;
{

	bp->b_ptr = bp->b_buf;
	bp->b_nleft = BUFSIZ - off % BUFSIZ;
	bp->b_off = off;
	bp->b_link = biobufs;
	biobufs = bp;
}

int	bwrerror;

bwrite(p, cnt, bp)
	register char *p;
	register int cnt;
	register struct biobuf *bp;
{
	register int put;
	register char *to;

top:
	if (cnt == 0)
		return;
	if (bp->b_nleft) {
		put = bp->b_nleft;
		if (put > cnt)
			put = cnt;
		bp->b_nleft -= put;
		to = bp->b_ptr;
		asm("movc3 r8,(r11),(r7)");
		bp->b_ptr += put;
		p += put;
		cnt -= put;
		goto top;
	}
	if (cnt >= BUFSIZ) {
		if (bp->b_ptr != bp->b_buf)
			bflush1(bp);
		put = cnt - cnt % BUFSIZ;
		if (boffset != bp->b_off)
			lseek(biofd, bp->b_off, 0);
		if (write(biofd, p, put) != put) {
			bwrerror = 1;
			error(1, "Output write error");
		}
		bp->b_off += put;
		boffset = bp->b_off;
		p += put;
		cnt -= put;
		goto top;
	}
	bflush1(bp);
	goto top;
}

bflush()
{
	register struct biobuf *bp;

	if (bwrerror)
		return;
	for (bp = biobufs; bp; bp = bp->b_link)
		bflush1(bp);
}

bflush1(bp)
	register struct biobuf *bp;
{
	register int cnt = bp->b_ptr - bp->b_buf;

	if (cnt == 0)
		return;
	if (boffset != bp->b_off)
		lseek(biofd, bp->b_off, 0);
	if (write(biofd, bp->b_buf, cnt) != cnt) {
		bwrerror = 1;
		error(1, "Output write error");
	}
	bp->b_off += cnt;
	boffset = bp->b_off;
	bp->b_ptr = bp->b_buf;
	bp->b_nleft = BUFSIZ;
}

bflushc(bp, c)
	register struct biobuf *bp;
	char	c;
{

	bflush1(bp);
	bputc(c, bp);
}

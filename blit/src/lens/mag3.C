#include <jerq.h>

magby3(from,to,fjump,tjump,hcount,vcount,bits,valid)
register char *to,*bits;
register unsigned char *from;
register int fjump, tjump, hcount, valid;
int vcount;
{
	register int i,j;
	register char *bitsource;
	register char *bats;
	int excess;
	bats = bits + valid;
	bits += 3;
	i=vcount-1;
	--hcount;
	excess = 3 - valid;
	--valid;
	to -= excess;
	tjump -= excess;
	do {
		bitsource = bats + *(--from) * 3;
		j = valid;
		*(--to) = *(--bitsource);
		if (--j != -1) {
			*(--to) = *(--bitsource);
			if (--j != -1)
				*(--to) = *(--bitsource);
		}
		j = hcount;
		while (--j != -1) {
			bitsource = bits + *(--from) * 3;
			*(--to) = *(--bitsource);
			*(--to) = *(--bitsource);
			*(--to) = *(--bitsource);
		}
		from += fjump;
		to += tjump;
	} while (--i != -1);
}


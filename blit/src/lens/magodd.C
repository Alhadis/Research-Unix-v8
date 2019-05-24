#include <jerq.h>

magbyodd(from,to,fjump,tjump,hcount,vcount,bits,valid,size)
register char *to,*bits;
register unsigned char *from;
register int fjump, size;
int tjump, vcount, hcount, valid;
{
	register int j,k,wcnt;
	register char *bitsource;
	register char odd;
	char *bats;
	int i,excess;
	bats = bits + valid;
	wcnt = size - 1;
	wcnt >>= 1;
	bits += size;
	i=vcount-1;
	--hcount;
	excess = size - valid;
	odd = valid&1;
	--valid;
	valid >>= 1;
	to -= excess;
	tjump -= excess;
	
	do {
		bitsource = bats + *(--from) * size;
		j = valid;
		if (odd)
			goto vmagic;
		do {
			*(--to) = *(--bitsource);
vmagic:
			*(--to) = *(--bitsource);
		}while (--j != -1);
		j = hcount;
		while (--j != -1) {
			bitsource = bits + *(--from) * size;
			k = wcnt;
			goto magic;
			do {
				*(--to) = *(--bitsource);
magic:
				*(--to) = *(--bitsource);
			}while (--k != -1);
		}
		from += fjump;
		to += tjump;
	} while (--i != -1);
}


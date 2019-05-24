#include <jerq.h>

magbyeven(from,to,fjump,tjump,hcount,vcount,bits,valid,size)
register Word *to,*bits;
register unsigned char *from;
register int size;
int fjump, tjump, vcount, hcount, valid;
{
	register int j,k,wcnt;
	register Word *bitsource;
	register char odd,vodd;
	Word *bats;
	int excess,i;
	size >>= 1;
	vodd = size&1;
	wcnt = (size - 1)>>1;
	tjump /= 2;
	valid = (valid + 1)>>1;
	bats = bits + valid;
	excess = size - valid;
	odd = valid&1;
	--valid;
	valid >>= 1;
	bits += size;
	i=vcount-1;
	--hcount;
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
		} while (--j != -1);
		j = hcount;
		while (--j != -1) {
			bitsource = bits + *(--from) * size;
			k = wcnt;
			if (vodd)
				goto magic;
			do {
				*(--to) = *(--bitsource);
magic:
				*(--to) = *(--bitsource);
			} while (--k != -1);
		}
		from += fjump;
		to += tjump;
	} while (--i != -1);
}


#include <jerq.h>

magby2(from,to,fjump,tjump,hcount,vcount,bits)
register Word *to,*bits;
register unsigned char *from;
register int fjump, tjump, hcount;
int vcount;
{
	register int i,j;
	register char odd;
	tjump /= 2;
	i=vcount-1;
	odd = hcount&1;
	--hcount;
	hcount >>=1;
	do {
		j = hcount;
		if (odd)
			goto magic;
		do {
			*(--to) = bits[*(--from)];
magic:
			*(--to) = bits[*(--from)];
		} while (--j != -1);
		from += fjump;
		to += tjump;
	} while (--i != -1);
}


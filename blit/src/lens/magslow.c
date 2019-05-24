#include <jerq.h>

magslow(from,to,fjump,tjump,hcount,vcount,bits,valid,size)
register char *to;
register unsigned char *from;
register int fjump, tjump, hcount, size, valid;
char *bits;
int vcount;
{
	register int i,j,k,m,acc,size8,initial;
	int l, excess8, excess;

	excess = size-valid;
	to -= excess;
	tjump -= excess;
	size8 = size * 8;
	excess8 = excess * 8;
	for (l = vcount; l>0; --l) {
		initial = excess8;
		for (m = hcount; m>0; --m) {
			i = *--from;
			for (j = initial; j < size8; j += 8) {
				acc = 0;
				for (k = 0; k < 8; k++)
					if (i & (1<<((k+j)/size)))
						acc |= (1<<k);
				*--to = acc;
			}
			initial = 0;
		}
		from += fjump;
		to += tjump;
	}
}

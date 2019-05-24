/*	@(#)spellin.c	1.1	*/
#include <stdio.h>
#include "hash.h"

#define S (BYTE*sizeof(long))
#define B (BYTE*sizeof(unsigned))
unsigned tabword;
unsigned short index[NI];
unsigned wp;		/* word pointer*/
int bp =B;	/* bit pointer*/
int extra;

/*	usage: hashin N
	where N is number of words in dictionary
	and standard input contains sorted, unique
	hashed words in octal
*/
main(argc,argv)
char **argv;
{
	long h,k,d;
	register i;
	long count;
	long w;
	long x;
	int t,u;
	extern double huff();
	extern long ftell();
	long seekpt;
	double atof();
	double z;
	double nwords;
	k = 0;
	u = 0;
	if(argc!=2) {
		fprintf(stderr,"spellin: arg count\n");
		exit(1);
	}
	nwords = atof(argv[1]);
	z = huff((1L<<HASHWIDTH)/nwords);
	fprintf(stderr, "spellin: expected code widths = %f", z);
	z += sizeof(tabword)*BYTE/2*(double)(1<<INDEXWIDTH)/nwords;
	fprintf(stderr, " +breakage = %f\n", z); /*t half word per bin */
	whuff();
	seekpt = ftell(stdout);
	fwrite((char*)index, sizeof(*index), NI, stdout); /*dummy data */
	for(count=0; scanf("%lo", &h) == 1; ++count) {
		if((t=h>>(HASHWIDTH-INDEXWIDTH)) != u) {
			if(bp!=B)
				newword();
			bp = B;
			while(u<t)
				index[++u] = wp;
			k =  (long)t<<(HASHWIDTH-INDEXWIDTH);
		}
		d = h-k;
		k = h;
		for(;;) {
			for(x=d;;x/=2) {
				i = encode(x,&w);
				if(i>0)
					break;
			}
			if(i>B) {
				append((unsigned)(w>>(i-B)), B);
				append((unsigned)(w<<(B+B-i)), i-B);
			} else
				append((unsigned)(w<<(B-i)), i);
			d -= x;
			if(d>0)
				extra++;
			else
				break;
		}
	}
	if(bp!=B)
		newword();
	while(++u<NI)
		index[u] = wp;
	newword();	/* padding allows one out-of-bounds fetch */
	newword();
	newword();
	fseek(stdout, seekpt, 0);	/* overwrite dummy data */
	fwrite((char*)index, sizeof(*index), NI, stdout);
	fprintf(stderr, "spellin: %ld items, %d extra, %u words occupied\n",
		count,extra,wp);
	fprintf(stderr, "spellin: %f table bits/item, ", 
		((float)BYTE*wp)*sizeof(tabword)/count);
	fprintf(stderr, "%f table+index bits\n",
		BYTE*((float)wp*sizeof(tabword) + sizeof(index))/count);
	return(0);
}

append(w, i)
register unsigned w;
register i;
{
	for(;;) {
		tabword |= w>>(B-bp);
		i -= bp;
		if(i<0) {
			bp = -i;
			return;
		}
		w <<= bp;
		bp = B;
		newword();
	}
}

newword()
{
	fwrite((char*)&tabword, sizeof(tabword), 1, stdout);
	wp++;
	tabword = 0;
}

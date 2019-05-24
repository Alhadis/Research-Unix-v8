/*	@(#)hashlook.c	1.4	*/
#include <stdio.h>
#include "hash.h"
#include "huff.h"

unsigned *table;
unsigned short index[NI];

#define B (BYTE*sizeof(unsigned))
#define L (BYTE*sizeof(long)-1)
#define MASK (~(1L<<L))

#ifdef pdp11	/*sizeof(unsigned)==sizeof(long)/2 */
#define fetch(wp,bp)\
	(((((long)wp[0]<<B)|wp[1])<<(B-bp))|(wp[2]>>bp))
#else 		/*sizeof(unsigned)==sizeof(long)*/
#define fetch(wp,bp) (bp==B?wp[0]:((wp[0]<<(B-bp))|(wp[1]>>bp)))
#endif

hashlook(s)
char *s;
{
	long h;
	long t;
	register bp;
	register unsigned *wp;
	int i;
	long sum;
	unsigned *tp;

	h = hash(s);
	t = h>>(HASHWIDTH-INDEXWIDTH);
	wp = &table[index[t]];
	tp = &table[index[t+1]];
	bp = B;
	sum = (long)t<<(HASHWIDTH-INDEXWIDTH);
	for(;;) {
		{/*	this block is equivalent to
			 bp -= decode((fetch(wp,bp)>>1)&MASK, &t);*/
			long y;
			long v;
			y = (fetch(wp,bp)>>1) & MASK;
			if(y < cs) {
				t = y >> (L+1-w);
				bp -= w-1;
			}
			else {
				for(bp-=w,v=v0; y>=qcs; y=(y<<1)&MASK,v+=n)
					bp -= 1;
				t = v + (y>>(L-w));
			}
		}
		while(bp<=0) {
			bp += B;
			wp++;
		}
		if(wp>=tp&&(wp>tp||bp<B))
			return(0);
		sum += t;
		if(sum<h)
			continue;
		return(sum==h);
	}
}


prime(argc,argv)
char **argv;
{
	register FILE *f;
	register fd;
	extern char *malloc();
	if(argc <= 1)
		return(0);
#ifndef pdp11
	if(sizeof(long) > sizeof(unsigned))
		abort();	/*wrong fetch macro*/
#endif
#ifdef pdp11	/* because of insufficient address space for buffers*/
	fd = dup(0);
	close(0);
	if(open(argv[1], 0) != 0)
		return(0);
	f = stdin;
	if(rhuff(f)==0
	|| read(fileno(f), (char *)index, NI*sizeof(*index)) != NI*sizeof(*index)
	|| (table = (unsigned*)malloc(index[NI-1]*sizeof(*table))) == 0
	|| read(fileno(f), (char*)table, sizeof(*table)*index[NI-1])
	   != index[NI-1]*sizeof(*table))
		return(0);
	close(0);
	if(dup(fd) != 0)
		return(0);
	close(fd);
#else
	if((f = fopen(argv[1], "ri")) == NULL)
		return(0);
	if(rhuff(f)==0
	|| fread((char*)index, sizeof(*index),  NI, f) != NI
	|| (table = (unsigned*)malloc(index[NI-1]*sizeof(*table))) == 0
	|| fread((char*)table, sizeof(*table), index[NI-1], f)
	   != index[NI-1])
		return(0);
	fclose(f);
#endif
	hashinit();
	return(1);
}

#ifdef	alloc
#undef	alloc
#undef	free
#endif
#ifdef debug
#define ASSERT(p) if(!(p))botch("p");else
botch(s)
char *s;
{
	printf("assertion botched: %s\n",s);
	abort();
}
#else
#define ASSERT(p)
#endif

/*	C storage allocator
 *	first-fit strategy
 *	works with noncontiguous, but monotonically linked, arena
 *	each block is preceded by a ptr to the (pointer of) 
 *	the next following block
 *	blocks are exact number of words long 
 *	aligned to the data type requirements of ALIGN
 *	pointers to blocks must have BUSY bit 0
 *	bit in ptr is 1 for busy, 0 for idle
 *	gaps in arena are merely noted as busy blocks
 *	last block of arena is empty and
 *	has a pointer to first
 *	idle blocks are coalesced during space search
 *
 *	a different implementation may need to redefine
 *	ALIGN, NALIGN, BUSY, INT
 *	where INT is integer type to which a pointer can be cast
*/
#define INT long
#define ALIGN int
#define NALIGN 1
#define WORD sizeof(union store)
#define BUSY 1
#define NULL 0
#define testbusy(p) ((INT)(p)&BUSY)
#define setbusy(p) (union store *)((INT)(p)|BUSY)
#define clearbusy(p) (union store *)((INT)(p)&~BUSY)

union store {
	struct {
		union store *Uptr;
		char * Uproc;	/* pointer to process of allocating guy */
	} u;
	union store *ptr;
	ALIGN dummy[NALIGN];
	int calloc;	/*calloc clears an array of integers*/
};
#define	proc	u.Uproc

char *allocstartp;	/* &end, but passed in because this is in ROM */
char *allocendp;	/* should adjust according to load */
#define	START	allocstartp
#define	FIRSTWORD ((union store *)(START))
#define	LASTWORD ((union store *)(allocendp-4))
static	union store *allocb;	/*arena base*/
#ifdef REALLOC
static	union store *allocx;	/*for benefit of realloc*/
#endif

allocinit(s, e)
	int *s, *e;
{
	allocstartp=(char *)s;
	allocendp=(char *)e;
	FIRSTWORD->ptr = LASTWORD;
	LASTWORD->ptr = (union store *)(START+1);
	allocb = (union store *)START;
}

char *
realalloc(nbytes, whichproc)
	unsigned nbytes;
	char *whichproc;
{
	register union store *p, *q;
	register nw;
	register union store *allocp;
	static int temp;

	nw = (nbytes+WORD+WORD-1)/WORD;
	ASSERT(allock(allocb));
	for(; ; ) {	/* done at most twice */
		p = allocb;
		allocp = allocb;
		for(temp=0; ; ) {
			if(!testbusy(p->ptr)) {
				while(!testbusy((q=p->ptr)->ptr)) {
					ASSERT(q>p);
					p->ptr = q->ptr;
					allocp = p;
				}
				if(q>=p+nw && p+nw>=p)
					goto found;
			}
			q = p;
			p = clearbusy(p->ptr);
			if(p <= q) {
				ASSERT(p==allocb);
				if(p != allocb)
					return(NULL);
				if(++temp > 1)
					break;
			}
		}
		return NULL;	/* get more space, someday */
	}
found:
	allocp = p + nw;
	if(q>allocp) {
#ifdef REALLOC
		allocx = allocp->ptr;
#endif
		allocp->ptr = p->ptr;
	}
	p->ptr = setbusy(allocp);
	/* clear the storage, for jerqs only */
	for(q=p+1; q<p+nw; q++){
		q->ptr=0;
		q->proc=0;	/* cough */
	}
	p->proc = whichproc;
	return((char *)(p+1));
}

char *
alloc(nbytes)
	unsigned nbytes;
{
	return realalloc(nbytes, (char *)0);
}

/*	freeing strategy tuned for LIFO allocation
*/
free(ap)
char *ap;
{
	register union store *p = (union store *)ap-1;

	ASSERT(allock(p));
	ASSERT(testbusy(p->ptr));
	p->ptr = clearbusy(p->ptr);
	p->proc = 0;
	ASSERT(p->ptr > p);
}

/*	free all storage associated with the named process
*/
freeall(whichproc)
	register char *whichproc;
{
	register union store *p, *r;

	if ((p=allocb)->proc == whichproc)	/* first block on chain */
		free((char *)(p+1));
	for(p=allocb; (r=clearbusy(p->ptr)) > p; p=r)	/* rest of chain... */
		if((r->proc == whichproc) && (r != LASTWORD))
			free((char *)(r+1));		/* ...except LASTWORD */
	gcfreeall(whichproc);
}

#ifdef REALLOC
/*	realloc(p, nbytes) reallocates a block obtained from alloc()
 *	and freed since last call of alloc()
 *	to have new size nbytes, and old content
 *	returns new location, or 0 on failure
*/

char *
realloc(pp, nbytes)
char *pp;
unsigned nbytes;
{
	register union store *q;
	register union store *p = (union store *)pp;
	union store *s, *t;
	register unsigned nw;
	unsigned onw;

	ASSERT(allock(p-1));
	if(testbusy(p[-1].ptr))
		free((char *)p);
	onw = p[-1].ptr - p;
	q = (union store *)alloc(nbytes);
	if(q==NULL || q==p)
		return((char *)q);
	ASSERT(q<p||q>p[-1].ptr);
	s = p;
	t = q;
	nw = (nbytes+WORD-1)/WORD;
	if(nw<onw)
		onw = nw;
	while(onw--!=0)
		*t++ = *s++;
	ASSERT(clearbusy(q[-1].ptr)-q==nw);
	if(q<p && q+nw>=p)
		(q+(q+nw-p))->ptr = allocx;
	ASSERT(allock(q-1));
	return((char *)q);
}
#endif

#ifdef debug
allock(q)
union store *q;
{
#ifdef longdebug
	register union store *p, *r;
	int x;
	x = 0;
	p = allocb;
	if(((union store *)START)->ptr==0)
		return(1);
	for( ; (r=clearbusy(p->ptr)) > p; p=r) {
		if(p==q)
			x++;
	}
	return(r==allocb&(x==1|p==q));
#else
	return(q>=allocb);
#endif
}
abort(){
 	printf("abort\n");
	for(;;);
}
#endif

/*      REALLY wimpy storage allocator!
 *  alloc.c        One may not free at all.
 */

static char *allocp;
static int remain=0;
extern char *sbrk();
#define NULL 0

char *alloc(n)
int n;
{
    register char *cp;

    n=(n+3)&(~3);   /* integer number of words... faster? */
    if (remain<n){
	remain=(n+1023)&(~1023);
	allocp=sbrk(remain);}
    if (allocp==NULL) return(NULL);
    remain-=n;
    cp=allocp;
    allocp+=n;
    return(cp);}



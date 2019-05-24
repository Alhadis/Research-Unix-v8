#include "inet.h"
#if NINET > 0

#include "../h/param.h"
#include "../h/stream.h"
#include "../h/conf.h"
#include "../h/inet/in.h"
#include "../h/inet/ip.h"
#include "../h/inet/ip_var.h"
#include "../h/inet/mbuf.h"
#include "sparam.h"

#define NBLOCK (NBLKBIG+NBLK64+NBLK16+NBLK4)

struct block *
bp_get()
{
	register struct block *bp;

	bp = allocb(64);
	if(bp)
		bp->next = 0;
	return(bp);
}

bp_check(bp)
register struct block *bp;
{
	extern struct block cblock[];

	while(bp){
		if(bp < cblock || cblock >= &cblock[NBLOCK])
			panic("bp_check bad bp");
		if(bp->rptr == 0 || bp->wptr == 0 || bp->base == 0 || bp->lim == 0)
			panic("bp_check 0");
		if(bp->rptr >= bp->lim || bp->rptr < bp->base)
			panic("bp_check rptr");
		if(bp->wptr > bp->lim || bp->wptr < bp->base)
			panic("bp_check wptr");
		bp = bp->next;
	}
}

/* given a char *, come up with the block that holds it. yuk. */
int dtom_hits, dtom_misses;
u_char *xfirst16, *xfirst64, *xfirst1024, *xp;
struct block *xbp;

struct block *
bp_dtom(p)
u_char *p;
{
	extern struct block cblock[];
	extern u_char blkdata[];
	register u_char *first16, *first64, *first1024;
	register struct block *bp;

	/* guess. the order of things in blkdata is NBLK4, NBLK16, ... */
	first16 = &blkdata[4 * NBLK4];
	first64 = &first16[16 * NBLK16];
	first1024 = &first64[64 * NBLK64];
	if(p < first16){
		bp = &cblock[(p - blkdata) / 4];
	} else if(p < first64){
		bp = &cblock[NBLK4 + (p - first16) / 16];
	} else if(p < first1024){
		bp = &cblock[NBLK4 + NBLK16 + (p - first64) / 64];
	} else {
		bp = &cblock[NBLK4 + NBLK16 + NBLK64 + (p - first1024) / 1024];
	}
	if(bp->base && bp->lim && bp->rptr && bp->wptr
	   && (p >= bp->base) && (p < bp->lim)){
		dtom_hits++;
		return(bp);
	}
	xfirst16 = first16;
	xfirst64 = first64;
	xfirst1024 = first1024;
	xp = p;
	xbp = bp;
	dtom_misses++;
	for(bp = &cblock[NBLOCK-1]; bp >= &cblock[0]; --bp){
		if(bp->base == 0 || bp->lim == 0 || bp->rptr == 0 || bp->wptr == 0)
			continue;
		if((p >= bp->base) && (p < bp->lim)){
			return(bp);
		}
	}
	panic("bp_dtom");
	/* NOTREACHED */
}

/* bp_pullup: make the first block have at least len bytes */
struct block *
bp_pullup(bp, len)
register struct block *bp;
{
	register struct block *m, *n, *nn;
	int count;

	n = bp;
	if(len > MAXBLEN)
		goto bad;
	m = allocb(MAXBLEN);
	if(m == 0)
		goto bad;
	do{
		count = MIN(BSZ(m) - BLEN(m), len);
		if(count > BLEN(n))
			count = BLEN(n);
		bcopy(n->rptr, m->wptr, (unsigned)count);
		len -= count;
		m->wptr += count;
		n->rptr += count;
		if(BLEN(n))
			break;
		nn = n->next;
		freeb(n);
		n = nn;
	} while(n);
	if(len){
		freeb(m);
		goto bad;
	}
	m->next = n;
	return(m);
bad:
	printf("m_pullup bad\n");
	bp_free(n);
	return(0);
}

bp_free(bp)
register struct block *bp;
{
	register struct block *p;

	while(bp){
		p = bp->next;
/*
		if((bp->rptr >= bp->lim) || (bp->wptr > bp->lim))
			panic("bp_free");
		if((bp->rptr < bp->base) || (bp->wptr < bp->base)){
			printf("bp 0x%x, rptr 0x%x\n", bp, bp->rptr);
			panic("bp_free1");
		}
*/
		freeb(bp);
		bp = p;
	}
}

struct block *
bp_copy(m, off, len)
register struct block *m;
int off;
register int len;
{
	register struct block *n, **np;
	struct block *top;

	if(len == 0)
		return(0);
	if(off < 0 || len < 0)
		panic("m_copy");
	while(off > 0){
		if(m == 0)
			panic("m_copy 1");
		if(off < BLEN(m))
			break;
		off -= BLEN(m);
		m = m->m_next;
	}
	np = &top;
	top = 0;
	while(len > 0){
		if(m == 0)
			panic("m_copy 2");
#undef FAKE
#ifdef FAKE
		n = allocb(1);	/* fake block, will adjust pointers */
#else
		n = allocb(len);
#endif FAKE
		*np = n;
		if(n == 0)
			goto nospace;
		n->next = 0;
#ifdef FAKE
		/* fake them up */
		n->rptr = mtod(m, u_char *)+off;
		n->wptr = n->rptr + MIN(len, BLEN(m) - off);
		/* pu meht ekaf */
#else
		n->wptr += MIN(len, BLEN(m) - off);
		bcopy(mtod(m, caddr_t)+off, mtod(n, caddr_t),
			(unsigned)BLEN(n));
#endif FAKE
		len -= BLEN(n);
		off = 0;
		m = m->next;
		np  = &n->m_next;
	}
	return(top);
nospace:
	m_freem(top);
	return(0);
}

m_adj(mp, len)
struct block *mp;
register int len;
{
	register struct block *m, *n;

	if((m = mp) == NULL)
		return;
	if(len >= 0){
		while(m && len > 0){
			if(BLEN(m) <= len){
				len -= BLEN(m);
				m->wptr = m->rptr;
				m = m->m_next;
			} else {
				m->rptr += len;
				break;
			}
		}
	} else {
		len = -len;
		while(len > 0 && m && BLEN(m) != 0){
			while(m && BLEN(m) != 0){
				n = m;
				m = m->next;
			}
			if(BLEN(n) <= len){
				len -= BLEN(n);
				n->wptr = n->rptr;
				m = mp;
			} else {
				n->wptr -= len;
				break;
			}
		}
	}
}

m_cat(m, n)
register struct mbuf *m, *n;
{
	register struct mbuf *xn;

	while(m->m_next)
		m = m->m_next;

	while(n){
		if((m->wptr + BLEN(n)) >= m->lim){
			/* just join the two chains */
			m->m_next = n;
			return;
		}
		/* splat the data from one into the other */
		bcopy(mtod(n, caddr_t), m->wptr, BLEN(n));
		m->wptr += BLEN(n);
		xn = n->next;
		m_free(n);
		n = xn;
	}
}


/*	in_cksum.c	6.1	83/07/29	*/
/*
 * yuck, but i can't get it right myself.
 */

/*
 * Checksum routine for Internet Protocol family headers (VAX Version).
 *
 * This routine is very heavily used in the network
 * code and should be modified for each CPU to be as fast as possible.
 */

in_cksum(m, len)
	register struct mbuf *m;
	register int len;
{
	register u_short *w;		/* on vax, known to be r9 */
	register int sum = 0;		/* on vax, known to be r8 */
	register int mlen = 0;

	for (;;) {
		/*
		 * Each trip around loop adds in
		 * word from one mbuf segment.
		 */
		w = mtod(m, u_short *);
		if (mlen == -1) {
			/*
			 * There is a byte left from the last segment;
			 * add it into the checksum.  Don't have to worry
			 * about a carry-out here because we make sure
			 * that high part of (32 bit) sum is small below.
			 */
			sum += *(u_char *)w << 8;
			w = (u_short *)((char *)w + 1);
			mlen = BLEN(m) - 1;
			len--;
		} else
			mlen = BLEN(m);
		m = m->m_next;
		if (len < mlen)
			mlen = len;
		len -= mlen;
		/*
		 * Force to long boundary so we do longword aligned
		 * memory operations.  It is too hard to do byte
		 * adjustment, do only word adjustment.
		 */
		if (((int)w&0x2) && mlen >= 2) {
			sum += *w++;
			mlen -= 2;
		}
		/*
		 * Do as much of the checksum as possible 32 bits at at time.
		 * In fact, this loop is unrolled to make overhead from
		 * branches &c small.
		 *
		 * We can do a 16 bit ones complement sum 32 bits at a time
		 * because the 32 bit register is acting as two 16 bit
		 * registers for adding, with carries from the low added
		 * into the high (by normal carry-chaining) and carries
		 * from the high carried into the low on the next word
		 * by use of the adwc instruction.  This lets us run
		 * this loop at almost memory speed.
		 *
		 * Here there is the danger of high order carry out, and
		 * we carefully use adwc.
		 */
		while ((mlen -= 32) >= 0) {
#undef ADD
			asm("clrl r0");		/* clears carry */
#define ADD		asm("adwc (r9)+,r8;");
			ADD; ADD; ADD; ADD; ADD; ADD; ADD; ADD;
			asm("adwc $0,r8");
		}
		mlen += 32;
		while ((mlen -= 8) >= 0) {
			asm("clrl r0");
			ADD; ADD;
			asm("adwc $0,r8");
		}
		mlen += 8;
		/*
		 * Now eliminate the possibility of carry-out's by
		 * folding back to a 16 bit number (adding high and
		 * low parts together.)  Then mop up trailing words
		 * and maybe an odd byte.
		 */
		{ asm("ashl $-16,r8,r0; addw2 r0,r8");
		  asm("adwc $0,r8; movzwl r8,r8"); }
		while ((mlen -= 2) >= 0) {
			asm("movzwl (r9)+,r0; addl2 r0,r8");
		}
		if (mlen == -1) {
			sum += *(u_char *)w;
		}
		if (len == 0)
			break;
		/*
		 * Locate the next block with some data.
		 * If there is a word split across a boundary we
		 * will wrap to the top with mlen == -1 and
		 * then add it in shifted appropriately.
		 */
		for (;;) {
			if (m == 0) {
				printf("cksum: out of data\n");
				goto done;
			}
			if (BLEN(m))
				break;
			m = m->m_next;
		}
	}
done:
	/*
	 * Add together high and low parts of sum
	 * and carry to get cksum.
	 * Have to be careful to not drop the last
	 * carry here.
	 */
	{ asm("ashl $-16,r8,r0; addw2 r0,r8; adwc $0,r8");
	  asm("mcoml r8,r8; movzwl r8,r8"); }
	return (sum);
}

in_addr
in_netof(x)
in_addr x;
{
	if(IN_CLASSC(x))
		return(x&IN_CLASSC_NET);
	else if(IN_CLASSB(x))
		return(x&IN_CLASSB_NET);
	else
		return(x&IN_CLASSA_NET);
}

in_addr
in_hostof(x)
in_addr x;
{
	if(IN_CLASSC(x))
		return(x&IN_CLASSC_HOST);
	else if(IN_CLASSB(x))
		return(x&IN_CLASSB_HOST);
	else
		return(x&IN_CLASSA_HOST);
}

/*
 *  Routes are kept in a circular list.  Ip_default_route points to the
 *  "first" position in the list.  On each acess, the accessed element is
 *  moved to this first position.
 */
#define NROUTES 50
struct ip_route ip_routes[NROUTES];
int Nip_route = NROUTES;		/* let netstat know number of routes */
struct ip_route ip_default_route = { 0, 0, &ip_default_route };

ip_doroute(dst, gate)
in_addr dst, gate;
{
	register struct ip_route *rp, *save;
	register struct ipif *ifp;

	if(gate){
		/* no-ops are ignored */
		if (dst == gate)
			return(0);

		/* don't accept an indirect route, if we have a direct one */
		for(ifp = ipif; ifp < &ipif[NINET]; ifp++){
			if((ifp->flags&IFF_UP)
			   && ifp->that == dst)
				return(0);
		}
	}
	/* look through existing routes (looks at ip_default_route first)*/
	rp = &ip_default_route;
	do {
		if (dst == rp->next->dst) {
			if (gate) {
				rp->next->gate = gate;
			} else {
				rp->next->dst = rp->next->gate = 0;
				rp->next = rp->next->next;
			}
			return(0);
		}
		rp = rp->next;
	} while (rp != &ip_default_route);
	if (gate == 0)
		return(0);
	/* add a new route */
	for(rp = &ip_routes[0]; rp < &ip_routes[NROUTES]; rp++)
		if(rp->dst == 0) {
			rp->dst = dst;
			rp->gate = gate;
			rp->next = ip_default_route.next;
			ip_default_route.next = rp;
			return(0);
		}
	return(1);
}

/* Look for a route on the circular list.  If the route is found, move
 * it to the beginning of the list.
 */
struct ip_route_info
ip_route(dst)
in_addr dst;
{
	extern unsigned long in_netof();
	unsigned long netof_dst;
	register struct ip_route *rp, *trp;
	struct ip_route_info info;

	/* look for host routes (start after ip_default_route) */
	for(rp = &ip_default_route; rp->next != &ip_default_route; rp=rp->next)
		if (dst == rp->next->dst) {
			/* make sure the interface exists */
			info.addr = rp->next->gate;
			info.ifp = ip_ifonnetof(info.addr);
			if(info.ifp == 0)
				break;
			/* move to first */
			trp = rp->next;
			rp->next = rp->next->next;
			trp->next = ip_default_route.next;
			ip_default_route.next = trp;
			return(info);
		}
	/* now try nets (start after ip_default_route) */
	netof_dst = in_netof(dst);
	for (rp = &ip_default_route; rp->next != &ip_default_route; rp=rp->next)
		if(netof_dst == rp->next->dst){
			/* make sure the interface exists */
			info.addr = rp->next->gate;
			info.ifp = ip_ifonnetof(info.addr);
			if(info.ifp == 0)
				break;
			/* move to first */
			trp = rp->next;
			rp->next = rp->next->next;
			trp->next = ip_default_route.next;
			ip_default_route.next = trp;
			return(info);
		}
	/* try a network to which we are directly connected */
	info.addr = dst;
	info.ifp = ip_ifonnetof(dst);
	if (info.ifp)
		return info;
	/* if all else fails, use default route */
	info.addr = ip_default_route.gate;
	info.ifp = ip_ifonnetof(info.addr);
	return(info);
}

bp_len(bp)
register struct block *bp;
{
	int n;

	n = 0;
	while(bp){
		n += BLEN(bp);
		bp = bp->next;
	}
	return(n);
}

in_addr
ip_hoston(dst)
in_addr dst;
{
	struct ip_route_info info;

	info = ip_route(dst);
	if(info.ifp == 0)
		return(0);
	return(info.ifp->thishost);
}

in_lnaof(i)
register u_long i;
{

	if(IN_CLASSA(i))
		return((i)&IN_CLASSA_HOST);
	else if(IN_CLASSB(i))
		return((i)&IN_CLASSB_HOST);
	else
		return((i)&IN_CLASSC_HOST);
}
#endif NINET

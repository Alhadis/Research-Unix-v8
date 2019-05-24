/*	ip_input.c	6.1	83/08/16	*/

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/stream.h"
#include "../h/inet/mbuf.h"
#include "../h/inet/in.h"
#include "../h/inet/ip.h"
#include "../h/inet/ip_var.h"

int	ipqmaxlen = 50;

u_char	ipcksum = 1;
struct	ip *ip_reass();

/*
 * Ip input routine.  Checksum and byte swap header.  If fragmented
 * try to reassamble.  If complete and fragment queue exists, discard.
 * Process options.  Pass to next level.
 */
ip_input(m)
register struct mbuf *m;
{
	register struct ip *ip;
	register struct ipq *fp;
	struct mbuf *m0;
	register int i;
	int hlen;

	if (m == 0)
		return;
	if (BLEN(m) < sizeof (struct ip) &&
	    (m = m_pullup(m, sizeof (struct ip))) == 0) {
		ipstat.ips_toosmall++;
		return;
	}
	ip = mtod(m, struct ip *);
	if ((hlen = ip->ip_hl << 2) > BLEN(m)) {
		if ((m = m_pullup(m, hlen)) == 0) {
			ipstat.ips_badhlen++;
			return;
		}
		ip = mtod(m, struct ip *);
	}
	if (ipcksum)
		if (ip->ip_sum = in_cksum(m, hlen)) {
			ipstat.ips_badsum++;
			goto bad;
		}

	/*
	 * Convert fields to host representation.
	 */
	ip->ip_len = ntohs((u_short)ip->ip_len);
	if (ip->ip_len < hlen) {
		ipstat.ips_badlen++;
		goto bad;
	}
	ip->ip_id = ntohs(ip->ip_id);
	ip->ip_off = ntohs((u_short)ip->ip_off);

	/*
	 * Check that the amount of data in the buffers
	 * is as at least much as the IP header would have us expect.
	 * Trim mbufs if longer than we expect.
	 * Drop packet if shorter than we expect.
	 */
	i = -ip->ip_len;
	m0 = m;
	for (;;) {
		i += BLEN(m);
		if (m->m_next == 0)
			break;
		m = m->m_next;
	}
	if (i != 0) {
		if (i < 0) {
			ipstat.ips_tooshort++;
			goto bad;
		}
		if (i <= BLEN(m))
			m->wptr -= i;
		else
			m_adj(m0, -i);
	}
	m = m0;

	/*
	 * Process options and, if not destined for us,
	 * ship it on.  ip_dooptions returns 1 when an
	 * error was detected (causing an icmp message
	 * to be sent).
	 */
	ip->ip_dst = ntohl(ip->ip_dst);
	ip->ip_src = ntohl(ip->ip_src);
	if (hlen > sizeof (struct ip) && ip_dooptions(ip))
		return;

	if (ip_ifwithaddr(ip->ip_dst) == 0) {
		ip_forward(ip);
		return;
	}

	/*
	 * Look for queue of fragments
	 * of this datagram.
	 */
	if(ipq.next == 0 && ipq.prev == 0)	/* init, only once */
		ipq.next = ipq.prev = &ipq;
	for (fp = ipq.next; fp != &ipq; fp = fp->next)
		if (ip->ip_id == fp->ipq_id &&
		    ip->ip_src == fp->ipq_src &&
		    ip->ip_dst == fp->ipq_dst &&
		    ip->ip_p == fp->ipq_p)
			goto found;
	fp = 0;
found:

	/*
	 * Adjust ip_len to not reflect header,
	 * set ip_mff if more fragments are expected,
	 * convert offset of this to bytes.
	 */
	ip->ip_len -= hlen;
	((struct ipasfrag *)ip)->ipf_mff = 0;
	if (ip->ip_off & IP_MF)
		((struct ipasfrag *)ip)->ipf_mff = 1;
	ip->ip_off <<= 3;

	/*
	 * If datagram marked as having more fragments
	 * or if this is not the first fragment,
	 * attempt reassembly; if it succeeds, proceed.
	 */
	if (((struct ipasfrag *)ip)->ipf_mff || ip->ip_off) {
		ip = ip_reass((struct ipasfrag *)ip, fp);
		if (ip == 0)
			return;
		hlen = ip->ip_hl << 2;
		m = dtom(ip);
	} else
		if (fp)
			ip_freef(fp);

	/*
	 * Switch out to protocol's input routine.
	 */
	ipdrint(m, (unsigned int)(ip->ip_p));
	return;
bad:
	m_freem(m);
}

/*
 * Take incoming datagram fragment and try to
 * reassemble it into whole datagram.  If a chain for
 * reassembly of this datagram already exists, then it
 * is given as fp; otherwise have to make a chain.
 */
struct ip *
ip_reass(ip, fp)
	register struct ipasfrag *ip;
	register struct ipq *fp;
{
	register struct mbuf *m = dtom(ip);
	register struct ipasfrag *q;
	struct mbuf *t;
	int hlen = ip->ip_hl << 2;
	int i, next;

	/*
	 * Presence of header sizes in mbufs
	 * would confuse code below.
	 */
	m->rptr += hlen;

	/*
	 * If first fragment to arrive, create a reassembly queue.
	 */
	if (fp == 0) {
		if ((t = m_get(M_WAIT, MT_FTABLE)) == NULL)
			goto dropfrag;
		t->m_next = 0;
		fp = mtod(t, struct ipq *);
		insque(fp, &ipq);
		fp->ipq_ttl = IPFRAGTTL;
		fp->ipq_p = ip->ip_p;
		fp->ipq_id = ip->ip_id;
		fp->ipq_next = fp->ipq_prev = (struct ipasfrag *)fp;
		fp->ipq_src = ((struct ip *)ip)->ip_src;
		fp->ipq_dst = ((struct ip *)ip)->ip_dst;
		q = (struct ipasfrag *)fp;
		goto insert;
	}

	/*
	 * Find a segment which begins after this one does.
	 */
	for (q = fp->ipq_next; q != (struct ipasfrag *)fp; q = q->ipf_next)
		if (q->ip_off > ip->ip_off)
			break;

	/*
	 * If there is a preceding segment, it may provide some of
	 * our data already.  If so, drop the data from the incoming
	 * segment.  If it provides all of our data, drop us.
	 */
	if (q->ipf_prev != (struct ipasfrag *)fp) {
		i = q->ipf_prev->ip_off + q->ipf_prev->ip_len - ip->ip_off;
		if (i > 0) {
			if (i >= ip->ip_len)
				goto dropfrag;
			m_adj(dtom(ip), i);
			ip->ip_off += i;
			ip->ip_len -= i;
		}
	}

	/*
	 * While we overlap succeeding segments trim them or,
	 * if they are completely covered, dequeue them.
	 */
	while (q != (struct ipasfrag *)fp && ip->ip_off + ip->ip_len > q->ip_off) {
		i = (ip->ip_off + ip->ip_len) - q->ip_off;
		if (i < q->ip_len) {
			q->ip_len -= i;
			q->ip_off += i;
			m_adj(dtom(q), i);
			break;
		}
		q = q->ipf_next;
		m_freem(dtom(q->ipf_prev));
		ip_deq(q->ipf_prev);
	}

insert:
	/*
	 * Stick new segment in its place;
	 * check for complete reassembly.
	 */
	ip_enq(ip, q->ipf_prev);
	next = 0;
	for (q = fp->ipq_next; q != (struct ipasfrag *)fp; q = q->ipf_next) {
		if (q->ip_off != next)
			return (0);
		next += q->ip_len;
	}
	if (q->ipf_prev->ipf_mff)
		return (0);

	/*
	 * Reassembly is complete; concatenate fragments.
	 */
	q = fp->ipq_next;
	m = dtom(q);
	t = m->m_next;
	m->m_next = 0;
	m_cat(m, t);
	q = q->ipf_next;
	while (q != (struct ipasfrag *)fp) {
		t = dtom(q);
		q = q->ipf_next;
		m_cat(m, t);
	}

	/*
	 * Create header for new ip packet by
	 * modifying header of first packet;
	 * dequeue and discard fragment reassembly header.
	 * Make header visible.
	 */
	ip = fp->ipq_next;
	ip->ip_len = next;
	((struct ip *)ip)->ip_src = fp->ipq_src;
	((struct ip *)ip)->ip_dst = fp->ipq_dst;
	remque(fp);
	(void) m_free(dtom(fp));
	m = dtom(ip);
	m->rptr -= sizeof (struct ipasfrag);
	return ((struct ip *)ip);

dropfrag:
	m_freem(m);
	return (0);
}

/*
 * Free a fragment reassembly header and all
 * associated datagrams.
 */
ip_freef(fp)
	struct ipq *fp;
{
	register struct ipasfrag *q, *p;

	for (q = fp->ipq_next; q != (struct ipasfrag *)fp; q = p) {
		p = q->ipf_next;
		ip_deq(q);
		m_freem(dtom(q));
	}
	remque(fp);
	(void) m_free(dtom(fp));
}

/*
 * Put an ip fragment on a reassembly chain.
 * Like insque, but pointers in middle of structure.
 */
ip_enq(p, prev)
	register struct ipasfrag *p, *prev;
{

	p->ipf_prev = prev;
	p->ipf_next = prev->ipf_next;
	prev->ipf_next->ipf_prev = p;
	prev->ipf_next = p;
}

/*
 * To ip_enq as remque is to insque.
 */
ip_deq(p)
	register struct ipasfrag *p;
{

	p->ipf_prev->ipf_next = p->ipf_next;
	p->ipf_next->ipf_prev = p->ipf_prev;
}

/*
 * IP timer processing;
 * if a timer expires on a reassembly
 * queue, discard it.
 */
ip_slowtimo()
{
	register struct ipq *fp;
	int s = spl6();

	fp = ipq.next;
	if (fp == 0) {
		splx(s);
		return;
	}
	while (fp != &ipq) {
		--fp->ipq_ttl;
		fp = fp->next;
		if (fp->prev->ipq_ttl == 0)
			ip_freef(fp->prev);
	}
	timeout(ip_slowtimo, (caddr_t)0, hz);
	splx(s);
}

/*
 * Drain off all datagram fragments.
 */
ip_drain()
{

	while (ipq.next != &ipq)
		ip_freef(ipq.next);
}

/*
 * Do option processing on a datagram,
 * possibly discarding it if bad options
 * are encountered.
 */
ip_dooptions(ip)
	struct ip *ip;
{
	register u_char *cp;
	int opt, optlen, cnt;

	cp = (u_char *)(ip + 1);
	cnt = (ip->ip_hl << 2) - sizeof (struct ip);
	for (; cnt > 0; cnt -= optlen, cp += optlen) {
		opt = cp[0];
		if (opt == IPOPT_EOL)
			break;
		if (opt == IPOPT_NOP)
			optlen = 1;
		else
			optlen = cp[1];
		switch (opt) {

		default:
			break;
#ifdef FAT_CHANCE
		/*
		 * Source routing with record.
		 * Find interface with current destination address.
		 * If none on this machine then drop if strictly routed,
		 * or do nothing if loosely routed.
		 * Record interface address and bring up next address
		 * component.  If strictly routed make sure next
		 * address on directly accessible net.
		 */
		case IPOPT_LSRR:
		case IPOPT_SSRR:
			if (cp[2] < 4 || cp[2] > optlen - (sizeof (long) - 1))
				break;
			sin = (struct in_addr *)(cp + cp[2]);
			ipaddr.sin_addr = *sin;
			ifp = if_ifwithaddr((struct sockaddr *)&ipaddr);
			type = ICMP_UNREACH, code = ICMP_UNREACH_SRCFAIL;
			if (ifp == 0) {
				if (opt == IPOPT_SSRR)
					goto bad;
				break;
			}
			t = ip->ip_dst; ip->ip_dst = *sin; *sin = t;
			cp[2] += 4;
			if (cp[2] > optlen - (sizeof (long) - 1))
				break;
			ip->ip_dst = sin[1];
			if (opt == IPOPT_SSRR &&
			    if_ifonnetof(in_netof(ip->ip_dst)) == 0)
				goto bad;
			break;

		case IPOPT_TS:
			code = cp - (u_char *)ip;
			type = ICMP_PARAMPROB;
			ipt = (struct ip_timestamp *)cp;
			if (ipt->ipt_len < 5)
				goto bad;
			if (ipt->ipt_ptr > ipt->ipt_len - sizeof (long)) {
				if (++ipt->ipt_oflw == 0)
					goto bad;
				break;
			}
			sin = (struct in_addr *)(cp+cp[2]);
			switch (ipt->ipt_flg) {

			case IPOPT_TS_TSONLY:
				break;

			case IPOPT_TS_TSANDADDR:
				if (ipt->ipt_ptr + 8 > ipt->ipt_len)
					goto bad;
				if (ifinet == 0)
					goto bad;	/* ??? */
				*sin++ = ((struct sockaddr_in *)&ifinet->if_addr)->sin_addr;
				break;

			case IPOPT_TS_PRESPEC:
				ipaddr.sin_addr = *sin;
				if (if_ifwithaddr((struct sockaddr *)&ipaddr) == 0)
					continue;
				if (ipt->ipt_ptr + 8 > ipt->ipt_len)
					goto bad;
				ipt->ipt_ptr += 4;
				break;

			default:
				goto bad;
			}
			*(n_time *)sin = iptime();
			ipt->ipt_ptr += 4;
#endif FATCHANCE
		}
	}
	return (0);
}

/*
 * Strip out IP options, at higher
 * level protocol in the kernel.
 * Second argument is buffer to which options
 * will be moved, and return value is their length.
 */
ip_stripoptions(ip, mopt)
	struct ip *ip;
	struct mbuf *mopt;
{
	register int i;
	register struct mbuf *m;
	int olen;

	olen = (ip->ip_hl<<2) - sizeof (struct ip);
	m = dtom(ip);
	ip++;
	if (mopt) {
		mopt->wptr = mopt->base + olen;
		mopt->rptr = mopt->base;
		bcopy((caddr_t)ip, mtod(m, caddr_t), (unsigned)olen);
	}
	i = BLEN(m) - (sizeof (struct ip) + olen);
	bcopy((caddr_t)ip+olen, (caddr_t)ip, (unsigned)i);
	m->wptr -= olen;
}

int	ipforwarding = 1;
extern	ipprintfs;
/*
 * Forward a packet.  If some error occurs return the sender
 * and icmp packet.  Note we can't always generate a meaningful
 * icmp message because icmp doesn't have a large enough repetoire
 * of codes and types.
 */
ip_forward(ip)
	register struct ip *ip;
{
	struct mbuf *mopt;

	if(ipprintfs)
		printf("forward: src %x dst %x ttl %x\n", ip->ip_src,
			ip->ip_dst, ip->ip_ttl);
	if (ipforwarding == 0) {
		return;
	}
	if (ip->ip_ttl < IPTTLDEC) {
		return;
	}
	ip->ip_ttl -= IPTTLDEC;
	mopt = m_get(M_DONTWAIT, MT_DATA);
	if (mopt == NULL) {
		m_freem(dtom(ip));
		return;
	}
	mopt->next = 0;

	ip_stripoptions(ip, mopt);

	ip_output(dtom(ip), mopt, IP_FORWARDING);
}

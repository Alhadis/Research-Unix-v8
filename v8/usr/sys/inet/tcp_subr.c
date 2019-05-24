/*	tcp_subr.c	6.1	83/07/29	*/
#include "tcp.h"
#if NTCP > 0

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/stream.h"
#include "../h/inet/mbuf.h"
#include "../h/inet/in.h"
#include "../h/inet/ip.h"
#include "../h/inet/ip_var.h"
#include "../h/inet/tcp.h"
#include "../h/inet/tcp_fsm.h"
#include "../h/inet/tcp_seq.h"
#include "../h/inet/tcp_timer.h"
#include "../h/inet/tcp_var.h"
#include "../h/inet/tcpip.h"
#include "../h/inet/socket.h"
#include "../h/inet/tcpdebug.h"

extern int tcp_maxseg;

/*
 * Create template to be used to send tcp packets on a connection.
 * Call after host entry created, allocates an mbuf and fills
 * in a skeletal tcp/ip header, minimizing the amount of work
 * necessary when the connection is used.
 */
struct tcpiphdr *
tcp_template(tp)
	struct tcpcb *tp;
{
	register struct socket *so = tp->t_socket;
	register struct mbuf *m;
	register struct tcpiphdr *n;

	m = m_get(M_WAIT, MT_HEADER);
	if (m == NULL)
		return (0);
	m->next = 0;
	m->wptr += sizeof (struct tcpiphdr);
	n = mtod(m, struct tcpiphdr *);
	n->ti_next = n->ti_prev = 0;
	n->ti_x1 = 0;
	n->ti_pr = 6;	/* tcp protocol number */
	n->ti_len = htons(sizeof (struct tcpiphdr) - sizeof (struct ip));
	n->ti_src = so->so_laddr;
	n->ti_dst = so->so_faddr;
	n->ti_sport = so->so_lport;
	n->ti_dport = so->so_fport;
	n->ti_seq = 0;
	n->ti_ack = 0;
	n->ti_x2 = 0;
	n->ti_off = 5;
	n->ti_flags = 0;
	n->ti_win = 0;
	n->ti_sum = 0;
	n->ti_urp = 0;
	return (n);
}

/*
 * Send a single message to the TCP at address specified by
 * the given TCP/IP header.  If flags==0, then we make a copy
 * of the tcpiphdr at ti and send directly to the addressed host.
 * This is used to force keep alive messages out using the TCP
 * template for a connection tp->t_template.  If flags are given
 * then we send a message back to the TCP which originated the
 * segment ti, and discard the mbuf containing it and any other
 * attached mbufs.
 *
 * In any case the ack and sequence number of the transmitted
 * segment are as specified by the parameters.
 */
tcp_respond(tp, ti, ack, seq, flags)
	struct tcpcb *tp;
	register struct tcpiphdr *ti;
	tcp_seq ack, seq;
	int flags;
{
	struct mbuf *m;
	int win = 0, tlen;

	if (tp) {
		win = sbrcvspace(tp->t_socket);
	}
	if (flags == 0) {
		m = m_get(M_DONTWAIT, MT_HEADER);
		if (m == NULL)
			return;
		m->next = 0;
		m->wptr += sizeof (struct tcpiphdr) + 1;
		*mtod(m, struct tcpiphdr *) = *ti;
		ti = mtod(m, struct tcpiphdr *);
		flags = TH_ACK;
		tlen = 1;
	} else {
		m = dtom(ti);
		m_freem(m->m_next);
		m->m_next = 0;
		m->rptr = (u_char *)ti;
		m->wptr = m->rptr + sizeof (struct tcpiphdr);
#define xchg(a,b,type) { type t; t=a; a=b; b=t; }
		xchg(ti->ti_dst, ti->ti_src, u_long);
		xchg(ti->ti_dport, ti->ti_sport, u_short);
#undef xchg
		tlen = 0;
	}
	ti->ti_next = ti->ti_prev = 0;
	ti->ti_x1 = 0;
	ti->ti_len = htons((u_short)(sizeof (struct tcphdr) + tlen));
	ti->ti_seq = htonl(seq);
	ti->ti_ack = htonl(ack);
	ti->ti_x2 = 0;
	ti->ti_off = sizeof (struct tcphdr) >> 2;
	ti->ti_flags = flags;
	ti->ti_win = htons((u_short)win);
	ti->ti_urp = 0;
	ti->ti_src = htonl(ti->ti_src);
	ti->ti_dst = htonl(ti->ti_dst);
	ti->ti_sport = htons(ti->ti_sport);
	ti->ti_dport = htons(ti->ti_dport);
	ti->ti_sum = in_cksum(m, sizeof (struct tcpiphdr) + tlen);
	((struct ip *)ti)->ip_len = sizeof (struct tcpiphdr) + tlen;
	((struct ip *)ti)->ip_ttl = TCP_TTL;

	ti->ti_dst = ntohl(ti->ti_dst);
	ti->ti_src = ntohl(ti->ti_src);
	tcp_debug(ti, 1);
	tcp_ldout(m);
}

/*
 * Create a new TCP control block, making an
 * empty reassembly queue and hooking it to the argument
 * protocol control block.
 */
struct tcpcb *
tcp_newtcpcb(so)
	struct socket *so;
{
	register struct tcpcb *tp;
	extern struct tcpcb tcpcb[];

	tp = &tcpcb[so->so_dev];
	if(tp->t_template)
		(void) m_free(dtom(tp->t_template));
	bzero(tp, sizeof(struct tcpcb));
	tp->seg_next = tp->seg_prev = (struct tcpiphdr *)tp;
	/*
	 * If the default maximum IP packet size is 576 bytes
	 * and a standard IP header is 20 bytes, with a TCP
	 * header of 20 bytes plus the options necessary to
	 * upgrade it to something higher, then initialize the
	 * maximum segment size to 576 - (20 + 20 + 8 + slop).
	 */
	tp->t_maxseg = tcp_maxseg;	/* satisfy the rest of the world */
	tp->t_flags = 0;		/* sends options! */
	tp->t_socket = so;
	so->so_tcpcb = tp;
	return (tp);
}

/*
 * Drop a TCP connection.
 * If connection is synchronized,
 * then send a RST to peer.
 */
struct tcpcb *
tcp_drop(tp)
	register struct tcpcb *tp;
{

	if (TCPS_HAVERCVDSYN(tp->t_state)) {
		tp->t_state = TCPS_CLOSED;
		(void) tcp_output(tp);
	}
	return (tcp_close(tp));
}

/*
 * Close a TCP control block:
 *	discard all space held by the tcp
 *	discard internet protocol block
 *	wake up any sleepers
 */
struct tcpcb *
tcp_close(tp)
	register struct tcpcb *tp;
{
	register struct tcpiphdr *t;
	struct socket *so = tp->t_socket;
	register struct mbuf *m;

	if(!tp)
		panic("tcp_close");
	t = tp->seg_next;
	while (t != (struct tcpiphdr *)tp) {
		t = (struct tcpiphdr *)t->ti_next;
		m = dtom(t->ti_prev);
		remque(t->ti_prev);
		m_freem(m);
	}
	if (tp->t_template)
		(void) m_free(dtom(tp->t_template));
	tp->t_template = 0;
	if (tp->t_tcpopt)
		(void) m_free(dtom(tp->t_tcpopt));
	tp->t_tcpopt = 0;
	if (tp->t_ipopt)
		(void) m_free(dtom(tp->t_ipopt));
	tp->t_ipopt = 0;
	so->so_tcpcb = 0;
	soisdisconnected(so);
	return ((struct tcpcb *)0);
}

/*
 * For debugging save time event occurred, the direction of the message,
 * and the tcp header.  Then increment the pointer to the tcp
 * debug queue.
 */
int Nbugarr=SIZDEBUG;
struct tcpdebug bugarr[SIZDEBUG];	/* buffer to store the debug info in */
int tcpdbg_ind=0;			/* index into bugarr at last entry */

tcp_debug(ti, code)
struct tcpiphdr *ti;
{
	bugarr[tcpdbg_ind].stamp = time;
	bugarr[tcpdbg_ind].inout = code;
	bugarr[tcpdbg_ind].savhdr = ti->ti_t;
	tcpdbg_ind = (tcpdbg_ind + 1) % SIZDEBUG;
}

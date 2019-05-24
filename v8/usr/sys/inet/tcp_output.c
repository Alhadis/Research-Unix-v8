/*	tcp_output.c	6.1	83/07/29	*/
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
#define	TCPOUTFLAGS
#include "../h/inet/tcp_fsm.h"
#include "../h/inet/tcp_seq.h"
#include "../h/inet/tcp_timer.h"
#include "../h/inet/tcp_var.h"
#include "../h/inet/tcpip.h"
#include "../h/inet/socket.h"

int tcp_busy;	/* set/unset by tcp_output callers, to keep out timer */

/*
 * Initial options.
 */
u_char	tcp_initopt[4] = { TCPOPT_MAXSEG, 4, 0x0, 0x0, };
extern	tcpprintfs;

/*
 *  Settable maximum tcp segment size.
 */
extern int tcp_maxseg;

/*
 * Tcp output routine: figure out what should be sent and send it.
 */
tcp_output(tp)
	register struct tcpcb *tp;
{
	register struct socket *so = tp->t_socket;
	register int len;
	struct mbuf *m0;
	int off, flags, win, error;
	register struct mbuf *m;
	register struct tcpiphdr *ti;
	u_char *opt;
	unsigned optlen = 0;
	int sendalot;

	if(tcp_busy != 1)
		panic("tcp_busy");
	/*
	 * Determine length of data that should be transmitted,
	 * and flags that will be used.
	 * If there is some data or critical controls (SYN, RST)
	 * to send, then transmit; otherwise, investigate further.
	 */
again:
	sendalot = 0;
	off = tp->snd_nxt - tp->snd_una;
	len = MIN(sosndcc(so), tp->snd_wnd+tp->t_force) - off;
	if (len < 0)
		return (0);	/* ??? */	/* past FIN */
	if (len > tp->t_maxseg) {
		len = tp->t_maxseg;
		sendalot = 1;
	}

	flags = tcp_outflags[tp->t_state];
	if (tp->snd_nxt + len < tp->snd_una + sosndcc(so))
		flags &= ~TH_FIN;
	if (flags & (TH_SYN|TH_RST|TH_FIN))
		goto send;
	if (SEQ_GT(tp->snd_up, tp->snd_una))
		goto send;

	/*
	 * Sender silly window avoidance.  If can send all data,
	 * a maximum segment, at least 1/4 of window do it,
	 * or are forced, do it; otherwise don't bother.
	 */
	if (len) {
		if (len == tp->t_maxseg || off+len >= sosndcc(so))
			goto send;
		if (len * 4 >= tp->snd_wnd)		/* a lot */
			goto send;
		if (tp->t_force)
			goto send;
	}

	/*
	 * Send if we owe peer an ACK.
	 */
	if (tp->t_flags&TF_ACKNOW)
		goto send;


	/*
	 * Calculate available window in i, and also amount
	 * of window known to peer (as advertised window less
	 * next expected input.)  If this is 35% or more of the
	 * maximum possible window, then want to send a segment to peer.
	 */
	win = sbrcvspace(so);
	if (win > 0 &&
	    ((100*(win-(tp->rcv_adv-tp->rcv_nxt))/sorcvhiwat(so)) >= 35))
		goto send;

	/*
	 * TCP window updates are not reliable, rather a polling protocol
	 * using ``persist'' packets is used to insure receipt of window
	 * updates.  The three ``states'' for the output side are:
	 *	idle			not doing retransmits or persists
	 *	persisting		to move a zero window
	 *	(re)transmitting	and thereby not persisting
	 *
	 * tp->t_timer[TCPT_PERSIST]
	 *	is set when we are in persist state.
	 * tp->t_force
	 *	is set when we are called to send a persist packet.
	 * tp->t_timer[TCPT_REXMT]
	 *	is set when we are retransmitting
	 * The output side is idle when both timers are zero.
	 *
	 * If send window is closed, there is data to transmit, and no
	 * retransmit or persist is pending, then go to persist state,
	 * arranging to force out a byte to get more current window information
	 * if nothing happens soon.
	 */
	if (tp->snd_wnd == 0 && sosndcc(so) &&
	    tp->t_timer[TCPT_REXMT] == 0 && tp->t_timer[TCPT_PERSIST] == 0) {
		tp->t_rxtshift = 0;
		tcp_setpersist(tp);
	}

	/*
	 * No reason to send a segment, just return.
	 */
	return (0);

send:
	/*
	 * Grab a header mbuf, attaching a copy of data to
	 * be transmitted, and initialize the header from
	 * the template for sends on this connection.
	 */
	MGET(m, M_DONTWAIT, MT_HEADER);
	if (m == NULL)
		return (1);
	m->next = 0;
	m->wptr += sizeof (struct tcpiphdr);
	if (len) {
		m->m_next = m_copy(so->so_wq->first, off, len);
		if (m->m_next == 0)
			len = 0;
	}
	ti = mtod(m, struct tcpiphdr *);
	if (tp->t_template == 0)
		panic("tcp_output");
	bcopy((caddr_t)tp->t_template, (caddr_t)ti, sizeof (struct tcpiphdr));

	/*
	 * Fill in fields, remembering maximum advertised
	 * window for use in delaying messages about window sizes.
	 */
	ti->ti_seq = tp->snd_nxt;
	ti->ti_ack = tp->rcv_nxt;
	ti->ti_seq = htonl(ti->ti_seq);
	ti->ti_ack = htonl(ti->ti_ack);
	/*
	 * Before ESTABLISHED, force sending of initial options
	 * unless TCP set to not do any options.
	 */
	if (tp->t_state < TCPS_ESTABLISHED) {
		if (tp->t_flags&TF_NOOPT)
			goto noopt;
		opt = tcp_initopt;
		optlen = sizeof (tcp_initopt);
		*(u_short *)(opt + 2) = tcp_maxseg;	/* 2048/2 in 4.2 */
		*(u_short *)(opt + 2) = htons(*(u_short *)(opt + 2));
	} else {
		if (tp->t_tcpopt == 0)
			goto noopt;
		opt = mtod(tp->t_tcpopt, u_char *);
		optlen = BLEN(tp->t_tcpopt);
	}
	if (opt) {
		m0 = m->m_next;
		m->m_next = m_get(M_DONTWAIT, MT_DATA);
		if (m->m_next == 0) {
			(void) m_free(m);
			m_freem(m0);
			return (1);
		}
		m->m_next->m_next = m0;
		m0 = m->m_next;
		m0->wptr += optlen;
		bcopy((caddr_t)opt, mtod(m0, caddr_t), optlen);
		opt = (u_char *)(mtod(m0, caddr_t) + optlen);
		while (BLEN(m0) & 0x3) {
			m0->wptr++;
			*opt++ = TCPOPT_EOL;
		}
		optlen = BLEN(m0);
		ti->ti_off = (sizeof (struct tcphdr) + optlen) >> 2;
	}
noopt:
	ti->ti_flags = flags;
	win = sbrcvspace(so);
	if (win < sorcvhiwat(so) / 4)	/* avoid silly window */
		win = 0;
	if (win > 0)
		ti->ti_win = htons((u_short)win);
	if (SEQ_GT(tp->snd_up, tp->snd_nxt)) {
		ti->ti_urp = tp->snd_up - tp->snd_nxt;
		ti->ti_urp = htons(ti->ti_urp);
		ti->ti_flags |= TH_URG;
	} else
		/*
		 * If no urgent pointer to send, then we pull
		 * the urgent pointer to the left edge of the send window
		 * so that it doesn't drift into the send window on sequence
		 * number wraparound.
		 */
		tp->snd_up = tp->snd_una;		/* drag it along */
	/*
	 * If anything to send and we can send it all, set PUSH.
	 * (This will keep happy those implementations which only
	 * give data to the user when a buffer fills or a PUSH comes in.)
	 */
	if (len && off+len == sosndcc(so))
		ti->ti_flags |= TH_PUSH;

	/*
	 * Put TCP length in extended header, and then
	 * checksum extended header and data.
	 */
	if (len + optlen) {
		ti->ti_len = sizeof (struct tcphdr) + optlen + len;
		ti->ti_len = htons((u_short)ti->ti_len);
	}
	ti->ti_src = htonl(ti->ti_src);
	ti->ti_dst = htonl(ti->ti_dst);
	ti->ti_sport = htons(ti->ti_sport);
	ti->ti_dport = htons(ti->ti_dport);
	ti->ti_sum = in_cksum(m, sizeof (struct tcpiphdr) + (int)optlen + len);
	ti->ti_src = ntohl(ti->ti_src);
	ti->ti_dst = ntohl(ti->ti_dst);

	/*
	 * In transmit state, time the transmission and arrange for
	 * the retransmit.  In persist state, reset persist time for
	 * next persist.
	 */
	if (tp->t_force == 0) {
		/*
		 * Advance snd_nxt over sequence space of this segment.
		 */
		if (flags & (TH_SYN|TH_FIN))
			tp->snd_nxt++;
		tp->snd_nxt += len;
		if (SEQ_GT(tp->snd_nxt, tp->snd_max))
			tp->snd_max = tp->snd_nxt;

		/*
		 * Time this transmission if not a retransmission and
		 * not currently timing anything.
		 */
		if (SEQ_GT(tp->snd_nxt, tp->snd_max) && tp->t_rtt == 0) {
			tp->t_rtt = 1;
			tp->t_rtseq = tp->snd_nxt - len;
		}

		/*
		 * Set retransmit timer if not currently set.
		 * Initial value for retransmit timer to tcp_beta*tp->t_srtt.
		 * Initialize shift counter which is used for exponential
		 * backoff of retransmit time.
		 */
		if (tp->t_timer[TCPT_REXMT] == 0 &&
		    tp->snd_nxt != tp->snd_una) {
			TCPT_RANGESET(tp->t_timer[TCPT_REXMT],
			    tcp_beta * tp->t_srtt, TCPTV_MIN, TCPTV_MAX);
			tp->t_rtt = 0;
			tp->t_rxtshift = 0;
		}
		tp->t_timer[TCPT_PERSIST] = 0;
	} else {
		if (SEQ_GT(tp->snd_una+1, tp->snd_max))
			tp->snd_max = tp->snd_una+1;
	}


	/*
	 * Fill in IP length and desired time to live and
	 * send to IP level.
	 */
	((struct ip *)ti)->ip_len = sizeof (struct tcpiphdr) + optlen + len;
	((struct ip *)ti)->ip_ttl = TCP_TTL;

	tcp_debug(ti, 1);
	error = tcp_ldout(m);
	if (error)
		return (error);


	/*
	 * Data sent (as far as we can tell).
	 * If this advertises a larger window than any other segment,
	 * then remember the size of the advertised window.
	 * Drop send for purpose of ACK requirements.
	 */
	if (win > 0 && SEQ_GT(tp->rcv_nxt+win, tp->rcv_adv))
		tp->rcv_adv = tp->rcv_nxt + win;
	tp->t_flags &= ~(TF_ACKNOW|TF_DELACK);
	if (sendalot && tp->t_force == 0)
		goto again;
	return (0);
}

tcp_setpersist(tp)
	register struct tcpcb *tp;
{

	if (tp->t_timer[TCPT_REXMT])
		panic("tcp_output REXMT");
	/*
	 * Start/restart persistance timer.
	 */
	TCPT_RANGESET(tp->t_timer[TCPT_PERSIST],
	    ((int)(tcp_beta * tp->t_srtt)) << tp->t_rxtshift,
	    TCPTV_PERSMIN, TCPTV_MAX);
	tp->t_rxtshift++;
	if (tp->t_rxtshift >= TCP_MAXRXTSHIFT)
		tp->t_rxtshift = 0;
}

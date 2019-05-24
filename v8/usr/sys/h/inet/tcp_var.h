/*	tcp_var.h	6.1	83/07/29	*/

/*
 * Kernel variables for tcp.
 */

/*
 * Tcp control block, one per tcp; fields:
 */
struct tcpcb {
	struct	tcpiphdr *seg_next;	/* sequencing queue */
	struct	tcpiphdr *seg_prev;
	struct	socket *t_socket;
	short	t_state;		/* state of this connection */
	short	t_timer[TCPT_NTIMERS];	/* tcp timers */
	short	t_rxtshift;		/* log(2) of rexmt exp. backoff */
	struct	mbuf *t_tcpopt;		/* tcp options */
	struct	mbuf *t_ipopt;		/* ip options */
	short	t_maxseg;		/* maximum segment size */
	char	t_force;		/* 1 if forcing out a byte */
	u_char	t_flags;
#define	TF_ACKNOW	0x01			/* ack peer immediately */
#define	TF_DELACK	0x02			/* ack, but try to delay it */
#define	TF_DONTKEEP	0x04			/* don't use keep-alives */
#define	TF_NOOPT	0x08			/* don't use tcp options */
	struct	tcpiphdr *t_template;	/* skeletal packet for transmit */
	struct	inpcb *t_inpcb;		/* back pointer to internet pcb */
/*
 * The following fields are used as in the protocol specification.
 * See RFC783, Dec. 1981, page 21.
 */
/* send sequence variables */
	tcp_seq	snd_una;		/* send unacknowledged */
	tcp_seq	snd_nxt;		/* send next */
	tcp_seq	snd_up;			/* send urgent pointer */
	tcp_seq	snd_wl1;		/* window update seg seq number */
	tcp_seq	snd_wl2;		/* window update seg ack number */
	tcp_seq	iss;			/* initial send sequence number */
	u_short	snd_wnd;		/* send window */
/* receive sequence variables */
	short	rcv_wnd;		/* receive window */
	tcp_seq	rcv_nxt;		/* receive next */
	tcp_seq	rcv_up;			/* receive urgent pointer */
	tcp_seq	irs;			/* initial receive sequence number */
/*
 * Additional variables for this implementation.
 */
/* receive variables */
	tcp_seq	rcv_adv;		/* advertised window */
/* retransmit variables */
	tcp_seq	snd_max;		/* highest sequence number sent
					   used to recognize retransmits */
/* transmit timing stuff */
	short	t_idle;			/* inactivity time */
	short	t_rtt;			/* round trip time */
	tcp_seq	t_rtseq;		/* sequence number being timed */
	float	t_srtt;			/* smoothed round-trip time */
/* out-of-band data */
	char	t_oobflags;		/* have some */
	char	t_iobc;			/* input character */
#define	TCPOOB_HAVEDATA	0x01
};

struct	tcpstat {
	int	tcps_badsum;
	int	tcps_badoff;
	int	tcps_hdrops;
	int	tcps_badsegs;
	int	tcps_unack;
	int	tcps_timeouts[4];	/* timeouts while ESTABLISHED */
	int	tcps_duplicates;	/* other side retransmitting */
	int	tcps_delayed;		/* packets out of sequence? */
};

#ifdef KERNEL
struct	tcpstat tcpstat;	/* tcp statistics */
struct	tcpiphdr *tcp_template();
struct	tcpcb *tcp_close(), *tcp_drop();
struct	tcpcb *tcp_timers(), *tcp_disconnect(), *tcp_usrclosed();
#endif

/*
 * socket emulation; actually structures for translating
 * between streams and tcp control blocks.
 * note that these sockets typically also take the place of
 * the 4.2BSD internet control blocks.
 */

struct socket{
	struct		tcpcb *so_tcpcb;
	struct		queue *so_rq;
	struct		queue *so_wq;
	int		so_rcount;
	int		so_wcount;	/* because q->count is inaccurate */
	struct		socket *so_head; /* parent who listened */
	int		so_dev;
	int		so_state;
	int		so_options;
	in_addr		so_laddr;
	in_addr		so_faddr;
	tcp_port	so_lport;
	tcp_port	so_fport;
	int		so_oobmark;
	int		so_delimcnt;	/* to detect logical eof */
};

#define SO_DONTROUTE	0x1
#define SO_KEEPALIVE	0x2
#define SO_ACCEPTCONN	0x4	/* this is real */

#define SS_OPEN		0x1	/* by user */
#define SS_PLEASEOPEN	0x2	/* waiting for user open */
#define SS_RCVATMARK	0x4	/* some kind of OOB */
#define SS_WAITING	0x8	/* wait for user control */
#define	SS_ACTIVE	0x10	/* has tcp action */
#define SS_HANGUP	0x20	/* HANGUP on TH_FIN */
#define SS_HUNGUP	0x40	/* socket has been hung up (avoid multiple) */
#define SS_WCLOSED	0x80	/* write side is closed */

#define socantsendmore(so) (so->so_delimcnt>1 || so->so_state&SS_WCLOSED)
#define sbrcvspace(so)	(so->so_rq ?\
			 (sorcvhiwat(so) - so->so_rcount)\
			 : 0)
#define sosndcc(so)	(so->so_wq ? (so->so_wcount) : 0)
#define sototcpcb(so)	(so->so_tcpcb)
#define sorcvhiwat(so)	(so->so_rq ? (so->so_rq->qinfo->limit) : 0)
#define sohasoutofband(so)

#ifdef KERNEL
extern struct socket *so_lookup();
extern struct socket *sonewconn();
#endif

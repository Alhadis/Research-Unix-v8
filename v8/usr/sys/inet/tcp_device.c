/*
 * tcp_device.c
 */

#include "tcp.h"
#if NTCP
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/stream.h"
#include "../h/ioctl.h"
#include "../h/ttyld.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/inet/in.h"
#include "../h/inet/mbuf.h"
#include "../h/inet/tcp.h"
#include "../h/inet/tcp_timer.h"
#include "../h/inet/tcp_seq.h"
#include "../h/inet/tcp_var.h"
#include "../h/inet/tcp_user.h"
#include "../h/inet/tcp_fsm.h"
#include "../h/inet/socket.h"

extern int tcp_busy;	/* set to discourage timers & ensuing panics */

int	nodev(), tcpdopen(), tcpdclose(), tcpdput();
int	tcpdosrv(), tcpdisrv();
static	struct qinit tcpdrinit = { nodev, tcpdisrv, tcpdopen, tcpdclose, 2048, 64 };
		/* was 1024, 64 originally */
	struct qinit tcpdwinit = { tcpdput, tcpdosrv, tcpdopen, tcpdclose, 512, 64 };
		/* was 256, 64 originally */
struct	streamtab tcpdinfo = { &tcpdrinit, &tcpdwinit };

int Ntcp = NTCP;	/* for netstat */
struct tcpcb tcpcb[NTCP];
struct socket tcpsocks[NTCP];

tcpdopen(q, dev)
register struct queue *q;
dev_t dev;
{
	struct socket *so;

	dev = minor(dev);
	if(dev >= NTCP)
		return(0);
	so = &tcpsocks[dev];
	if((dev&01) == 0 && (so->so_state&SS_ACTIVE) == 0)
		return(0);
	if(so->so_state&SS_WAITING)
		return(0);
	if((so->so_options&SO_ACCEPTCONN) && (so->so_state&SS_OPEN))
		return(0);
	if((so->so_state & (SS_ACTIVE|SS_OPEN|SS_PLEASEOPEN)) == SS_ACTIVE)
		return(0);
	if(q->ptr && dev&01)	/* re-opening outgoing port */
		return(0);
	if(q->ptr)
		return(1);
	tcp_busy++;
	if((so->so_state & SS_PLEASEOPEN) == 0){
		bzero(so, sizeof(struct socket));
		so->so_state |= SS_WAITING;
	}
	so->so_state |= SS_OPEN;
	so->so_dev = dev;
	so->so_rq = q;
	so->so_wq = WR(q);
	q->ptr = (caddr_t)so;
	WR(q)->flag |= QNOENB|QBIGB;
	WR(q)->ptr = (caddr_t)so;
	--tcp_busy;
	if(so->so_state & SS_PLEASEOPEN){
		so->so_state &= ~SS_PLEASEOPEN;
		qenable(WR(q));		/* to force out rcv wnd update */
	}
	return(1);
}

tcpdclose(q)
register struct queue *q;
{
	struct socket *so;
	struct tcpcb *tp;

	so = (struct socket *)q->ptr;
	tcp_busy++;
	so->so_state &= ~(SS_OPEN|SS_WAITING);
	so->so_rq = so->so_wq = (struct queue *) 0;
	so->so_wcount = so->so_rcount = 0;
	tp = so->so_tcpcb;
	if(tp == 0){
		--tcp_busy;
		return;
	}
	if(tp->t_state > TCPS_LISTEN)
		tcp_disconnect(tp);
	else
		tcp_close(tp);
	--tcp_busy;
}

tcpdput(q, bp)
register struct queue *q;
register struct block *bp;
{
	struct socket *so;
	int s, x;
	register union stmsg *sp;
	struct tcpcb *tp;

	so = (struct socket *)q->ptr;
	switch(bp->type){
	case M_IOCTL:
		sp = (union stmsg *)bp->rptr;
		bp->type = M_IOCACK;
		switch(sp->ioc0.com){
		case TIOCSETP:
		case TIOCSETN:
			x = sp->ioc1.sb.sg_ispeed;
			bp->wptr = bp->rptr;
			bp->type = M_IOCACK;
			qreply(q, bp);
			if(x == 0)
				tcp_putctl(OTHERQ(q), M_HANGUP);
			return;
		case TIOCGETP:
			sp->ioc1.sb.sg_ispeed =
			  sp->ioc1.sb.sg_ospeed = B9600;
			break;
		case TCPIOHUP:
			so->so_state |= SS_HANGUP;
			bp->wptr = bp->rptr;
			bp->type = M_IOCACK;
			qreply(q, bp);
			return;
		default:
			bp->type = M_IOCNAK;
		}
		qreply(q, bp);
		return;
	case M_DATA:
		if(socantsendmore(so)){
			freeb(bp);
			return;
		}
		s = spl6();
		so->so_delimcnt = 0;
		so->so_wcount += BLEN(bp);	/* BEFORE the putq */
		putq(q, bp);
		splx(s);
		if((so->so_options&SO_ACCEPTCONN) == 0
		   && (so->so_state&SS_WAITING) == 0
		   && (tp = so->so_tcpcb)){
			/* so as to invoke tcp_output() */
			if(tp->t_state > TCPS_CLOSE_WAIT)
				printf("data after CLOSE_WAIT?\n");
			qenable(q);
		}
		break;
	case M_DELIM:
		if(so->so_options&SO_ACCEPTCONN){
			printf("DELIM on listener\n");
		} else if(so->so_state & SS_WAITING){
			putq(q, bp);
			/* to invoke tcpduser */
			qenable(q);
		} else {
			/* two back to back delims constitute logical eof */
			freeb(bp);
			s = spl6();
			if(socantsendmore(so)){
				splx(s);
				return;
			}
			if(++(so->so_delimcnt) > 1){
				splx(s);
				qenable(q);
			} else
				splx(s);
		}
		break;
	default:
		freeb(bp);
		break;
	}
}

tcpdosrv(q)
struct queue *q;
{
	register struct socket *so;
	register struct tcpcb *tp;

	so = (struct socket *)q->ptr;
	if (so->so_state&SS_WCLOSED)
		return;
	tcp_busy++;
	if(so->so_delimcnt > 1){
		so->so_state |= SS_WCLOSED;
		if((tp = so->so_tcpcb) == 0){
			printf("delimcnt but no tp\n");
			--tcp_busy;
			return;
		}
		tp = tcp_usrclosed(tp);
		if(tp)
			tcp_output(tp);
	} else {
		if((so->so_options&SO_ACCEPTCONN) == 0
		   && (so->so_state&SS_WAITING) == 0
		   && (tp = so->so_tcpcb)){
			tcp_output(tp);
		} else {
			tcpduser(so);
		}
	}
	--tcp_busy;
}

tcpdrint(bp, so)
register struct block *bp;
struct socket *so;
{
	register struct block *bp1;
	register struct queue *q;

	q = so->so_rq;
	if(q){
		while(bp){
			bp1 = bp->next;
			so->so_rcount += bp->wptr - bp->rptr;
			if(bp->wptr == bp->rptr)
				freeb(bp);
			else
				putq(q, bp);
			bp = bp1;
		}
	} else {
		printf("tcpdrint but no so->so_rq\n");
		bp_free(bp);
	}
}

tcpdisrv(q)
struct queue *q;
{
	struct socket *so = (struct socket *)(q->ptr);
	struct block *bp;

	while((q->next->flag&QFULL) == 0){
		if(bp = getq(q)){
			if(bp->type == M_DATA)
				so->so_rcount -= bp->wptr - bp->rptr;
			if(so->so_rcount < 0)
				panic("so_rcount");
			(*q->next->qinfo->putp)(q->next, bp);
		} else
			break;
	}
	if(q->count <= q->qinfo->lolimit)
		qenable(OTHERQ(q));	/* update remote send window */
}

/*
 * imitation tcp_usrreq
 */
tcpduser(so)
register struct socket *so;
{
	extern struct ipif *ip_ifwithaddr();
	struct tcpuser *tu;
	struct block *bp, *bp1, *head;
	register struct tcpcb *tp;

	bp = bp1 = head = NULL;
	while(bp = getq(so->so_wq)){
		if(bp->type != M_DATA){
			freeb(bp);
		} else if(bp1 == NULL){
			bp1 = head = bp;
			bp->next = NULL;
		} else {
			bp1->next = bp;
			bp1 = bp;
			bp->next = NULL;
		}
	}
	if(head == NULL)
		return;
	so->so_wcount = 0;
	bp = head;
	if(BLEN(bp) < sizeof(struct tcpuser)){
/*
		printf("tcpuser short\n");
*/
		bp_free(bp);
		return;
	}
	tu = (struct tcpuser *)bp->rptr;
	if(so->so_tcpcb)
		printf("%d: tcpduser w/ tcpcb\n", so->so_dev);
	switch(tu->cmd){
	case TCPC_CONNECT:
		if(so->so_state&SS_ACTIVE)
			goto bad;
		if (tu->src != INADDR_ANY) {
			/* has the user has specified a legal local address? */
			if (ip_ifwithaddr(tu->src) == 0)
				goto bad;
		} else {
			/* pick a local address related to the destination */
			tu->src = ip_hoston(tu->dst);
			if(tu->src == INADDR_ANY)
				goto bad;
		}
		if(tcp_attach(so))
			goto bad;
		if(sobind(so, tu->src, tu->sport))
			goto bad;
		tp = so->so_tcpcb;
		so->so_fport = tu->dport;
		so->so_faddr = tu->dst;
		so->so_options = 0;
		tp->t_template = tcp_template(tp);
		if(tp->t_template == 0)
			goto bad;
		soisconnecting(so);
		tp->t_state = TCPS_SYN_SENT;
		tp->t_timer[TCPT_KEEP] = TCPTV_KEEP;
		tp->iss = tcp_iss; tcp_iss += TCP_ISSINCR/2;
		tcp_sendseqinit(tp);
		so->so_state &= ~SS_WAITING;
		tcp_output(tp);
		break;
	case TCPC_LISTEN:
		if(so->so_state&SS_ACTIVE)
			goto bad;
		if (tu->src != INADDR_ANY) {
			/* has the user has specified a legal local address? */
			if (ip_ifwithaddr(tu->src) == 0)
				goto bad;
		}
		if(tcp_attach(so))
			goto bad;
		if(sobind(so, tu->src, tu->sport))
			goto bad;
		tp = so->so_tcpcb;
		tp->t_state = TCPS_LISTEN;
		so->so_options |= SO_ACCEPTCONN;
		so->so_fport = tu->dport;
		so->so_state &= ~SS_WAITING;
		break;
	default:
		goto bad;
	}
	bp_free(bp);
	return;
bad:
	bp_free(bp);
	tcp_hungup(so);
}

tcp_attach(so)
struct socket *so;
{
	register struct tcpcb *tp;
	extern struct tcpcb *tcp_newtcpcb();

	tp = tcp_newtcpcb(so);
	if(tp == 0)
		return(1);
	tp->t_socket = so;
	tp->t_state = TCPS_CLOSED;
	return(0);
}

struct tcpcb *
tcp_disconnect(tp)
register struct tcpcb *tp;
{
	struct socket *so = tp->t_socket;

	if(tp->t_state < TCPS_ESTABLISHED)
		tp = tcp_close(tp);
	else {
		soisdisconnecting(so);
		tp = tcp_usrclosed(tp);
		if(tp)
			tcp_output(tp);
	}
	return(tp);
}

struct tcpcb *
tcp_usrclosed(tp)
register struct tcpcb *tp;
{

	switch(tp->t_state){

	case TCPS_CLOSED:
	case TCPS_LISTEN:
	case TCPS_SYN_SENT:
		tp->t_state = TCPS_CLOSED;
		tp = tcp_close(tp);
		break;

	case TCPS_SYN_RECEIVED:
	case TCPS_ESTABLISHED:
		tp->t_state = TCPS_FIN_WAIT_1;
		break;

	case TCPS_CLOSE_WAIT:
		tp->t_state = TCPS_LAST_ACK;
		break;
	}
	if(tp && tp->t_state >= TCPS_FIN_WAIT_2)
		soisdisconnected(tp->t_socket);
	return(tp);
}

tcp_isconnected(so)
struct socket *so;
{
	struct block *bp;
	struct tcpreply *tr;
	struct socket *rso;

	if(so->so_head)
		rso = so->so_head;
	else
		rso = so;
	if((rso->so_state & SS_OPEN) == 0){
		printf("isconnected, no fd ref\n");
		return;
	}
	bp = allocb(64);
	if(bp == 0)
		return;
	bp->next = NULL;
	bp->type = M_DATA;
	bp->wptr += sizeof(struct tcpreply);
	tr = (struct tcpreply *)bp->rptr;
	tr->reply = TCPR_OK;
	tr->dport = so->so_fport;
	tr->dst = so->so_faddr;
	tr->src = so->so_laddr;
	tr->tcpdev = so->so_dev;
	tcpdrint(bp, rso);
}

tcp_hungup(so)
register struct socket *so;
{
	register struct queue *q;

	q = so->so_rq;
	if(q == 0)
		return;
	tcp_putctl(q, M_HANGUP);
}

/*
 * find a spare odd tcp device for a new passive-end
 * connection.
 */
struct socket *
tcp_newconn(so)
struct socket *so;
{
	struct socket *nso;

	if(so->so_rq && (so->so_rq->flag&QFULL)){
		printf("listen %d q full\n", so->so_lport);
		return(0);
	}
	for(nso = &tcpsocks[0]; nso < &tcpsocks[NTCP]; nso += 2){
		if((nso->so_state & (SS_OPEN|SS_ACTIVE)) == 0){
			bzero(nso, sizeof(struct socket));
			nso->so_head = so;
			nso->so_dev = nso - tcpsocks;
			return(nso);
		}
	}
	return(0);
}

struct socket *
so_lookup(faddr, fport, laddr, lport)
in_addr faddr, laddr;
tcp_port fport, lport;
{
	register struct socket *so, *match = 0;
	register in_addr net, host;
	int matchwild = 5, wildcard;

	net = in_netof(laddr);
	host = in_hostof(laddr);
	for(so = &tcpsocks[0]; so < &tcpsocks[NTCP]; so++){
		if(so->so_tcpcb == 0)
			continue;
		if((so->so_state&(SS_OPEN|SS_ACTIVE)) == 0)
			continue;
		if(so->so_state & SS_WAITING)
			continue;
		wildcard = 0;

		if(so->so_lport != TCPPORT_ANY){
			if(so->so_lport != lport)
				continue;
		} else
				wildcard++;		/* port is wildcarded */

		if(so->so_laddr == net){
			wildcard++;			/* host is wildcarded */
		} else if(so->so_laddr != INADDR_ANY){
			if(laddr == INADDR_ANY)
				wildcard += 2;		/* host & net wildcarded */
			else if(so->so_laddr != laddr)
				continue;
		} else {
			if(laddr != INADDR_ANY)
				wildcard += 2;		/* host & net wildcarded */
		}
		if(so->so_faddr != INADDR_ANY){
			if(faddr == INADDR_ANY)
				wildcard += 2;		/* host & net wildcarded */
			else if(so->so_faddr != faddr ||
				so->so_fport != fport)
				continue;
		} else {
			if(faddr)
				wildcard += 2;		/* host & net wildcarded */
		}
		if(wildcard < matchwild){
			match = so;
			matchwild = wildcard;
			if(matchwild == 0)
				break;
		}
	}
	return(match);
}

/* n chars were acked; drop them now */
sbsnddrop(so, n)
register struct socket *so;
register int n;
{
	register struct queue *q;
	register int i;
	register struct block *bp;

	q = so->so_wq;
	if(q == 0)
		return;
	bp = 0;
	while(n > 0 && (bp = getq(q))){
		i = MIN(BLEN(bp), n);
		bp->rptr += i;
		n -= i;
		so->so_wcount -= i;
		if(bp->rptr >= bp->wptr){
			freeb(bp);
			bp = 0;
		} else if(n > 0){
			panic("sbsnddrop");
		}
	}
	if(bp)
		putbq(q, bp);
}


sobind(so, addr, port)
register struct socket *so;
register in_addr addr;
register tcp_port port;
{
	register struct socket *sp;
	static tcp_port next_port = 600;

	so->so_lport = 0;
	if(port){
		/* what about, for instance, restarting rlogind when
		 * people are rlogin'd here?
		 */
		for(sp = &tcpsocks[0]; sp < &tcpsocks[NTCP]; sp++){
			if(sp->so_tcpcb == 0)
				continue;
			if((sp->so_state&(SS_OPEN|SS_ACTIVE)) == 0)
				continue;
			if(sp->so_lport == port && sp->so_laddr == addr) {
				return(1);
			}
		}
		so->so_lport = port;
		so->so_laddr = addr;
		return(0);
	}
	/* pick one for him */
	if(next_port >= 1024)
		next_port = 600;
	port = next_port;
	while(1){
		if(sobind(so, addr, next_port) == 0){
			next_port++;
			return(0);
		}
		next_port++;
		if(next_port >= 1024)
			next_port = 600;
		if(next_port == port)	/* tried them all */
			break;
	}
	return(1);
}

tcp_cantrcvmore(so)
register struct socket *so;
{
	register struct queue *q;

	q = so->so_rq;
	if(q == NULL)
		return;
	if(so->so_state & SS_HANGUP)
		tcp_putctl(q, M_HANGUP);
	else {
		/* two delims ensure a zero length read at the process */
		tcp_putctl(q, M_DELIM);
		tcp_putctl(q, M_DELIM);
	}
}

tcp_putctl(q, c)
register struct queue *q;
{
	register struct block *bp;

	if ((bp = allocb(0)) == NULL) {
		printf("tcp_putctl: no more blocks\n");
		return(0);
	}
	bp->type = c;
	putq(q, bp);
	return(1);
}
#endif

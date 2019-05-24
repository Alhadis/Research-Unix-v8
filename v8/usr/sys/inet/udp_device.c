/*
 * udp device driver.
 */

#include "udp.h"
#if NUDP
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/stream.h"
#include "../h/ioctl.h"
#include "../h/ttyld.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/inet/in.h"
#include "../h/inet/ip.h"
#include "../h/inet/ip_var.h"
#include "../h/inet/udp.h"
#include "../h/inet/udp_var.h"
#include "../h/inet/udp_user.h"

int	nodev(), udpdopen(), udpdclose(), udpdput();
int	udpdosrv();
static	struct qinit udpdrinit = { nodev, NULL, udpdopen, udpdclose, 0, 0 };
static	struct qinit udpdwinit = { udpdput, udpdosrv, udpdopen, udpdclose,
				   UDP_BODY_LIMIT, 64 };
struct	streamtab udpdinfo = { &udpdrinit, &udpdwinit };

int Nudp = NUDP;	/* for netstat */
struct udp udpconn[NUDP];

in_addr udpbaddst;

udpdopen(q, dev)
register struct queue *q;
dev_t dev;
{
	struct udp *udp;

	dev = minor(dev);
	if(dev >= NUDP)
		return(0);
	udp = &udpconn[dev];
	if(q->ptr)
		return(0);
	udp->rq = q;
	udp->flags = UDP_INIT;
	udp->head = udp->tail = 0;
	q->ptr = (caddr_t)udp;
	q->flag |= QDELIM;
	WR(q)->flag |= QBIGB;
	WR(q)->ptr = (caddr_t)udp;
	return(1);
}

udpdclose(q)
register struct queue *q;
{
	struct udp *udp;

	udp = (struct udp *)q->ptr;
	udp->rq = (struct queue *)0;
	q->ptr = (caddr_t)0;
}

udpdput(q, bp)
register struct queue *q;
register struct block *bp;
{
	struct udp *udp;
	int s, x;
	register union stmsg *sp;
	struct foo{
		u_long dst;
		u_long gate;
	} *foo;

	udp = (struct udp *)q->ptr;
	switch(bp->type){
	case M_IOCTL:
		bp->type = M_IOCNAK;
		bp->wptr = bp->rptr;
		qreply(q, bp);
		return;
	case M_DATA:
		putq(q, bp);
		break;
	case M_DELIM:
		putq(q, bp);
		qenable(q);
		break;
	default:
		freeb(bp);
		break;
	}
}

udpdosrv(q)
register struct queue *q;
{
	register struct block *bp;
	register struct udp *udp;

	udp = (struct udp *)q->ptr;
	while(bp = getq(q)){
		bp->next = 0;
		if(bp->type != M_DATA) {
			freeb(bp);
			if (udp->flags & UDP_INIT)
				udpduser(udp->head, udp);
			else {
				if ((udp->flags & UDP_CONNECTED) == 0) {
					/* get user-supplied destination */
					if(udpdaddr(udp)) {
						bp_free(udp->head);
						udp->head = udp->tail = NULL;
						return;
					}
				} else if (udp->head == NULL) {
					/* zero length message */
					udp->head = allocb(0);
				}
				udp_output(udp->head, udp);
			}
			udp->head = udp->tail = NULL;
		} else if(udp->head == 0){
			udp->head = udp->tail = bp;
		} else {
			udp->tail->next = bp;
			udp->tail = bp;
		}
	}
}

/* extract address from a message */
udpdaddr(udp)
register struct udp *udp;
{
	struct udpaddr *uap;

	/* get destination */
	if((udp->head = bp_pullup(udp->head, sizeof(struct udpaddr))) == NULL)
		return(1);
	uap = (struct udpaddr *)udp->head->rptr;
	udp->dst = uap->host;
	udp->dport = uap->port;
	udp->head->rptr += sizeof(struct udpaddr);

	/* host address for datagrams depends on route */
	if ((udp->flags & UDP_CONNECTED) == 0) {
		udp->src = ip_hoston(udp->dst);
		if(udp->src == 0) {
			udpbaddst = udp->dst;
			return(1);
		}
	}
	return(0);
}

udpdrint(bp, ui)
register struct block *bp;
struct udpiphdr *ui;
{
	register struct block *bp1;
	register struct queue *q;
	register struct udp *udp;
	register struct udpaddr *uap;
	int	ps = spl6();

	/* find the udp port */
	for (udp = udpconn; udp < &udpconn[NUDP]; ++udp) {
		if (udp->rq == 0)
			continue;
		if (udp->sport != ui->ui_dport)
			continue;
		if (udp->flags & UDP_LISTEN)
			break;
		if ((udp->flags & UDP_CONNECTED) == 0)
			break;
		if (udp->dport == ui->ui_sport && udp->dst == ui->ui_src)
			break;
	}

	if (udp == &udpconn[NUDP]) {
		bp_free(bp);
		splx(ps);
		return;
	}
	if (udp->flags & UDP_LISTEN) {
		udp->dport = ui->ui_sport;
		udp->dst = ui->ui_src;
		if (udp->src == 0) {
			/* use host name connecting end called us */
			udp->src = ui->ui_dst;
		}
		udp->flags |= UDP_CONNECTED;
		udp->flags &= ~UDP_LISTEN;
		udpreply(udp);
	}
	splx(ps);

	if((q = udp->rq) == NULL) {
		printf("udpdrint but no read queue\n");
		bp_free(bp);
		return;
	}
	if(q->next->flag & QFULL) {
		bp_free(bp);
		return;
	}
	if ((udp->flags & UDP_CONNECTED) == 0) {
		/* if not acting as a connected protocol, pass user source addr */
		bp1 = allocb(sizeof(struct udpaddr));
		if (bp1 == NULL) {
			printf("udprint: out of blocks\n");
			bp_free(bp);
			return;
		}
		bp1->wptr += sizeof(struct udpaddr);
		uap = (struct udpaddr *)bp1->rptr;
		uap->host = ui->ui_src;
		uap->port = ui->ui_sport;
		putq(q->next, bp1);
	}
	while(bp){
		bp1 = bp->next;
		putq(q->next, bp);
		bp = bp1;
	}
	putctl(q->next, M_DELIM);
}

/*
 * imitation udp_usrreq
 */
udpduser(bp, udp)
register struct block *bp;
register struct udp *udp;
{
	struct udpuser *uu;

	if ((bp = bp_pullup(bp, sizeof(struct udpuser))) == 0) {
		printf("udpuser short\n");
		bp_free(bp);
		return;
	}
	uu = (struct udpuser *)bp->rptr;
	switch(uu->cmd){
	case UDPC_DATAGRAM:
		if (udpbind(udp, uu->sport))
			goto bad;
		udp->flags &= ~UDP_INIT;
		udpreply(udp);
		break;
	case UDPC_CONNECT:
		if (udpbind(udp, uu->sport))
			goto bad;
		udp->dport = uu->dport;
		udp->dst = uu->dst;
		udp->flags |= UDP_CONNECTED;
		udp->flags &= ~UDP_INIT;
		udpreply(udp);
		/* pick a local address related to the destination */
		udp->src = ip_hoston(udp->dst);
		if(udp->src == 0)
			goto bad;
		break;
	case UDPC_LISTEN:
		if (udpbind(udp, uu->sport))
			goto bad;
		udp->flags |= UDP_LISTEN;
		udp->flags &= ~UDP_INIT;
		break;
	default:
bad:		putctl(udp->rq->next, M_HANGUP);
		return;
	}
	bp_free(bp);
}

udpreply(udp)
register struct udp *udp;
{
	register struct queue *q;
	struct udpreply ur;

	ur.reply = UDPR_OK;
	ur.dport = udp->dport;
	ur.dst = udp->dst;
	ur.udpdev = udp - udpconn;

	q = udp->rq;
	putcpy(q->next, &ur, sizeof(ur));
	putctl(q->next, M_DELIM);
}

udpbind(udp, port)
register struct udp *udp;
register int port;
{
	register struct udp *udp2;
	static udp_port udpport = 600;

	udp->sport = 0;
	if(port){
		for(udp2 = &udpconn[0]; udp2 < &udpconn[NUDP]; udp2++){
			if(udp2->rq == 0)
				continue;
			if(udp2->sport == port)
				return(1);
		}
		udp->sport = port;
		return(0);
	}

	/* pick one for him */
	if(udpport >= 1024)
		udpport = 600;
	port = udpport;
	while(1){
		if(udpbind(udp, udpport) == 0){
			udpport++;
			return(0);
		}
		udpport++;
		if(udpport >= 1024)
			udpport = 600;
		if(udpport == port)	/* tried them all */
			break;
	}
	return(1);
}

#endif

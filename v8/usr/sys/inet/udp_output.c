/*	udp_output.c	6.1	83/07/29	*/
#include "udp.h"
#if NUDP > 0

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/stream.h"
#include "../h/inet/mbuf.h"

#include "../h/inet/in.h"
#include "../h/inet/ip.h"
#include "../h/inet/ip_var.h"
#include "../h/inet/udp.h"
#include "../h/inet/udp_var.h"

#define	IPPROTO_UDP	17
#define	UDP_DATA_LEN	(1500-sizeof(struct udpiphdr))

udp_output(bp, udp)
register struct block *bp;
register struct udp *udp;
{
	register struct block *head;
	register struct udpiphdr *ui;
	register int len = 0, tlen;
	int flags;

	/*
	 * Calculate data length and get a mbuf
	 * for UDP and IP headers.
	 */

	head = bp;
	while(bp) {
		len += BLEN(bp);
		bp = bp->next;
	}

	if (NULL == (bp = allocb(sizeof(struct udpiphdr)))) {
		printf("Can't allocate block for udpiphdr\n");
		bp_free(head);
		return;
	}
	bp->wptr += sizeof(struct udpiphdr);
/*
 * point "next" of allocated block containing udpiphdr to data
 * point head to this block
 */
	bp->next = head;

	/*
	 * Fill in mbuf with extended UDP header
	 * and addresses and length put into network format.
	 */
	ui = mtod(bp, struct udpiphdr *);
	ui->ui_next = ui->ui_prev = 0;
	ui->ui_x1 = 0;
	ui->ui_pr = IPPROTO_UDP;
	ui->ui_len = (u_short)len + sizeof (struct udphdr);
	ui->ui_len = htons(ui->ui_len);
	ui->ui_src = htonl(udp->src);
	ui->ui_dst = htonl(udp->dst);
	ui->ui_sport = htons(udp->sport);
	ui->ui_dport = htons(udp->dport);
	ui->ui_ulen = ui->ui_len;

	/* Stuff checksum and output datagram. */
	ui->ui_sum = 0;
	if ((ui->ui_sum = in_cksum(bp, sizeof(struct udpiphdr)+len)) == 0)
		ui->ui_sum = -1;


	/* Put length and timeout time into the `real' ip header. */
	((struct ip *)ui)->ip_len = sizeof(struct udpiphdr)+len;
	((struct ip *)ui)->ip_ttl = MAXTTL;

	/* Ip expects internet addresses in host order. */
	ui->ui_src = ntohl(ui->ui_src);
	ui->ui_dst = ntohl(ui->ui_dst);
	udp_ldout(bp);
}

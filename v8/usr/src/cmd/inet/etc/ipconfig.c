#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sgtty.h>
#include <sys/ethernet.h>
#include "config.h"

extern errno;


extern unsigned short htons();
extern int ip_ld;

main(argc, argv)
char *argv[];
{
	char *dev, *me, *it, *arp;
	unsigned long myaddr, hisaddr, inaddr;
	int ipfd, enfd, x, ld;

	if(argc < 4){
		fprintf(stderr, "Usage: %s device my-addr his-addr [arp-device]\n",
			argv[0]);
		exit(1);
	}
	dev = argv[1];
	me = argv[2];
	it = argv[3];
	if(argc > 4)
		arp = argv[4];
	else
		arp = 0;

	myaddr = in_address(me);
	if(myaddr == 0){
		fprintf(stderr, "ipconfig: unknown host %s\n", me);
		exit(1);
	}
	hisaddr = in_address(it);
	if(hisaddr == 0){
		fprintf(stderr, "ipconfig: unknown host/net %s\n", it);
		exit(1);
	}
	signal(SIGHUP, SIG_IGN);
	ipfd = open(dev, 2);
	if(ipfd < 0){
		perror(dev);
		exit(1);
	}

	if(arp){
		x = htons((unsigned short)ETHERPUP_IPTYPE);
		if(ioctl(ipfd, ENIOTYPE, &x) < 0){
			perror("ENIOTYPE");
			exit(1);
		}
	}
	if(ioctl(ipfd, FIOPUSHLD, &ip_ld) < 0){
		perror("PUSHLD");
		exit(1);
	}
	if(ioctl(ipfd, IPIOLOCAL, &myaddr) < 0){
		perror("IPIOLOCAL");
		exit(1);
	}
	if(hisaddr & 0xff){
		ioctl(ipfd, IPIOHOST, &hisaddr);
	} else {
		ioctl(ipfd, IPIONET, &hisaddr);
	}
	if(arp == 0){
		pause();	/* forever, hopefully */
		exit(0);
	}

	if(ioctl(ipfd, IPIOARP, 0) < 0){
		perror("IPIOARP");
		exit(1);
	}
	enfd = open(arp, 2);
	if(enfd < 0){
		perror(arp);
		exit(1);
	}
	doarp(ipfd, enfd, myaddr);
}


/*
 * Address resolution
 */

struct ether_arp{
	/* driver goo */
	struct ether_in arp_ether;

	/* arp stuff */
	u_short	arp_hrd;
#define ARPHRD_ETHER	1
	u_short	arp_pro;
	u_char	arp_hln;
	u_char	arp_pln;
	u_short	arp_op;
#define ARPOP_REQUEST	1
#define ARPOP_REPLY	2
	u_char	arp_sha[6];	/* sender ether addr */
	u_char	arp_spa[4];	/* sender internet addr */
	u_char	arp_tha[6];	/* target ether addr */
	u_char	arp_tpa[4];	/* target internet addr */
};

u_char broadaddr[6] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

doarp(ipfd, enfd, myinaddr)
u_long myinaddr;
{
	u_char myenaddr[6];
	fd_set rdfds;
	struct ether_arp *ap;
	int x, n;
	char buf[2000];
	u_long spa, tpa;
	struct goo{
		u_long inaddr;
		u_char enaddr[6];
	} goo;

	x = htons((u_short)ETHERPUP_ARPTYPE);
	if(ioctl(enfd, ENIOTYPE, &x) < 0){
		perror("ENIOTYPE");
		exit(1);
	}
	if(ioctl(enfd, ENIOADDR, myenaddr) < 0){
		perror("ENIOADDR");
		exit(1);
	}

	FD_ZERO(rdfds);
	while(1){
		FD_SET(ipfd, rdfds);
		FD_SET(enfd, rdfds);

		if(select(20, &rdfds, 0, 2000) < 0){
			if(errno == EINTR)
				continue;
			perror("select");
			exit(1);
		}
		if(FD_ISSET(ipfd, rdfds)){
			if(read(ipfd, &tpa, sizeof(tpa)) != sizeof(tpa)){
				perror("in read");
				continue;
			}
			arpwhohas(enfd, ipfd, myenaddr, myinaddr, tpa);
		}
		if(FD_ISSET(enfd, rdfds)){
			if(read(enfd, buf, sizeof(buf)) <= 0){
				perror("en read");
				exit(1);
			}
			arpinput(ipfd, enfd, myenaddr, myinaddr, buf);
		}
	}
}

arpwhohas(enfd, ipfd, myenaddr, myinaddr, addr)
u_char myenaddr[6];
u_long addr, myinaddr;
{
	struct goo{
		u_long inaddr;
		u_char enaddr[6];
	} goo;
	struct ether_arp a;

	if(addr == (myinaddr & 0xffffff00)){
		goo.inaddr = addr;
		bcopy(broadaddr, goo.enaddr, 6);
		ioctl(ipfd, IPIORESOLVE, &goo);
		return;
	}
	if(addr == myinaddr){
		goo.inaddr = addr;
		bcopy(myenaddr, goo.enaddr, 6);
		ioctl(ipfd, IPIORESOLVE, &goo);
		return;
	}
	bcopy(broadaddr, a.arp_ether.dhost, 6);
	a.arp_ether.type = htons(ETHERPUP_ARPTYPE);

	a.arp_hrd = htons(ARPHRD_ETHER);
	a.arp_pro = htons(ETHERPUP_IPTYPE);
	a.arp_hln = 6;
	a.arp_pln = 4;
	a.arp_op = htons(ARPOP_REQUEST);

	bcopy(myenaddr, a.arp_sha, 6);
	myinaddr = htonl(myinaddr);
	bcopy(&myinaddr, a.arp_spa, 4);
	addr = htonl(addr);
	bcopy(&addr, a.arp_tpa, 4);

	write(enfd, &a, sizeof(a));
}

arpinput(ipfd, enfd, myenaddr, myinaddr, ap)
u_char myenaddr[6];
u_long myinaddr;
struct ether_arp *ap;
{
	struct goo{
		u_long inaddr;
		u_char enaddr[6];
	} goo;
	u_long spa, tpa;


	bcopy(ap->arp_spa, &spa, sizeof(spa));
	bcopy(ap->arp_tpa, &tpa, sizeof(tpa));
	spa = ntohl(spa);
	tpa = ntohl(tpa);

	if(ntohs(ap->arp_pro) != ETHERPUP_IPTYPE)
		return;
	if(!bcmp(ap->arp_sha, myenaddr, 6))
		return;
	if(spa == myinaddr){
		printf("forgery from me!!!\n");
		if(ap->arp_op == ARPOP_REQUEST)
			goto reply;
		return;
	}
	if(ap->arp_op == ntohs(ARPOP_REPLY)){
		goo.inaddr = spa;
		bcopy(ap->arp_sha, goo.enaddr, 6);
		if(ioctl(ipfd, IPIORESOLVE, &goo) < 0)
			perror("IPIORESOLVE");
		return;
	}
reply:
	if(tpa != myinaddr)
		return;
	ap->arp_hrd = htons(ARPHRD_ETHER);
	ap->arp_pro = htons(ETHERPUP_IPTYPE);
	ap->arp_op = htons(ARPOP_REPLY);

	tpa = htonl(spa);
	spa = htonl(myinaddr);
	bcopy(&tpa, ap->arp_tpa, 4);
	bcopy(&spa, ap->arp_spa, 4);
	bcopy(ap->arp_sha, ap->arp_tha, 6);
	bcopy(myenaddr, ap->arp_sha, 6);

	bcopy(ap->arp_tha, ap->arp_ether.dhost, 6);
	ap->arp_ether.type = htons(ETHERPUP_ARPTYPE);

	write(enfd, ap, sizeof(struct ether_arp));
}

bcmp(a, b, n)
u_char *a, *b;
{
	while(n-- > 0){
		if(*a != *b)
			return(1);
		a++, b++;
	}
	return(0);
}

arppr(a)
struct ether_arp a;
{
	u_long spa, tpa;

	printf("dhost "); enpr(a.arp_ether.dhost);
	printf("shost "); enpr(a.arp_ether.shost);
	printf("type  %x\n", ntohs(a.arp_ether.type));
	a.arp_hrd = ntohs(a.arp_hrd);
	a.arp_pro = ntohs(a.arp_pro);
	a.arp_op = ntohs(a.arp_op);

	bcopy(a.arp_spa, &spa, sizeof(spa));
	bcopy(a.arp_tpa, &tpa, sizeof(tpa));
	tpa = ntohl(tpa);
	spa = ntohl(spa);

	printf("hrd %d pro %x op %d spa %x tpa %x\n",
		a.arp_hrd, a.arp_pro, a.arp_op, spa, tpa);
	printf("sha "); enpr(a.arp_sha);
	printf("tha "); enpr(a.arp_tha);
}

enpr(en)
u_char *en;
{
	int i;

	for(i = 0; i < 6; i++)
		printf("%02x ", (en[i])&0xff);
	printf("\n");
}

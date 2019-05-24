#include <stdio.h>
#include <sys/param.h>
#include <sys/stream.h>
#include <sys/inet/in.h>
#define KERNEL	1	/* get kernel definitions */
#include <sys/inet/tcp.h>
#include <sys/inet/socket.h>
#include <sys/inet/udp.h>
#include <sys/inet/ip_var.h>
#include <sys/inet/tcp_timer.h>
#define TCPSTATES 1
#include <sys/inet/tcp_fsm.h>
#include <sys/inet/tcp_var.h>
#include <sys/inet/tcpdebug.h>

static int strip = 0; /* true if we're stripping the high bit from kernel addrs */

/* predeclared */
doseek();
void doseekoff();
void doread();

main(argc, argv)
char *argv[];
{
	char *xinu, *kmem;

	xinu = "/unix";
	kmem = "/dev/kmem";

	if(argc > 3)
		kmem = argv[3];
	if(argc > 2)
		xinu = argv[2];
	kern_init(xinu, kmem);

	if(argc < 2) {
		tcps(0);
		udps();
	}
	else if(strcmp(argv[1], "-i") == 0)
		interfaces();
	else if(strcmp(argv[1], "-s") == 0)
		statistics();
	else if(strcmp(argv[1], "-r") == 0)
		routes();
	else if(strcmp(argv[1], "-a") == 0)
		arps();
	else if(strcmp(argv[1], "-c") == 0) {
		tcps(0);
		udps();
	}
	else if(strcmp(argv[1], "-C") == 0)
		tcps(1);
	else if(strcmp(argv[1], "-t") == 0)
		debugtcp();
	else
		fprintf(stderr, "Usage: %s -icCrat [unix [vmcore]]\n", argv[0]);
}

#include "nlist.h"
struct nlist nl[] = {
	{"_ipif", 0},
#define NL_INET 0
	{"_ipstat", 0},
#define NL_IPSTAT 1
	{"_tcpstat", 0},
#define NL_TCPSTAT 2
	{"_Ntcp", 0},
#define NL_NTCP 3
	{"_tcpsocks", 0},
#define NL_TCP 4
	{"_ip_routes", 0},
#define NL_ROUTE 5
	{"_ip_arps", 0},
#define NL_ARP 6
	{"_Ninet", 0},
#define NL_NINET 7
	{"_Nip_route", 0},
#define NL_NROUTE 8
	{"_Nip_arp", 0},
#define NL_NARP 9
	{"_ip_default_route", 0},
#define NL_DR 10
	{"_bugarr", 0},
#define NL_TCPDEB 11
	{"_Nbugarr", 0},
#define NL_NBUGARR 12
	{"_Nudp", 0},
#define NL_NUDP 13
	{"_udpconn", 0},
#define NL_UDP 14
	{0, 0}
};
int kern_fd;

kern_init(xinu, kmem)
char *xinu, *kmem;
{
	int i;
	nlist(xinu, nl);
	if((long)nl[0].n_value == 0){
		fprintf(stderr, "nlist %s failed\n", xinu);
		exit(1);
	}
	if(strcmp(kmem, "/dev/kmem") != 0){
		for(i = 0; nl[i].n_name; i++){
			nl[i].n_value &= 0xffffff;
		}
		strip = 1;
	}
	kern_fd = open(kmem, 0);
	if(kern_fd < 0){
		perror(kmem);
		exit(1);
	}
}

interfaces()
{
	extern char *xflags();
	struct ipif ipif[128];
	int i, ninet;

	if (doseek(NL_NINET) < 0) {
		printf("Internet not compiled into this kernel\n");
		return;
	}
	doread((char *)&ninet, sizeof ninet);
	if (ninet > (sizeof(ipif)/sizeof(struct ipif)))
		ninet = sizeof(ipif)/sizeof(struct ipif);
	printf("%-4s %-12s %-12s  %5s %6s %6s %6s %6s\n",
		"Mtu", "Network", "Address", "Flags",
		"Ipkts", "Ierrs", "Opkts", "Oerrs");
	doseek(NL_INET);
	doread((char *)ipif, ninet*sizeof(struct ipif));
	for(i = 0; i < ninet; i++){
		if((ipif[i].flags&IFF_UP) == 0)
			continue;
		printf("%-4d %-12.12s ", ipif[i].mtu, in_host(ipif[i].that));
		printf("%-12.12s  %5s %6d %6d %6d %6d\n",
			in_host(ipif[i].thishost),
			xflags(ipif[i].flags, "UHA?", "   "),
			ipif[i].ipackets, ipif[i].ierrors,
			ipif[i].opackets, ipif[i].oerrors);
	}
}

statistics()
{
	struct ipstat stats;
	struct tcpstat tcpstat;

	if (doseek(NL_IPSTAT) < 0) {
		printf("Internet not compiled into this kernel\n");
		return;
	}
	doread((char *)&stats, sizeof(stats));
	printf("IP:\n");
	printf("%6d bad sums\n", stats.ips_badsum);
	printf("%6d short packets\n", stats.ips_tooshort);
	printf("%6d short data\n", stats.ips_toosmall);
	printf("%6d bad header lengths\n", stats.ips_badhlen);
	printf("%6d real bad header lengths\n", stats.ips_badlen);
	printf("%6d queue overflows\n", stats.ips_qfull);
	printf("%6d output routing errors\n", stats.ips_route);
	printf("%6d fragmented packets\n", stats.ips_fragout);
	if (doseek(NL_TCPSTAT) < 0) {
		printf("Tcp not compiled into this kernel\n");
		return;
	}
	doread((char *)&tcpstat, sizeof(tcpstat));
	printf("TCP:\n");
	printf("%6d bad sums\n", tcpstat.tcps_badsum);
	printf("%6d bad offsets\n", tcpstat.tcps_badoff);
	printf("%6d header drops\n", tcpstat.tcps_hdrops);
	printf("%6d bad segments\n", tcpstat.tcps_badsegs);
	printf("%6d retransmit timeouts\n", tcpstat.tcps_timeouts[0]);
	printf("%6d persist timeouts\n", tcpstat.tcps_timeouts[1]);
	printf("%6d keep-alive timeouts\n", tcpstat.tcps_timeouts[2]);
	printf("%6d quiet time timeouts\n", tcpstat.tcps_timeouts[3]);
	printf("%6d duplicate packets received\n", tcpstat.tcps_duplicates);
	printf("%6d possibly late packets received\n", tcpstat.tcps_delayed);
}

tcps(flag)
{
	int i, ntcp;
	struct socket so;
	struct tcpcb tcpcb;
	extern char *xflags();
	char b1[100], b2[100];
	struct in_service *sp;
#define SS_INTERESTING (SS_RCVATMARK|SS_OPEN|SS_ACTIVE|SS_WAITING|SS_PLEASEOPEN)

	if (doseek(NL_NTCP) < 0) {
		printf("Tcp not compiled into this kernel\n");
		return;
	}
	doread((char *)&ntcp, sizeof ntcp);
	printf("Proto Dev Wque Rque State %18s %18s %14s\n",
		"Remote Addr", "Local Addr", "Cstate");
	for(i = 0; i < ntcp; i++){
		doseekoff(NL_TCP, sizeof(so)*i);
		doread((char *)&so, sizeof(so));
		if((so.so_state&SS_INTERESTING) == 0)
			continue;
		if(so.so_tcpcb){
			if(strip)
				so.so_tcpcb =
					(struct tcpcb *)((int)(so.so_tcpcb)&0xffffff);
			lseek(kern_fd, so.so_tcpcb, 0);
			doread((char *)&tcpcb, sizeof(tcpcb));
		} else {
			tcpcb.t_state = 0;
		}
		sp = in_service(0, "tcp", so.so_fport);
		sprintf(b1, "%.11s.%.6s", in_host(so.so_faddr), sp->name);
		sp = in_service(0, "tcp", so.so_lport);
		sprintf(b2, "%.11s.%.6s", in_host(so.so_laddr), sp->name);
		if(!flag || tcpcb.t_state != TCPS_LISTEN)
		printf("  tcp  %02d %4d %4d %5s %18.18s %18.18s %14s\n",
			so.so_dev,
			so.so_wcount, so.so_rcount,
			xflags(so.so_state, "OPRWA?", "      "),
			b1, b2,
			tcpstates[tcpcb.t_state]);
#ifdef ALL
		if(!flag || tcpcb.t_state != TCPS_LISTEN)
			printf("template %x\n", tcpcb.t_template);
#endif
		if(flag && tcpcb.t_state != TCPS_LISTEN){
			printf("  Timers:\n");
			printf("\tretransmit %d\n\tpersist %d\n\tkeepalive %d\n\t2msl %d\n",
				tcpcb.t_timer[0], tcpcb.t_timer[1],
				tcpcb.t_timer[2], tcpcb.t_timer[3]);
			printf("  Send sequence variables:");
			printf("\n\tunacked %d\n\tnext %d\n\turgent ptr %d",
				tcpcb.snd_una, tcpcb.snd_nxt, tcpcb.snd_up);
			printf("\n\tinitial number %d\n\twindow %d\n\thighest sent %d\n",
				tcpcb.iss, tcpcb.snd_wnd, tcpcb.snd_max);
			printf("  Receive sequence variables:");
			printf("\n\tnext %d\n\turgent ptr %d",
				tcpcb.rcv_nxt, tcpcb.rcv_up);
			printf("\n\tinitial number %d\n\twindow %d\n\tadvertised %d\n",
				tcpcb.irs, tcpcb.rcv_wnd, tcpcb.rcv_adv);
			printf("  Transmit timing:");
			printf("\n\tinactive %d\n\tround trip %d\n\tseq # timed %d\n\tsmoothed round trip %f\n",
				tcpcb.t_idle, tcpcb.t_rtt, tcpcb.t_rtseq,
				tcpcb.t_srtt);
			printf("  Status:");
			printf("\n\tmax segment size %d", tcpcb.t_maxseg);
			printf("\n\tforcing out byte %d", tcpcb.t_force);
			printf("\n\tflags 0x%x\n", tcpcb.t_flags);
			printf("\n");
		}
	}
}

udps()
{
	int i, nudp;
	struct udp udp[132];
	extern char *xflags();
	char b1[100], b2[100];
	struct in_service *sp;

	if (doseek(NL_NUDP) < 0) {
		printf("Udp not compiled into this kernel\n");
		return;
	}
	doread((char *)&nudp, sizeof nudp);
	printf("\nProto Dev State %18s %18s\n",
		"Remote Addr", "Local Addr");
	doseek(NL_UDP);
	doread((char *)udp, nudp*sizeof(struct udp));
	for (i = 0; i < nudp; i++) {
		if(udp[i].rq == 0)
			continue;
		sp = in_service(0, "udp", udp[i].sport);
		sprintf(b1, "%.11s.%.6s", in_host(udp[i].src), sp->name);
		sp = in_service(0, "udp", udp[i].dport);
		sprintf(b2, "%.11s.%.6s", in_host(udp[i].dst), sp->name);
		printf("  udp  %02d %5s %18.18s %18.18s\n",
			i, xflags(udp[i].flags, "ILC?", "      "),
			b2, b1);
	}
}

char *
xflags(fl, fs, buf)
char *fs, *buf;
{
	int i, len;
	char *os;

	os = buf;
	len = strlen(fs);
	for(i = 0; i < len; i++){
		if(fl & (1<<i))
			*buf++ = fs[i];
	}
	*buf++ = '\0';
	return(os);
}

routes()
{
	struct ip_route r[256];
	int i;
	int nroute;

	doseek(NL_DR);
	doread((char *)r, sizeof(struct ip_route));
	if (r[0].gate != 0)
		printf("Default route is %-14.14s\n\n", in_host(r[0].gate));
	doseek(NL_NROUTE);
	doread((char *)&nroute, sizeof nroute);
	if (nroute > (sizeof(r)/sizeof(struct ip_route)))
		nroute = sizeof(r)/sizeof(struct ip_route);
	doseek(NL_ROUTE);
	doread((char *)r, nroute*sizeof(struct ip_route));
	printf("%-14s %-14s\n", "Destination", "Gateway");
	for(i = 0; i < nroute; i++){
		if(r[i].dst){
			printf("%-14.14s ", in_host(r[i].dst));
			printf("%-14.14s\n", in_host(r[i].gate));
		}
	}
}

arps()
{
	struct ip_arp a[256];
	int i, j, narp;

	if (doseek(NL_NARP) < 0) {
		printf("Arp not compiled into this kernel\n");
		return;
	}
	doread((char *)&narp, sizeof narp);
	if (narp > (sizeof(a)/sizeof(struct ip_arp)))
		narp = sizeof(a)/sizeof(struct ip_arp);
	doseek(NL_ARP);
	doread((char *)a, narp*sizeof(struct ip_arp));
	printf("%-10.10s Ether-Address\n", "Host");
	for(i = 0; i < narp; i++){
		if(a[i].inaddr){
			printf("%-10.10s ", in_host(a[i].inaddr));
			for(j = 0; j < 6; j++){
				printf("%02x ", a[i].enaddr[j]);
			}
			printf("\n");
		}
	}
}

debugtcp()
{
	struct tcpdebug loc_bugarr[SIZDEBUG];
	int last, inc, newlast, todo;
	time_t savtim, oldtim;
	char *io;

	printf("qInd In/Out     Seq No     Ack No sPort dPort Window Flags\n\n");
	last = -1;
	newlast = 0;
	oldtim = 0;

	/*
	Forever, do the following:  read the tcp debug array (bugarr), find
	the youngest entry, check to make sure this last entry is not in the
	same position in the debug queue as the last time the check was made
	(if it is the same position, check to see if the time stamps are
	different because exactly SIZDEBUG entries may have been made), and
	print out all the entries from the oldest to this youngest.  Note that
	before printing the entries from the header, they have to be converted
	from network order to host order.
	*/

	while (1) {
		if (doseek(NL_TCPDEB) < 0) {
			printf("Tcp debugging not compiled into this kernel\n");
			return;
		}
		doread((char *)loc_bugarr, sizeof(loc_bugarr));
		savtim = 0;
		inc = 0;
		while (inc < SIZDEBUG) {
			if (loc_bugarr[inc].stamp > savtim) {
				savtim = loc_bugarr[inc].stamp;
				newlast = inc;
			}
			inc++;
		}
		if (last == -1) last = newlast;	/* first time thru this process */
		if ((last != newlast || oldtim != savtim) && (savtim > 0)) {
			if ((todo = newlast - last) < 0)
				todo = SIZDEBUG - (last - newlast);
			else if (todo == 0)
				todo = SIZDEBUG;
			inc = (last + 1) % SIZDEBUG;
			while (todo) {
				if (loc_bugarr[inc].inout == 0)
					io = "in";
				else
					io = "out";
				printf("%4d   %3s  %10d %10d %5d %5d %5d   %x\n",
					inc, io,
					ntohl(loc_bugarr[inc].savhdr.th_seq),
					ntohl(loc_bugarr[inc].savhdr.th_ack),
					ntohs(loc_bugarr[inc].savhdr.th_sport),
					ntohs(loc_bugarr[inc].savhdr.th_dport),
					ntohs(loc_bugarr[inc].savhdr.th_win),
					ntohs(loc_bugarr[inc].savhdr.th_flags));
				inc = (inc + 1) % SIZDEBUG;
				todo--;
			} /* end while there are more packets to print out */
			printf("\n");
			fflush(stdout);
		} /* end if packets came in since the last check */
		last = newlast;
		oldtim = savtim;
		sleep (1);
	} /* end while forever */
} /* end procedure debugtcp */

doseek(nlitem)
	unsigned int nlitem;
{
	if (nl[nlitem].n_value == 0)
		return -1;
	if(lseek(kern_fd, (long)nl[nlitem].n_value, 0) == -1){
		perror("seek");
		exit(1);
	}
	return 0;
}

void
doseekoff(nlitem, offset)
	unsigned int nlitem;
	unsigned int offset;
{
	if(lseek(kern_fd, (long)(nl[nlitem].n_value+offset), 0) == -1){
		perror("seek");
		exit(1);
	}
}

void
doread(addr, size)
	char *addr;
	unsigned int size;
{
	if(read(kern_fd, addr, size) < 0){
		perror("read");
		exit(1);
	}
}

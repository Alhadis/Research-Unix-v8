#include <sys/inet/tcp_user.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include "config.h"

extern errno;
unsigned int tcp_localaddr = INADDR_ANY;
unsigned int tcp_acceptaddr;

tcp_sock()
{
	int fd, n;
	char name[32];

	for(n = 01; n < 100; n += 2){
		sprintf(name, "/dev/tcp%02d", n);
		fd = open(name, 2);
		if(fd >= 0)
			return(fd);
		if(errno != ENXIO)
			break;
	}
	perror(name);
	return(-1);
}

tcp_connect(fd, lport, faddr, fport)
int fd;
tcp_port lport, fport;
in_addr faddr;
{
	struct tcpreply tr;

	errno = 0;
	if(tcp_cmd(fd, TCPC_CONNECT, lport, faddr, fport) < 0)
		return(-1);
	if(tcp_wait(fd, &tr) < 0)
		return(-1);
	if(tr.reply != TCPR_OK)
		return(-1);
	return(0);
}

tcp_wait(fd, trp)
int fd;
struct tcpreply *trp;
{
	int n;
	/* timeout? */
	n = read(fd, trp, sizeof(struct tcpreply));
	if(n != sizeof(struct tcpreply))
		return(-1);
	return(0);
}

tcp_listen(fd, lport, faddr, fport)
int fd;
tcp_port lport, fport;
in_addr faddr;
{
	struct tcpreply tr;

	if(tcp_cmd(fd, TCPC_LISTEN, lport, faddr, fport) < 0)
		return(-1);
	return(0);
}

tcp_cmd(fd, cmd, lport, faddr, fport)
int fd;
int cmd;
tcp_port lport, fport;
in_addr faddr;
{
	struct tcpuser tu;

	tu.cmd = cmd;
	tu.src = tcp_localaddr;
	tu.dst = faddr;
	tu.sport = lport;
	tu.dport = fport;
	if(write(fd, &tu, sizeof(tu)) != sizeof(tu)){
		perror("cmd write");
		return(-1);
	}
	return(0);
}

tcp_accept(fd, faddrp, fportp, devp)
int fd;
in_addr *faddrp;
tcp_port *fportp;
int *devp;
{
	char name[32];
	int nfd;
	struct tcpreply tr;

	if(tcp_wait(fd, &tr) < 0)
		return(-1);
	if(tr.reply != TCPR_OK)
		return(-1);
	*devp = tr.tcpdev;
	sprintf(name, "/dev/tcp%02d", tr.tcpdev);
	nfd = open(name, 2);
	if(nfd < 0)
		return(-1);
	if (faddrp != 0)
		*faddrp = tr.dst;
	if (fportp != 0)
		*fportp = tr.dport;
	tcp_acceptaddr = tr.src;
	return(nfd);
}


tcp_rcmd(host, port, locuser, remuser, cmd, fd2p)
char *host, *port, *locuser, *remuser, *cmd;
int *fd2p;
{
	char buf[1];
	int fd;
	unsigned int faddr, fport;
	struct in_service *sp;

	faddr = in_address(host);
	if(faddr == 0){
		fprintf(stderr, "%s: unknown host\n", host);
		return(-1);
	}
	sp = in_service(port, "tcp", 0);
	if(sp == 0){
		fprintf(stderr, "%s: unknown service\n", port);
		return(-1);
	}
	fport = sp->port;
	fd = tcp_sock();
	if(fd < 0){
		perror(host);
		return(-1);
	}
	if(tcp_connect(fd, 0, faddr, fport) < 0){
		fprintf(stderr, "%s: connection failed\n", host);
		goto bad;
	}

	if(fd2p == 0)
		write(fd, "", 1);
	else
		goto bad;	/* later */

	write(fd, locuser, strlen(locuser)+1);
	write(fd, remuser, strlen(remuser)+1);
	write(fd, cmd, strlen(cmd)+1);

	if(read(fd, buf, 1) != 1){
		perror(host);
		goto bad;
	}
	if(buf[0] != 0){
		while(read(fd, buf, 1) == 1){
			write(2, buf, 1);
			if(buf[0] == '\n')
				break;
		}
		goto bad;
	}
	return(fd);
bad:
	close(fd);
	return(-1);
}


/*
 * things users need to know to talk to /dev/tcp*
 * open a free tcp device, write a tcpuser struct on it,
 * then wait for a tcpreply.
 */

/* the following is defined in kernel .h's */
#ifndef KERNEL
#include <sys/inet/in.h>
#include <sys/inet/tcp.h>
#endif

struct tcpuser{
	int cmd;
	tcp_port sport, dport;
	in_addr src, dst;
};
#define TCPC_LISTEN	1
#define TCPC_CONNECT	2

struct tcpreply{
	int reply;
	/* for listen: */
	tcp_port sport, dport;
	in_addr src, dst;
	int tcpdev; 	/* minor device # */
};
#define TCPR_OK		1
#define TCPR_SORRY	2

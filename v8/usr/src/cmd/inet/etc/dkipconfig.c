#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sgtty.h>
#include <sys/inet/in.h>
#include "config.h"

#define RESETTIME 15*60

static char *cmdname;
static in_addr myaddr;

catch_hup()
{
	signal(SIGHUP, catch_hup);
}

main(argc, argv)
char *argv[];
{
	int other_end, my_pid;

	cmdname = argv[0];
	switch(argc){
	case 3:
		ipconfig(0, argv[1], argv[2]);
		pause();
		break;
	case 4:
		my_pid = getpid();
		setpgrp(my_pid, my_pid);
		signal(SIGHUP, catch_hup);
		while(1) {
			other_end = callup(argv[1], argv[3], argv[2]);
			if (other_end < 0) {
				fprintf(stderr, "%s: call failed\n", cmdname);
				sleep(60);
				continue;
			}
			if (ioctl(other_end, TIOCSPGRP, 0) < 0) {
				fprintf(stderr, "%s: couldn't set pgrp\n", cmdname);
				close(other_end);
				sleep(60);
				continue;
			}
			ipconfig(other_end, argv[2], argv[3]);
			pause();
			close(other_end);
		}
		break;
	default:
		fprintf(stderr, "Usage: %s dk-address my-ip-addr his-ip-addr\n",
			cmdname);
		exit(1);
	}
}

static int
ipconfig(ipfd, me, it)
	int ipfd;
	char *me, *it;
{
	in_addr hisaddr, inaddr;
	int x;

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
	x = 10;		/* IP line disc # */
	if(ioctl(ipfd, FIOPUSHLD, &x) < 0){
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
}

static int
callup(host, a1, a2)
	char *host, *a1, *a2;
{
	char cmd[512];

	strcpy(cmd, DKIPCONFIG);
	strcat(cmd, " ");
	strcat(cmd, a1);
	strcat(cmd, " ");
	strcat(cmd, a2);
	return(tdkexec(host, cmd));
}

static int
still_there(ipfd)
	int ipfd;
{
	return ioctl(ipfd, IPIOLOCAL, &myaddr) >= 0;
}

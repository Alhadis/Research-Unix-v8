#include <stdio.h>
#include <errno.h>
#include <dk.h>
#include <sgtty.h>
#include <sys/stream.h>
#include <signal.h>

/*
 * program to run a command
 * on another cpu on Datakit w/ tranarent ioctls
 */

int     rem;            /* remote file descriptor */
extern	int mesg_ld;
extern	errno;
extern	char *dkerror;

struct mesg{
	short type;
	short size;
};

char    *std    = "-";

#define MAXCHARS 8192
char args[MAXCHARS];

char *bldargs(argv)
	register char *argv[];
{
	register char *s=args, *t;
	while(t= *argv++){	/* assignment = */
		while(*s = *t++)
			if(s++ >= &args[MAXCHARS-1]){
				fprintf(stderr, "rx: arg list too long\n");
				exit(1);
			}
		*s++=' ';
	}
	s[-1]='\0';
	return(args);
}

char *
basename(s)
	register char *s;
{
	register char *t;
	extern char *rindex();
	t=rindex(s, '/');
	return(t? t+1 : s);
}

hupcatch()
{

	if(ioctl(0, FIOLOOKLD, 0) == mesg_ld)
		ioctl(0, FIOPOPLD, 0);
	exit(0);
}

main(argc, argv)
char **argv;
{
	char *host, *cmd;

        host = cmd = basename(argv[0]);
	if(strcmp(host, "rx")==0 || strcmp(host, "rexec")==0
	   || strcmp(host, "nrx")==0){
		host=argv[1];
		argv++;
	}
	if(host==0){
		fprintf(stderr, "usage: %s host [command]\n", cmd);
		exit(1);
	}
	if(argv[1]==0){
		fprintf(stderr, "%s: cannot dcon\n", cmd);
		exit(1);
	}
        rem = mesgexec(host, bldargs(&argv[1]));
        if (rem<0) {
		fprintf(stderr, "%s: call to %s failed: %s\n", cmd , host, dkerror);
                exit(1);
	}
	if(ioctl(0, FIOPUSHLD, &mesg_ld) < 0){
		fprintf(stderr, "%s: can't push mesg_ld\n", cmd);
		exit(1);
	}
	signal(SIGHUP, hupcatch);
	signal(SIGPIPE, SIG_IGN);

	go(rem);

	hupcatch();
	/* NOTREACHED */
}

go(fd)
{
	int rbits, wbits, n;
	char buf[4096+sizeof(struct mesg)];
	struct mesg *mp;

	mp = (struct mesg *) buf;
	wbits = 0;
	while(1){
		rbits = 1 | (1<<fd);
		if(select(20, &rbits, &wbits, 20000) < 0){
			if(errno != EINTR)
				return;
			continue;
		}
		if(rbits & 1){
			n = read(0, buf, sizeof(buf));
			if(n <= 0)
				return;
			if(write(fd, buf, n) != n)
				return;
			if(mp->type == M_FLUSH)
				remflush();
		}
		if(rbits & (1<<fd)){
			n = read(fd, buf, sizeof(buf));
			if(n <= 0)
				return;
			if(mp->type == M_HANGUP)
				return;
			if(mp->type == M_IOCTL){
				doioctl(buf, n);
			} else {
				if(write(1, buf, n) != n)
					return;
			}
		}
	}
}

doioctl(buf, n)
char *buf;
{
	struct mesg *mp;
	struct iofoo{
		int cmd;
		union{
			int i;
			char errno;
			struct insld insld;
		} u;
	} *iop;
	int cmd, ld;

	iop = (struct iofoo *)(buf + sizeof(struct mesg));
	mp = (struct mesg *) buf;

	cmd = iop->cmd;
	n -= sizeof(struct mesg);
	n -= sizeof(iop->cmd);
	switch(cmd){
	case FIOLOOKLD:
		if(n > 0)
			ld = iop->u.i;
		else
			ld = 0;
		ld++;
		if(ioctl(1, FIOLOOKLD, &ld) < 0)
			goto bad;
		iop->cmd = ld;
		n = sizeof(iop->cmd);
		break;

	case FIOPOPLD:
		if(n > 0)
			ld = iop->u.i;
		else
			ld = 0;
		ld++;
		if(ioctl(1, FIOPOPLD, &ld) < 0)
			goto bad;
		n = 0;
		break;

	case FIOPUSHLD:
		iop->u.insld.level = 0;
		/* fall through... */
	case FIOINSLD:
		iop->u.insld.level++;
		if(ioctl(1, FIOINSLD, &(iop->u.insld)) < 0)
			goto bad;
		n = 0;
		break;

	default:
		write(1, buf, sizeof(struct mesg) + mp->size);
		return;
	}
	/* locally successful */
	mp->type = M_IOCACK;
	mp->size = n;
	write(rem, buf, sizeof(struct mesg) + mp->size);
	return;
bad:
	mp->type = M_IOCNAK;
	mp->size = sizeof(struct iofoo);
	iop->u.errno = errno;
	write(rem, buf, sizeof(struct mesg) + mp->size);
}

remflush()
{
	char buf[5000];
	struct mesg *mp;
	int n;

	mp = (struct mesg *) buf;
	mp->type = M_IOCTL;
	mp->size = sizeof(int);
	write(rem, buf, mp->size + sizeof(struct mesg));

	while((n = read(rem, buf, sizeof(buf))) > 0){
		if(mp->type == M_IOCNAK || mp->type == M_IOCACK){
			return;
		}
	}
}

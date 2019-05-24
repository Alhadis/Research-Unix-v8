#include <stdio.h>
#include <dk.h>
#include <sgtty.h>
#include <sys/stream.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>

/*
 * program to connect to
 * another cpu on Datakit w/ transparent ioctls
 */

int     rem;            /* remote file descriptor */
extern	int mesg_ld, dkp_ld;
extern	errno;
extern	char *dkerror;
struct	sgttyb sgbuf;
char	*ttyn;
int	perms;

struct mesg{
	short type;
	short size;
};

struct sigmsg {
	struct	mesg m;
	char	sig[1];
};

main(argc, argv)
char **argv;
{
	char *host;
	char dest[128];
	struct stat sb;
	extern int hupcatch();
	char *ttyname();

       	while(argc > 1 && argv[1][0] == '-'){
		argv++;
		argc--;
	}
	host = argv[1];

	if(host==0){
		printf("usage: %s host\n", argv[0]);
		exit(1);
	}

	ioctl(0, TIOCGETP, &sgbuf);
	sprintf(dest, "%s.mesgdcon", host);
	rem = tdkdial(dest, 0);
        if (rem<0) {
		printf("%s: call to %s failed: %s\n", argv[0], dest, dkerror);
                exit(1);
	}
	if(dkproto(rem, dkp_ld) < 0){
		printf("%s: can't push dk line discipline\n", argv[0]);
		exit(1);
	}
	if(tdklogin(rem) < 0){
		printf("%s: can't log in\n", argv[0]);
		exit(1);
	}
	signal(SIGHUP, hupcatch);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	if ((ttyn = ttyname(0)) != NULL
	&&   stat(ttyn, &sb) >= 0) {
		perms = sb.st_mode & ~S_IFMT;
		chmod(ttyn, 0);
	}
	ioctl(0, TIOCFLUSH, (char *)0);	/* race with readahead still possible */
	if(ioctl(0, FIOPUSHLD, &mesg_ld) < 0){
		printf("%s: can't push mesg_ld\n", argv[0]);
		finish(1);
	}

	go(rem);

	finish(0);
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
			if (mp->type==M_SIGNAL
			 && ((struct sigmsg *)mp)->sig[0]==SIGQUIT) {
				dolocal(fd, buf, n);
				continue;
			}
			if(write(fd, buf, n) != n)
				return;
			if(mp->type == M_FLUSH) {
				remflush();
				rbits = 0;
			}
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

hupcatch()
{
	finish(0);
}

finish(sts)
{
	struct mesg m;

	if(ioctl(0, FIOLOOKLD, 0) == mesg_ld)
		ioctl(0, FIOPOPLD, 0);
	if (ttyn)
		chmod(ttyn, perms);
	if (sts == 0)
		printf("Eof\n");
	exit(sts);
}

dolocal(fp, buf, n)
char *buf;
{
	char lbuf[128+1];
	register char *lp;
	struct sgttyb nsgbuf;
	register struct mesg *mp = (struct mesg *)buf;

	ioctl(0, FIOPOPLD, (char *)0);
	ioctl(0, TIOCFLUSH, (char *)0);
	ioctl(0, TIOCGETP, &nsgbuf);
	ioctl(0, TIOCSETP, &sgbuf);
	chmod(ttyn, perms);
	for (;;) {
		lp = lbuf;
		printf( "dcon>> ");
		fflush(stdout);
		while (lp < &lbuf[128] && read(0, lp, 1)>0 && *lp!='\n')
			lp++;
		*lp = '\0';
		if (*lbuf=='i')
			break;
		else if (*lbuf=='q'||*lbuf=='x'||*lbuf=='.')
			exit(0);
		else if (*lbuf=='!') {
			system(lbuf+1);
			printf("!!\n");
			fflush(stdout);
			mp->type = 0;
			break;
		} else if (*lbuf=='\0') {
			mp->type = 0;
			break;
		} else {
			printf("[qx.] to exit, i for quit signal, !cmd for shell\n");
			fflush(stdout);
		}
	}
	ioctl(0, TIOCSETP, &nsgbuf);
	ioctl(0, FIOPUSHLD, &mesg_ld);
	chmod(ttyn, 0);
	if (mp->type)
		write(fp, buf, n);
}

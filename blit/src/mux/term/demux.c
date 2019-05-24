#include <jerq.h>
#include <layer.h>
#include <jioctl.h>
#include "queue.h"
#include "../msgs.h"
#include "jerqproc.h"
#include "pconfig.h"
#include "proto.h"
#include "packets.h"

extern struct Proc *kbdproc;
extern char *itox();
extern scrltimeout;
int rebootflag;

demux(){
	while(!rebootflag){
		while(RCVQUEUE.c_cc==0)
			sw(0);
		precv((char)qgetc(&RCVQUEUE));
	}
	nap(60);
	reboot();
}

int
recvchars(l, p, n)
	int l; char * p; int n;
{
	register struct Proc *pp;
	register char *s;
	register unsigned char *cp;
	register int i;

	if(l==0){	/* that's me!! */
		doctl(p, n);
		Pcdata=C_UNBLK;	/* only needed for UTS */
		return 0;
	}
	pp= &proctab[l];
	if((pp->state&BUSY) && pp->traptype<2)	/* proc may be dead */
		setrun(pp);
	else
		return 0;	/* why bother? */
	if((i=n)>0){
		if(i>(sizeof(pp->cbuf)-pp->nchars))
			return 1;	/* oops! */
		cp=pp->cbufpin;
		s=p;
		do{
			pp->nchars++;
			*cp++ = *s++;
			if(cp>= &pp->cbuf[sizeof(pp->cbuf)])
				cp=pp->cbuf;
		}while(--i>0);
		pp->cbufpin=cp;
	}
	if(pp->nchars<=CBSIZE && !(pp->state&BLOCKED))
		Pcdata = C_UNBLK;
	else if(++pconvs[l].user > NPCBUFS)	/* Inc. # of packets blocked */
		pconvs[l].user = NPCBUFS;
	return 0;
}

doctl(s, n)
	register unsigned char *s;
{
	unsigned char cmd=s[0];
	register struct Proc *p= &proctab[s[1]];
	extern boot(), windowstart();
	extern int end;

	switch(cmd){
	case JEXIT&0xFF:
		Psend(0, (char *)0, 0, C_EXIT);
		break;
	case JDELETE&0xFF:
		delete(p->layer);
		break;
	case JTTYC:
		if(n!=sizeof (struct ttycmesg)){
#			ifdef	DEBUG
			error("JTTYC n!=sizeof ttycmesg", itox((unsigned long)n));
#			endif
			return;
		}
		copyb(&s[2], &p->ttychars, sizeof (struct ttychars));
		break;
	case JTIMO&0xFF:
		if(n!=3){
#			ifdef	DEBUG
			error("JTIMO n!=3", itox((unsigned long)n));
#			endif
			return;
		}
		Prtimeout=s[1];
		Pxtimeout=s[2];
		scrltimeout=10*s[2];
		break;
	case JBOOT&0xFF:
	case JTERM&0xFF:
	case JZOMBOOT&0xFF:
#		ifdef	DEBUG
		if(n!=2){
			error("doctl n!=2", itox((unsigned long)n));
			return;
		}
#		endif
		if(s[1]==0){	/* i.e. demux */
			rebootflag++;
			break;
		}
		freemem(p);
		shutdown(p);
		p->nchars=0;
		p->cbufpout = p->cbufpin;
		restart(p, cmd==(JTERM&0xFF)? windowstart : boot);
		if(cmd==(JZOMBOOT&0xFF))
			p->state|=ZOMBOOT;
		setrun(p);
		break;
	default:
		error("unk ctl", itox((unsigned long)cmd));
	}
}
copyb(a, b, n)
	register char *a, *b;
	register n;
{
	while(n--)
		*b++=*a++;
}

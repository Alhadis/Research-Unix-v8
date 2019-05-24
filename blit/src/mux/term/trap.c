#include <jerq.h>
#include <layer.h>
#include "jerqproc.h"
#include <font.h>

short	*patchedspot;
short	patch;
char *	itox();
extern long	traploc;
extern short traptype;
extern Layer *whichlayer();
struct Proc	*debugger;

/* Called at spl7() */
trap(s)
	char *s;
{
	P->traploc = traploc;
	P->traptype = traptype;
	if(patchedspot){
		*patchedspot=patch;
		patchedspot=0;
	}
	spl0();
	P->state|=ZOMBIE;
	if(debugger && P->traptype<2){	/* BPT or trace */
		sw(0);
		/* ZOMBIE bit is now off; we are free to continue */
		return;
	}
	P->nchars=0;
	P->cbufpin=P->cbufpout=P->cbuf;
	copy(s);
	copy(" at ");
	copy(itox(*(unsigned long *)traploc-(unsigned long)P->fcn));

	if(P-proctab<=1){	/* DEMUX, CONTROL */
		*P->cbufpin = '\0';
		error(P-proctab==0?"demux":"control", P->cbuf);
	}
	copy(" -- core not dumped\r\n");
	/* The mpxkill(1, P) has been removed */
	/*
	 * This is a bit subtle.  The ZOMBIE bit implies only that sw()
	 * will never schedule, but we are about to call windowproc
	 * explicitly.  That will cause it to start up, print the message
	 * and sit indefinitely, because its ZOMBIE bit is on.
	 */
	shutdown(P);
	windowproc();
}

copy(s)
	register char *s;
{
	register unsigned char *p = P->cbufpin;

	while(*s)
	{
		*p++= *s++;
		P->nchars++;
		if(p>= &P->cbuf[sizeof(P->cbuf)])
			p=P->cbuf;
	}
	P->cbufpin=p;
}

char *
itox(n)
	register unsigned long n;
{
	static char hex[10];
	register char *hp;

	hp = &hex[sizeof hex];

	*--hp='\0';
	if(n>0)
		do{
			if((*--hp=(n&0xf)+'0') > '9')
				*hp+='A'-'9'-1;
			n>>=4;
		}while(n);
	else
		*--hp='0';

	return hp;
}

error(s1, s2)
	char *s1, *s2;
{
	jrectf(Rect(0, 0, 400, 100), F_OR);
	jmoveto(Pt(10, 50));
	jstring(s1);
	jstring(": ");
	jstring(s2);
	P->state|=ZOMBIE;
	sw(0);
}
/*
**sstep(s)
**	char *	s;
**{
**	jrectf(Rect(XMAX-300, YMAX-50, XMAX, YMAX), F_CLR);
**	string(&defont, s, &display, Pt(XMAX-290, YMAX-30), F_XOR);
**	do; while(button12()==0);
**	if(button1())
**		(*(long *)0)=0;
**	do; while(button12());
**	*DADDR = 156*(1024/4);
**	jrectf(Rect(XMAX-300, YMAX-50, XMAX, YMAX), F_XOR);
**}
**help(s, n)
**	char *s;
**	unsigned long n;
**{
**	static char buf[64];
**	strcpy(buf, s);
**	strcat(buf, itox(n));
**	sstep(buf);
**}
*/

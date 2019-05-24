#include <jerq.h>
#include <layer.h>
#include <jerqproc.h>

#define	ypos(a)	(P->rect.corner.y-muldiv(a, dy, nbytes))

long getlong();

static Texture cup = {
	0x0100, 0x00E0, 0x0010, 0x03E0, 0x0400, 0x0FE0, 0x123C, 0x1FE2,
	0x101A, 0x101A, 0x1002, 0x103C, 0x1810, 0x6FEC, 0x4004, 0x3FF8,
};

boot(){
	register argc;
	register char **argv;
	register char *address;
	char *Ualloc();
	register unsigned nbytes, i, nargchars;
	register dy=P->rect.corner.y-P->rect.origin.y;
	register oypos=P->rect.corner.y;

	Urequest(MOUSE);
	Ucursswitch(&cup);
	argc=getlong();
	nargchars=getlong();
	argv=(char **)Ualloc(nargchars+(argc+1)*sizeof(char *));
	if(argv==0)
		address==0;
	else
		/* nbytes=text+data, the amount to be downloaded */
		address=Ualloc((unsigned)((nbytes=
			(unsigned)(getlong()+getlong()))+getlong()));
	P->fcn=(int(*)())address;
	mpxublk(P);
	sendnchars(4, (char *)&P->fcn);
	bldargs(argc, argv);
	for(i=0; i<nbytes; i++){
		*address++=getchar();
		if((i&127)==0){
			lrectf(P->layer, Rect(P->rect.origin.x, ypos(i),
				P->rect.corner.x, oypos), F_XOR);
			oypos=ypos(i);
		}
	}
	mpxublk(P);
	clear(P->rect, 1);
	Urequest(0);
	P->state|=USER;
	setdata(P);
#define	udp	((struct udata *)P->fcn)
	udp->argc=argc;
	udp->argv=argv;	/* these get set as arguments to main in notsolow.o */
	if(P->state&ZOMBOOT){
		P->state&=~ZOMBOOT;
		P->state|=ZOMBIE;
		zombexec(P->fcn);
	}
	exec(P->fcn);
}
bldargs(argc, argv)
	register char **argv;
{
	register i;
	register char *p=(char *)(argv+argc+1);
	for(i=0; i<argc; i++){
		*argv++=p;
		do
			*p=getchar();
		while(*p++);
	}
	*argv++=0;
}
setdata(p)
	register struct Proc *p;
{
	register struct udata *u=((struct udata *)p->fcn);
	u->Drect=p->rect;
	u->Jdisplayp=p->layer;
}
getchar(){
	register c;
	register struct Proc *p=P;

	while(p->nchars==0){
		mpxublk(p);
		sw(0);
	}
	c = *(p->cbufpout)++;
	if(p->cbufpout >= &p->cbuf[sizeof(p->cbuf)])
		p->cbufpout = p->cbuf;
	if((--p->nchars)==0)
		mpxublk(p);	/* shouldn't be necessary, but... */
	return c;
}

long
getlong(){
	long l;
	register char *p=(char *)&l;
	register i;
	for(i=0; i<4; i++)
		*p++=getchar();
	return(l);
}

asm("global exec			");
asm("	exec:				");
asm("		mov.l	4(%sp), %a1	");
asm("		mov.l	0406, %a0	");
asm("		lea.l	2298(%a0), %sp	");	/* &P->stack[STKSIZ]-4 */
asm("		mov.l	%a1, 4(%a0)	");	/* P->fcn */
asm("		jmp	(%a1)		");

asm("global zombexec			");
asm("	zombexec:				");
asm("		mov.l	4(%sp), %a2	");
asm("		mov.l	0406, %a0	");
asm("		lea.l	2298(%a0), %sp	");	/* &P->stack[STKSIZ]-4 */
asm("		mov.l	%a2, 4(%a0)	");	/* P->fcn */
asm("		mov.l	&0, (%sp)	");
asm("		jsr	sw		");	/* switch out */
asm("		jmp	(%a2)		");

shutdown(p)
	register struct Proc *p;
{
	extern struct Proc *debugger;
	extern struct Proc *kbdproc;
	Lbox(p->layer);
	if(kbdproc!=p)
		shade(p->layer);
	if(debugger==p)
		debugger=0;
	mpxublk(p);
	p->state&=~(KBDLOCAL|MOUSELOCAL|GOTMOUSE|USER);
	p->nticks=0;
	qclear(&p->kbdqueue);
	if((p->state&ZOMBIE)==0)
		freemem(p);
}
freemem(p)
	register struct Proc *p;
{
	extern int end;
	if(p->fcn > (int (*)())&end)
		freeall((char *)p);
}

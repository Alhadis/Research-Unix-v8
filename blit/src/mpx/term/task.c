#include <jerq.h>
#include <layer.h>
#include <jerqproc.h>

struct Proc *
newproc(f)
	int (*f)();
{
	register struct Proc *u;
	register i;

	for(i=0,u=proctab; i<NPROC; i++,u++) {
		if((u->state&BUSY)==0){
			u->state |= BUSY;
			restart(u, f);
			u->layer=0;
			u->nchars = 0;
			u->cbufpin = u->cbuf;
			u->cbufpout = u->cbuf;
			u->traptype = 0;
			u->traploc = 0;
			return(u);
		}
	}
	return(0);
}

restart(p, loc)
	register struct Proc *p;
	register (*loc)();
{
	register long *q;
	register i;
	p->fcn=loc;
	q=(long *)&p->stack[STKSIZ];
	*--q=(long)loc;	/*pc*/
	*--q=0;		/*old fp*/
	p->sp=q;
	for(i=0; i<10; i++)
		*--q=0;	/*10 registers*/
}

sw(r){
	/*
	 * This is static so scheduling is fair
	 */
	static struct Proc *p=proctab;
	extern Transition;
	p=P;
	if(r==0)	/* I don't want to run */
		setnorun(p);
    loop:
	do{
		p++;	/* 'cos we are the current p */
		if(p>=&proctab[NPROC])
			p=proctab;
		if((p->state&(RUN|ZOMBIE))==RUN && p!=P){
			resume(p);
			return;	/* we have been enabled */
		}
	}while(p!=P);
	if(p->state&RUN)
		return;	/* forge ahead */
	goto loop;	/* wait for somebody to get ready to run */
}

#include <jerq.h>
#include <layer.h>
#include "queue.h"
#include "jerqproc.h"
#include <acia.h>

/*
 * clockroutine() called at video interrupt time.
 *	reads chars off the keyboard, sends things to host
 */

short		second, ticks;
long		ticks0;
extern boot();
short	*patchedspot;
short	patch;
#define	CONTROL	1
#define	qpeekc(Q)	((Q)->c_head?((Q)->c_head->word):-1)

/*ARGSUSED*/
clockroutine(d0, d1, a0, a1, sr, upc)
	long d0, d1, a0, a1;
	short sr;
	short *upc;
{
	register c;
	extern struct Proc *kbdproc;
	register struct Proc *p;

	ticks0++;
	if(--ticks<=0){
		ticks=60;	/* really HZ */
		second=1;
	}

	for(p= &proctab[CONTROL+1]; p<&proctab[NPROC]; p++)
		if(p->nticks>0 && --p->nticks==0)
			p->state|=WAKEUP|RUN;
	setrun(&proctab[CONTROL]);
	if((p=kbdproc)==0)
		return;
	while(KBDQUEUE.c_cc>0){
		c=qgetc(&KBDQUEUE);
		if(c==0xEC){	/* C-S-BREAK; show what's up */
			rectf(&display, P->rect, F_XOR);
			do; while(button123()==0);
			if(button2()){
				patchedspot=upc;
				patch= *upc;
				*upc= -1;	/* Cause EMT inst. trap */
			}
			rectf(&display, P->rect, F_XOR);
			do; while(button123());
		}else
			qputc(&p->kbdqueue, c);
		if(p->state&KBDLOCAL)
			p->state|=WAKEUP;
	}
	if(!(p->state&KBDLOCAL))
	while((c=qpeekc(&p->kbdqueue))!=-1){
		if(p->fcn!=boot){	/* KLUDGE, but it helps */
			if(c==0xE0)	/* BREAK */
				c=0x7F;
			if(mpxsendchar(c, p)==-1)
				break;
		}
		(void)qgetc(&p->kbdqueue);
	}
	givemouse(p);
}
givemouse(p)
	register struct Proc *p;
{
	if((p->state&(MOUSELOCAL|USER)) == (MOUSELOCAL|USER)){
		register struct Mouse *m= &((struct udata *)p->fcn)->mouse;
		*m=mouse;
#define	o	p->rect.origin
#define	c	p->rect.corner
		m->jxy.x=muldiv(mouse.xy.x-o.x, XMAX, c.x-o.x);
		m->jxy.y=muldiv(mouse.xy.y-o.y, YMAX, c.y-o.y);
	}
}
sleep(s){
	register struct Proc *p=P;
	register alarmed=p->state&ALARMREQD;
	register long nticks;
	extern long ticks0;
	nticks=ticks0+p->nticks;
	alarm(s);
	Uwait(ALARM);
	/* a little dance because sleep calls alarm */
	if(alarmed){
		spl1();
		if(nticks>ticks0)
			p->nticks=nticks-ticks0;
		else	/* we missed his wakeup! */
			p->state|=WAKEUP;
		spl0();
		p->state|=ALARMREQD;
	}else
		p->state&=~ALARMREQD;
}
alarm(s){
	P->state|=ALARMREQD;
	if(s>0)
		P->nticks=s;
}
long
realtime(){
	return ticks0;
}

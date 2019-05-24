#include "r.h"
Textframe *workframe;
Point center();
Textframe *obslist[NFRAME+1];	/* last guy propagates a 0 on deletion */
curse(t)
	register Textframe *t;
{
	if(t && t->str->s && t->s1==t->s2){
		Point p;
		p=ptofchar(t, t->s1);
		Rectf(Rpt(p, Pt(p.x+1, p.y+newlnsz)), F_XOR);
	}
}
select(t, pt)	/* don't use "but" */
	register Textframe *t;
	Point pt;
{
	register fix, var, oldvar;
	if(t->s1 != t->s2)
		selectf(t, F_XOR);
	fix=charofpt(t, pt);	/* fixed point */
	oldvar=fix;			/* moving point tracks mouse */
	var=fix;
	while (button1()) {
		if(var != oldvar){
			t->s1=oldvar; t->s2=var;
			order(t);
			selectf(t, F_XOR);
			oldvar=var;
		}
		var=charofpt(t, mouse.xy);
	}
	t->s1=fix; t->s2=oldvar;
	order(t);
	t->selecthuge=0;
	Send(O_SELECT, t->s1, 2, data2(t->s2-t->s1));
}
type(t)
	register Textframe *t;
{
	register short i;
	register char c;
	register startn;
	static char magic[]="\260\377\033\b\027";
	if(P->kbdqueue.c_head->word==0260){	/* Wretched noscroll hack */
		kbdchar();
		curse(t);
		goto NOSCRL;
	}
	for(startn=t->s1; !button123(); wait(MOUSE)) {
		if(own()&KBD){
			curse(t);
			zerostring(TYPEIN);
			if(t->s2 > t->s1){
				if(t->selecthuge){
					mesg("sorry; first deselect that huge thing\n", 1);
					do; while(kbdchar()==-1);
					return;
				}
				Send(O_CUT, 0, 0, "");
				cut(t, TRUE, t==DIAG? diagclr : F_CLR);
			}
			if(t==DIAG && diagnewline){
				mesg("", FALSE);	/* clear the line */
				startn=0;
			}
			for (i=0; notin((c=kbdchar()), magic); i++){
				if((c&0x80) || c==0177 || (c<' ' && c!='\t' && c!='\r')){
					--i;	/* cause it didn't go */
					continue;
				}
				c&=0x7F;
				if(c=='\r'){
					if(t==DIAG){
						if(i>0){
							Send(O_INSERT, t->s1,
							   i, TYPEIN->s);
							instext(t, TYPEIN, t->s1);
						}
						senddiag();
						goto Return;
					}
					c='\n';
				}
				addstring(TYPEIN,c);
			}
			if (i > 0) {
				register s1=t->s1;
				register nsc=0;
				sendstr(t, O_INSERT, s1, i, TYPEIN->s);
				instext(t, TYPEIN, s1);
				if(!inscomplete){
					nsc=scroll(t, newlines(TYPEIN)+1);
					i-=nsc;
					startn=max(0, startn-nsc);
				}
				setsel(t, s1+i);
			}
			if (c == 033) {
				if(t->s2 >= startn){
					t->s1=startn;
					selectf(t, F_XOR);
					Send(O_SELECT, t->s1, 2, data2(t->s2-t->s1));
					t->selecthuge=0;
				}
		Return:
				curse(t);
				return;
			} else if(c=='\b' || c==027){
				if(t->s1>0){
					t->s1 -= (i=nback(t, c));
					Send(O_BACKSPACE, t->s1, 2, data2(i));
					deltext(t, t==DIAG? diagclr: F_CLR);
				}
			} else if((c&0xFF) == 0260) {
		NOSCRL:
				if(t==DIAG){
					if(workframe){
						buttonhit(center(workframe), 4);
						curse(workframe);
					}
				}else{
					buttonhit(center(DIAG), 4);
					curse(DIAG);
				}
				return;
			}
			curse(t);
		}
	}
}
Point
center(t)
	Textframe *t;
{
	return div(add(t->rect.origin, t->rect.corner), 2);
}
nback(t, c)
	register Textframe *t;
{
	register n=0, s1=t->s1;
	register char *s=t->str->s+s1;
	static char alphanl[]=
		"\n0123456789_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
#define	alphanum	alphanl+1
	if(s1 <= 0)
		return 0;
	if(c=='\b' || *--s=='\n')
		return 1;
	/* else it's ^W. first, get to an alphanumeric (or newline) */
	while(n<s1 && notin(*s, alphanl))
		--s, ++n;
	/* *s is alphanumeric or space; now back up to non-alphanumeric */
	while(n<s1 && !notin(*s, alphanum))
		--s, ++n;
	return n;
}
notin(c, s)
	register c;
	register unsigned char *s;
{
	c&=0xFF;
	while(*s)
		if(c == *s++)
			return FALSE;
	return TRUE;
}
newlines(s)
	register String *s;
{
	register char *p=s->s;
	register n=s->n;
	register nl=0;
	while(n-->0)
		if(*p++ == '\n')
			nl++;
	return nl;
}
senddiag()
{
	diagnewline=TRUE;
	/*
	 * Send a 0-length guy to signify the end.
	 */
	send(workframe?workframe->file : 0, O_DIAGNOSTIC, 0, 0, (char *)0);
	waitunix(&diagdone);
}
setsel(t, n)
	register Textframe *t;
	register n;
{
	t->s1=t->s2=n;
	t->selecthuge=0;
}
order(t)
	register Textframe *t;
{
	register a;
	if(t->s1 > t->s2){
		a=t->s1;
		t->s1=t->s2;
		t->s2=a;
	}
}
/*
 * Change to work in named frame, whether typing at DIAG or not.
 */
workinframe(t)
	register Textframe *t;
{
	if(workframe != t){
		if(workframe){
			rXOR(workframe->scrollrect);
			setchar(workframe->file, STARDOT, '*');
		}
		drawframe(t);
		workframe=t;
		setchar(t->file, STARDOT, '.');
	}
}
box(t)
	register Textframe *t;
{
	Rectf(t->totalrect, F_OR);
	Rectf(inset(t->totalrect, M), F_CLR);
}
drawframe(t)
	register Textframe *t;
{
	if(t == workframe)
		return;
	if(workframe)
		setchar(workframe->file, STARDOT, '*');
	if(t->obscured)
		dodraw(t);
	else	/* scroll bar and cursor both visible */
		rXOR(t->scrollrect);
	setchar(t->file, STARDOT, '.');
}
dodraw(t)
	register Textframe *t;
{
	box(t);
	ontop(t);
	if(t->str->s==0)	/* probably got closed */
		return;
	frameop(t, opdraw, t->rect.origin, t->str->s, t->str->n);
	if(complete)
		loadfile(t, t->str->n, 32767);
	selectf(t, F_XOR);
	tellseek(t, t->scrolly);
}
Textframe *
pttoframe(pt)
	Point pt;
{
	register Textframe **t;
	if(ptinrect(pt, DIAG->totalrect))
		return DIAG;
	for(t=obslist; t<&obslist[NFRAME]; t++)
		if(ptinrect(pt, (*t)->totalrect))
			return *t;
	return 0;
}
ontop(t)
	register Textframe *t;
{
	register Textframe **a;
	delobs(t);
	for(a=&obslist[NFRAME-2]; a>=obslist; --a)
		a[1]=a[0];
	obslist[0]=t;
	obscured(t);
}
delobs(t)
	register Textframe *t;
{
	register Textframe **a;
	for(a=obslist; a<&obslist[NFRAME]; a++)
		if(*a==t){
			do
				a[0]=a[1];
			while(++a<&obslist[NFRAME]);	/* obslist has extra element */
			return;
		}
}
obscured(wt)
	register Textframe *wt;
{
	register Textframe *t;
	for(t=frame; t<&frame[NFRAME]; t++)
		if(rectXrect(t->totalrect, wt->totalrect))
			t->obscured = TRUE;
	wt->obscured = FALSE;
}
buttons(updown)
{
	do wait(MOUSE); while((button123()!=0) != updown);
}

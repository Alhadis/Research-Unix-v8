#include <jerq.h>
#include "layer.h"
#include "queue.h"
#include "jerqproc.h"
#include "font.h"
extern	Font defont;

#define	NEWLINESIZE	16
#define CURSOR '\01'	/* cursor char in font */
Point	fudge={5, 3};
#define	LINEBUFSIZE 100
struct line{
	char buf[LINEBUFSIZE];
	char *bufp;
};
asm("	text				");
asm("	global windowstart		");
asm("	windowstart:			");
asm("		jsr	windowpr	");
windowproc(){
	struct line line;
	register unsigned char *s;
	Point curpos;
	Point org;
  restart:
	org=P->rect.origin;
	P->state&=~RESHAPED|MOVED;
	curpos=add(P->rect.origin, fudge);
	line.bufp=line.buf;
	echo(CURSOR, 0, &line, &curpos);	/* flip state */
	for(;;){
		while(P->nchars==0){
			mpxublk(P);
			sw(0);
		}
		if((P->state&(RESHAPED|MOVED))==RESHAPED)
			goto restart;
		if(P->state&MOVED){
			P->state&=~(MOVED|RESHAPED);
			curpos=add(sub(curpos, org), P->rect.origin);
			org=P->rect.origin;
		}
		echo(CURSOR, 0, &line, &curpos);	/* undraw cursor */
		s=P->cbufpout;
		while(P->nchars>0){
			P->nchars--;
			echo(*s++, 1, &line, &curpos);
			if(s>= &P->cbuf[sizeof(P->cbuf)])
				s=P->cbuf;
		}
		P->cbufpout=s;
		echo(CURSOR, 0, &line, &curpos);	/* draw at new spot */
		if(P->state&BLOCKED){
			while(P->state&BLOCKED)
				sw(0);
			mpxublk(P);
		}
	}
}
echo(c, advance, linep, pp)
	register struct line *linep;
	register Point *pp;
{
	register Fontchar *fp;
	Rectangle r;
	Point p;
	switch(c&=0x7F){
	default:
		fp=defont.info+c;
		if(fp->width+pp->x >= P->rect.corner.x)
			newline(linep, pp);
		p= *pp;
		r.origin.x=fp->x;
		r.corner.x=(fp+1)->x;
		r.origin.y=fp->top;
		r.corner.y=fp->bottom;
		p.y+=fp->top;
		if(P->layer->obs==0)
			bitblt(defont.bits, r, P->layer, p, F_XOR);
		else
			lblt(defont.bits, r, P->layer, p, F_XOR);
		if(advance){
			pp->x+=fp->width;
			if(linep->bufp<linep->buf+LINEBUFSIZE)
				*linep->bufp++=c;
		}
		break;
	case '\n':
		newline(linep, pp);
		break;
	case '\7':
		*((char *)(384*1024L+062)) = 2;	/* beep */
		break;
	case '\r':
		pp->x=P->rect.origin.x+5;
		break;
	case '\013':	/* ^K: reverse linefeed */
		if(pp->y>P->rect.origin.y+5)
			pp->y-=NEWLINESIZE;
		break;
	case '\b':
		backspace(linep, pp);
		break;
	case '\014':
		formfeed(linep, pp);
		break;
	case '\t':
		pp->x=nexttab(pp->x);
		if(pp->x>=P->layer->rect.corner.x)
			newline(linep, pp);
		if(linep->bufp<linep->buf+LINEBUFSIZE)
			*linep->bufp++=c;
		break;
	}
}
/*int eightspaces=8*dispatch[' '].c_wid;*/
int eightspaces=72;
nexttab(x){
	register xx=x-P->rect.origin.x-5;
	return(xx-(xx%eightspaces)+eightspaces+P->rect.origin.x+5);
}
backspace(linep, pp)
	register struct line *linep;
	register Point *pp;
{
	register char *p;
	register x=P->rect.origin.x+5;
	if(linep->bufp>linep->buf){
		for(p=linep->buf; p<linep->bufp-1; p++)
			if(*p=='\t')
				x=nexttab(x);
			else
				x+=defont.info[*p].width;
		pp->x=x;
		--linep->bufp;
		if(*p!='\t')
			echo(*p, 0, linep, pp);
	}
}
newline(linep, pp)
	struct line *linep;
	register Point *pp;
{
	register cursoff=0;
	if(pp->y+2*NEWLINESIZE > P->rect.corner.y-2){
		/* weirdness is because the tail of the arrow may be anywhere */
		if(rectXrect(Rect(mouse.xy.x-32, mouse.xy.y-32, mouse.xy.x+32,
				mouse.xy.y+32), P->rect)){
			cursinhibit();
			cursoff++;
		}
		lscroll(P->layer);
		clear(Rect(P->rect.origin.x, pp->y,
			P->rect.corner.x, P->rect.corner.y), 0);
		if(cursoff)
			cursallow();
	}else
		pp->y+=NEWLINESIZE;
	pp->x=P->rect.origin.x+fudge.x;
	linep->bufp=linep->buf;
}
lscroll(l)
	register Layer *l;
{
	Rectangle r;

	r = inset(l->rect, 2);
	r.origin.y += NEWLINESIZE;
	lbitblt(l, r, l, Pt(r.origin.x, r.origin.y-NEWLINESIZE), F_STORE);
}
formfeed(linep, pp)
	struct line *linep;
	Point *pp;
{
	cursinhibit();
	clear(P->rect, 1);
	cursallow();
	*pp=add(P->rect.origin, fudge);
	linep->bufp=linep->buf;
}

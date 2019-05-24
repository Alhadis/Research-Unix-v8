/*
 * terminfo:
 * jerq,
 * 	cr=^M, ind=^J, bel=^G, am, ht=^I, it#8, cud1=^J, cuf1=\EC,
 *	cuu1=\EA, cub1=^H, cols#87, lines#72, clear=^L, el=\EK,
 * 	cup=\EY%p2%' '%+%c%p1%' '%+%c, il1=\EF!, dl1=\EE!, ich1=\Ef!,
 *	dch1=\Ee!, il=\EF%p1%' '%+%c, dl=\EE%p1%' '%+%c,
 * 	ich=\Ef%p1%' '%+%c, dch=\Ee%p1%' '%+%c,
 * 	kcuu1=\EA, kcud1=\EB, kcuf1=\EC, kcub1=\ED, kbs=^H,
 * termcap:
 * jerq:\
 * 	:am:ta=^I:it#8:pt:do=^J:nd=\EC:up=\EA:bs:co#87:li#72:cl=^L:ce=\EK:\
 * 	:cm=\EY%r%+ %+ :al=\EF!:dl=\EE!:ic=\Ef!:dc=\Ee!:AL=\EF%+ :
 *	DL=\EE%+ :IC=\Ef%+ :DC=\Ee%+ :ku=\EA:kd=\EB:kr=\EC:kl=\ED:kb=^H:
 */
#include <jerq.h>
#include <font.h>
#include <queue.h>
/* Font properties */
#define	CW	9	/* width of a character */
#define	NS	14	/* newline size; height of a character */
#define	CURSOR	"\1"	/* By convention */

#define	XMARGIN	5	/* inset from border */
#define	YMARGIN	3

Point	cur;	/* in character positions */
int	xmax, ymax;
int	remote;	/* on line?  only works with dale's wiring mods */
Point	  pt();
Rectangle rect();
main()
{
	char buf[2];
	register i;

	buf[1]=0;
	remote=1;
	jinit();
	cursinhibit();
	request(RCV);	/* KBD gets handled in introutine() */
	xmax=(XMAX-2*XMARGIN)/CW-1;
	ymax=(YMAX-2*YMARGIN)/NS-1;
	cur.x=0;
	cur.y=0;
	curse();
	for(;;){
		wait(RCV);
		curse();
	Nocurse:
		buf[0] = qgetc(&RCVQUEUE)&0x7F;
		switch(buf[0]){
		case '\020':		/* boot */
			spl7();
			asm("	jmp	256*1024+8");
		case '\007':		/* bell */
			*((char *)(384*1024L+062)) = 2;
			break;
		case '\013':		/* up */
			--cur.y;
			break;
		case '\t':		/* tab modulo 8 */
			cur.x|=7;
			cur.x++;
			break;
		case '\033':
			switch(getchar()) {
			case 'Y':	/* position cursor */
				cur.x=getnum();
				cur.y=getnum();
				clippt(&cur);
				if(cur.x>xmax)	/* may clip to xmax+1 */
					cur.x=xmax;
				if(cur.y>ymax)	/* may clip to xmax+1 */
					cur.y=ymax;
				break;
			case 'A':	/* cursor up */
				--cur.y;
				break;
			case 'B':	/* cursor down */
				goto casenewline;
			case 'C':	/* cursor right */
				cur.x++;
				break;
			case 'D':	/* cursor left */
				goto casebackspace;
			case 'E':	/* delete i lines */
				i=getnum();
				scroll(Rect(0, cur.y+i, xmax+1, ymax+1), Pt(0, cur.y));
				break;
			case 'e':	/* delete i chars */
				i=getnum();
				scroll(Rect(cur.x+i, cur.y, xmax+1, cur.y+1), cur);
				break;
			case 'F':	/* insert i lines */
				i=getnum();
				scroll(Rect(0, cur.y, xmax+1, ymax+1-i), Pt(0, cur.y+i));
				break;
			case 'f':	/* insert i chars */
				i=getnum();
				scroll(Rect(cur.x, cur.y, xmax+1-i, cur.y+1), Pt(cur.x+i, cur.y));
				break;
			case 'K':	/* clear to EOL */
				clear(Rect(cur.x, cur.y, xmax+1, cur.y+1));
				break;
			}
			break;
		case '\b':		/* backspace */
		casebackspace:
			--cur.x;
			break;
		case '\n':		/* linefeed */
		casenewline:
			newline();
			break;
		case '\r':		/* carriage return */
			cur.x=0;
			break;

		case '\014':		/* clear screen */
			clear(Rect(0, 0, xmax+1, ymax+1));
			cur.x=0;
			cur.y=0;
			break;
		default:		/* ordinary char */
			string(&defont, buf, &display, pt(cur), F_STORE);
			cur.x++;
			break;
		}
		if(cur.x > xmax) {
			newline();
			cur.x = 0;
		}
		clippt(&cur);
		if(RCVQUEUE.c_cc>0)
			goto Nocurse;
		curse();
	}
}
newline()
{
	register nl;
	if(cur.y >= ymax){
		scroll(Rect(0, nl=nlcount(), xmax+1, ymax+1), Pt(0, 0));
		cur.y = ymax+1-nl;
	}else
		cur.y++;
}
nlcount()
{
	register struct cbuf *p,*pp;
	register int i=0;
	if(p=RCVQUEUE.c_head)	/* assignment = */
		for (i=0; (pp=p->next)!=0 && p->word!='\033' && p->word!='\013'; p=pp)
			if (p->word == '\n')
				i++;
	return(i>0? (i<ymax? i : ymax) : 1);
}
curse()
{
	string(&defont, CURSOR, &display, pt(cur), F_XOR);
}
getchar()
{
	while(RCVQUEUE.c_cc==0)
		wait(RCV);
	return qgetc(&RCVQUEUE)&0x7F;
}
getnum(){
	return getchar()-' ';
}
Point
pt(p)
	Point p;
{
	register short *a=&p.x;
	*a*=CW; *a+++=XMARGIN;
	*a*=NS; *a  +=YMARGIN;
	return p;
}
Rectangle
rect(r)
	Rectangle r;
{
	register short *a=&r.origin.x;
	*a*=CW; *a+++=XMARGIN;
	*a*=NS; *a+++=YMARGIN;
	*a*=CW; *a+++=XMARGIN;
	*a*=NS; *a  +=YMARGIN;
	return r;
}
/*
 * Scroll rectangle r horizontally or vertically to p.  Clear the
 * area that opens up.
 */
scroll(r, p)
	Rectangle r;
	Point p;
{
	clippt(&r.origin);
	clippt(&r.corner);
	clippt(&p);
	if(eqpt(p, r.origin))
		return;
	bitblt(&display, rect(r), &display, pt(p), F_STORE);
	if(p.x==r.origin.x){	/* vertical scroll */
		if(p.y<r.origin.y)	/* scroll up; clear bottom */
			clear(Rpt(Pt(p.x, r.corner.y-r.origin.y+p.y), r.corner));
		else			/* scroll down; clear top */
			clear(Rpt(r.origin, Pt(r.corner.x, p.y)));
	}else{			/* horizontal scroll */
		if(p.x<r.origin.x)	/* scroll left; clear right */
			clear(Rpt(Pt(r.corner.x-r.origin.x+p.x, p.y), r.corner));
		else			/* scroll right; clear left */
			clear(Rpt(r.origin, Pt(p.x, r.corner.y)));
	}
}
clippt(p)
	register Point *p;
{
	if(p->x<0)
		p->x=0;
	if(p->y<0)
		p->y=0;
	if(p->x>xmax+1)
		p->x=xmax+1;
	if(p->y>ymax+1)
		p->y=ymax+1;
}
clear(r)
	Rectangle r;
{
	rectf(&display, rect(r), F_CLR);
}
/*
 * introutine() called at video interrupt time.
 *	reads chars off the keyboard, sends things to host
 */
#define ESC	'\033'
#define ESCB	0200	/* means DON'T send escape first */
/*
 * Note that keypad keys send what they say, with the high bit set.
 * This means you usually get what you expect, but if you want
 * to tell them apart, work in raw mode. Arrows, SET-UP and PF's send
 * ESC-something.
 */
struct keytab{
	char	key;
	char	host;
}keytab[]={
	0xf1, 'A',	/* up arrow */
	0xf2, 'B',	/* down arrow */
	0xf3, 'D',	/* left arrow */
	0xf4, 'C',	/* right arrow */
	0xe1, ESCB|'7',	/* 7 */
	0xe2, ESCB|'8',	/* 8 */
	0xe3, ESCB|'9',	/* 9 */
	0xe4, ESCB|'-',	/* - */
	0xd0, ESCB|'4',	/* 4 */
	0xd1, ESCB|'5',	/* 5 */
	0xd2, ESCB|'6',	/* 6 */
	0xd3, ESCB|',',	/* , */
	0xc0, ESCB|'1',	/* 1 */
	0xc1, ESCB|'2',	/* 2 */
	0xc2, ESCB|'3',	/* 3 */
	0xc3, ESCB|'\r',/* enter */
	0xb1, ESCB|'0',	/* 0 */
	0xb2, ESCB|'.',	/* . */
	0xfe, 'w',	/* Setup */
	0xf6, 'x',	/* PF1 */
	0xf7, 'y',	/* PF2 */
	0xf8, 'z',	/* PF3 */
};

int blocked, ublocked;
#define	HIWAT	(NCHARS-100)
#define	LOWAT	100
introutine(){
	char c;
	register i;
	while(KBDQUEUE.c_cc>0){
		if ((c=qgetc(&KBDQUEUE)) & 0x80) {
			switch(c&0xFF){
			case 0xF9:	/* PF4 - toggle DTR */
				remote=!remote;
				dtrctl(remote);
				continue;
			case 0xE0:	/* BREAK */
				sendbreak();
				continue;
			case 0xB0:	/* NO SCRL */
				ublocked=!ublocked;
				continue;
			}
			for (i=0; i<sizeof(keytab)/sizeof(keytab[0]); i++) {
				if (c != keytab[i].key)
					continue;
				if ((c=keytab[i].host) == 0)
					break;
				if ((c&ESCB) == 0)
					sendchar(ESC);
				sendchar(c);
				break;
			}
		} else {
	Regular:
			if(c==0x7f)
				ublocked=0;
			if(c==0x11)		/* ^Q */
				ublocked=0;
			else if(c==0x13)	/* ^S */
				ublocked=1;
			else
				sendchar(c);
		}
	}
	if(!blocked && (RCVQUEUE.c_cc>HIWAT || ublocked)){
		sendchar(0x13);	/* ^S */
		blocked++;
	}
	if(blocked && RCVQUEUE.c_cc<LOWAT && !ublocked){
		sendchar(0x11);	/* ^Q */
		blocked=0;
	}
}

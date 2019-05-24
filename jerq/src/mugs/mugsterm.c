/*% 3cc -go #.m %
 */
#include <jerq.h>
#include "mugs.h"
#include <font.h>
#define	or	origin
#define	co	corner
char buf[100];
char squash[FSIZE][FSIZE];
Bitmap *whole;
char *menutext[]={
	"window",
	"save",
	"next",
	"exit",
	0
};
int xsize, ysize;
Rectangle msgrect, sl;
Point facep, big;
Menu menu={menutext};
picksquare(rp)
register Rectangle *rp;
{
	Point p, q;
	int r, s, maxr;
	boxcur();
	while(!button123())
		wait(CPU);
	if(!button3()){
		while(button123())
			wait(CPU);
		alive();
		return(0);
	}
	p=mouse.xy;
	maxr=min(min(p.x-big.x, p.y-big.y),
		min(big.x+xsize-p.x, big.y+ysize-p.y));
	r=0;
	drawsquare(p, r);
	while(button3()){
		q=sub(mouse.xy, p);
		s=max(abs(q.x), abs(q.y));
		if(r!=s && s<=maxr){
			drawsquare(p, r);
			drawsquare(p, s);
			r=s;
		}
	}
	drawsquare(p, r);
	if(r<=0){
		alive();
		return(0);
	}
	p=sub(p, big);
	rp->or=sub(p, Pt(r, r));
	rp->co=add(p, Pt(r, r));
	alive();
	return(1);
}
drawsquare(p, r)
Point p;
{
	box(Rpt(sub(p, Pt(r, r)), add(p, Pt(r, r))), F_XOR);
}
box(r, f)
Rectangle r;
{
	r.co=sub(r.co, Pt(1, 1));
	segment(&display, r.or, Pt(r.or.x, r.co.y), f);
	segment(&display, Pt(r.or.x, r.co.y), r.co, f);
	segment(&display, r.co, Pt(r.co.x, r.or.y), f);
	segment(&display, Pt(r.co.x, r.or.y) ,r.or, f);
}
readface(){
	getwhole();
	getsquash(Rect(0, 0, xsize, ysize));
	facep.x=(Drect.or.x+Drect.co.x+xsize+33-FSIZE)/2;
	facep.y=(Drect.or.y+Drect.co.y-FSIZE)/2;
	sl.or.x=big.x+xsize+10;
	sl.or.y=big.y;
	sl.co=add(sl.or, Pt(20, RES));
	drawface(0, RES);
}
main(){
	Rectangle r;
	request(KBD|MOUSE);
	msgrect.or.x=Drect.or.x+5;
	msgrect.or.y=Drect.co.y-20;
	msgrect.co=Drect.co;
	readface();
	for(;;){
		while(!(own()&MOUSE))
			wait(MOUSE);
		if(button12()){
			if(ptinrect(mouse.xy, sl))
				adjustface();
			else
				while(button123())
					wait(CPU);
		}
		if(button3()) switch(menuhit(&menu, 3)){
		case 0:
			bitblt(whole, whole->rect, &display, big, F_STORE);
			if(picksquare(&r)){
				drawborder(raddp(r, big));
				getsquash(r);
			}
			break;
		case 1:
			sendicon();
			break;
		case 2:
			readface();
			break;
		case 3:
			ok();
			while(!button123())
				wait(CPU);
			alive();
			if(button3()){
				sendchar(QUIT);
				exit(0);
			}
			while(button123())
				wait(CPU);
		}
		wait(CPU);
	}
}
int floyd[FSIZE+1][FSIZE+1];
Bitmap *icon;
drawface(lo, hi){
	register int *p, *f;
	register char *sq;
	register v, h, s, e, bright, contrast;
	if(!icon)
		icon=balloc(Rect(0, 0, FSIZE, FSIZE));
	if(!icon)
		return;
	dead();
	if(hi<lo){e=hi; hi=lo; lo=e;}
	if(lo<0) lo=0; else if(RES<lo) lo=RES;
	if(hi<0) hi=0; else if(RES<hi) hi=RES;
	bright=lo;
	contrast=hi-lo;
	if(contrast==0)
		contrast=1;
	sq=squash[0];
	for(v=0;v!=FSIZE;v++) for(h=0,f=floyd[v];h!=FSIZE;h++)
		*f++=max(0, min(muldiv((*sq++&0xFF)-bright, RES, contrast), RES));
	for(v=0;v!=FSIZE;v++){
		f=floyd[v];
		for(h=0,p=addr(icon, Pt(0, v));h<FSIZE;h+=32,p++){
			*p=0;
			for(s=0;s!=32 && s+h!=FSIZE;s++,f++){
				if((e=f[0])>RES/2)
					e-=RES;
				else
					*p|=1<<(31-s);
				f[FSIZE+1]+=e*3/8;
				f[FSIZE+2]+=e/4;
				f[1]+=e*3/8;
			}
		}
	}
	drawborder(raddp(Rect(0, 0, FSIZE, FSIZE), facep));
	bitblt(icon, icon->rect, &display, facep, F_STORE);
	drawborder(sl);
	rectf(&display, Rect(sl.or.x, sl.or.y,     sl.co.x, sl.or.y+lo), F_CLR);
	rectf(&display, Rect(sl.or.x, sl.or.y+lo, sl.co.x, sl.or.y+hi), F_OR);
	rectf(&display, Rect(sl.or.x, sl.or.y+hi, sl.co.x, sl.co.y), F_CLR);
	alive();
}
getwhole(){
	register Word *w;
	register x, y;
	dead();
	if(whole)
		bfree(whole);
	sendchar(WHOLE);
	switch(gtchar()){
	case WHOLE:
		break;
	case ARGC:
		pause("No more faces");
		break;
	case NGFILE:
		pause("Can't read face");
		break;
	default:
		err("WHOLE failure");
	}
	xsize=gtint();
	ysize=gtint();
	big.x=Drect.or.x;
	big.y=(Drect.origin.y+Drect.corner.y-ysize)/2;
	whole=balloc(Rect(0, 0, xsize, ysize));
	if(!whole)
		err("whole balloc failure");
	w=whole->base;
	for(y=0;y!=ysize;y++){
		for(x=0;x<xsize;x+=32)
			*w++=gtint();
		bitblt(whole, Rect(0, y, xsize, y+1),
			&display, add(big, Pt(0, y)), F_STORE);
	}
	alive();
}
getsquash(r)
Rectangle r;
{
	register h, v, hsize, vsize;
	dead();
	sendchar(SQUASH);
	sendint(r.or.x);
	sendint(r.or.y);
	sendint(r.co.x);
	sendint(r.co.y);
	if(gtchar()!=SQUASH)
		err("bad SQUASH");
	hsize=gtint();
	vsize=gtint();
	for(v=0;v!=vsize;v++) for(h=0;h!=hsize;h++)
		squash[v][h]=gtchar();
	alive();
}
gtchar(){
	register c;
	while((c=rcvchar())==-1)
		wait(RCV);
	return(c&0xff);
}
sendicon(){
	register v, h;
	char name[512];
	register char *s;
	if(!icon)
		return;
	getline(name, "Face name: ");
	dead();
	sendchar(SAVEFACE);
	s=name;
	while(*s)
		sendchar(*s++);
	sendchar('\n');
	if(gtchar()!=OK){
		alive();
		pause("write failed");
	}
	else{
		sendint(icon->rect.co.x-icon->rect.or.x);
		sendint(icon->rect.co.y-icon->rect.or.y);
		for(v=icon->rect.or.y;v!=icon->rect.co.y;v++)
			for(h=0;h!=icon->width;h++)
				sendint(icon->base[v*icon->width+h]);
		alive();
	}
	msg("");
}
gtint(){
	register i;
	i=gtchar();
	i=(i<<8)|gtchar();
	i=(i<<8)|gtchar();
	return((i<<8)|gtchar());
}
sendint(i)
register i;
{
	char buf[4];
	buf[0]=i>>24;
	buf[1]=i>>16;
	buf[2]=i>>8;
	buf[3]=i;
	sendnchars(4, buf);
}
Texture deadmouse = {
	 0x0000, 0x0114, 0xA208, 0x4100,
	 0x0000, 0x0008, 0x0004, 0x0082,
	 0x0441, 0xFFE1, 0x5FF1, 0x3FFE,
	 0x17F0, 0x03E0, 0x0000, 0x0000,
};
Texture *svcurs;
dead(){
	svcurs=cursswitch(&deadmouse);
}
Texture confirm = {
	 0x000E, 0x071F, 0x0317, 0x736F,
	 0xFBCE, 0xDB8C, 0xDBC0, 0xFB6C,
	 0x77FC, 0x0000, 0x0001, 0x0003,
	 0x94A6, 0x633C, 0x6318, 0x9490,
};
ok(){
	svcurs=cursswitch(&confirm);
}
Texture question={
	0x1e00, 0x3f80, 0x6180, 0xc0c0, 
	0xc0c0, 0x60c0, 0x0180, 0x0300, 
	0x0600, 0x0c00, 0x0c00, 0x0c00, 
	0x0000, 0x0000, 0x0c00, 0x0c00, 

};
what(){
	svcurs=cursswitch(&question);
}
Texture sweep = {
	 0x43FF, 0xE001, 0x7001, 0x3801,
	 0x1D01, 0x0F01, 0x8701, 0x8F01,
	 0x8001, 0x8001, 0x8001, 0x8001,
	 0x8001, 0x8001, 0x8001, 0xFFFF,
};
boxcur(){
	svcurs=cursswitch(&sweep);
}
alive(){
	cursswitch(svcurs);
}
msg(s)
char *s;
{
	rectf(&display, msgrect, F_CLR);
	nmsg(s);
}
nmsg(s)
char *s;
{
	rectf(&display, Rpt(string(&defont, s, &display, msgrect.or,
					F_STORE), msgrect.co), F_CLR);
}
pause(m)
char *m;
{
	msg(m);
	what();
	while(!button123())
		wait(CPU);
	while(button123())
		wait(CPU);
	alive();
}
err(s)
char *s;
{
	pause(s);
	exit(1);
}
getline(buf, prompt)
char buf[];
char *prompt;
{
	register int c;
	register char *s=buf;
	char line[512];
	sprintf(line, "%s\1", prompt);
	msg(line);
	wait(KBD);
	while((c=kbdchar())!='\n' && c!='\r'){
		switch(c){
		case -1:
			continue;
		case '\b':
			if(s!=buf)
				*--s='\0';
			break;
		case 027:	/* ctrl-w */
			while(s!=buf && !idchar(s[-1]))
				--s;
			while(s!=buf && idchar(s[-1]))
				--s;
			*s='\0';
			break;
		case '@':
			s=buf;
			*s='\0';
			break;
		default:
			*s++=c;
			*s='\0';
			break;
		}
		sprintf(line, "%s%s\1 ", prompt, buf);
		nmsg(line);
	}
}
idchar(c)
register c;
{
	return('a'<=c && c<='z' || 'A'<=c && c<='Z' || '0'<=c && c<='9' || c=='_');
}
drawborder(r)
Rectangle r;
{
	box(inset(r, -1), F_OR);
	box(inset(r, -2), F_CLR);
	box(inset(r, -3), F_OR);
}
adjustface(){
	int new, delta;
	static lo=0, hi=RES;
	if(button1()){
		hi=min(max(lo+1, mouse.xy.y-sl.or.y), RES-1);
		drawface(lo, hi);
		while(button1()){
			new=min(max(lo+1, mouse.xy.y-sl.or.y), RES-1);
			if(new!=hi)
				drawface(lo, hi=new);
			wait(CPU);
		}
	}
	else{
		delta=hi-lo;
		lo=min(max(0, mouse.xy.y-sl.or.y), RES-delta-1);
		drawface(lo, lo+delta);
		while(button2()){
			new=min(max(0, mouse.xy.y-sl.or.y), RES-delta-1);
			if(new!=lo)
				drawface(lo=new, new+delta);
			wait(CPU);
		}
		hi=lo+delta;
	}
}

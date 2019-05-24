#include <jerq.h>
#include <font.h>
/* user nice sys queue idle */
int vec[5];
Texture black={
	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	
	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	0xFFFF,	
};
Texture white={
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
};
Texture darkgrey = {
	0xDDDD, 0x7777, 0xDDDD, 0x7777, 0xDDDD, 0x7777, 0xDDDD, 0x7777,
	0xDDDD, 0x7777, 0xDDDD, 0x7777, 0xDDDD, 0x7777, 0xDDDD, 0x7777,
};
Texture lightgrey = {
	0x2222, 0x8888, 0x2222, 0x8888, 0x2222, 0x8888, 0x2222, 0x8888,
	0x2222, 0x8888, 0x2222, 0x8888, 0x2222, 0x8888, 0x2222, 0x8888,
};
Texture grey = {
	0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555,
	0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555,
};
Texture *txt[]={
	&black, &lightgrey, &darkgrey, &grey, &white
};
char time[30];
int	size=100;
Rectangle rect, new, old;
Point pt[4];
Bitmap *face;
int notzero;
main()
{
	register i, now;
	int oldvec[5];
	Rectangle tempr;
	tempr = display.rect;
	dellayer(P->layer);
	tempr.corner.y = tempr.origin.y + 60;
	if (tempr.corner.y > YMAX-8) {
		tempr.corner.y = YMAX-8;
		tempr.origin.y = YMAX-8-60;
	}
	Drect = inset(tempr,2);
	Jdisplayp = (Bitmap *) (P->layer = newlayer(tempr));
	if (!P->layer)
		exit();
	face = balloc(Rect(0,0,32,32));
	if (!face)
		exit();
	rectf(face,face->rect,F_CLR);

	request(RCV|MOUSE);
reshaped:
	rectf(&display, Drect, F_CLR);
	rect=Drect;
	rect.corner.y=rect.origin.y+16+4;
	rect=inset(rect, 4);
	size=rect.corner.x-rect.origin.x;
	rectf(&display, inset(rect, -2), F_OR);
	rectf(&display, rect, F_CLR);
	for(i=0; i<4; i++)
		vec[i]=oldvec[i]=0;
	vec[4]=oldvec[4]=size; /* assumes txt[4]==black */
	pt[0].y=pt[2].y=rect.origin.y;
	pt[1].y=pt[3].y=rect.corner.y;
	drawtime();
	zeromail();
	drawmail();
	for(;;sleep(3)){
		i=own();
		if(i&RCV)
			get();
		if((i&MOUSE) && notzero){
			drawmail();
			zeromail();
			drawmail();
		}
		new=old=rect;
		for(i=0; i<5; i++){
			now=relax(oldvec[i], vec[i]);
			old.corner.x=oldvec[i]+old.origin.x;
			new.corner.x=now+new.origin.x;
			sort(old.origin.x,old.corner.x, new.origin.x,new.corner.x);
			texture(&display, Rpt(pt[0], pt[1]), txt[i], F_XOR);
			texture(&display, Rpt(pt[2], pt[3]), txt[i], F_XOR);
			new.origin.x=new.corner.x;
			old.origin.x=old.corner.x;
			oldvec[i]=now;
		}
		if(P->state&RESHAPED){
			P->state&=~RESHAPED;
			goto reshaped;
		}
	}
}
relax(o, n)
	int o, n;
{
	register a;
	register s=1;
	if(o>n)
		s= -1;
	a=abs(o-n);
	if(a<2)
		return n;
	if(a<63)
		return o+s;
	return (63*o+n)/64;
}
sort(x)
	int x;
{
	register int *p= &x;
	register i,j;
	register t;
	for(i=0; i<3; i++){
		for(j=0; j<3; j++)
			if(p[j]>p[j+1]){
				t=p[j];
				p[j]=p[j+1];
				p[j+1]=t;
			}
	}
	for(i=0; i<4; i++)
		pt[i].x=p[i];
}
drawtime()
{
	string(&defont, time, &display, Pt(rect.origin.x, rect.corner.y+11), F_XOR);
}
drawmail()
{
/*
	string(&defont, mail, &display,
		Pt(rect.origin.x+19*defont.info[' '].width,
			rect.corner.y+11), F_XOR);
*/
	bitblt(face,face->rect,&display,
		Pt(rect.origin.x+19*defont.info[' '].width,rect.corner.y+4),
		F_XOR);
}
get(){
	register i, sum, nsum, c;
	int readbuf[5];
loop:
	for(sum=0,i=0; i<5; i++)
		sum+=readbuf[i]=xgetchar();
	switch(c=xgetchar()){
	default:
		do; while(xgetchar()!='\n');
		goto loop;
	case 'T':
		drawtime();
		gettime(readbuf);
		drawtime();
		break;
	case 'M':
		drawmail();
		getmail();
		drawmail();
		break;
	case '\n':
		for(nsum=0,i=0; i<4; i++)
			nsum+=vec[i]=((long)(readbuf[i])*size) / sum;
		vec[4]=size-nsum;
		break;
	}
}
xgetchar(){
	register c;
	if((c=rcvchar())==-1)
		wait(RCV), c=rcvchar();
	return c&0xFF;
}
gettime(s)
	int *s;
{
	register char *p=time;
	register i;
	for(i=0; i<5; i++)
		*p++ = *s++;
	while((*p=xgetchar()) != '\n')
		p++;
	*p=0;
}
HexToDec(Hex)
int Hex;
{
  if ((Hex>='0') && (Hex<='9')) return(Hex-'0');
  if ((Hex>='A') && (Hex<='F')) return(10+Hex-'A');
  if ((Hex>='a') && (Hex<='f')) return(10+Hex-'a');
  return(-1);
}

getmail()
{
	long *bits=(long *)face->base;
	int i,j,k,val;
	notzero = 1;
	for (i=0, j=0, val=0; i<8*32; i++) {
		if ((k = HexToDec(xgetchar())) == -1) {
			bombmail();
			goto beep;
		}
		val = (val<<4) | k;
		if (++j==8) {
			*bits++ = val;
			j = val = 0;
		}
	}
	if (xgetchar() != '\n')
		bombmail();
beep:
	ringbell();
}

zeromail()
{
	rectf(face,face->rect,F_CLR);
	notzero=0;
}

bombmail()
{
	rectf(face,face->rect,F_STORE);
	notzero = 1;
}

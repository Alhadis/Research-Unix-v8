/*
 * _line - draw a line from x0 to x1
 *	assumes already clipped, so no checking, just GO!
 * 	the line is closed w.r.t. the endpoints
 */
/*
 * From ACM Nov 1973 V16, #11, I thought, but look in Newman & Sproull
 */
#define	LINE_C		/* This file "owns" the header file line.h */
#define	LEFT	1024	/* line will be drawn right to left */
#include <jerq.h>
#include <line.h>
_line(b, x0, x1, fun)
	register Bitmap *b;
	Point x0, x1;
	Code fun;
{
	register d, yincr;
	short deltax, deltay;
	short /*unsigned*/ x, y;
	register UWord bit;
	register Word *screenp;
	register long screenincr;
	register d1;
	int d2;
	register count;
	Point temp;

	if(Jxmajor==0){
		if(x0.y>x1.y){
			temp=x0;
			x0=x1;
			x1=temp;
		}
		if(x0.x>x1.x)
			fun|=LEFT;
	}
	else if(x0.x>x1.x){
		temp=x0;
		x0=x1;
		x1=temp;
	}
	x=x0.x;
	y=x0.y;
	deltax=x1.x-x;
	deltay=x1.y-y;
	yincr=(deltay>0)? 1 : -1;
	screenp=addr(b,x0);
	screenincr=yincr*(int)b->width;	/* signed product please */
	bit=LASTBIT<<(WORDSIZE-1-(x&WORDMASK));	/* don't sign extend */
	deltax=abs(deltax);
	deltay=abs(deltay);
	if(Jxmajor==0)
    goto Steepline;
	count=deltax+1;	/* so we can say --count (faster) */
	if(deltay==0){			/* easy and fast */
		y=x+count;	/* sorry about overusing the variables */
		if(x&WORDMASK){
			d=topbits[x&WORDMASK];
			switch(fun){
			case F_STORE:
			case F_OR:
				if((x&~WORDMASK)==(y&~WORDMASK)){  /* same word */
					d^=topbits[y&WORDMASK];
					*screenp|=d;
					return;
				}
				*screenp++|=~d;
				x=(x|WORDMASK)+1;
				break;
			case F_CLR:
				if((x&~WORDMASK)==(y&~WORDMASK)){  /* same word */
					d^=topbits[y&WORDMASK];
					*screenp&=~d;
					return;
				}
				*screenp++&=d;
				x=(x|WORDMASK)+1;
				break;
			case F_XOR:
				if((x&~WORDMASK)==(y&~WORDMASK)){  /* same word */
					d^=topbits[y&WORDMASK];
					*screenp^=d;
					return;
				}
				*screenp++^=~d;
				x=(x|WORDMASK)+1;
				break;
			}
		}
		switch(fun){
		case F_STORE:
		case F_OR:
			while((x+=WORDSIZE)<=y)
				*screenp++=ONES;
			if(y&WORDMASK)
				*screenp|=topbits[y&WORDMASK];
			break;
		case F_CLR:
			while((x+=WORDSIZE)<=y)
				*screenp++=0;
			if(y&WORDMASK)
				*screenp&=~topbits[y&WORDMASK];
			break;
		case F_XOR:
			while((x+=WORDSIZE)<=y)
				*screenp++^=ONES;
			if(y&WORDMASK)
				*screenp^=topbits[y&WORDMASK];
			break;
		}
		return;
	}
	/* else */
	d=Jsetdda(x);
	d1=2*Jdminor;
	d2=d1-2*Jdmajor;
	switch(fun){
	case F_STORE:
	case F_OR:
		for(;;){
			*screenp|=bit;
			if(--count==0)
				return;
			if((bit>>=1)==0){
				screenp++;
				bit=FIRSTBIT;
			}
			if(d<0)
				d+=d1;
			else{
				d+=d2;
				screenp+=screenincr;
			}
		}
	case F_CLR:
		for(;;){
			*screenp&=~bit;
			if(--count==0)
				return;
			if((bit>>=1)==0){
				screenp++;
				bit=FIRSTBIT;
			}
			if(d<0)
				d+=d1;
			else{
				d+=d2;
				screenp+=screenincr;
			}
		}
	case F_XOR:
		for(;;){
			*screenp^=bit;
			if(--count==0)
				return;
			if((bit>>=1)==0){
				screenp++;
				bit=FIRSTBIT;
			}
			if(d<0)
				d+=d1;
			else{
				d+=d2;
				screenp+=screenincr;
			}
		}
	}
    Steepline:
	count=deltay+1;
	if(deltax==0){			/* easy and fast */
		switch(fun){
		case F_STORE:
		case F_OR:
			for(;;screenp+=screenincr){
				*screenp|=bit;
				if(--count==0)
					return;
			}
		case F_CLR:
			bit= ~bit;
			for(;;screenp+=screenincr){
				*screenp&=bit;
				if(--count==0)
					return;
			}
		case F_XOR:
			for(;;screenp+=screenincr){
				*screenp^=bit;
				if(--count==0)
					return;
			}
		}
	}
	/* else */
	d=Jsetdda(y);
	d1=2*Jdminor;
	d2=d1-2*Jdmajor;
	switch(fun){
	case F_STORE:
	case F_OR:
		for(;;screenp+=screenincr){
			*screenp|=bit;
			if(--count==0)
				return;
			if(d<0)
				d+=d1;
			else{
				d+=d2;
				if((bit>>=1)==0){
					screenp++;
					bit=FIRSTBIT;
				}
			}
		}
	case F_STORE|LEFT:
	case F_OR|LEFT:
		for(;;screenp+=screenincr){
			*screenp|=bit;
			if(--count==0)
				return;
			if(d<0)
				d+=d1;
			else{
				d+=d2;
				if((bit<<=1)==0){
					screenp--;
					bit=LASTBIT;
				}
			}
		}
	case F_CLR:
		for(;;screenp+=screenincr){
			*screenp&=~bit;
			if(--count==0)
				return;
			if(d<0)
				d+=d1;
			else{
				d+=d2;
				if((bit>>=1)==0){
					screenp++;
					bit=FIRSTBIT;
				}
			}
		}
	case F_CLR|LEFT:
		for(;;screenp+=screenincr){
			*screenp&=~bit;
			if(--count==0)
				return;
			if(d<0)
				d+=d1;
			else{
				d+=d2;
				if((bit<<=1)==0){
					screenp--;
					bit=LASTBIT;
				}
			}
		}
	case F_XOR:
		for(;;screenp+=screenincr){
			*screenp^=bit;
			if(--count==0)
				return;
			if(d<0)
				d+=d1;
			else{
				d+=d2;
				if((bit>>=1)==0){
					screenp++;
					bit=FIRSTBIT;
				}
			}
		}
	case F_XOR|LEFT:
		for(;;screenp+=screenincr){
			*screenp^=bit;
			if(--count==0)
				return;
			if(d<0)
				d+=d1;
			else{
				d+=d2;
				if((bit<<=1)==0){
					screenp--;
					bit=LASTBIT;
				}
			}
		}
	}
}

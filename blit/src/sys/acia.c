#include <jerq.h>
#include "/usr/jerq/include/acia.h"
#include "queue.h"
#define	pio2	((char *)(384*1024L+013))
#define	stat2	((char *)(384*1024L+011))
static dtr;
aciainit(){
	*stat2=A_RESET;
	*stat2=A_S2NB8|A_CDIV16|A_RE;
	dtr = 1;
}
rcvchar(){
	register c=qgetc(&RCVQUEUE);
	if(c!=-1)
		c&=0xFF;
	return c;
}
sendchar(c){
	qputc(&OUTQUEUE, c);
	aciatrint();
}
aciatrint(){
	register sr, c;
	sr=spl5();
	if (*stat2 & A_DCDBAR)
		c = *pio2;	/* Clear Carrier Failure Interrupt */
	if (*stat2&A_TDRE) {
		if (dtr == 0)
			c = -1;
		else
			c=qgetc(&OUTQUEUE);
		if (c!=-1) {
			*pio2=c;
			*stat2=A_S2NB8|A_CDIV16|A_RE|A_RSBLTE;
		} else
			*stat2= (dtr ? A_S2NB8|A_CDIV16|A_RE
				     : A_S2NB8|A_CDIV16|A_RE|A_RSBHTD);
	}
	splx(sr);
}

sendbreak(){
	if (dtr == 0)
		return(-1);
	*stat2=A_RSBLBTD;
	spl0();
	nap(10);
	*stat2=A_S2NB8|A_CDIV16|A_RE;
	return(0);
}

dtrctl(flg) {
	dtr=flg;
	aciatrint();
}

tstdcd() {
	return((*stat2 & A_DCDBAR) == 0);
}
trenable(){
	*stat2=A_S2NB8|A_CDIV16|A_RE|A_RSBLTE;
}
trdisable(){
	*stat2=A_S2NB8|A_CDIV16|A_RE;
}

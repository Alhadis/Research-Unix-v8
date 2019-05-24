/*
	display some bytes as a bitmap

	usage:	jx showbits < bytes
	kbd input: width of the bitmap in words as n<cr>, terminate by q<cr>
*/

#include <jerq.h>
#include <jerqio.h>
#define	SIZE	4000
char	f[SIZE];
int nbytes;
main(){
	register char *f, *p;
	register i,c;
	char buf[64];
	jinit();
	for(i=0,p=f; i<SIZE && (c=getchar())!=EOF; i++)
		*p++ = c;
	nbytes=i;
	jmoveto(Pt(10, 800));
	sprintf(buf, "%d bytes", nbytes);
	jstring(buf);
	request(KBD);
	for(;;)
		show(rin());
}
rin(){
	register n=0,c;
	jmoveto(Pt(10, 900));
	jstring("width?");
	for(;;){
		wait(KBD);
		c=kbdchar();
		if(c=='\r' || c=='\n')
			break;
		if(c=='q')
			exit();
		n=n*10+c-'0';
	}
	if(n==0)
		exit();
	cursinhibit();
	rectf(&display, Drect, F_CLR);
	cursallow();
	return n*2;
}
show(width){
	register i, j;
	register char *p, *q=f;
	register n=0;
	while(n<nbytes){
		if(n%width==0)
			p=(char *)addr(&display, add(Drect.origin,Pt(16, n/width)));
		*p++= *q++;
		n++;
	}
}

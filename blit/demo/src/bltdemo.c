#include <jerq.h>
Rectangle rect={400, 400, 501, 501};
main()
{
	register x, y;
	register long i;
	jinit();
	WonB();
	for(y=0; y<YMAX; y+=20){
		jmoveto(Pt(0, y));
		jlineto(Pt(XMAX, y), F_STORE);
	}
	for(x=0; x<XMAX; x+=20){
		jmoveto(Pt(x, 0));
		jlineto(Pt(x, YMAX), F_STORE);
	}
	for(x=0; x<300; x++){
		bitblt(&display, rect, &display, sub(rect.origin, Pt(1, 1)), F_STORE);
		rect=rsubp(rect, Pt(1, 1));
	}
	eor();
	for(i=200000; --i;) ;
	exit();
}
eor(){
	register long i, *p;
	p=(long *)display.base;
	for(i=0; i<25*1024L; i++)
		*p++^=0xFFFFFFFF;
}

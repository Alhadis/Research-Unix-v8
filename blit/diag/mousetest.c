#include <jerq.h>
char buf[64];
main(){
	jinit();
	binit();
	jBonW();
	spl0();
	for(;;nap(60)){
		sprintf(buf, "%x %x %d", *XMOUSE, *YMOUSE, mouse.buttons);
		jrectf(0, 0, 100, 100, F_CLR);
		jmoveto(20, 50);
		jstring(buf);
	}
}

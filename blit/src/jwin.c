#include "/usr/blit/include/jioctl.h"
struct winsize win;
main(){
	if(ioctl(0, JMPX, 0)==-1)
		printf("jwin: not mpx\n");
	else{
		ioctl(0, JWINSIZE, &win);
		printf("bytes:\t%d %d\nbits:\t%d %d\n",
			win.bytesx, win.bytesy, win.bitsx, win.bitsy);
	}
}

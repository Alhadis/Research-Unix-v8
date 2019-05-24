#include <jerq.h>
jinit(){
	extern int end;
	*DADDR = 156*(1024/4);
	*DSTAT=1;
	qinit();
	aciainit();
	binit();
	kbdinit();
	cursinit();
	allocinit(&end, (int *)(90*1024L));
	spl0();
	sleep(120);	/* wait for 68ld to exit */
}

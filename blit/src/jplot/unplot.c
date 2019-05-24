#include <stdio.h>
#include <sgtty.h>
#include "/usr/blit/include/jioctl.h"
#include "jcom.h"

main(){
	struct sgttyb jerq, savetty;
	char buf[2];

	ioctl(2, TIOCGETP, &jerq);
	savetty = jerq;
	jerq.sg_flags &= ~ECHO;
	jerq.sg_flags |= RAW;
	ioctl(2, TIOCSETN, &jerq);
	buf[0] = REQ;
	write(2, buf, 1);
	read(0, buf,2);
	buf[0] = EXIT;
	write(2, buf, 1);
	ioctl(2,TIOCSETN, &savetty);
}

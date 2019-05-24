#include <stdio.h>
#include "/usr/jerq/include/jioctl.h"

main()
{
	struct winsize jwin;
	if (ioctl(0, JWINSIZE, &jwin) >= 0) {
		printf("li=#%d:co=#%d:tc=vitty:\n",jwin.bytesy,jwin.bytesx);
	}
}

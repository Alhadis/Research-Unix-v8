#include <jerq.h>
#include "queue.h"
#include "/usr/jerq/include/acia.h"
#define	pio2	((char *)(384*1024L+062))
#define	stat2	((char *)(384*1024L+060))
kbdchar(){
	return qgetc(&KBDQUEUE);
}
kbdinit(){
	*stat2=A_RESET;
	*stat2=A_S2NB8|A_CDIV16|A_RE;
}
auto2(){
	qputc(&KBDQUEUE, *pio2&0xFF);
}

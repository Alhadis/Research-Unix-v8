#include <jerq.h>
Bitmap display={(Word *)(156*1024), XMAX>>WORDSHIFT, 
		0, 0, XMAX, YMAX};
asm("set	crc,	0x4036c");
asm("global	crc");

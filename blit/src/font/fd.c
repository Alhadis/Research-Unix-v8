#include <jerq.h>
#include <jerqio.h>
#include <font.h>

main()
{
	register c;
	extern putchar();

	printf("n=%d\n", defont.n);
	printf("wid=%d rect=(%d,%d)-(%d,%d)\n", defont.bits->width, defont.bits->rect);
	exit(1);
}

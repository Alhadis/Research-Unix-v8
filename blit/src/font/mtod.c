/*
	dump to mpx font defont out to disc

	usage: jx mtod > file
*/

#include <jerq.h>
#include <jerqio.h>
#include <font.h>

main()
{
	extern putchar();

	outfont(&defont, putchar);
	exit(0);
}

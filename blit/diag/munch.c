#include <jerq.h>
#include <font.h>

main()
{
	register long	i, *p;
	register x, y, j, dj;
	register unsigned long	bit;
	register char	*cp;
	char	buf[20];

	jinit();
	request(KBD);
	p = (long *) display.base;

	for (dj = 1; ; ) {
		sprintf(buf,"%d",dj);
		string(&defont,buf,&display,Pt(5,5),F_XOR);
		j = 0;
		do {
			if (kbdchar() == 'q')
				exit();
			for (x = 0; x < XMAX; x++) {
				y = (x ^ j) & (YMAX - 1);
				cp = (char *)(p + (y * 25)) + (x >> 3);
				*cp ^= (0x80 >> (x & 7));
			}
			j += dj;
		} while (j != dj * YMAX);
		string(&defont,buf,&display,Pt(5,5),F_XOR);
prime:
		dj++;
		for (j = 2; j < dj; j++)
			if (j*(dj/j) == dj)
				goto prime;
	}
}

/*
 * This guy puts up sets of strips and just sits there
 *
 */

#include <jerq.h>
#include <jerqio.h>

main()
{
	int i,j,x,y;
	char s[10];
	jxinit();
	BonW();
	rectf(&display,display.rect,F_XOR);
	for (j = 0; j < 32; j++) {
		y = j*32;
		sprintf(s,"%d",31-j);
		string(&display,s,Pt(0,y+20));
		for (i = 0; i < 25; i++) {
			x = i*32 + j;
			segment(&display,Pt(x,y),Pt(x,y+25),F_CLR);
		}
	}
	for (;;) {}
	for (;;) {}
}

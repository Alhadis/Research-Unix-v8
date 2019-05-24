#include <jerq.h>
#undef MPX

Point p = {400,512};

main()
{
	register i,j;
	request(KBD);
	while ((own()&KBD) == 0) {
		/*
		for (i = 0; i < 800; i += 2) {
			segment(&display,p,Pt(i,0),F_XOR);
			segment(&display,p,Pt(i,1023),F_XOR);
		}
		*/
		for (i = 112; i < 912; i += 2) {
			segment(&display,p,Pt(0,i),F_XOR);
			segment(&display,p,Pt(800,i),F_XOR);
		}
	}
}

#include <jerq.h>
main(){
	jinit();
	rectf(&display, Drect, F_XOR);
	elarc(&display, Pt(300, 300), 50, 50,
		Pt(350, 300), Pt(300, 350), F_XOR);
	sleep(300);
	rectf(&display, Drect, F_XOR);
	exit(0);
}

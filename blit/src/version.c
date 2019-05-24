#include <jerq.h>
/* Print ROM version number */
main(){
	jinit();
	jmoveto(Pt(0, 0));
	jstring(*(char **)(256*1024+016));
	request(KBD);
	wait(KBD);
	kbdchar();
	exit();
}

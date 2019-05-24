#include <jerq.h>
main(){
	register i;
	jinit();
	for(i=400; i>0; i-=2)
		jdisc(400, 500, i, F_XOR);
}

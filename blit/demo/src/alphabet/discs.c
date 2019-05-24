#include "alph.h"
extern done;

discs() {
	clearscreen();
	done=false;
	while (!done) {
		jdisc(Pt(r(180, 767-180), r(180, 1023-180)), r(50, 180),F_XOR);
		done = keyhit();
	}
}

#include "alph.h"
schwartz() {
	int i, j;

	rectf(&display, Drect, F_OR);
	i=100;
	while (!keyhit()) {
		rectf(&display, Rect(384-192, 512-256, 384+192, 512+256), F_XOR);
		for (j=0; j<=i; j++);
		i+=100;
		if (i==10000) i=100;
	}
}

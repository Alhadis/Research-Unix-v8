D=/usr/bin/factor
CFLAGS = -O

factor:	factor.o
	cc -o factor factor.o -lm

install:	factor
	rm -f $D
	cp factor $D
	strip $D
	chown bin $D
	chgrp bin $D
	chmod o-w $D

clean:
	rm -f factor *.o core a.out

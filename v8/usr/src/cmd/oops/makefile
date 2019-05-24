D=/bin/oops
CFLAGS = -O

oops:	oops.o
	cc -o oops oops.o -ljobs

install:	oops
	rm -f $D
	cp oops $D
	strip $D
	chown bin $D
	chgrp sys $D
	chmod o-w,g+ws $D

clean:
	rm -f oops *.o

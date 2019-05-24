D=/etc/quot
CFLAGS = -O

quot:	quot.o
	cc -o quot quot.o

install:	quot
	rm -f $D
	cp quot $D
	strip $D
	chown bin $D
	chgrp sys $D
	chmod o-w,g+ws $D

clean:
	rm -f quot *.o

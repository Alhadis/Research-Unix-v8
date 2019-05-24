D=/usr/bin/load
CFLAGS = -O

load:	load.o
	cc -o load load.o

install:	load
	rm -f $D
	cp load $D
	strip $D
	chown bin $D
	chgrp sys $D
	chmod o-w,g+ws $D

clean:
	rm -f load *.o

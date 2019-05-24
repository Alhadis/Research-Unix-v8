D=/etc/pstat
CFLAGS = -Od2

pstat:	pstat.o
	cc -o pstat pstat.o

install:	pstat
	rm -f $D
	cp pstat $D
	strip $D
	chown bin $D
	chgrp sys $D
	chmod o-w,g+ws $D

clean:
	rm -f pstat *.o

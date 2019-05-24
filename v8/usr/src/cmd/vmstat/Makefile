CFLAGS = -Od2

vmstat:	vmstat.o
	cc -o vmstat vmstat.o

install:	vmstat
	strip vmstat
	mv vmstat /bin
	chgrp sys /bin/vmstat
	chmod g+s /bin/vmstat

clean:
	rm -f vmstat *.o

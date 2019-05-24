CFLAGS = -Od2

dmesg:	dmesg.o
	cc -o dmesg dmesg.o

install:	dmesg
	strip dmesg
	cp dmesg /etc
	chgrp sys /etc/dmesg
	chmod g+s /etc/dmesg

clean:
	rm -f *.o dmesg

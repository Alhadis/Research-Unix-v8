CFLAGS = -Od2

iostat:	iostat.o
	cc -o iostat iostat.o

install:	iostat
	strip iostat
	mv iostat /bin
	chgrp sys /bin/iostat
	chmod g+s /bin/iostat

clean:
	rm -f iostat *.o

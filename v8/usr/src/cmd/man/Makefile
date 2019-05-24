CFLAGS = -Od2

man:	man.o
	cc -o man man.o

install:	man
	strip man
	mv man /usr/lib
	cp man.sh /usr/bin/man
	cp mancache.sh /usr/lib/mancache

clean:
	rm -f *.o man

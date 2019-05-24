all:	bb lprint nexit.o

bb:	bb.o

lprint:	lprint.o

install:	all
	cp bb nexit.o /usr/lib/prof
	strip /usr/lib/prof/bb
	cp lprint /usr/bin
	strip /usr/bin/lprint

clean:
	rm -r *.o bb lprint

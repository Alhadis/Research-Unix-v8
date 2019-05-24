D=/usr/bin/ct
CFLAGS = -Od2

ct:	ct.o
	cc -o ct ct.o -lc -ljobs

install:	ct
	rm -f $D
	cp ct $D
	strip $D
	chown root $D
	chmod og-w,u+s $D

clean:
	rm -f ct *.o

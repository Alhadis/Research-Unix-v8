D=/usr/bin/w
D2=/usr/bin/uptime
CFLAGS = -Od2

w:	w.o
	cc -o w w.o

install:	w
	rm -f $D $(D2)
	cp w $D
	strip $D
	ln $D $(D2)
	/etc/chown bin,sys $D
	chmod o-w,g+ws $D

clean:
	rm -f w *.o

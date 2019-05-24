D = /bin/ed
CFLAGS = -Od2

ed:	ed.o
	cc -o ed ed.o

install:	ed
	strip ed
	mv ed $D
	rm -f /bin/e
	ln /bin/ed /bin/e

clean:
	rm -f *.o ed

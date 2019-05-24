CFLAGS = -Od2

a.out:	m4.o m4ext.o m4macs.o m4y.o
	cc m4.o m4ext.o m4macs.o m4y.o

m4.o m4ext.o m4macs.o :	m4.h

list:
	pr m4.h m4.c m4ext.c m4macs.c m4y.y makefile

gcos:
	yacc m4y.y
	mv y.tab.c m4.tab.c
	fsend m4*.c m4.test*

install:	a.out
	strip a.out
	mv a.out /usr/bin/m4

clean:
	rm -f a.out *.o

DESTDIR=/usr/jerq
CC=/usr/jerq/bin/mcc

libc.a : crt
	cd crt; make
	-mv libc.a old.libc.a
	ar cr libc.a `mlorder crt/*.o | tsort`
	$(CC) -c csu/*.s

clean :
	cd crt; make clean

install :
	install libc.a $(DESTDIR)/lib/libc.a
	install crt0.o $(DESTDIR)/lib

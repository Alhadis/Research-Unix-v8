CFLAGS = -Od2

rmdir:	rmdir.o
	cc -o rmdir rmdir.o

install:	rmdir
	strip rmdir
	mv rmdir /bin
	chown root /bin/rmdir
	chmod og-w /bin/rmdir
	chmod u+s /bin/rmdir

clean:
	rm -f rmdir *.o

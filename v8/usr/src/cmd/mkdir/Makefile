CFLAGS = -Od2

mkdir:	mkdir.o
	cc -o mkdir mkdir.o

install:	mkdir
	strip mkdir
	mv mkdir /bin
	chown root /bin/mkdir
	chmod og-w /bin/mkdir
	chmod u+s /bin/mkdir

clean:
	rm -f mkdir *.o

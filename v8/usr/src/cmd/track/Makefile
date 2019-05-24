CFLAGS = -Od2

track:	track.o
	cc -o track track.o -ldk

install:	track
	strip track
	mv track /bin

clean:
	rm -f track *.o

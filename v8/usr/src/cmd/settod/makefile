CFLAGS = -Od2

settod:	settod.o
	cc -o settod settod.o -ldk

install:	settod
	strip settod
	mv settod /etc

clean:
	rm -f *.o settod

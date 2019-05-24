CFLAGS=-O

map: map.h map.o  libmap.a
	cc -n map.o libmap.a -lplot -lm -o map

libmap.a: 
	cd libmap; rm -f map.h; ln ../map.h; make

install:	map libmap.a
	strip map 
	cp map /usr/lib
	cp map.sh /usr/bin/map
	cp libmap.a /usr/lib
	ranlib /usr/lib/libmap.a

clean:
	rm -f map map.o libmap.a
	cd libmap; make clean

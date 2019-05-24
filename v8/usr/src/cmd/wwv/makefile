wwv:	wwv.o
	cc -s -o wwv  wwv.o -ldk

install:	wwv
	strip wwv
	mv wwv /usr/bin

clean:
	rm -f *.o wwv

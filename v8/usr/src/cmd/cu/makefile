cu:	cu.c
	cc -o cu cu.c -lc

install:	cu
	strip cu
	mv cu /usr/bin

clean:
	rm -f *.o cu

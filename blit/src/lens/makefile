CC=mcc
AS=mas
CFLAGS=

lens:	lens.o magn.o mag2.o mag3.o mageven.o magodd.o magslow.o
	mcc -o lens lens.o magn.o mag2.o mag3.o mageven.o magodd.o magslow.o

magn.o:	magn.c
	mcc -g -c magn.c

bytable.h:	magtbl
	magtbl > bytable.h

magtbl:	magtbl.c
	cc -o magtbl magtbl.c

install:	lens
	mopt lens
	cp m.out /usr/blit/mbin/lens.m
	mstrip /usr/blit/mbin/lens.m

clean:
	rm -f *.o m.out core magtbl


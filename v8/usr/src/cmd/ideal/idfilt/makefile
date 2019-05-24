idfilt.o	:idfilt.c idfilt.h

4014.o	:4014.c idfilt.h

4filt	:idfilt.o 4014.o
	cc -o 4filt 4014.o idfilt.o -l4014 -lm

202.o	:202.c idfilt.h

cv.o	:cv.c idfilt.h

tfilt	:idfilt.o 202.o
	cc -o tfilt 202.o idfilt.o

cvfilt	:idfilt.o cv.o
	cc -o cvfilt cv.o idfilt.o

aps.o	:aps.c

apsfilt	:aps.o idfilt.o
	cc -o apsfilt aps.o idfilt.o

idsort	:idsort.c
	cc -o idsort idsort.c

install:	apsfilt tfilt 4filt idsort
	rm -rf /usr/lib/ideal/idfilt
	mkdir /usr/lib/ideal/idfilt
	cp apsfilt tfilt 4filt idsort cvfilt /usr/lib/ideal/idfilt

clean:
	rm -f *.o a.out apsfilt tfilt 4filt idsort pfilt

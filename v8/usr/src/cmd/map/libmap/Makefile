CFLAGS=-O
../libmap.a: map.h \
	aitoff.o \
	albers.o \
	azequalarea.o \
	azequidist.o \
	bicentric.o \
	bonne.o \
	ccubrt.o \
	complex.o \
	conic.o \
	cubrt.o \
	cylequalarea.o \
	cylindrical.o \
	elco2.o \
	elliptic.o \
	gilbert.o \
	guyou.o \
	hex.o \
	homing.o \
	lambert.o \
	laue.o \
	mecca.o \
	mercator.o \
	mollweide.o \
	orthographic.o \
	perspective.o \
	polyconic.o \
	rectangular.o \
	sinusoidal.o \
	tetra.o \
	zcoord.o
	ar cr ../libmap.a *.o
	ranlib ../libmap.a
clean:
	rm -f *.o map.h

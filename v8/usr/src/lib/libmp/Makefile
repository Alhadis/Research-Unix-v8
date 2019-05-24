DESTDIR=
CFLAGS=
OBJS= pow.o gcd.o msqrt.o mdiv.o mout.o mult.o madd.o util.o halloc.o primetab.o 

libmp.a: $(OBJS)
	ar cr libmp.a $(OBJS)

install: libmp.a
	cp libmp.a ${DESTDIR}/usr/lib
	ranlib ${DESTDIR}/usr/lib/libmp.a

clean:
	rm -f *.o libmp.a

OFILES=cache.o pad.o term.o menu.o remote.o sf.o padrcv.o stats.o help.o\
	libC.o trace.o popen.o

libpads.a: $(OFILES)
	ar ruv libpads.a $(OFILES)
	ranlib libpads.a

trace.o: trace.c
	cc -c `pwd`/trace.c

%.o:	%.c ../pads.pri
	CC +E -I.. -I/usr/include -c `pwd`/$*.c

install: libpads.a
	cp libpads.a /usr/blit/lib
	ranlib /usr/blit/lib/libpads.a

clean:
	rm -f *.i *junk* core jim.recover libpads.a *.o a.out

a.out:	test.o libpads.a
	cc test.o libpads.a

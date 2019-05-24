OFILES=cache.o pad.o term.o menu.o remote.o sf.o padrcv.o\
	stats.o help.o libC.o trace.o popen.o extern.o
INC=

libpads.a: $(OFILES)
	ar ruv libpads.a $(OFILES)
	ranlib libpads.a

trace.o: trace.c
	cc -c `pwd`/trace.c

%.o:	%.c
	CC -I.. -c `pwd`/$*.c

clean:
	rm -f *.i *junk* core jim.recover a.out *.o libpads.a

a.out:	test.o libpads.a
	cc test.o libpads.a

install:	libpads.a
	cp libpads.a /usr/jerq/lib
	ranlib libpads.a

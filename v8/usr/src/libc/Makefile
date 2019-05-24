DESTDIR=

libc.a:
	cc -c -O crt/*.s
	cc -c -O gen/*.[cs]
	rm errlst.o
	make errlst.o
	cc -c -O math/*.c
	cc -c -O stdio/*.c
	cc -c -O sys/*.s
	cp stdio/doprnt.S doprnt.c
	cc -E doprnt.c | as -o doprnt.o
	rm doprnt.c
	-for i in *.o ; do ld -x -r $$i; mv a.out $$i; done
	ar cr libc.a `lorder *.o | tsort`
	ar ma flsbuf.o libc.a exit.o
	ar m libc.a cleanup.o

errlst.o: gen/errlst.c
	cc -S gen/errlst.c
	ed - <gen/:errfix errlst.s
	as -o errlst.o errlst.s
	rm errlst.s

clean:
	rm -f *.o libc.a

install: libc.a crt0.o mcrt0.o
	cp $(DESTDIR)/lib/libc.a  liboc.a
	cp libc.a $(DESTDIR)/lib/libc.a
	ranlib $(DESTDIR)/lib/libc.a
	cp crt0.o $(DESTDIR)/lib
	cp mcrt0.o $(DESTDIR)/lib

crt0.o:	csu/crt0.s
	cc -S csu/crt0.s

mcrt0.o:	csu/mcrt0.s
	cc -S csu/mcrt0.s

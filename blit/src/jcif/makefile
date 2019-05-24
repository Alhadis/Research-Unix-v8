CFLAGS=
CFILES=main.c alloc.c
OFILES=main.o cif.o alloc.o

all: jcif.m jcif

jcif: $(OFILES) jerq.o
	cc -o jcif $(OFILES) jerq.o -ll

jcif.m: rect.o poly.o
	mcc $(CFLAGS) rect.o poly.o
	mopt a.out
	mv m.out jcif.m
	rm a.out

rect.o: rect.c
	mcc -c $(CFLAGS) rect.c

poly.o: poly.c
	mcc -c $(CFLAGS) poly.c

cif.o: cif.c cif.h lex.yy.c

lex.yy.c: cif.lex
	lex cif.lex

$(OFILES): cif.h

print:
	@pr cif.h cif.lex cif.y $(CFILES)

wc:
	wc cif.h cif.lex cif.y $(CFILES)

clean:
	rm -f *.o cif.c lex.yy.c jcif jcif.m *.out
install:	jcif jcif.m
	cp jcif /usr/blit/bin
	cp jcif.m /usr/blit/mbin

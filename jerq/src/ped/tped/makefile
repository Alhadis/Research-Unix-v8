HFILES=device.h hped.h 
CFILES=main.c plot.c sub.c rot.c hread.c fspline.c
FILES=main.o sub.o plot.o rot.o hread.o fspline.o
LFILES=../hped/hrange.o ../hped/hsub.o ../hped/hwide.o ../hped/hblit.o ../hped/hspline.o
tped: $(FILES) $(LFILES) 
	cc -g $(FILES) $(LFILES) -lm -o tped
save:
	push -v astro/grigg makefile $(CFILES) $(HFILES) Itped
bowell:
	push -v astro/bowell $(HFILES) $(CFILES) makefile tped tped
bundle:
	bundle  makefile  $(HFILES) $(CFILES) > tbundle
#linked from hped:  hped.h hread.c fspline.c

clean:
	rm -f *.o tped core
install:	tped
	cp tped /usr/bin

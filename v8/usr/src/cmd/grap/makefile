# ALLOC = malloc.o
CFLAGS = -g
YFLAGS = -d -D
OFILES = main.o input.o print.o frame.o for.o coord.o ticks.o plot.o label.o misc.o $(ALLOC)
CFILES = main.c input.c print.c frame.c for.c coord.c ticks.c plot.c label.c misc.c
SRCFILES = grap.y grapl.l grap.h $(CFILES)

grap:	grap.o grapl.o $(OFILES) grap.h 
	$(CC) $(CFLAGS) grap.o grapl.o $(OFILES) -lm -o grap

install:	grap
	cp grap /usr/bin

$(OFILES) grapl.o:	grap.h prevy.tab.h

grap.c:	grap.h grap.y			# grap.o: grap.h
	yacc -d grap.y
	mv y.tab.c grap.c

y.tab.h:	grap.o

prevy.tab.h:	y.tab.h
	-cmp -s y.tab.h prevy.tab.h || cp y.tab.h prevy.tab.h

prcan:	README WISH $(SRCFILES) makefile
	@prcan $(CAN) $? $(OTHERS)
	@touch prcan

bin:
	cp /usr/bin/grap /usr/safe
	cp a.out /usr/bin/grap

clean:
	rm -f *.o a.out y.tab.h grap grap.c prevy.tab.h

backup:	$(SRCFILES) makefile grap.1 *.g grap.defines WISH
	push snb $? grap
	touch backup

bowell:	$(SRCFILES) makefile grap.1 grap.defines WISH
	push bowell $? /src/cmd/grap
	touch backup

bundle:
	@bundle README $(SRCFILES) makefile grap.1 grap.defines

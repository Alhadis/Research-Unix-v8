CFLAGS = -g      # you might prefer -O
YFLAGS = -d

OFILES = picy.o picl.o main.o print.o misc.o symtab.o blockgen.o boxgen.o \
	 circgen.o arcgen.o linegen.o movegen.o textgen.o \
	 input.o for.o pltroff.o $(ALLOC)
CFILES = main.c print.c misc.c symtab.c blockgen.c boxgen.c circgen.c \
	 arcgen.c linegen.c movegen.c textgen.c \
	 input.c for.c pltroff.c
SRCFILES = picy.y picl.l pic.h $(CFILES)

pic:	$(OFILES)
	$(CC) -o pic $(CFLAGS) $(OFILES) -lm

$(OFILES):	pic.h pic.ydef

y.tab.h:	picy.o

pic.ydef:	y.tab.h
	-cmp -s y.tab.h pic.ydef || cp y.tab.h pic.ydef

prcan:	WISH README UNDOC picy.y picl.l pic.h  $(CFILES) PS-PEmacros
	@prcan $?
	@touch prcan

bundle:
	@bundle picy.y picl.l pic.h $(CFILES) makefile UNDOC README PS-PEmacros

backup:	picy.y picl.l pic.h $(CFILES) makefile pictest.a PS-PEmacros UNDOC
	push snb $? pic
	touch backup

bowell:	picy.y picl.l pic.h $(CFILES) makefile pictest.a PS-PEmacros UNDOC
	push bowell $? /src/cmd/pic
	touch bowell

memo:
	refer -e picmemo | pic | eqn | troff -ms $(TFLAGS) >pic.out 2>pic.mess

clean:
	rm -f *.o pic y.tab.h a.out pic.ydef

install:
	strip pic
	cp pic /usr/bin

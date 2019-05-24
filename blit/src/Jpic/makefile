YFLAGS = -d
OFILES = main.o print.o misc.o symtab.o blockgen.o boxgen.o circgen.o \
	arcgen.o linegen.o movegen.o troffgen.o textgen.o \
	pic2.o pltroff.o
CFILES = main.c print.c misc.c symtab.c blockgen.c boxgen.c circgen.c \
	arcgen.c linegen.c movegen.c troffgen.c textgen.c \
	pic2.c pltroff.c

a.out:	picy.o picl.o $(OFILES) pic.h 
	cc  -o a.out picy.o picl.o $(OFILES) -lm

$(OFILES):	pic.h

lineseg:	lineseg.o picy.o picl.o $(OFILES)
	cc -o lineseg picy.o picl.o lineseg.o $(OFILES) -lm

y.tab.h:	picy.o

pic.ydef:	y.tab.h
	-cmp -s y.tab.h pic.ydef || cp y.tab.h pic.ydef

list:
	@pr WISH UNDOC picy.y picl.l pic.h  $(CFILES)

gout:	gout.c
	cc -o gout gout.c

gin:	gin.c
	cc -o gin gin.c

src:
	cp makefile pic*.[ylch] pltroff.c driver.c /usr/src/cmd/pic

bin:
	cp pic /usr/bin
	strip /usr/bin/pic

memo:
	refer -e picmemo | pic | eqn | troff -ms $(TFLAGS) >pic.out 2>pic.mess

grapic: grapic.c
	cc -DPIC -O grapic.c -lplot -lm -o grapic

jp:	jp.c
#	/usr/m/bin/mcc jp.c -b 256 -o jp -lj
	/usr/m/bin/mcc -c jp.c
	/usr/m/bin/mld -o jp -b 256 /usr/jerq/lib/l.o jp.o /usr/jerq/lib/sys.a -lj /usr/m/lib/libc.a
x:	x.c
#	/usr/m/bin/mcc x.c -b 256 -o x -lj
	/usr/m/bin/mcc -c x.c
	/usr/m/bin/mld -o x -b 256 /usr/jerq/lib/l.o x.o /usr/jerq/lib/sys.a -lj /usr/m/lib/libc.a

epic:	epic.c
	cc -o epic epic.c
clean:
	rm -f *.o lex.yy.c y.tab.c core a.out y.tab.h
install:	a.out
	cp a.out /usr/blit/bin/Jpic
	strip /usr/blit/bin/Jpic

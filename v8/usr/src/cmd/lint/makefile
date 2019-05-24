# lint - description file

NAME = lint
BINDIR = /usr/bin
LINTDIR = /usr/lib/lint
M=../pcc1/mip
CFLAGS = -DFLEXNAMES -DFMTARGS -I$M -I. -O
LDFLAGS =

OFILES1 = cgram.o comm1.o lint1.o optim.o pftn.o scan.o trees.o xdefs.o
LNFILES = llib-lc.ln llib-port.ln

all: lint1 lint2 ${LNFILES}

clean:
	-rm -f cgram.c *.o *.ln lint[12]

install: all
	cp lint[12] llib* ${LINTDIR}
	cp lint.sh ${BINDIR}/${NAME}

lint1: ${OFILES1}
	${CC} ${LDFLAGS} -o lint1 ${OFILES1}

lint2: lint2.c lint.h $M/manifest
	${CC} ${CFLAGS} ${LDFLAGS} -o lint2 lint2.c

cgram.c: $M/cgram.y
	yacc $M/cgram.y
	mv y.tab.c cgram.c

cgram.o: $M/manifest $M/mfile1 macdefs cgram.c
	${CC} -c ${CFLAGS} cgram.c

comm1.o: $M/manifest $M/mfile1 $M/common macdefs $M/comm1.c
	${CC} -c ${CFLAGS} $M/comm1.c

optim.o: $M/manifest macdefs $M/mfile1 $M/optim.c
	${CC} -c ${CFLAGS} $M/optim.c

pftn.o: $M/manifest macdefs $M/mfile1 $M/pftn.c
	${CC} -c ${CFLAGS} $M/pftn.c

lint1.o: $M/manifest macdefs $M/mfile1 lint.h
	${CC} -c ${CFLAGS} lint1.c

scan.o: $M/manifest macdefs $M/mfile1 $M/scan.c
	${CC} -c ${CFLAGS} $M/scan.c

trees.o: $M/manifest macdefs $M/mfile1 $M/trees.c
	${CC} -c ${CFLAGS} $M/trees.c

xdefs.o: $M/manifest $M/mfile1 macdefs $M/xdefs.c
	${CC} -c ${CFLAGS} $M/xdefs.c

llib-lc.ln: llib-lc lint1
	-/lib/cpp -C -Dlint llib-lc | ./lint1 -v > llib-lc.ln

llib-port.ln: llib-port lint1
	-/lib/cpp -C -Dlint llib-port | ./lint1 -puv > llib-port.ln

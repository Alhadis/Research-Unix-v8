INSTALL=/usr/blit/lib/ccom
M=../mip
I=-I. -I$M
CFLAGS=-O
SDB=$M/sdb.h
OFILES= cgram.o xdefs.o scan.o pftn.o trees.o optim.o local.o reader.o\
	local2.o match.o allo.o comm1.o table.o cost.o cgen.o
MFILES= $M/common $M/cgram.y $M/sty.y $M/manifest $M/mfile1 $M/mfile2 \
	$M/pftn.c $M/scan.c $M/comm1.c $M/trees.c $M/optim.c $M/xdefs.c \
	$M/reader.c $M/cgen.c $M/cost.c $M/allo.c $M/match.c $(SDB)
SFILES=	g stin local.c local2.c macdefs
comp: $(OFILES)
	cc $(CFLAGS) $(OFILES)
	mv a.out comp
sty : $M/manifest $M/sty.y macdefs $M/mfile2 $M/common
	yacc $M/sty.y
	cc $I $(CFLAGS) -o sty y.tab.c
	rm y.tab.c
trees.o: $M/manifest macdefs $M/mfile1 $(SDB) $M/trees.c
	cc -c $I $(CFLAGS) $M/trees.c
optim.o: $M/manifest macdefs $M/mfile1 $(SDB) $M/optim.c
	cc -c $I $(CFLAGS) $M/optim.c
pftn.o: $M/manifest macdefs $M/mfile1 $(SDB) $M/pftn.c
	cc -c $I $(CFLAGS) $M/pftn.c
local.o: $M/manifest macdefs $M/mfile1 $(SDB)
	cc -c $I $(CFLAGS) local.c
scan.o: $M/manifest macdefs $M/mfile1 $(SDB) $M/scan.c
	cc -c $I $(CFLAGS) $M/scan.c
xdefs.o: $M/manifest $M/mfile1 $(SDB) macdefs $M/xdefs.c
	cc -c $I $(CFLAGS) $M/xdefs.c
cgram.o: $M/manifest $M/mfile1 $(SDB) macdefs $M/cgram.c
	cc -c $I $(CFLAGS) $M/cgram.c
$M/cgram.c: $M/cgram.y
	yacc $M/cgram.y
	mv y.tab.c $M/cgram.c
comm1.o: $M/manifest $M/mfile1 $(SDB) $M/common macdefs $M/comm1.c
	cc -c $I $(CFLAGS)  $M/comm1.c
table.o: $M/manifest $M/mfile2 macdefs table.c
	cc -c $I table.c
table.c: sty stin
	sty <stin >table.c
cost.o: $M/manifest $M/mfile2 macdefs $M/cost.c
	cc -c $I $(CFLAGS) $M/cost.c
cgen.o: $M/manifest $M/mfile2 macdefs $M/cgen.c
	cc -c $I $(CFLAGS) $M/cgen.c
reader.o: $M/manifest $M/mfile2 macdefs $M/reader.c
	cc -c $I $(CFLAGS) $M/reader.c
local2.o: $M/manifest $M/mfile2 macdefs
	cc -c $I $(CFLAGS) local2.c
match.o: $M/manifest $M/mfile2 macdefs $M/match.c $(SDB)
	cc -c $I $(CFLAGS) $M/match.c
allo.o: $M/manifest $M/mfile2 macdefs $M/allo.c
	cc -c $I $(CFLAGS) $M/allo.c
clean:
	rm -f comp
	rm -f *.o mout sty table.c
lintall:
	lint -hpv $I $M/cgram.c $M/xdefs.c $M/scan.c $M/pftn.c $M/cgen.c $M/cost.c $M/trees.c $M/optim.c local.c $M/reader.c local2.c $M/match.c $M/allo.c $M/comm1.c table.c
install: comp
	cp comp $(INSTALL)

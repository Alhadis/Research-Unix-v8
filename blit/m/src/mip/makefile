
INSTALL=/ccom
M=.
I=
CFLAGS=-O
CC=cc
SDB=$M/sdb.h
OFILES= cgram.o xdefs.o scan.o pftn.o trees.o optim.o local.o reader.o\
	local2.o match.o allo.o comm1.o table.o cost.o cgen.o
MFILES= $M/common $M/cgram.y $M/sty.y $M/manifest $M/mfile1 $M/mfile2 \
	$M/pftn.c $M/scan.c $M/comm1.c $M/trees.c $M/optim.c $M/xdefs.c \
	$M/reader.c $M/cgen.c $M/cost.c $M/allo.c $M/match.c $(SDB)
SFILES=	g stin local.c local2.c macdefs
comp: $(OFILES)
	$(CC) $(CFLAGS) $(OFILES)
	mv a.out comp
sty : $M/manifest $M/sty.y macdefs $M/mfile2 $M/common
	yacc $M/sty.y
	cc $I $(CFLAGS) -o sty y.tab.c
	rm y.tab.c
trees.o: $M/manifest macdefs $M/mfile1 $(SDB) $M/trees.c
	$(CC) -c $I $(CFLAGS) $M/trees.c
optim.o: $M/manifest macdefs $M/mfile1 $(SDB) $M/optim.c
	$(CC) -c $I $(CFLAGS) $M/optim.c
pftn.o: $M/manifest macdefs $M/mfile1 $(SDB) $M/pftn.c
	$(CC) -c $I $(CFLAGS) $M/pftn.c
local.o: $M/manifest macdefs $M/mfile1 $(SDB)
	$(CC) -c $I $(CFLAGS) local.c
scan.o: $M/manifest macdefs $M/mfile1 $(SDB) $M/scan.c
	$(CC) -c $I $(CFLAGS) $M/scan.c
xdefs.o: $M/manifest $M/mfile1 $(SDB) macdefs $M/xdefs.c
	$(CC) -c $I $(CFLAGS) $M/xdefs.c
cgram.o: $M/manifest $M/mfile1 $(SDB) macdefs $M/cgram.c
	$(CC) -c $I $(CFLAGS) $M/cgram.c
$M/cgram.c: $M/cgram.y
	yacc $M/cgram.y
	mv y.tab.c $M/cgram.c
comm1.o: $M/manifest $M/mfile1 $(SDB) $M/common macdefs $M/comm1.c
	$(CC) -c $I $(CFLAGS)  $M/comm1.c
table.o: $M/manifest $M/mfile2 macdefs table.c
	$(CC) -c $I table.c
table.c: sty stin
	sty <stin >table.c
cost.o: $M/manifest $M/mfile2 macdefs $M/cost.c
	$(CC) -c $I $(CFLAGS) $M/cost.c
cgen.o: $M/manifest $M/mfile2 macdefs $M/cgen.c
	$(CC) -c $I $(CFLAGS) $M/cgen.c
reader.o: $M/manifest $M/mfile2 macdefs $M/reader.c
	$(CC) -c $I $(CFLAGS) $M/reader.c
local2.o: $M/manifest $M/mfile2 macdefs
	$(CC) -c $I $(CFLAGS) local2.c
match.o: $M/manifest $M/mfile2 macdefs $M/match.c $(SDB)
	$(CC) -c $I $(CFLAGS) $M/match.c
allo.o: $M/manifest $M/mfile2 macdefs $M/allo.c
	$(CC) -c $I $(CFLAGS) $M/allo.c
clean:
	rm -f comp stout
	rm -f *.o mout sty table.c
lintall:
	lint -hpv $I $M/cgram.c $M/xdefs.c $M/scan.c $M/pftn.c $M/cgen.c $M/cost.c $M/trees.c $M/optim.c local.c $M/reader.c local2.c $M/match.c $M/allo.c $M/comm1.c table.c
fort: comp fort.o freader.o fallo.o fmatch.o ftable.o flocal2.o fcomm2.o
	cc $(CFLAGS) fort.o freader.o fallo.o fmatch.o ftable.o flocal2.o fcomm2.o
	mv a.out fort
fort.o: fort.h $M/fort.c
	cc -c $I $(CFLAGS) $M/fort.c
freader.o: reader.o
	cc -c $I $(CFLAGS) $M/freader.c
fallo.o: allo.o
	cc -c $I $(CFLAGS) $M/fallo.c
fmatch.o: match.o
	cc -c $I $(CFLAGS) $M/fmatch.c
ftable.o: table.o
	cc -c $I $(CFLAGS) $M/ftable.c
flocal2.o: local2.o
	cc -c $I $(CFLAGS) $M/flocal2.c
fcomm2.o: $M/common
	cc -c $I $(CFLAGS) $M/fcomm2.c
fort.o freader.o fallo.o fmatch.o ftable.o flocal2.o fcomm2.o: $M/mfile2 $M/manifest macdefs 
cinstall: install
	cp comp $(INSTALL)
install:: $(SFILES) $(MFILES)
	cp $? /usr/m/mcc
	touch install
to70:: $(SFILES)
	push r70 $? /usr/scj/mcc
	touch to70
to70:: $(MFILES)
	cd $M; make to70
	touch to70
toikeya:: $(SFILES)
	push ikeya $? /usr/scj/mcc
	touch toikeya
toikeya:: $(MFILES)
	cd $M; make toikeya
	touch toikeya
stout: comp
	regress >stout

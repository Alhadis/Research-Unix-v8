
CC =	mcc
CFLAGS =	-DBLIT
LDFLAGS =
HFILES = 	host.h pacman.h style.h
CFILES =	blt.c board.c blitfonts.c fruit.c host.c main.c monsters.c pacman.c prf.c sched.c sounds.c
OFILES =	blt.o board.o blitfonts.o fruit.o host.o main.o monsters.o pacman.o prf.o sched.o sounds.o
a.out:	$(OFILES)
	$(CC) $(LDFLAGS) $(OFILES)

blitfonts.c: bits2blit
	-bits2blit >blitfonts.c

bits2blit:   fonts.c bits2blit.c
	cc -o bits2blit -DBLIT bits2blit.c fonts.c

$(OFILES):	$(HFILES)

install:	a.out
	cp a.out /usr/blit/demo/mpx/pacman
	mstrip /usr/blit/demo/mpx/pacman

clean:
	-rm -f *.o errs core a.out

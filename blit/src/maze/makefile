.SUFFIXES: .c .o
CFILES = comm.c maze.c icon.c view.c
OFILES = comm.o maze.o icon.o view.o
PROGS = maze.m mazewar Mazewar rmtmaze ump

all: $(PROGS)

maze.m: $(OFILES)
	mcc -g -o maze.m $(OFILES)

clean:
	rm *.o $(PROGS)

# for machines running the umpire
install:	all
	cp maze.m ump rmtmaze /usr/blit/lib
	cp mazewar /usr/blit/bin
	mstrip -gv /usr/blit/lib/maze.m

# for machines not running the umpire
rmtinstall:	maze.m Mazewar
	cp maze.m /usr/blit/lib
	mstrip -gv /usr/blit/lib/maze.m
	cp Mazewar /usr/blit/bin/mazewar
pp print:
	@pp -tmazewar maze.h $(CFILES) ump.c mazewar.c rmtmaze.c Mazewar.c | dcan

lint:
	lint -I/usr/blit/include $(CFILES)



.c.o:
	mcc -g -c $<

$(OFILES):	maze.h
mazewar: mazewar.c
	cc -o mazewar mazewar.c

Mazewar: Mazewar.c
	cc -o Mazewar Mazewar.c -ldk

ump: ump.c
	cc -o ump ump.c 

rmtmaze: rmtmaze.c
	cc -o rmtmaze rmtmaze.c

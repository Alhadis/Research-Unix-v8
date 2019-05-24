MHOME = /usr/lib/monk
MBIN = /usr/bin

MAKE = make

LDFLAGS = -s
FRC =

CFLAGS = -O -I.
INSDIR = $(MHOME)

PROGS = \
	cite \
	soelim

all:	$(PROGS) monkprog

cite: cite.o
	$(CC) cite.o $(LDFLAGS) -o cite

monkprog:
	cd monk; $(MAKE) MHOME=$(MHOME)

soelim: soelim.o
	$(CC) soelim.o $(LDFLAGS) -o soelim

clean:
	-rm -f *.o
	cd monk; $(MAKE) MHOME=$(MHOME) clean

clobber: clean
	-rm -f $(PROGS)
	cd monk; $(MAKE) MHOME=$(MHOME) clobber

install: all
	cp $(PROGS) $(MHOME)
	cd monk; $(MAKE) MHOME=$(MHOME) install
	cp monk.sh $(MBIN)/monk
	cp monksample.sh $(MBIN)/monksample

CC=3cc
CFLAGS=-g
HOSTCC=cc

all: think thinkblt thinkblt.m

THINK=think.o thinksort.o
think: & $(THINK)
	$(HOSTCC) $(CFLAGS) -o $@ $(THINK) -ldk
think.o: think.c
	$(HOSTCC) $(CFLAGS) -c think.c

thinkblt: thinkblt.o
	$(HOSTCC) $(CFLAGS) -o $@ thinkblt.o -ldk
thinkblt.o: thinkblt.c
	$(HOSTCC) $(CFLAGS) -c thinkblt.c

THINKTERM=thinkterm.o thinkprint.o thinkmap.o thinkchar.o \
	flash.o reshape.o buttons.o mice.o
thinkblt.m: & $(THINKTERM)
	$(CC) -g $(THINKTERM) -o $@
thinkchar.o thinkprint.o thinkmap.o: think.h
thinkblt.o thinkterm.o: thinkblt.h

THINKCOL=thinkcol.o thinksort.o
thinkcol: & $(THINKCOL)
	$(HOSTCC) $(CFLAGS) -o $@ $(THINKCOL) -ldk
thinkcol.o: thinkcol.c
	$(HOSTCC) $(CFLAGS) -c thinkcol.c
thinksort.o: thinksort.c
	$(HOSTCC) $(CFLAGS) -c thinksort.c

clean:
	rm -f think thinkblt *.m *.o thinkblt.m

HNAME=thinkblt
TNAME=thinkblt.m
HNAME2=think
HPROG=/usr/jerq/bin/$(HNAME)
TPROG=/usr/jerq/mbin/$(TNAME)
HPROG2=/usr/jerq/bin/$(HNAME2)

install: $(HNAME) $(TNAME) $(HNAME2)
	cp $(HNAME)	$(HPROG)
	chmod a=rx,u+w	$(HPROG)
	strip		$(HPROG)
	cprs $(TNAME)	$(TPROG)
	chmod a=r,u+w	$(TPROG)
	cp $(HNAME2)	$(HPROG2)
	chmod a=rx,u+w	$(HPROG2)
	strip		$(HPROG2)

ship: install
	ship $(HPROG) $(TPROG) $(HPROG2)
	touch ship

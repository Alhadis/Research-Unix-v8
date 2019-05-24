JERQBIN=/usr/blit/bin
JERQM=/usr/blit/mbin/proof.m
JERQINCLUDE=/usr/blit/include
JERQFONT=/usr/blit/font
JERQMISSING=$(JERQFONT)/MISSING
JLD=$(JERQBIN)/68ld
OBJ=main.o io.o font.o map.o troff.o page.o

CC=cc
LFLAGS=-DJERQM=\"$(JERQM)\" -DJLD=\"$(JLD)\" -DJERQMISSING=\"$(JERQMISSING)\" -DJERQFONT=\"$(JERQFONT)\"
CFLAGS=$(LFLAGS) -g

.c.o:
	$(CC) $(CFLAGS) -c $<

proof: $(OBJ)
	$(CC) $(CFLAGS) -o proof $(OBJ)

main.o:	main.c ../comm.h
font.o:	font.c ../comm.h
io.o:	io.c ../comm.h

clean:
	rm -f *.o proof 

install:	$(JERQBIN)/proof

$(JERQBIN)/proof:	proof
	cp proof $(JERQBIN)/proof

pp:
	pp -t"Host Proof" Makefile ../comm.h host.h main.c font.c io.c map.c page.c troff.c | dcan

lint:
	lint $(LFLAGS) main.c io.c font.c map.c troff.c page.c

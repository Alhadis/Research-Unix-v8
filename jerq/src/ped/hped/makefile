CFLAGS=-O
HCFILES=hped.h Unix.h device.h dmain.c hedit.c hcomm.c  hread.c hwrite.c hsub.c  hfnt.c  hwork.c hbmap.c hrange.c hblit.c help.c  hwide.c hspline.c fspline.c huser.c
HFILES=dmain.o hedit.o hcomm.o  hread.o hwrite.o hsub.o  hfnt.o  hwork.o hbmap.o hrange.o hblit.o help.o  hwide.o hspline.o fspline.o huser.o
BFILES=hmain.o hedit.o hcomm.o  hread.o hwrite.o hsub.o  hfnt.o  hwork.o hbmap.o hrange.o hblit.o help.o  hwide.o hspline.o fspline.o huser.o
BCFILES=beauty.h locus.h hform.c aux.c solve.c
BOFILES=hform.o aux.o solve.o
ped: $(HFILES) $(BOFILES)
	cc $(HFILES) $(BOFILES) -lm -o ped
blit: $(BFILES)
	cc $(BFILES) $(BOFILES) -lm -o blitped
$(HFILES): hped.h
$(BOFILES): beauty.h locus.h
hread.o fspline.o: device.h
dmain.o: Unix.h

canM:
	@pr $(HCFILES) | 4can
lint:
	lint $(HCFILES)
save:
	push -v astro/grigg `ls -t *.[ch] makefile saved | sed /saved/q`  Iyped
	date > saved
swift:
	pr $(HCFILES) | swift opr
bowell:
	push -v astro/bowell README $(HCFILES) $(BCFILES) makefile ped hped
bundle:
	bundle README makefile  $(HCFILES) $(BCFILES) > hbundle

clean:
	rm -f *.o core ped
install:	ped
	cp ped /usr/jerq/bin

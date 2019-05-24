BACKDIR = /src/cmd/view2d/rksmith
PRECIOUS = Makefile box.f rkstri.c
BACKMACH = bowell

test: rkstri
	rkstri -v <testdata >tmp
	regrid -n10 -v tmp >t

rkstri: rkstri.o box.o
	cc -o rkstri rkstri.o box.o ../sys.o -lview2d

clean:
	rm -f *.o a.out core tmp t

backup: Backup
Backup: $(PRECIOUS)
	push -v $(BACKMACH) $? $(BACKDIR)
	touch Backup

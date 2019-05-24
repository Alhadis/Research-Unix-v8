CC=mcc

blitblt: blitblt.c
	cc blitblt.c -ldk -o blitblt

BBOBJ=bbterm.o sendbitmap.o sendch.o buttons.o kbdstring.o mice.o
bbterm.m: $(BBOBJ)
	mcc $(BBOBJ) 
	mopt a.out
	mv m.out bbterm.m
	rm a.out

bcan.out: bcan.c
	cc bcan.c -o bcan.out

bdump: bdump.c
	cc bdump.c -o bdump

clean:
	rm -f *.o *.m *.out blitblt
install:	blitblt bbterm.m
	cp blitblt /usr/blit/bin
	strip /usr/blit/bin/blitblt
	cp bbterm.m /usr/blit/mbin

CC=mcc
CFLAGS=
a.out:	swar.o
	mcc $(CFLAGS) swar.o
swar.o:	player0.icon player1.icon accel.h missile.icon deathstar.icon boom.icon
install:	a.out
	mopt a.out
	cp m.out /usr/blit/demo/mpx/swar
	mstrip -g /usr/blit/demo/mpx/swar
clean:
	rm -f *.o m.out core a.out

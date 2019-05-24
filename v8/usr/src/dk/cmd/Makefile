CFLAGS=-O
all:	dcon pull push rx mgrproc nrx ndcon dkhup track dkzap dkdialsub

dkdialsub:	dkdialsub.o
	cc -o dkdialsub dkdialsub.o -ldk

dkhup:	dkhup.o
	cc -o dkhup dkhup.o -ldk

dkzap:	dkzap.o
	cc -o dkzap dkzap.o -ldk

mgrproc:	mgrproc.o
	cc -o mgrproc mgrproc.o -ldk

dcon:	dcon.o
	cc -o dcon dcon.o -ldk

pull:	pull.o
	cc -o pull pull.o -ldk

push:	push.o
	cc -o push push.o -ldk

rx:	rx.o
	cc -o rx rx.o -ldk

track:	track.o
	cc -o track track.o -ldk

ndcon:	ndcon.o
	cc -o ndcon ndcon.o -ldk

nrx:	nrx.o
	cc -o nrx nrx.o -ldk


clean:
	rm -f *.o mgrproc dcon push pull rx track nrx ndcon dkhup dkzap dkdialsub

install:	all
	cp dcon pull push rx nrx ndcon track /usr/bin
	strip /usr/bin/dcon /usr/bin/pull /usr/bin/push /usr/bin/rx /usr/bin/nrx /usr/bin/ndcon /usr/bin/track
	rm -f /usr/lib/Rpush /usr/lib/Rpull
	ln /usr/bin/pull /usr/lib/Rpull
	ln /usr/bin/push /usr/lib/Rpush
	cp mgrproc dkhup dkzap dkdialsub /etc
	strip /etc/mgrproc /etc/dkhup /etc/dkzap dkdialsub
	/etc/chown root /etc/mgrproc; chmod u+s,go-w /etc/mgrproc

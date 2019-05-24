MACHINES = alice encke forbes ikeya kwee mrkos \
	panther piggot polya rabbit research seki snb \
	sola swift west wild

all:	snocone sc2

sc:	snocone
	spitbol -s16k snocone
	mv a.out sc

sc2:	sc snocone.sc
	./sc snocone.sc
	mv a.out sc2
	./sc2
	mv a.out sc2

install: sc2
	cp sc2 /usr/bin/snocone

ship:
	source='research ark' dest="$(MACHINES)" \
		ship /usr/bin/snocone /usr/lib/snocone

clean:
	rm -f sc sc2 a.out

CFLAGS = -O

all: spellprog spellin hashmake hashcheck hlista hlistb hstop test

spellprog: spellprog.o hash.o hashlook.o huff.o
	cc spellprog.o hash.o hashlook.o huff.o -o spellprog

spellin: spellin.o huff.o hash.h
	cc spellin.o huff.o -o spellin

hashcheck: hashcheck.o hash.o huff.o hash.h
	cc hashcheck.o hash.o huff.o -o hashcheck

hashmake: hashmake.o hash.o 
	cc hashmake.o hash.o -o hashmake

hash.o: hash.c hash.h
	cc $(CFLAGS) -c hash.c
	
hashlook.o: hashlook.c hash.h huff.h
	cc $(CFLAGS) -c hashlook.c
	
spellin.o: spellin.c hash.h
	cc $(CFLAGS) -c spellin.c
	
hashmake.o: hashmake.c hash.h
	cc $(CFLAGS) -c hashmake.c
	
hashcheck.o: hashcheck.c hash.h
	cc $(CFLAGS) -c hashcheck.c
	
huff.o: huff.c huff.h
	cc $(CFLAGS) -c huff.c
	
lists: hlista hlistb hstop
	rm htemp1

htemp1: list local hashmake
	cat list local | ./hashmake >htemp1

hlista: htemp1 american hashmake spellin
	./hashmake <american | sort -u - htemp1 >htemp2
	./spellin `wc htemp2|sed -n 's/\([^ ]\) .*/\1/p' ` <htemp2 >hlista
	rm htemp2

hlistb: htemp1 british hashmake spellin
	./hashmake <british | sort -u - htemp1 >htemp2
	./spellin `wc htemp2|sed -n 's/\([^ ]\) .*/\1/p' ` <htemp2 >hlistb
	rm htemp2

hstop: stop hashmake spellin
	./hashmake <stop | sort -u >htemp2
	./spellin `wc htemp2|sed -n 's/\([^ ]\) .*/\1/p' ` <htemp2 >hstop
	rm htemp2

test: spellprog hlista hlistb hstop spelltest
	./spelltest

install: spellprog hlista hlistb stop spell.sh hstop
	strip spellprog
	mv spellprog /usr/lib/spell
	cp hlista hlistb hstop /usr/dict
	cp spell.sh /usr/bin/spell

clean:
	rm -f htemp1 *.o spellprog hlista hlistb hstop spellin hashmake hashcheck

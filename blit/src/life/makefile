all:	hostlife life.m

hostlife:	hostlife.c
	cc -o hostlife hostlife.c

life.m:	mlife.c
	mcc mlife.c
	mopt a.out
	mv m.out life.m
	rm a.out

install:	all
	cp life.m /usr/blit/mbin
	cp hostlife /usr/blit/lib
	cp life.sh /usr/blit/bin/life
	strip /usr/blit/lib/hostlife

clean:
	rm -f hostlife life.m *.o

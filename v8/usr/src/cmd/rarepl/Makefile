CFLAGS= -O
all: rarct rarepl

install:	all
	cp rarct rarepl /etc
	strip /etc/rarct /etc/rarepl

clean:
	rm -f *.o rarct rarepl

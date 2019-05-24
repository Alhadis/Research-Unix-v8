all:	apserver apnews remotestamp

apserver: apserver.c
	vcc -g -o apserver apserver.c

remotestamp:	apserver
	cp apserver /usr/jerq/lib
	chmod 775 /usr/jerq/lib/apserver
	dest='alice research grigg' ship /usr/jerq/lib/apserver
	touch remotestamp

apnews:	apnews.o
	vcc apnews.o -g -o apnews /usr/jerq/lib/libpads.a -ldk
	strip -s apnews

apnews.o: apnews.c
	CFRONTOPTS=+VE CPPOPTS=-I/usr/jerq/include /usr/tac/bin/CCG `pwd`/apnews.c

clean:
	rm -f *junk* *.i *.cppd core jim.recover apserver apnews remotestamp *.o

install: apnews
	cp apnews /usr/jerq/bin
	chmod 775 /usr/jerq/bin/apnews


CFILES=jim.c pattern.c file.c string.c msgs.c unix.c
FILES=jim.o pattern.o file.o string.o msgs.o unix.o
CFLAGS=

xjim:	$(FILES)
	$(CC) $(CFLAGS) $(FILES) -o xjim

ftest:	ftest.o string.o file.o
	cc -o ftest ftest.o string.o file.o

xljim:	$(FILES)
	lcomp $(CFILES) && cp a.out xljim && rm a.out

$(FILES):	file.h

jim.o msgs.o:	msgs.h


archive:
	archive -a file.h msgs.h $(CFILES)

lint:
	lint -h -u -x $(CFILES)

pp:
	@pp -t"Unix Jim" file.h msgs.h $(CFILES) | dcan

print:
	@pr file.h msgs.h $(CFILES)

install:	xjim
	cp xjim /usr/blit/bin/jim

clean:
	rm -f xjim *.o

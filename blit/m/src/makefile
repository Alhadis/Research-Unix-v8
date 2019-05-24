CFLAGS = -I/usr/blit/include

all:	mcc mnm msize mstrip

mcc:	mcc.c
	$(CC) $(CFLAGS) -o mcc mcc.c

mnm:	mnm.c
	$(CC) $(CFLAGS) -o mnm mnm.c

msize:	msize.c
	$(CC) $(CFLAGS) -o msize msize.c

mstrip:	mstrip.c
	$(CC) $(CFLAGS) -o mstrip mstrip.c

install:	mcc mnm msize mstrip
	cp $^ /usr/blit/bin
clean:
	rm -f *.o mcc mnm msize mstrip core

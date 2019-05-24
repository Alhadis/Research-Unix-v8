CC=/usr/blit/bin/mcc -g -I..
OFILES= paint.o buttons.o lib.o pad.o lineops.o cursor.o\
	master.o host.o protocol.o cache.o scripthit.o

pads.m:$(OFILES)
	$(CC) $(OFILES)
	/usr/blit/bin/mopt a.out|grep "%"
	mv m.out pads.m

%.o:	%.c univ.h ../pads.pri
	$(CC) -c $*.c

clean:
	rm -f *junk* core jim.recover *.o pads.m a.out

install:
	cp pads.m /usr/blit/mbin

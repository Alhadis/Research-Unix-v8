CC=mcc
CFLAGS=
OFILES=jek.o flash.o tiny.o bigapl.o tinyapl.o dclip.o dline.o jdlineto.o dlsegment.o

jek.m:	$(OFILES)
	mcc -o jek.m $(OFILES) -llayer

install:	jek.m
	mopt jek.m
	cp m.out /usr/blit/mbin/tek4014.m
	mstrip /usr/blit/mbin/tek4014.m

binstall:	jek.m
	cp jek.m /usr/blit/mbin/tek4014.m
	mstrip /usr/blit/mbin/tek4014.m

clean:
	rm -f *.o m.out core

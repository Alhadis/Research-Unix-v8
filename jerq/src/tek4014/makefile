CC=3cc
OFILES=jek.o flash.o tiny.o bigapl.o tinyapl.o dclip.o dline.o jdlineto.o dlsegment.o

jek.m:	$(OFILES)
	3cc -o jek.m $(OFILES) -llayer

install:	jek.m
	cp jek.m /usr/jerq/mbin/tek4014.m

clean:
	rm -f *.o core jek.m

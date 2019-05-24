JCFILES=jped.h device.h jmain.c jcomm.c jplot.c jfill.c jresh.c jmke.c jdraw.c jsub.c jtext.c jmvc.c jscl.c   jmenu.c  jfont.c
JFILES=jmain.o jcomm.o jplot.o jfill.o jresh.o jmke.o jdraw.o jsub.o jtext.o jmvc.o jscl.o   jmenu.o  jfont.o
CC=3cc
jped: $(JFILES)
	$(CC) $(JFILES) 
	mv a.out ped.m
$(JFILES): jped.h
bundle:
	bundle  $(JCFILES) > 56bundle
obundle:
	bundle README makefile device.h jped.h *.c > 56bundle

clean:
	rm -f *.o a.out ped.m core
install:	jped
	cp ped.m /usr/jerq/mbin/ped.m

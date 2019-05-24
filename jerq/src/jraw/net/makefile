.SUFFIXES: .c .e .i .s .o
CFLAGS = -g
CC = CC
OFILES = thing.o conn.o wire.o text.o box.o master.o main.o
CFILES = thing.c conn.c wire.c text.c box.c master.c main.c
HFILES = thing.h text.h conn.h wire.h box.h master.h

jraw.net: $(OFILES)
	$(CC) -g -o jraw.net $(OFILES)

install: jraw.net
	cp jraw.net /usr/jerq/lib

.c.s:
	$(CC) -S $(CFLAGS) $<

.c.i:
	$(CC) -Fc -.i $(CFLAGS) $<

.c.e:
	$(CC) -Fc -.e -E $(CFLAGS) $<

print:
	cat $(HFILES) > junk
	print geom.h junk $(CFILES)
	rm junk

files:
	@echo $(CFILES)

hfiles:
	@echo $(HFILES)

clean:
	rm -f jraw.net *.o net

# object dependencies!!
# depends on cdepend..
# depend:
#	(echo '/^#dstart/+1,$$c'; cdepend $(OFILES); echo '.'; echo 'w'; echo 'q')|ed - makefile

#dstart
thing.o: geom.h thing.h
conn.o: geom.h thing.h text.h wire.h conn.h /usr/include/stdio.h
wire.o: geom.h thing.h text.h wire.h box.h
text.o: geom.h thing.h text.h
box.o: geom.h thing.h text.h wire.h conn.h box.h /usr/include/stdio.h
master.o: geom.h thing.h text.h wire.h conn.h box.h master.h
main.o: geom.h thing.h text.h wire.h conn.h box.h master.h

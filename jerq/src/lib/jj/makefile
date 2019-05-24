L=libjj.a
INCL=/usr/jerq/include
CC=3cc
AS=/usr/jerq/bin/vax/m32as
AR=/usr/jerq/bin/3ar
DCAN=dcan
CFLAGS=-DMUX

OBJ=mhit.o kbdstr.o confirm.o clip.o 3d.o
HFILES=/usr/jerq/include/3d.h /usr/jerq/include/menu.h
CFILES=3d.c clip.c confirm.c kbdstr.c mhit.c

$L:	$(OBJ)
	$(AR) uv $L $^


new:	$(OBJ)
		$(AR) uv $L `mlorder $(FILES) | tsort`
		$(AR) ma clip.o $L line.o
		rm $(FILES)

print:
	@pr $(CFILES)

pp:
	pp -fPO -t'Lib jj' $(HFILES) $(CFILES) | $(DCAN)

lint:
	lint -h -c -u -n -I/usr/jerq/include $(CFILES)

$L(mhit.o):	$(INCL)/menu.h

.PRECIOUS:	$L

ship:
	ship $(HFILES) /usr/jerq/lib/libjj.a
clean:
	rm -f *.o libjj.a
install:	$L
	rm -f /usr/jerq/lib/libjj.a
	ln $L /usr/jerq/lib/libjj.a

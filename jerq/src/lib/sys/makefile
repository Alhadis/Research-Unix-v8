L=libsys.a
INCL=/usr/jerq/include
CC=3cc
AS=3as
AR=3ar
CFLAGS=-J

OBJ=acia.o buttons.o cursblt.o cursor.o kbd.o queue.o setup.o
CFILES=acia.c buttons.c cursblt.s cursor.c kbd.c queue.c setup.c

$L:	$(OBJ)
	$(AR) uv $L $^

print:
	@pr $(CFILES)

lint:
	lint -h -c -u -n -I/usr/jerq/include $(CFILES)
install:	$L
	rm -f /usr/jerq/lib/libsys.a
	ln $L /usr/jerq/lib/libsys.a
clean:
	rm -f *.o core libsys.a

L=frame.a
CFLAGS=-gOmDMPXTERM -I..
CC=mcc
AR=ar

OBJ=$L(string.o) $L(deltext.o) $L(frameop.o) $L(instext.o) $L(newframe.o) \
	$L(ptofchar.o) $L(select.o)
FILES=string.o deltext.o frameop.o instext.o newframe.o ptofchar.o select.o
CFILES=string.c deltext.c frameop.c instext.c newframe.c ptofchar.c  select.c

$L:	$(OBJ)

print:
	@pr frame.h $(CFILES)

lint:
	lint -I/usr/jerq/include $(CFILES) -lj

lorder:	$(FILES)
	rm frame.a
	ar cr frame.a `mlorder $(FILES) | tsort `

$L(%.o):	%.c frame.h
	$(CC) -c $(CFLAGS) $%.c && ar r $L $%.o && rm -f $%.o

clean:
	rm -f *.o core frame.a

CFLAGS=-g -O
CC=mcc
FILES=deltext.o frameop.o instext.o newframe.o ptofchar.o
CFILES=deltext.c frameop.c instext.c newframe.c ptofchar.c 

frame.a:	$(FILES)
	ar uv frame.a $(FILES)

print:
	@pr frame.h $(CFILES)

lint:
	lint -I/usr/blit/include $(CFILES) -lj

$(FILES):	frame.h

clean:
	rm -f *.o core

CFILES=cip.h cip.c buttons.c things.c draw.c edit.c pic.c \
	track.c commands.c near.c transform.c menus.c readPic.c

OBJ=buttons.o cip.o commands.o draw.o edit.o  menus.o readPic.o\
	near.o pic.o things.o track.o transform.o

CC=mcc
CFLAGS=-j

cip:	$(OBJ)
	mcc -j -o cip $(OBJ)

install:	cip
	cp cip /usr/blit/lib/cip.j

$(OBJ):	cip.h

print:
	@pr $(CFILES) | alice lp0 &


lint:	
	lint -bu cip.c draw.c things.c edit.c track.c pic.c commands.c near.c\
	buttons.c -lj

clean:
	rm -f cip *.o core

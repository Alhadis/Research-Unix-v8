OBJ=a.o batcomp.o bomb.o event.o explode.o image.o rock.o ship.o
FILES=a.c batcomp.c bomb.c event.c explode.c image.c rock.c ship.c
CC=mcc
CFLAGS=

a.out:	$(OBJ)
	$(CC) $(CFLAGS) $(OBJ)

install:	a.out
	mopt a.out
	mstrip m.out
	cp m.out /usr/blit/demo/mpx/asteroids
clean:
	rm -f *.o core a.out m.out

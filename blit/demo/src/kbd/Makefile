MAIN     = keyboard.h kbd.c
KEYBOARD = keyboard.h keyboard.c
KBDSYS   = kbd.o keyboard.o
CC	 = mcc

a.out:		$(KBDSYS)
		$(CC) $(KBDSYS) && mopt a.out && mv m.out a.out

kbd.o:		$(MAIN)

keyboard.o:	$(KEYBOARD)

install:	a.out
		mv a.out /usr/blit/demo/mpx/kbd
		
clean:
		rm -f *.o a.out

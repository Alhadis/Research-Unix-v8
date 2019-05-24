CC=3cc
CFILES = 	digits.c globals.c player.c\
		chk_coll.c chk_mecoll.c error.c exit_game.c\
		init_game.c init_round.c killplayer.c personchoose.c\
		playdisp.c put_chunk.c init_scr.c scorpchoose.c\
		shotchoose.c slugchoose.c spiderchoose.c start_stuff.c\
		wormchoose.c
OFILES = 	digits.o globals.o player.o\
		chk_coll.o chk_mecoll.o error.o exit_game.o\
		init_game.o init_round.o killplayer.o personchoose.o\
		playdisp.o put_chunk.o init_scr.o scorpchoose.o\
		shotchoose.o slugchoose.o spiderchoose.o start_stuff.o\
		wormchoose.o

a.out: playdefs.h $(OFILES)
	$(CC) $(LDFLAGS) $(OFILES)

clean:
	-rm -f *.o core a.out

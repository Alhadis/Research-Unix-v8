OBJ=smiley.o smileyf.o explode.o
CC=/usr/jerq/bin/mcc
CFLAGS=-j

smiley:	$(OBJ)
	$(CC) $(CFLAGS) -o $(OBJ)
install:
	cp smiley /usr/jerq/demo/stand-alone
	/usr/jerq/bin/mstrip /usr/jerq/demo/stand-alone/smiley

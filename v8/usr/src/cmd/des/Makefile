CFLAGS = -Od2

OBJ = BOXES.o compkeys.o des.o encrypt.o

encrypt: $(OBJ)
	cc -o encrypt $(OBJ)

install:	encrypt
	rm -f /usr/bin/encrypt /usr/bin/decrypt
	strip encrypt
	mv encrypt /usr/bin
	ln /usr/bin/encrypt /usr/bin/decrypt

clean:
	rm -f $(OBJ) encrypt

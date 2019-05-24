HDR=a.out.h mld.h list.h
SRC=doarg.c doto.c glob.c list.c main.c output.c reloc.c str.c sym.c
OBJ=doarg.o doto.o glob.o list.o main.o output.o reloc.o str.o sym.o
X=
CFLAGS=-g
mld:	$(OBJ)
	$(CC) $(CFLAGS) -o mld $(OBJ) $X
install:	mld
	cp mld /usr/blit/bin
$(OBJ):	$(HDR)
list:
	pr $(HDR) $(SRC) | can
pp:
	pp $(HDR) $(SRC) ^ dcan
clean:
	rm -f *.o mld core

OBJ=bitfile.o fill.o image.o main.o menu.o paintfunc.o state.o util.o
CODE=bitfile.c fill.c image.c main.c menu.c paintfunc.c state.c util.c
SRC=paint.h $(CODE)
CC=3cc
CFLAGS=
paint.m:	$(OBJ)
		$(CC) $(CFLAGS) -o paint.m $(OBJ) -ljj
$(OBJ):		paint.h
listing:
		prcan $(SRC)
lint:
		lint -I/usr/jerq/include $(CODE) -lj -ljj
clean:
	rm -f *.o paint.m core
install:	paint.m
	cp paint.m /usr/jerq/mbin
